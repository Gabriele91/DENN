#include "Denn/Denn/Population.h"
#include "Denn/NeuralNetwork/NeuralNetwork.h"

namespace Denn
{
    //context
    Population::Population(size_t np, const NeuralNetwork& master)
    {
        size_t np_id = 0;
        for(size_t l = 0; l < master.size(); ++l)
        for(size_t m = 0; m < master[l].size(); ++m)
        {
            IndividualMap imap({IndividualMap::IndexPair{l,m}});
            m_sub_pops.push_back(std::make_shared<SubPopulation>(np, imap, master));
        }
    }
    Population::Population(const std::vector<size_t>& nps, const NeuralNetwork& master)
    {
        size_t np_id = 0;
        for(size_t l = 0; l < master.size(); ++l)
        for(size_t m = 0; m < master[l].size(); ++m)
        {
            IndividualMap imap({IndividualMap::IndexPair{l,m}});
            m_sub_pops.push_back(std::make_shared<SubPopulation>(nps[np_id++], imap, master));
        }
    }

    Population::Population(size_t np, const Attributes& attrs, const NeuralNetwork& master)
    {
        size_t np_id = 0;
        for(size_t l = 0; l < master.size(); ++l)
        for(size_t m = 0; m < master[l].size(); ++m)
        {
            IndividualMap imap({IndividualMap::IndexPair{l,m}});
            m_sub_pops.push_back(std::make_shared<SubPopulation>(np, attrs, imap, master));
        }
    }
    Population::Population(const std::vector<size_t>& nps,const Attributes& attrs, const NeuralNetwork& master)
    {
        size_t np_id = 0;
        for(size_t l = 0; l < master.size(); ++l)
        for(size_t m = 0; m < master[l].size(); ++m)
        {
            IndividualMap imap({IndividualMap::IndexPair{l,m}});
            m_sub_pops.push_back(std::make_shared<SubPopulation>(nps[np_id++], attrs, imap, master));
        }
    }

    //return ptr
    Population::SPtr Population::get_ptr()
    {
        return this->shared_from_this();
    }
    
    //shared copy
    Population::SPtr Population::copy() const
    {
        Population::SPtr copy = std::make_shared<Population>();
        size_t np_id = 0;
        for(size_t id = 0; id < size() ; ++id)
        {
            copy->m_sub_pops.push_back((*this)[np_id]->copy());
        }
        return copy;
    }

    //size
    size_t Population::size() const
    {
        return m_sub_pops.size();
    }

    //sub pop
	SubPopulation::SPtr& Population::operator[](size_t i)
    {
        return m_sub_pops[i];
    }
	SubPopulation::SPtr Population::operator[](size_t i) const
    {
        return m_sub_pops[i];
    }

    //swap
    void Population::swap(size_t pop_id, size_t ind_id)
    {
        m_sub_pops[pop_id]->swap(ind_id);
    }
    void Population::swap_best(size_t pop_id, bool minimize)
    {
        m_sub_pops[pop_id]->swap_best(minimize);
    }
    void Population::swap_best(bool minimize)
    {
        for(size_t p = 0; p != size(); ++p) m_sub_pops[p]->swap_best(minimize);
    }
}