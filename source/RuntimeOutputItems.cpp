#include "Denn/RuntimeOutput.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{


    class SilentOutput : public Denn::RuntimeOutput
    {
    public:
	    SilentOutput(std::ostream& stream,const Solver& solver):RuntimeOutput(stream, solver) {}
    };
    REGISTERED_RUNTIME_OUTPUT(SilentOutput,"silent")

    class AllFitnessOutput : public Denn::RuntimeOutput
    {
    public:
	    AllFitnessOutput(std::ostream& stream,const Solver& solver):RuntimeOutput(stream, solver) {}
        
        virtual void end_a_sub_pass() override 
        { 
            for(auto subpop : population())
            for(auto i      : subpop->parents())
                output() << i->eval() << "; ";
            output() << std::endl;
        }
    };
    REGISTERED_RUNTIME_OUTPUT(AllFitnessOutput,"all-fitness")


    class FullOutput : public Denn::RuntimeOutput
    {
    public:

        FullOutput(std::ostream& stream,const Solver& solver):RuntimeOutput(stream, solver) {}
        
        virtual void start() override
        { 
            output() << "Denn/ start" << std::endl;
            m_start_time = Denn::Time::get_time();
            m_n_sub_pass = 0;
            m_n_pass     = 0;      
            //clean line
            clean_line();
            //output
            write_output(); 
            output() << std::endl;     
        }

        virtual void start_a_pass() override 
        { 
            //reset
            m_sub_pass_time  = Denn::Time::get_time(); 
            m_n_sub_pass     = 0;
        }

        virtual void end_a_pass() override 
        { 
            //clean line
            clean_line();
            //output
            write_output(); 
            output() << std::endl;
            //count
            ++m_n_pass;
        }

        virtual void end_a_sub_pass() override 
        { 
            ++m_n_sub_pass;
            //compute pass time
            double pass_per_sec = (double(m_n_sub_pass) / (Denn::Time::get_time() - m_sub_pass_time));
            //clean line
            clean_line();
            //write output
            output() << double(long(pass_per_sec*10.))/10.0 << " [it/s], ";
            write_output(); 
            output() << "\r";
        }

        virtual void end() override
        { 
            double time_of_execution = Denn::Time::get_time() - m_start_time ;
            Scalar test_result = solver().test_function_eval(solver().current_best_network());
            output() 
            << "Denn/ end [ test: " 
            << test_result
            << ", time: " 
            << time_of_execution
            << " ]" 
            << std::endl;
        }


    protected:

        double m_start_time;
        double m_sub_pass_time;
        long   m_n_pass;
        long   m_n_sub_pass;

        virtual void write_output()
        {
            write_local_pass();
            output() << ", best: ";
            write_global_best("[ acc: ",", cross: ");
        }

        virtual void clean_line()
        {
            #ifdef _WIN32
            //clean line
            for(short i=0;i!=11;++i) output() << "          ";
            //end row
            output() << "\r";
            #else 
            ::printf("%c[2K", 27);
            #endif
        }

	virtual void write_local_pass() 
	{
            size_t n_sub_pass = *parameters().m_sub_gens;
	    output() << (n_sub_pass * m_n_pass + m_n_sub_pass);
	}

    virtual void write_global_best
    (
		const std::string& open="[ ", 
		const std::string& separetor=", ", 
		const std::string& closer=" ]"
	) 
	{
	    output() 
	    << open 
	    << solver().current_best_eval() 
        << closer;
	}

    };
    REGISTERED_RUNTIME_OUTPUT(FullOutput,"full")

    class BenchOutput : public Denn::RuntimeOutput
    {
    public:

	    BenchOutput(std::ostream& stream, const Solver& solver):RuntimeOutput(stream, solver) {}
        
        virtual void start() override
        { 
            output() << "=== START ===" << std::endl;
            m_start_time = Denn::Time::get_time();
            m_n_pass = 0;
            //clean line
            clean_line();
            //output
            write_output(true); 
            output() << std::endl;
        }

        virtual void start_a_pass() override 
        { 
            //reset
            m_sub_pass_time = Denn::Time::get_time(); 
            m_n_sub_pass    = 0;
        }

        virtual void end_a_pass() override 
        { 
            //count
            ++m_n_pass;
            //clean line
            clean_line();
            //output
            write_output(true); 
            output() << std::endl;
        }

        // virtual void start_a_sub_pass() override 
        // { 
        //     ++m_n_sub_pass;
        //     //compute pass time
        //     double pass_per_sec = (double(m_n_sub_pass) / (Denn::Time::get_time() - m_sub_pass_time));
        //     //clean line
        //     clean_line();
        //     //write output
        //     write_output();
        //     output() << " ...\t" << double(long(pass_per_sec*10.))/10.0 << " [it/s] ";
        //     output() << "\r";
        // }

        virtual void end() override
        { 
            double time_of_execution = Denn::Time::get_time() - m_start_time ;
            Scalar test_result = solver().test_function_eval(solver().current_best_network());
            output() << "+ TEST\t" << test_result       << std::endl;
            output() << "+ TIME\t" << time_of_execution << std::endl;
            output() << "=== END ===" << std::endl;
        }

    protected:

        double m_start_time;
        double m_sub_pass_time;
        long   m_n_pass;
        long   m_n_sub_pass;


        virtual void write_output(bool execute_test = false)
        {
            //////////////////////////////////////////////////////////
            size_t n_sub_pass = *parameters().m_sub_gens;
            Scalar validation =  solver().current_best_eval();
            ///////////////////////////////////////////////////////////
            output() << "|-[" << (m_n_pass) * n_sub_pass; 
            output() << "]->[ACC_VAL:" << cut_digits(validation) << "]";
            if(execute_test && *parameters().m_compute_test_per_pass)
            {
                Scalar test_result = solver().test_function_eval(solver().current_best_network());
                output() << "[ACC_TEST:" << cut_digits(test_result) << "]";
            }
            output() << "[TIME:" << (Denn::Time::get_time() - m_start_time) << "]";
        }

        virtual void clean_line()
        {
            #ifdef _WIN32
            //clean line
            for(short i=0;i!=11;++i) output() << "          ";
            //end row
            output() << "\r";
            #else 
            ::printf("%c[2K", 27);
            #endif
        }

        static double cut_digits(double val)
        {
            return double(long(std::round(val * 10000))) / 10000;
        }

    };
    REGISTERED_RUNTIME_OUTPUT(BenchOutput,"bench")
}
