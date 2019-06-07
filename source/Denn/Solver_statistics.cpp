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
            jstats["struct"] = nn_struct_to_json_object(m_start_network);
            //avg
            #if 1
            if(*parameters().m_stats_avg)
            {
                auto avgnn = m_start_network.copy();
                for(const auto& sub : population()) sub->avg()->copy_to(*avgnn);
                jstats["avgs"] = nn_weights_to_json_array(*avgnn);
            }
            #endif
            //variance
            #if 1
            if(*parameters().m_stats_variance)
            {
                auto varnn = m_start_network.copy();
                for(const auto& sub : population()) sub->variance()->copy_to(*varnn);
                jstats["variances"] = nn_weights_to_json_array(*varnn);
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
            //save in the same file or in multiple files
            if(*parameters().m_stats_onefile)
            {
                Json o_json;
                //append mode
                if(Filesystem::exists(parameters().m_stats_output))
                    o_json.parser(Filesystem::text_file_read_all(parameters().m_stats_output));
                else 
                    o_json.document() = JsonArray();
                //gen
                auto gen = pass * (*parameters().m_sub_gens) + sub_pass;
                //add as name
                jstats["generation"] = double(gen);
                //add new sample
                o_json.document().array().push_back(jstats);
                //dump
                std::ofstream(*parameters().m_stats_output) << o_json;
            }
            else 
            {
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
}