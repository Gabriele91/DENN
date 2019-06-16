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
        (!(
              *parameters().m_stats_avg  
           || *parameters().m_stats_variance
           || *parameters().m_stats_distance
        ))
            return;
        //generation
        auto gen = pass * (*parameters().m_sub_gens) + sub_pass;
        //samples
        if(     *parameters().m_stats_samples > 1                    //get all?
            && ( gen && (gen % (*parameters().m_stats_samples)) != 0 //get a sample
            && (!( (gen+1) == *parameters().m_generations ))         //have to get last (in onder to save)
        )
        )
        {
            return;
        }
        //array of stats
        JsonObject jstats;
        //layout
        jstats["struct"] = nn_struct_to_json_object(m_start_network);
        //save generation
        jstats["generation"] = double(gen);
        //avg
        if(*parameters().m_stats_avg)
        {
            auto avgnn = m_start_network.copy();
            for(const auto& sub : population()) sub->avg()->copy_to(*avgnn);
            jstats["avgs"] = nn_weights_to_json_array(*avgnn);
        }
        //variance
        if(*parameters().m_stats_variance)
        {
            auto varnn = m_start_network.copy();
            for(const auto& sub : population()) sub->variance()->copy_to(*varnn);
            jstats["variances"] = nn_weights_to_json_array(*varnn);
        }
        //distances
        #if 1
        if(*parameters().m_stats_distance)
        {
            JsonArray jsub_distances;
            for(const auto&  sub : population()) jsub_distances.push_back(sub->distance_avg());
            jstats["distances"] = jsub_distances;
        }
        #endif
        //save in the same file or in multiple files
        if(*parameters().m_stats_onefile)
        {
            //first
            if(!m_statistics.document().is_array())
            {
                //init as array
                m_statistics.document() = JsonArray();
            }
            //add new sample
            m_statistics.document().array().push_back(std::move(jstats));
            //save
            if( (gen+1) == *parameters().m_generations )
            { 
                //dump
                std::ofstream(*parameters().m_stats_output) << m_statistics;
                //clean
                m_statistics.document().array().clear();
                m_statistics.document().array().shrink_to_fit();
            }
        }
        else
        {
            //ouput file
            auto path = Filesystem::get_directory(parameters().m_stats_output);
            auto name = Filesystem::get_basename(parameters().m_stats_output);
            auto ext  = Filesystem::get_extension(parameters().m_stats_output);
            //ouput
            auto outputpath = path + "/" +  name + "_" + std::to_string(gen) + ext;	
            //dump
            std::ofstream(outputpath) << Json(jstats);
        }
    }
}