#pragma once
#include <string>
#include <vector>
#include "Variant.h"

namespace Denn
{
namespace Dump
{
	inline std::string json_null(){ return "null"; }

	inline std::string json_bool(bool b){ return b ? "true" : "false"; }

    std::string json_string(const std::string& str);
    
    std::string json_variant(const Variant& value);
    
	template < typename T >  inline std::string json_number(const T& value){ return std::to_string(value); }
	
	template < typename T >  inline std::string json_matrix(const T& matrix)
	{
		Eigen::IOFormat matrix_to_json_array(Eigen::FullPrecision, Eigen::DontAlignCols, ", ", ", ", "[", "]", "[", "]");
		std::ostringstream ssout; ssout << matrix.format(matrix_to_json_array); return ssout.str();
	}

	template < typename T > inline std::string json_array(const std::vector< T >& t_list)
	{
		if (!t_list.size()) return "[]";
		//concanate names
		std::stringstream s_out;
		s_out << "[";
		for (size_t i = 0; i != t_list.size() - 1; ++i) s_out << std::to_string(t_list[i]) << ", ";
		s_out << std::to_string(t_list.back());
		s_out << "]";
		//end
		return s_out.str();
	}
    
    template<> std::string json_array<bool>(const std::vector< bool >& b_list);
    
    template<> std::string json_array<std::string>(const std::vector< std::string >& str_list);
    
    template<> std::string json_array<MatrixF>(const std::vector< MatrixF >& m_list);
    
    template<> std::string json_array<MatrixD>(const std::vector< MatrixD >& m_list);
    
    template<>  std::string json_array<MatrixLD>(const std::vector< MatrixLD >& m_list);
    
}
}
