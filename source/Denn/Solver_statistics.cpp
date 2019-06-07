#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"
#include "Denn/Core/Json.h"
#include "Denn/Utilities/Networks.h"
#include <fstream>

namespace Denn
{
    void Solver::execute_statistics(size_t pass, size_t sub_pass)
    {
        if
        (
           *parameters().m_stats_avg 
        || *parameters().m_stats_variance
        || *parameters().m_stats_distance
        )
        {
            //array of stats
            JsonObject jstats;
            //layout
            jstats["layout"] = get_string_from_network(m_start_network);
            //avg
            #if 1
            if(*parameters().m_stats_avg)
            {
                auto avgnn = m_start_network.copy();
                for(const auto& sub : population()) sub->avg()->copy_to(*avgnn);
                jstats["avgs"] = nn_to_json_object(*avgnn);
            }
            #endif
            //variance
            #if 1
            if(*parameters().m_stats_variance)
            {
                auto varnn = m_start_network.copy();
                for(const auto& sub : population()) sub->variance()->copy_to(*varnn);
                jstats["variances"] = nn_to_json_object(*varnn);
            }
            #endif
            //distances
            #if 1
            if(*parameters().m_stats_variance)
            {
                JsonArray jsub_distances;
                for(const auto&  sub : population()) jsub_distances.push_back(sub->distance_avg());
                jstats["distances"] = jsub_distances;
            }
            #endif
            //ouput file
			auto path = Filesystem::get_directory(parameters().m_stats_output);
			auto name = Filesystem::get_basename(parameters().m_stats_output);
			auto ext  = Filesystem::get_extension(parameters().m_stats_output);
            //gen
            auto gen = pass * (*parameters().m_sub_gens) + sub_pass;
            //ouput
			auto outputpath = path + "/" +  name + "_" + std::to_string(gen) + ext;	
            //dump
            std::ofstream(outputpath) << Json(jstats);
        }
    }
}