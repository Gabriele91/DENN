#pragma once
#include "Config.h"

namespace Denn
{
	//dec class
	class Solver;
	class EvolutionMethod;
	class Parameters;
	class Random;
	class DataSet;
	class Individual;
	class NeuralNetwork;

    //Evaluate function
	class Evaluation : public std::enable_shared_from_this<Evaluation>
	{		
    public:
        //ref to Evaluation
        using SPtr = std::shared_ptr<Evaluation>;
		//return ptr
		SPtr get_ptr() { return this->shared_from_this(); }
		//Evaluation info
        Evaluation();
        //methods
        virtual bool minimize() const = 0;
        virtual Scalar operator () (const NeuralNetwork&, const DataSet&) = 0;	
    };

	//class factory of Evaluation methods
	class EvaluationFactory
	{

	public:
		//public
		static Evaluation::SPtr get(const std::string& name);
		static void append(const std::string& name, Evaluation::SPtr);
		//util
		template < typename T >
		static std::shared_ptr< T > get(const std::string& name)
		{
			return std::dynamic_pointer_cast<T>(get(name));
		}
		//list of methods
		static std::vector< std::string > list_of_evaluators();
		static std::string names_of_evaluators(const std::string& sep = ", ");
		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class EvaluationItem
	{
		EvaluationItem(const std::string& name)
		{
			EvaluationFactory::append(name, std::static_pointer_cast<Evaluation>(std::make_shared<T>()));
		}

	public:

		static EvaluationItem<T>& instance(const std::string& name)
		{
			static EvaluationItem<T> objectItem(name);
			return objectItem;
		}

	};


	#define REGISTERED_EVALUATION(class_,name_)\
	namespace\
	{\
		static const EvaluationItem<class_>& _Denn_ ## class_ ## _EvaluationItem= EvaluationItem<class_>::instance( name_ );\
	}
}