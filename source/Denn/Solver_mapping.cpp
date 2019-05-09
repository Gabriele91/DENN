#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{
	/////////////////////////////////////////////////
	//map population
	PopulationDescription Solver::one_sub_population()
	{
		size_t np = 0;
		IndividualMap imap;
		PopulationDescription desc;
        for(size_t l = 0; l < m_start_network.size(); ++l)
		{
			for(size_t m = 0; m < m_start_network[l].size(); ++m)
			{
				imap.push(IndividualMap::IndexPair{l,m});
				np += m_start_network[l][m].rows();
			}
		}
		if(*parameters().m_np_perc > Scalar(0))
			np = np * (*parameters().m_np_perc);
		else
			np =  *parameters().m_np;		
		desc.push_back(SubPopulationDescription{ np, imap });
		return desc;
	}
	PopulationDescription Solver::layer_sub_population()
	{
		PopulationDescription desc;
        for(size_t l = 0; l < m_start_network.size(); ++l)
			if(m_start_network[l].size())
			{
				size_t np = 0;
				IndividualMap imap;
				for(size_t m = 0; m < m_start_network[l].size(); ++m)
				{
					imap.push(IndividualMap::IndexPair{l,m});
					np += m_start_network[l][m].rows();
				}
				if(*parameters().m_np_perc > Scalar(0))
					np = np * (*parameters().m_np_perc);
				else
					np =  *parameters().m_np;
				desc.push_back(SubPopulationDescription{ np, imap });
			}
		return desc;
	}
	PopulationDescription Solver::matrix_sub_population()
	{
		PopulationDescription desc;
        for(size_t l = 0; l < m_start_network.size(); ++l)
        for(size_t m = 0; m < m_start_network[l].size(); ++m)
        {
			size_t np =  *parameters().m_np_perc > Scalar(0)
						? size_t(m_start_network[l][m].rows() * *parameters().m_np_perc) 
						:*parameters().m_np;

			desc.push_back(SubPopulationDescription{ 
				np, 
        		IndividualMap({ 
					IndividualMap::IndexPair{l,m}
				})
			});
		}
		return desc;
	}
}