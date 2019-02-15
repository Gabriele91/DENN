#pragma once
#include "Config.h"
#include "Arguments.h"
#include "ParameterInfo.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace Denn
{
	//////////////////////////////////////////////
	class Parameters
	{
	public:

		enum ReturnType
		{
			FAIL,
			SUCCESS,
			ONLYINFO 
		};

		ReadOnly<std::string>			m_dataset_filename        { "dataset" };
		ReadOnly<size_t>			    m_batch_size			  { "batch_size", size_t(100) };
		ReadOnly<size_t>			    m_batch_offset			  { "batch_offset", size_t(0) };
		ReadOnly<std::string> 			m_output_filename         { "output"  };
		ReadOnly<std::string>		    m_runtime_output_type     { "runtime_output",            "bench",    true /* false? */ };
		ReadOnly<std::string>		    m_runtime_output_file     { "runtime_output_file",            "",    true /* false? */ };
		ReadOnly<bool>				    m_compute_test_per_pass   { "compute_test_per_pass",    bool(true),  true /* false? */ };
		ReadOnly<bool>				    m_serialize_neural_network{ "serialize_neural_network", bool(true),  false /* true? */ };
        ReadOnly<bool>                  m_use_validation          { "use_validation",           bool(true),  true /* false? */ };
        ReadOnly<bool>                  m_reval_pop_on_batch      { "reval_pop_on_batch",       bool(true),  true /* false? */ };

		ReadOnly<unsigned int>	         m_seed			 { "seed", (unsigned int)(std::random_device{}())  };

		ReadOnly<std::string>	         m_instance      { "instance", "default" };
		ReadOnly<size_t>	             m_generations   { "generations", size_t(1000) };
		ReadOnly<size_t>	             m_sub_gens      { "sub_gens"  , size_t(100)   };
		ReadOnly<size_t>	             m_np            { "number_parents",size_t(16) };
		//DE
		ReadOnly<Scalar>	             m_default_f     { "f_default",Scalar(1.0)   };
		ReadOnly<Scalar>	             m_default_cr    { "cr_default",Scalar(1.0)   };
		//JDE
		ReadOnly<Scalar>	             m_jde_f         { "f_jde", Scalar(0.1)   };
		ReadOnly<Scalar>	             m_jde_cr        { "cr_jde", Scalar(0.1)   };
		//JADE/SHADE/LSHADE
		ReadOnly<size_t>	             m_archive_size { "archive_size", size_t(0) };
		ReadOnly<Scalar>	             m_f_cr_adapt   { "f_cr_adapt", Scalar(0.1) };
		//SHADE/LSHADE
		ReadOnly<size_t>	             m_shade_h      { "shade_h", size_t(10) };
		//LSHADE
		ReadOnly<size_t>	             m_min_np              { "min_number_parents"     ,size_t(4)   };
		ReadOnly<size_t>	             m_max_nfe             { "max_number_fitness_eval",size_t(160) };
		ReadOnly<Scalar>	             m_mu_cr_terminal_value{ "mu_cr_terminal_value"   ,Scalar(0.0) };
		//Current to P best
		ReadOnly<Scalar>	             m_perc_of_best { "perc_of_best", Scalar(0.1) };
		//DEGL
		ReadOnly<Scalar>                m_degl_scalar_weight{ "degl_scalar_weight",Scalar(0.5) };
		ReadOnly<size_t>                m_degl_neighborhood { "degl_neighborhood", Scalar(1)   };
		//TDE
		ReadOnly<Scalar>				 m_trig_m        { "trig_m", Scalar(0.05) };
		//Init distribution		
		ReadOnly<std::string>	         m_distribution  { "distribution", "uniform" };
		//uniform
		ReadOnly<Scalar>	             m_uniform_max     { "uniform_max", Scalar( 1.0 ) };
		ReadOnly<Scalar>	             m_uniform_min     { "uniform_min", Scalar(-1.0)  };
		//normal
		ReadOnly<Scalar>	             m_normal_mu     { "normal_mu", Scalar( 0.0 ) };
		ReadOnly<Scalar>	             m_normal_sigma  { "normal_sigma", Scalar(0.01)  };
		//DE common info
		ReadOnly<Scalar>	             m_clamp_max     { "clamp_max",  Scalar( 1.0) };
		ReadOnly<Scalar>	             m_clamp_min     { "clamp_min",  Scalar(-1.0) };
		ReadOnly<bool>	                 m_restart_enable{ "restart_enable", bool(false), false };
		ReadOnly<long>	                 m_restart_count { "restart_count", size_t(-1)    };
		ReadOnly<Scalar>	             m_restart_delta { "restart_delta", Scalar(0.001) };
		ReadOnly<size_t>	             m_threads_omp   { "threads_omp", size_t(2) };
		ReadOnly<size_t>	             m_threads_pop   { "threads_pop", size_t(2) };
		ReadOnly<size_t>	             m_history_size  { "history_size", size_t(1) };
		//type of DE
		ReadOnly<std::string>                m_mutation_type { "mutation","rand/1" };
		ReadOnly<std::vector<std::string> >  m_mutations_list_type { "mutations_list", std::vector<std::string>{ "degl", "curr_p_best" } };
		ReadOnly<std::string>                m_crossover_type{ "crossover","bin" };
		ReadOnly<std::string>                m_evolution_type    { "evolution_method","JDE" };
		ReadOnly<std::string>                m_sub_evolution_type{ "sub_evolution_method","JDE" };
		//backpropagation + SGD
		ReadOnly<Scalar>					 m_learning_rate{ "learning_rate", Scalar(0.001) };
		ReadOnly<Scalar>					 m_decay	    { "decay", Scalar(0.00005) };
		ReadOnly<Scalar>					 m_momentum	    { "momentum", Scalar(0.9) };
		ReadOnly<bool>					     m_nesterov     { "nesterov", bool(true) };
		//NN	
		ReadOnly< std::string >				 m_network { "network" , "fully_connected(,)", true };
		//params info
		std::vector< ParameterInfo >     m_params_info;
	
		Parameters();
		Parameters(int nargs, const char **vargs, bool jump_first = true);		
		ReturnType get_params(int nargs, const char **vargs, bool jump_first = true);

	private:

		ReturnType from_args(int nargs, const char **vargs);
        ReturnType from_config(const std::string& source, int nargs, const char **vargs);
		ReturnType from_json(const std::string& source);

		static bool compare_n_args(const std::vector< std::string >& keys, const char* arg);
		static std::string return_n_space(size_t n);
		std::string make_help() const;
        std::string make_help_json() const;

	};

}
