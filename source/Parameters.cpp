#include "Denn/Parameters.h"
#include "Denn/RuntimeOutput.h"
#include "Denn/SerializeOutput.h"
#include "Denn/Instance.h"
#include "Denn/Version.h"
#include "Denn/Utilities/Networks.h"
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Crossover.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Solver.h"
#include <sstream>
#include <iostream>

namespace Denn
{     
    Parameters::Parameters() 
    :m_params_info
    ({
        ParameterInfo {
            m_instance, "Type of instance", { "--test_type", "-ins", "-tt" },
            [this](Arguments& args) -> bool  
            { 
                std::string str_m_type = args.get_string() ;
                //all lower case
                std::transform(str_m_type.begin(),str_m_type.end(), str_m_type.begin(), ::tolower);
                //save
				m_instance = str_m_type;
                //ok 
                return InstanceFactory::exists(*m_instance);
            }
            , { "string", InstanceFactory::list_of_instances() }
        },
        ParameterInfo { 
            m_generations, "Global number of generation [or backpropagation pass]", { "-t", "-g"  }
        },
        ParameterInfo { 
            m_sub_gens, "Number of generation [or backpropagation pass] per batch", { "-s" , "-sg" }
        },
        ParameterInfo {
            m_seed, "Random generator seed", { "-sd"  }
        },
        ParameterInfo {
            m_np, "Number of parents", { "-np"  }
        },
        ParameterInfo {
            m_np_perc, "Percentage of parents respect to parameters", { "-npp"  }
        },
        ParameterInfo {
            m_conet_build, "How to build the main network", { "-cnet" },
            [this](Arguments& args) -> bool  
            { 
                std::string str_m_type = args.get_string();
                //all lower case
                std::transform(str_m_type.begin(),str_m_type.end(), str_m_type.begin(), ::tolower);
                //save
				m_conet_build = str_m_type;
                //ok 
                return BuildNetwork::exists(*m_conet_build);
            }
            , { "string", BuildNetwork::list_of_build() }
        },
        ParameterInfo {
            m_batch_size, "Batch size", { "-b" },
            [this](Arguments& args) -> bool  
            { 
                //int value
                m_batch_size = args.get_int();
                //if m_batch_offset == 0 then offset = size
                if(!(*m_batch_offset)) m_batch_offset = *m_batch_size;
                //ok 
                return true;
            }
        },
        ParameterInfo {
            m_batch_offset, "Batch offset, how many records will be replaced in the next batch [<= batch size]", { "-bo" }
        },
        ParameterInfo {
            m_use_validation, "Use the validation test", { "-uv" }
        },
        ParameterInfo {
            m_reval_pop_on_batch, "Reevaluate the population on change of a batch", { "-rpob" }
        },
        ParameterInfo {
            m_evolution_type, "Type of evolution method", { "--evolution",    "-em"  },
            [this](Arguments& args) -> bool  
            { 
                std::string str_m_type = args.get_string() ;
                //all lower case
                std::transform(str_m_type.begin(),str_m_type.end(), str_m_type.begin(), ::toupper);
                //save
				m_evolution_type = str_m_type;
                //ok 
                return EvolutionMethodFactory::exists(*m_evolution_type);
            }
            , { "string", EvolutionMethodFactory::list_of_evolution_methods() }
        },
        ParameterInfo {
              m_mutation_type
		    , { m_evolution_type }
            , "Type of DE mutation"
            , { "-m"  }
            , [this](Arguments& args) -> bool
              {
                  std::string str_m_type = args.get_string() ;
                  //all lower case
                  std::transform(str_m_type.begin(),str_m_type.end(), str_m_type.begin(), ::tolower);
                  //save
                  m_mutation_type = str_m_type;
                  //ok
                  return MutationFactory::exists(*m_mutation_type);
              }
            , { "string", MutationFactory::list_of_mutations() }
        },
        ParameterInfo {
              m_crossover_type
			, { m_evolution_type }
            , "Type of DE crossover"
            , { "-co"  }
            , [this](Arguments& args) -> bool
              {
                  std::string str_c_type = args.get_string();
                  //all lower case
                  std::transform(str_c_type.begin(),str_c_type.end(), str_c_type.begin(), ::tolower);
                  //save
                  m_crossover_type = str_c_type;
                  //ok
                  return CrossoverFactory::exists(*m_crossover_type);
              }
            , { "string", CrossoverFactory::list_of_crossovers() }
        },

        ParameterInfo { 
            m_default_f, "Default F factor for DE", { "-f"  }
        },
        ParameterInfo { 
            m_default_cr, "Default CR factor for DE", { "-cr" }
        },
                            
        ParameterInfo { 
              m_jde_f
			, { m_evolution_type,{ Variant("JDE") } }
			, "Probability of change of F (JDE)"
			, { "-jf"  }
        },
        ParameterInfo { 
              m_jde_cr
		    , { m_evolution_type,{ Variant("JDE") } }
			, "Probability of change of CR (JDE)"
		    , { "-jcr"  }
        },
				
		ParameterInfo{
			 m_archive_size
			, { m_evolution_type,{ Variant("JADE"), Variant("SHADE"), Variant("L-SHADE") } }
			, "Archive size (JADE/SHADE/L-SHADE)",{ "-as" }
		},				
		ParameterInfo{
			  m_f_cr_adapt
			, { m_evolution_type,{ Variant("JADE") } }
		    , "Auto adaptation factor of f and cr parameters (JADE)"
		    , { "-afcr" }
		},

		ParameterInfo{
			  m_shade_h
			, { m_evolution_type,{ Variant("SHADE"), Variant("L-SHADE") } }
		    , "Size of archive of mu_f and mu_cr (SHADE/L-SHADE)",{ "-shah" }
		}, 

		ParameterInfo{
			m_min_np
			, { m_evolution_type,{ Variant("L-SHADE") } }
			, "Minimum number of parents (L-SHADE)",{ "-mnp" }
		},
		ParameterInfo{
			  m_max_nfe
			, { m_evolution_type,{ Variant("L-SHADE") } }
			, "Maximum number of fitness evaluation (L-SHADE)",{ "-mnfe" }
		},
		ParameterInfo{
			  m_mu_cr_terminal_value
			, { m_evolution_type,{ Variant("L-SHADE") } }
			, "Terminal value of MU Cr (L-SHADE)",{ "-mcrtv" }
		},

		ParameterInfo {
			  m_perc_of_best
			, { m_mutation_type, { Variant("curr_p_best") } }
			, "Percentage of best (curr_p_best)",{ "-pob", "-p" }
		},	

		ParameterInfo{
			  m_degl_scalar_weight
			, { m_mutation_type, { Variant("degl"), Variant("pro_degl"), Variant("deglbp") } }
			, "Scalar weight used to combinate local mutant with global mutant (degl/pro_degl/deglbp)",{ "-glw" }
		},				
		ParameterInfo{
			  m_degl_neighborhood
			, { m_mutation_type, { Variant("degl"), Variant("pro_degl"), Variant("deglbp") } }
			, "Neighborhood of target where is searched the best (degl/pro_degl/deglbp)",{ "-glnn" }
		},	
		ParameterInfo{
			m_trig_m, "Probability of the mutation to be trigonometric (TDE)",{ "-tm" }
        },        
        ParameterInfo{ 
            m_distribution, "Choose the distribution for initialization", { "-distr"  },
            [this](Arguments& args) -> bool 
            {
                //get distribution
				m_distribution = args.get_string() ;
                //test
                return
                (
                   *m_distribution ==  "uniform" 
                || *m_distribution == "normal" 
                );
            } 
            , { "string", { "uniform", "normal" } }
        },   
        ParameterInfo{ 
              m_uniform_min
			, { m_distribution,{ Variant("uniform") } }
            , "Minimum size of weight in uniform initialization"
            , { "-umin"  }
        },
        ParameterInfo{ 
              m_uniform_max
			, { m_distribution,{ Variant("uniform") } }
            , "Maximum size of weight in uniform initialization"
            , { "-umax"  }
        },
        ParameterInfo{ 
              m_normal_mu
			, { m_distribution,{ Variant("normal") } }
            , "Mu parameter of normal distribution for initialization"
            , { "-nmu"  }
        },
        ParameterInfo{ 
              m_normal_sigma
			, { m_distribution,{ Variant("normal") } }
            , "Sigma parameter of normal distribution for initialization"
            , { "-nsigma"  }
        },
        ParameterInfo{ 
            m_clamp_min, "Minimum size of weight", { "-cmin"  }
        },
        ParameterInfo{ 
            m_clamp_max, "Maximum size of weight", { "-cmax"  }
        },
        ParameterInfo{ 
			m_learning_rate, "Learning rate of backpropagation for each pass", { "-lrate", "-lrbp"  },
            [this](Arguments& args) -> bool 
            {
				m_learning_rate = args.get_double() ;
                return Scalar(0.0) <= *m_learning_rate;
            } 
        },
        ParameterInfo{ 
			m_decay, "Decay factor of backpropagation for each pass", { "-dbp"  },
            [this](Arguments& args) -> bool 
            {
				m_decay = args.get_double() ;
                return Scalar(0.0) <= *m_decay;
            } 
        },    
        ParameterInfo{ 
			m_momentum, "Momentum of backpropagation for each pass", { "-mbp"  },
            [this](Arguments& args) -> bool 
            {
				m_momentum = args.get_double() ;
                return Scalar(0.0) <= *m_momentum;
            } 
        },       
        ParameterInfo{ 
			m_nesterov, "Momentum of backpropagation for each pass", { "-nbp"  }
        },       
		ParameterInfo{
            m_network, "network layout", { "-nn"  },
            [this](Arguments& args) -> bool 
            {
				//clear
				m_network = "";
				//ok
				for (int count = 0; !args.end_vals(); ++count)
				{
					std::string str_c_type = args.get_string();
					//all lower case
					std::transform(str_c_type.begin(),str_c_type.end(), str_c_type.begin(), ::tolower);
					//add space
					if(count) m_network.get() += " ";
					//append
					m_network.get() += str_c_type;
				}
                //test
				return std::get<1>(get_network_from_string_test(m_network.get()));
            }
			,{ "string" }
		},
        ParameterInfo{
            m_dataset_filename, "Path of dataset file (gz) or network file input (json)", { "-d", "-i" }
        },
        ParameterInfo{ 
            m_output_filename, "Path of output file (json)", { "-o" } 
        },
        ParameterInfo{
            m_runtime_output_file, "Write the runtime output to stream [::cout, ::cerr, <file>]", { "-rof"  }
        },
        ParameterInfo{
            m_compute_test_per_pass, "Compute the test accuracy for each pass", { "-ctps"  }
        },
        ParameterInfo{
            m_serialize_neural_network, "Serialize the neural network of best individual", { "-snn"  }
        },
        ParameterInfo{ 
            m_threads_omp, "Number of threads using by OpenMP", { "-omp"  }
        },
        ParameterInfo{ 
            m_threads_pop, "Number of threads using for  generate a new population", { "-tp"  }
        },
        ParameterInfo{
            "Print list of instances", { "--instances-list", "-ilist"  }, 
            [this](Arguments& args) -> bool { std::cout << InstanceFactory::names_of_instances() << std::endl; return true; } 
        },
        ParameterInfo{
            "Print list of evolution methods", { "--evolution_method-list", "--evolution-list",    "-elist"  }, 
            [this](Arguments& args) -> bool { std::cout << EvolutionMethodFactory::names_of_evolution_methods() << std::endl; return true; } 
        },
        ParameterInfo{
            "Print list of muations", { "--mutation-list",    "-mlist"  }, 
            [this](Arguments& args) -> bool { std::cout << MutationFactory::names_of_mutations() << std::endl; return true; } 
        },
        ParameterInfo{
            "Print list of crossovers", { "--crossover-list",    "-colist"  }, 
            [this](Arguments& args) -> bool { std::cout << CrossoverFactory::names_of_crossovers() << std::endl; return true; } 
        },
        ParameterInfo{
            "Print list of runtime output", { "--runtime_output-list",    "-rolist"  }, 
            [this](Arguments& args) -> bool { std::cout << RuntimeOutputFactory::names_of_runtime_outputs() << std::endl; return true; } 
        },
		ParameterInfo{
			"Print list of serializer",{ "--serializer-list",    "-slist" },
			[this](Arguments& args) -> bool { std::cout << SerializeOutputFactory::names_of_serialize_outputs() << std::endl; return true; }
		},
		ParameterInfo{
			"Print list of layers",{ "--layer-list",    "-llist" },
			[this](Arguments& args) -> bool { std::cout << LayerFactory::names_of_layers() << std::endl; return true; }
		},
        ParameterInfo{
            "Print the help", { "--help",    "-h"  },
            [this](Arguments& args) -> bool { std::cout << make_help();  return true; }
        },
        ParameterInfo{
            "Print the json help", { "--jhelp",    "-jh"  },
            [this](Arguments& args) -> bool { std::cout << make_help_json();  return true; }
        },
        ParameterInfo{
            "Displays the program's version", { "--version",    "-v"  },
            [this](Arguments& args) -> bool { std::cout << "version: " << DENN_VER_STR << std::endl;  return true; }
        }
    })
    {
    }
			
