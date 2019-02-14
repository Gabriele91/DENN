//
//  DennDump.cpp
//  DENN
//
//  Created by Gabriele Di Bari on 09/10/17.
//  Copyright © 2017 Gabriele. All rights reserved.
//
#include "Denn/Core/Dump.h"


namespace Denn
{
namespace Dump
{
    std::string json_string(const std::string& str)
    {
        //copy ptr
        const char *tmp = str.c_str();
        //start '"'
        std::string out = "\"";
        //push all chars
        while (*tmp)
        {
            switch (*tmp)
            {
                case '\n':
                    out += "\\n";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                case '\b':
                    out += "\\b";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\f':
                    out += "\\f";
                    break;
                case '\a':
                    out += "\\a";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\?':
                    out += "\\?";
                    break;
                case '\'':
                    out += "\\\'";
                    break;
                case '\"':
                    out += "\\\"";
                    break;
                default:
                    out += *tmp;
                    break;
            }
            ++tmp;
        }
        return out + "\"";
    }
    
    template<> std::string json_array<bool>(const std::vector< bool >& b_list)
    {
        if (!b_list.size()) return "[]";
        //concanate names
        std::stringstream s_out;
        s_out << "[";
        for (size_t i = 0; i != b_list.size() - 1; ++i) s_out << json_bool(b_list[i]) << ", ";
        s_out << json_bool(b_list.back());
        s_out << "]";
        //end
        return s_out.str();
    }
    
    template<> std::string json_array<std::string>(const std::vector< std::string >& str_list)
    {
        if (!str_list.size()) return "[]";
        //concanate names
        std::stringstream s_out;
        s_out << "[";
        for (size_t i = 0; i != str_list.size() - 1; ++i) s_out << json_string(str_list[i]) << ", ";
        s_out << json_string(str_list.back());
        s_out << "]";
        //end
        return s_out.str();
    }
    
    template<> std::string json_array<MatrixF>(const std::vector< MatrixF >& m_list)
    {
        if (!m_list.size()) return "[]";
        //concanate names
        std::stringstream s_out;
        s_out << "[";
        for (size_t i = 0; i != m_list.size() - 1; ++i) s_out << json_matrix(m_list[i]) << ", ";
        s_out << json_matrix(m_list.back());
        s_out << "]";
        //end
        return s_out.str();
    }
    
    template<> std::string json_array<MatrixD>(const std::vector< MatrixD >& m_list)
    {
        if (!m_list.size()) return "[]";
        //concanate names
        std::stringstream s_out;
        s_out << "[";
        for (size_t i = 0; i != m_list.size() - 1; ++i) s_out << json_matrix(m_list[i]) << ", ";
        s_out << json_matrix(m_list.back());
        s_out << "]";
        //end
        return s_out.str();
    }
    
    template<>  std::string json_array<MatrixLD>(const std::vector< MatrixLD >& m_list)
    {
        if (!m_list.size()) return "[]";
        //concanate names
        std::stringstream s_out;
        s_out << "[";
        for (size_t i = 0; i != m_list.size() - 1; ++i) s_out << json_matrix(m_list[i]) << ", ";
        s_out << json_matrix(m_list.back());
        s_out << "]";
        //end
        return s_out.str();
    }
    
    std::string json_variant(const Variant& value)
    {
        Eigen::IOFormat matrix_to_json_array(Eigen::FullPrecision, Eigen::DontAlignCols, ", ", ", ", "[", "]", "[", "]");
        switch (value.get_type())
        {
            case VariantType::VR_NONE:     return json_null(); break;
            case VariantType::VR_BOOL:     return json_bool(value.get<bool>()); break;
            case VariantType::VR_CHAR:     return json_number(value.get<char>());  break;
            case VariantType::VR_SHORT:    return json_number(value.get<short>()); break;
            case VariantType::VR_INT:      return json_number(value.get<int>());   break;
            case VariantType::VR_LONG:     return json_number(value.get<long>());  break;
            case VariantType::VR_LONGLONG: return json_number(value.get<long long>()); break;
            case VariantType::VR_UCHAR:    return json_number(value.get<unsigned char>());  break;
            case VariantType::VR_USHORT:   return json_number(value.get<unsigned short>()); break;
            case VariantType::VR_UINT:     return json_number(value.get<unsigned int>()); break;
            case VariantType::VR_ULONG:    return json_number(value.get<unsigned long>()); break;
            case VariantType::VR_ULONGLONG:return json_number(value.get<unsigned long long>()); break;
            case VariantType::VR_FLOAT:    return json_number(value.get<float>());  break;
            case VariantType::VR_DOUBLE:   return json_number(value.get<double>());  break;
            case VariantType::VR_LONG_DOUBLE: return json_number(value.get<long double>());  break;
                
            case VariantType::VR_FLOAT_MATRIX:        return json_matrix<MatrixF>(value.get<MatrixF>()); break;
            case VariantType::VR_DOUBLE_MATRIX:       return json_matrix<MatrixD>(value.get<MatrixD>()); break;
            case VariantType::VR_LONG_DOUBLE_MATRIX:  return json_matrix<MatrixLD>(value.get<MatrixLD>()); break;
                
                //case VariantType::VR_INDIVIDUAL:         (*this) = ref.get<Individual>(); break;
                //case VariantType::VR_POPULATION:         (*this) = ref.get<Population>(); break;
                
            case VariantType::VR_STD_VECTOR_SHORT:             return json_array<short>(value.get< std::vector<short> >()); break;
            case VariantType::VR_STD_VECTOR_INT:               return json_array<int>(value.get< std::vector<int> >()); break;
            case VariantType::VR_STD_VECTOR_LONG:              return json_array<long>(value.get< std::vector<long> >()); break;
            case VariantType::VR_STD_VECTOR_LONGLONG:          return json_array<long long>(value.get< std::vector<long long> >()); break;
                
            case VariantType::VR_STD_VECTOR_USHORT:             return json_array<unsigned short>(value.get< std::vector<unsigned short> >()); break;
            case VariantType::VR_STD_VECTOR_UINT:               return json_array<unsigned int>(value.get< std::vector<unsigned int> >()); break;
            case VariantType::VR_STD_VECTOR_ULONG:              return json_array<unsigned long>(value.get< std::vector<unsigned long> >()); break;
            case VariantType::VR_STD_VECTOR_ULONGLONG:          return json_array<unsigned long long>(value.get< std::vector<unsigned long long> >()); break;
                
            case VariantType::VR_STD_VECTOR_FLOAT:             return json_array<float>(value.get< std::vector<float> >()); break;
            case VariantType::VR_STD_VECTOR_DOUBLE:            return json_array<double>(value.get< std::vector<double> >()); break;
            case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       return json_array<long double>(value.get< std::vector<long double> >()); break;
            case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      return json_array<MatrixF>(value.get< std::vector<MatrixF> >()); break;
            case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     return json_array<MatrixD>(value.get< std::vector<MatrixD> >()); break;
            case VariantType::VR_STD_VECTOR_LONG_DOUBLE_MATRIX:return json_array<MatrixLD>(value.get< std::vector<MatrixLD> >()); break;
                
            case VariantType::VR_C_STRING:
            case VariantType::VR_STD_STRING:           return json_string(value.get< std::string >()); break;
            case VariantType::VR_STD_VECTOR_STRING:    return json_array(value.get< std::vector<std::string> >()); break;
                
            case VariantType::VR_PTR:                   return std::to_string(value.get<size_t>()); break;
            default: return ""; break;
        }
    }
}
}
