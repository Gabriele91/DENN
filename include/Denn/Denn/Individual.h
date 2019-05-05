#pragma once

#include "Denn/Config.h"
#include "Denn/NeuralNetwork/NeuralNetwork.h"

namespace Denn
{
	//attributes
	enum class Attribute : size_t
	{
		F, 
		CR,
		P,
		MAX
	};
	//class dec
	class SubPopulation;
	//alias	
	using Attributes = std::array< Scalar, size_t(Attribute::MAX) >;
	////////////////////////////////////////////////////////////////////////
	class Individual : public std::enable_shared_from_this< Individual >
	{
	public:
		//ref to individual
		using Scalar     = Denn::Scalar;
		using SPtr       = std::shared_ptr<Individual>;

		//return ptr
		SPtr get_ptr();
		//shared copy
		SPtr copy() const;

		//attributes
		Individual();
		Individual(SubPopulation* subpop, AlignedMapMatrix weights);
		Individual(SubPopulation* subpop, ConstAlignedMapMatrix weights);
		Individual(SubPopulation* subpop, Attributes attrs, AlignedMapMatrix weights);
		Individual(SubPopulation* subpop, Attributes attrs, ConstAlignedMapMatrix weights);

		//copy attributes from a other individual
		void copy_from(const Individual& individual);
		void copy_attributes(const Individual& individual);

		//get attribute
		Scalar& eval();
		Scalar& f ();
		Scalar& cr();
		Scalar& p ();
		Scalar& attribute(Attribute);

		Scalar eval() const;
		Scalar f () const;
		Scalar cr() const;
		Scalar p () const;
		Scalar attribute(Attribute) const;
		
		//matrix
		AlignedMapMatrix matrix();
		ConstAlignedMapMatrix matrix() const;

		auto array() { return  matrix().array(); }
		auto array() const { return  matrix().array(); }

		//cast
		operator AlignedMapMatrix ();
		operator ConstAlignedMapMatrix () const;
		
		//denn help function
		void no_0_weights();

		//get subpopulation ref
		const SubPopulation* subpopulation() const { return m_subpopulation; }

	protected:
		//subpop
		SubPopulation* m_subpopulation;
	    //attributes
		Scalar m_eval{ std::numeric_limits<Scalar>::max() };
		std::array< Scalar, size_t(Attribute::MAX) > m_attributes;
		Matrix m_weights;
	};
	//Alias list of Individuals
	using IndividualList  = std::vector< Individual::SPtr >;
	//compute distance
	template <>
	inline Individual::Scalar distance_pow2<const Individual>(const Individual& a, const Individual& b)
	{
		return distance_pow2<ConstAlignedMapMatrix>(a.matrix(), b.matrix());
	} 

}