    Parameters::Parameters(int nargs, const char **vargs, bool jump_first) : Parameters()
    {
		if (!get_params(nargs, vargs, jump_first)) throw std::runtime_error("fail to parse parameters");
    }

	Parameters::ReturnType Parameters::get_params(int nargs, const char **vargs, bool jump_first)
	{
        if (!nargs || (nargs <= 1 && jump_first)) return ONLYINFO;
		//config
        if (Denn::Filesystem::get_extension(vargs[jump_first]) == ".config")
        {
            return from_config(Denn::Filesystem::text_file_read_all(vargs[jump_first]), nargs - 1 - jump_first, &vargs[jump_first+1]);
        }
        else if (Denn::Filesystem::get_extension(vargs[jump_first]) == ".json")
        {
            return from_json(Denn::Filesystem::text_file_read_all(vargs[jump_first]));
        }
		else
		{
			return from_args(nargs-1, &vargs[jump_first]);
        }
	}

    bool Parameters::compare_n_args(const std::vector< std::string >& keys, const char* arg)
    {
        for (const std::string& key : keys) if (strcmp(key.c_str(), arg) == 0) return true;
        return false;
    }

    std::string Parameters::return_n_space(size_t n)
    {
        std::string out;
        while(n--) out+=" ";
        return out;
    }

