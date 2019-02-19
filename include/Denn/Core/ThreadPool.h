#pragma once
#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>

namespace Denn
{
	//Promise
	using Promise     = std::future<void>;
	using PromiseList = std::vector< Promise >;
	//
	class ThreadPool
	{
	public:
		//type ok callback
		using TaskFunction = std::function<void(void)>;
		//list of task to do
		using ConditionVariable = std::condition_variable;
		using Mutex			    = std::mutex;
		using Tasks				= std::queue < TaskFunction >;
		using Workers			= std::vector< std::thread >;

		//constructors
		ThreadPool() {}
		ThreadPool(size_t n_threads)
		{
			init(n_threads);
		}

		//destructor
		virtual ~ThreadPool()
		{
			if (m_workers.size())
			{
				close();
			}
		}

		//add a task with return
		template<class function>
		inline auto push_task(function&& fun_task)
		{
			//get return
			using return_type = typename std::result_of<function()>::type;
			//make task
			auto task = std::make_shared < std::packaged_task<return_type()> >(fun_task);
			//return future result
			std::future<return_type> res = task->get_future();
			{
				//lock m_woker_tasks
				std::unique_lock<std::mutex> lock(this->m_workers_mutex);
				// don't allow enqueueing after stopping the pool
				if (this->m_workers_stop) throw std::runtime_error("push a task on stopped thread pool");
				//push
				this->m_workers_tasks.emplace([task]() { (*task)(); });
			}
			//notify push
			m_workers_condition.notify_one();
			//return promise
			return res;
		}

		//add a task with return + args
		template<class function, class... arguments>
		inline auto push_task(function&& f, arguments&&... args) -> std::future<typename std::result_of<function(arguments...)>::type>
		{
			//get return
			using return_type = typename std::result_of<function(arguments...)>::type;
			//make task
			auto task = std::make_shared< std::packaged_task<return_type()> >
			(
				std::bind(std::forward<function>(f), std::forward<arguments>(args)...)
			);
			//return future result
			std::future<return_type> res = task->get_future();
			{
				//lock m_woker_tasks
				std::unique_lock<std::mutex> lock(this->m_workers_mutex);
				// don't allow enqueueing after stopping the pool
				if (this->m_workers_stop) throw std::runtime_error("push a task on stopped thread pool");
				//push
				this->m_workers_tasks.emplace([task]() { (*task)(); });
			}
			//notify push
			m_workers_condition.notify_one();
			//return promise
			return res;
		}

		//add a simple task (no return, no args, no promes)
		inline void push_simple_task(TaskFunction&& task)
		{
			//lock
			{
				//lock m_woker_tasks
				std::unique_lock<std::mutex> lock(this->m_workers_mutex);
				// don't allow enqueueing after stopping the pool
				if (this->m_workers_stop) throw std::runtime_error("push a task on stopped thread pool");
				//push
				this->m_workers_tasks.emplace(std::forward<TaskFunction>(task));
			}
			//notify push
			m_workers_condition.notify_one();
		}

		//wait complate all tasks
		void wait_all_tasks()
		{
			while (m_workers.size()) 
			{
				//lock
				std::unique_lock<std::mutex> lock(m_workers_mutex);
				//no more staks
				if(m_workers_tasks.empty() && !m_tasks_in_execution)
					return;
			};
		}

		//init
		inline void init(size_t n_threads)
		{
			//enable
			this->m_workers_stop = false;
			this->m_tasks_in_execution = 0;
			//init
			for (size_t i = 0; i < n_threads; ++i)
			{
				m_workers.emplace_back(
				[this]()
				{
					while (true)
					{
						TaskFunction task;
						{
							//lock thread
							std::unique_lock<std::mutex> lock(this->m_workers_mutex);
							//wait condition
							this->m_workers_condition.wait(lock,
							[this]()
							{
								return this->m_workers_stop || !this->m_workers_tasks.empty();
							});
							//exit case
							if (this->m_workers_stop && this->m_workers_tasks.empty())
								return;
							//get task
							task = std::move(this->m_workers_tasks.front());
							//pop
							this->m_workers_tasks.pop();
							//inc counter of task in execution
							++m_tasks_in_execution;
						}
						//execute
						task();
						//dec counter
						{
							//lock thread
							std::unique_lock<std::mutex> lock(this->m_workers_mutex);
							//dec counter of task in execution
							--m_tasks_in_execution;
						}
					}
				}
				);
			}
		}

		//stop
		inline void close()
		{
			//stop
			{
				std::unique_lock<std::mutex> lock(m_workers_mutex);
				m_workers_stop = true;
			}
			//notify to all
			m_workers_condition.notify_all();
			//wait all
			for (std::thread& worker : m_workers) worker.join();
			//clear workers list
			m_workers.clear();
		}

	protected:

		Workers			  m_workers;		
		size_t			  m_tasks_in_execution;
		bool			  m_workers_stop;
		Mutex			  m_workers_mutex;
		Tasks			  m_workers_tasks;
		ConditionVariable m_workers_condition;

	};
}