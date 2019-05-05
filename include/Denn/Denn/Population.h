#pragma once
#include "Denn/Config.h"
#include "Individual.h"
#include "SubPopulation.h"

namespace Denn
{
    //class dec
    class NeuralNetwork;
    class Population;
    class SubPopulationGroup;
    //Population
    class Population  : public std::enable_shared_from_this< Population >
    {
    public:
        //alias
        using Iterator          = SubPopulationList::iterator;
        using ConstIterator     = SubPopulationList::const_iterator;
        using SPtr              = std::shared_ptr< Population >;


		//return ptr
		SPtr get_ptr();
        
		//shared copy
		SPtr copy() const;

        //context        
		Population(){}
        Population(size_t np, const NeuralNetwork& master);
        Population(const std::vector<size_t>& nps, const NeuralNetwork& master);
        Population(size_t np,const Attributes& attrs, const NeuralNetwork& master);
        Population(const std::vector<size_t>& nps,const Attributes& attrs, const NeuralNetwork& master);
        
        //size
        size_t size() const;

        //sub pop
		SubPopulation::SPtr& operator[](size_t i);
	    SubPopulation::SPtr operator[](size_t i) const;

        //iterators
        Iterator begin(){ return m_sub_pops.begin(); }
        Iterator end(){ return m_sub_pops.end(); }
        ConstIterator begin() const { return m_sub_pops.begin(); }
        ConstIterator end() const { return m_sub_pops.end(); }
        
        //swap
        void swap(size_t pop_id, size_t ind_id);
        void swap_best(bool minimize = true);
        void swap_best(size_t pop_id, bool minimize = true);

    protected:
        //sliders
        SubPopulationList m_sub_pops;
		//Variant
		friend class Variant;
    };
}