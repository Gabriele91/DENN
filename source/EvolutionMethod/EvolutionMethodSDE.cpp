#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
#if 0
	class SDEMethod : public EvolutionMethod
	{
	public:
		//PHistoryMethod
		SDEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm)
		{
			//init
			m_max_distance = 1.0;
			//sub method
			m_sub_method = EvolutionMethodFactory::create(parameters().m_sub_evolution_type, m_algorithm);
		}
		//..		
		virtual void start() override { m_sub_method->start(); };
		virtual void start_a_gen_pass(DoubleBufferPopulation& g_population) override { m_sub_method->start_a_gen_pass(g_population); };
		virtual void start_a_subgen_pass(DoubleBufferPopulation& population) override 
		{ 
			m_sub_method->start_a_subgen_pass(population);
		};

		virtual void create_a_individual(DoubleBufferPopulation& population, size_t target, Individual& i_output) override { m_sub_method->create_a_individual(population, target, i_output); };
		virtual	void selection(DoubleBufferPopulation& population) override { m_sub_method->selection(population); };
		
		virtual void end_a_subgen_pass(DoubleBufferPopulation& population) override { m_sub_method->end_a_subgen_pass(population); };
		
		virtual void end_a_gen_pass(DoubleBufferPopulation& population) override { m_sub_method->end_a_gen_pass(population); };
		virtual const VariantRef get_context_data() const override { return m_sub_method->get_context_data(); }

	protected:

		//Ev md
		EvolutionMethod::SPtr m_sub_method;
		
		//max_distance in a specie
		Scalar m_max_distance;

		//Find sub specie

		//compute distance btw 2 networks
		static Scalar distance(const NeuralNetwork& net_a, const NeuralNetwork& net_b)
		{
			Scalar dis_pow_2 = 0;
			for (size_t l = 0; l != net_a.size(); ++l)		//for all layers
			for (size_t m = 0; m != net_a[l].size(); ++m)   //for all matrix
			for (size_t r = 0; r != net_a[l][m].rows(); ++r)//for all rows
			for (size_t c = 0; c != net_a[l][m].cols(); ++c)//for all cols  //n.b. eigen is col major
			{
				Scalar dif = net_a[l][m](r, c) - net_b[l][m](r, c);
				dis_pow_2 += dif*dif;
			}
			return std::sqrt(dis_pow_2);
		}

	};
	REGISTERED_EVOLUTION_METHOD(SDEMethod, "SDE")
#endif
}