    std::string Parameters::make_help() const
    {
        std::stringstream s_out;
        //header
        s_out << "Usage: denn [options] <arg>" ;
        s_out << std::endl;				
        s_out << "Options:";				
        s_out << std::endl;
        //style
        const size_t max_space_line         = 60;
        const size_t padding_to_description = 2;
        const size_t padding_left           = 4;
        const std::string separetor(", ");
        //determinate space line init
        size_t space_line_init = 0;
        //
        for(auto& param : m_params_info)
        {
            size_t n_space_used = 0;
            for(auto& key : param.m_arg_key) n_space_used += key.size() + separetor.size();
            space_line_init = std::max(n_space_used, space_line_init);
        }
        //add padding
        space_line_init += padding_to_description;
        std::string padding_left_str = return_n_space(padding_left);
        //print
        for(auto& param : m_params_info)
        {
            size_t space_line = space_line_init;
            s_out << padding_left_str;
            for(auto& key : param.m_arg_key)
            {
                s_out << key << separetor;
                space_line -= key.size() + separetor.size();
            }
            s_out << return_n_space(Denn::clamp<size_t>(space_line,0,max_space_line));
            s_out << param.m_description;
			//show choises
			if (param.m_domain.m_type == ParameterDomain::CHOISE)
			{
				s_out << " " << Dump::json_array<std::string>(param.m_domain.m_choises) << "\n";
			}
			else
			{
				s_out << std::endl;
			}
        }
        s_out << std::endl;
        return s_out.str();
    }
	    
