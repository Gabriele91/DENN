#pragma once

#include "Denn/Config.h"
#include "Denn/NeuralNetwork/NeuralNetwork.h"

namespace Denn
{
	//class dec
	class SubPopulation;
	//description of layout of a individual
	struct IndividualMap
	{
		//list
		using IndexPair =  std::pair< size_t, size_t > ;
		using IndexPairList = std::vector< IndexPair >;  
		using Iterator = std::vector< IndexPair >::iterator;  
		using CIterator = std::vector< IndexPair >::const_iterator;  
		//struct
		IndexPairList m_indexs;
		//constructor
		IndividualMap() = default;
		IndividualMap(const IndexPairList& list) : m_indexs(list) { }
		//add
		size_t size() const { return m_indexs.size(); }
		size_t layer(size_t i){ return  m_indexs[i].first; }
		size_t matrix(size_t i){ return  m_indexs[i].second; }
		//push 
		void push(const IndexPair& pair){ m_indexs.push_back(pair); }
		void clear(){ m_indexs.clear(); }
		//iterators
		Iterator begin() { return m_indexs.begin(); }
		CIterator begin() const { return m_indexs.begin(); }
		Iterator end() { return m_indexs.end(); }
		CIterator end() const { return m_indexs.end(); }

	};
	//attributes
	enum class Attribute : size_t
	{
		F, 
		CR,
		P,
		MAX
	};
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
		//shared copy
		SPtr copy(SubPopulation* subpop) const;

		//attributes
		Individual();
		Individual(SubPopulation* subpop, const NeuralNetwork& nn);
		Individual(SubPopulation* subpop, Attributes attrs, const NeuralNetwork& nn);
		
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
		AlignedMapMatrix matrix(const Matrix& msize);
		ConstAlignedMapMatrix matrix(const Matrix& msize) const;

		ColArray& array();
		const ColArray& array() const;
		
		//denn help function
		void no_0_weights();

		//get subpopulation ref
		const SubPopulation* subpopulation() const;
		const IndividualMap* map() const;
		
		//apply to network
		void copy_to(NeuralNetwork& nn);
		//copy value to individual
		void copy_from(const NeuralNetwork& nn);

	protected:
		//subpop
		SubPopulation* m_subpopulation;
	    //attributes
		Scalar m_eval{ std::numeric_limits<Scalar>::max() };
		std::array< Scalar, size_t(Attribute::MAX) > m_attributes;
		ColArray m_weights;
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