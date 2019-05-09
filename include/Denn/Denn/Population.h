#pragma once
#include "Denn/Config.h"
#include "Individual.h"
#include "SubPopulation.h"

namespace Denn
{
    //class dec
    class NeuralNetwork;
    class Population;
    //subpop desc
    struct SubPopulationDescription
    {
        size_t m_np;
        IndividualMap m_map;
    };
    //alias
    using PopulationDescription = std::vector<SubPopulationDescription>;
    //help func
    inline size_t compute_max_np(const PopulationDescription& popdecs)
    {
        if(!popdecs.size()) return 0;
        size_t np = popdecs[0].m_np;
        for(size_t i = 1; i < popdecs.size(); ++i) 
            np = std::max(np, popdecs[i].m_np);
        return np;
    }
    //help func
    inline size_t compute_min_np(const PopulationDescription& popdecs)
    {
        if(!popdecs.size()) return 0;
        size_t np = popdecs[0].m_np;
        for(size_t i = 1; i < popdecs.size(); ++i) 
            np = std::min(np, popdecs[i].m_np);
        return np;
    }
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
        Population(const PopulationDescription& decs, const NeuralNetwork& master);
        Population(const PopulationDescription& decs, const Attributes& attrs, const NeuralNetwork& master);
        
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
        
        //info
        const PopulationDescription& description() const;

        //swap
        void swap(size_t pop_id, size_t ind_id);
        void swap_best(bool minimize = true);
        void swap_best(size_t pop_id, bool minimize = true);

    protected:
        //pop
        PopulationDescription m_pop_descs;
        //sliders
        SubPopulationList m_sub_pops;
		//Variant
		friend class Variant;
    };
}