    std::string Parameters::make_help_json() const
    {
        std::stringstream s_out;
        std::string arg_space("  ");
        std::string in_arg_inspace("  ");
        s_out << "[\n";
        for(size_t i = 0; i != m_params_info.size(); ++i)
        {
            auto& param = m_params_info[i];
            if(!param.m_associated_variable) continue;
            
            if(i) s_out << ",\n";
            s_out << arg_space << "{\n";
            s_out << arg_space << in_arg_inspace << "\"name\"" << " : "	<< Dump::json_string(param.m_associated_variable->name()) << ",\n";
            s_out << arg_space << in_arg_inspace << "\"args:\"" << " : " << Dump::json_array<std::string>(param.m_arg_key) << ",\n";
			s_out << arg_space << in_arg_inspace << "\"description\"" << " : " << Dump::json_string(param.m_description) << ",\n";
			s_out << arg_space << in_arg_inspace << "\"serializable\"" << " : " << Dump::json_bool(param.serializable()) << ",\n";
            s_out << arg_space << in_arg_inspace << "\"domain\"" << " : ";
            switch (param.m_domain.m_type)
            {
                case ParameterDomain::MONO:
                    s_out << Dump::json_string(param.m_domain.m_domain) << "\n";
                break;
                case ParameterDomain::CHOISE:
                    s_out << Dump::json_array<std::string>(param.m_domain.m_choises) << "\n";
                break;
                default:
                    s_out << Dump::json_null() << "\n";
                break;
            }
            s_out << arg_space << "}";
        }
        s_out << "\n]";
        s_out << std::endl;
        return s_out.str();
    }
}
