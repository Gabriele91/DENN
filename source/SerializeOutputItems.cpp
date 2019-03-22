#include "Denn/SerializeOutput.h"
#include "Denn/Parameters.h"
#include "Denn/NeuralNetwork.h"

namespace Denn
{
	class JSONSerializeOutput : public SerializeOutput
	{
	public:

		JSONSerializeOutput(std::ostream& ostream, const Parameters& params) : SerializeOutput(ostream, params)
		{
			output() << "{" << std::endl;
		}

		virtual ~JSONSerializeOutput()
		{
			output() << "}" << std::endl;
		}

		virtual void serialize_parameters(const Denn::Parameters& args) override
		{
			output() << "\t\"arguments\" :" << std::endl;
			output() << "\t{" << std::endl;
			//get only serializables
			std::vector < const ParameterInfo* > params_serializable;
			for (auto& params : args.m_params_info)
				if (params.serializable())  params_serializable.emplace_back(&params);
			//serialize
			for (size_t i = 0; i != params_serializable.size(); ++i)
			{
				auto* variable = params_serializable[i]->m_associated_variable;
				output() << "\t\t\""
					<< variable->name() << "\" : "
					<< Dump::json_variant(variable->variant())
					<< ((i + 1) != params_serializable.size() ? "," : "")
					<< std::endl;
			}

			output() << "\t}," << std::endl;
		}

		virtual void serialize_best(double time, Denn::Scalar accuracy, const Denn::NeuralNetwork& network) override
		{
			output() << "\t\"time\" : " << Dump::json_number(time) << "," << std::endl;
			output() << "\t\"accuracy\" : " << Dump::json_number(accuracy) << "," << std::endl;
			//...
			if (!*parameters().m_serialize_neural_network) return;
			//serialize net
			output() << "\t\"network\" : [" << std::endl;
			for (size_t i = 0; i != network.size(); ++i)
			{
				output()
					<< "\t\t[";
				for (size_t m = 0; m != network[i].size(); ++m)
				{
					output()
						<< Dump::json_matrix< Matrix >(network[i][m])
						<< ((m != network[i].size() - 1) ? "," : "");
				}
				output()
					<< ((i != network.size() - 1) ? "]," : "]")
					<< std::endl;
			}
			output() << "\t]" << std::endl;
		}

	private:

	};
	REGISTERED_SERIALIZE_OUTPUT(JSONSerializeOutput, ".json")

	class CSVSerializeOutput : public SerializeOutput
	{
	public:

		CSVSerializeOutput(std::ostream& ostream, const Parameters& params) : SerializeOutput(ostream, params)
		{
		}

		virtual ~CSVSerializeOutput()
		{
		}

		virtual void serialize_parameters(const Denn::Parameters& args) override
		{
			//get only serializables
			std::vector < const ParameterInfo* > params_serializable;
			for (auto& params : args.m_params_info)
				if (params.serializable())  params_serializable.emplace_back(&params);
			//print header
			for (size_t i = 0; i != params_serializable.size(); ++i)
				output() << params_serializable[i]->m_associated_variable->name() << "; ";
			output() << "time; accuracy" << (*parameters().m_serialize_neural_network ? "; neutal network" : "");
			output() << std::endl;
			//serialize
			for (size_t i = 0; i != params_serializable.size(); ++i)
				output() << variant_to_str(params_serializable[i]->m_associated_variable->variant()) << "; ";
		}

		virtual void serialize_best(double time, Denn::Scalar accuracy, const Denn::NeuralNetwork& network) override
		{
			output() << time << "; " << accuracy << "; " ;
			//no full?
			if (!*parameters().m_serialize_neural_network)
			{
				output() << std::endl;
				return;
			}
			//print NN
			for (size_t i = 0; i != network.size(); ++i)
			{
				output()
					<< "[";
				for (size_t m = 0; m != network[i].size(); ++m)
				{
					output()
						<< to_string_matrix< Matrix >(network[i][m])
						<< ((m != network[i].size() - 1) ? "," : "");
				}
				output()
					<< ((i != network.size() - 1) ? "]," : "]");
			}
			output() << std::endl;

		}

	private:

		template < typename T >
		static std::string to_string_matrix(const T& matrix)
		{
			Eigen::IOFormat matrix_to_json_array(Eigen::FullPrecision, Eigen::DontAlignCols, ", ", ", ", "[", "]", "[", "]");
			std::ostringstream ssout; ssout << matrix.format(matrix_to_json_array); return ssout.str();
		}

		template< typename T >
		static std::string vector_to_str(const std::vector< T >& vec_of_t)
		{
			std::stringstream ssout;
			ssout << "[ ";
			if (vec_of_t.size()) ssout << std::to_string(vec_of_t[0]);
			for (size_t i = 1; i < vec_of_t.size(); ++i) ssout << ", " << std::to_string(vec_of_t[i]);
			ssout << " ]";
			return ssout.str();
		}

		template< typename T >
		static std::string vector_to_str_matrix(const std::vector< T >& vec_of_t)
		{
			std::stringstream ssout;
			ssout << "[ ";
			if (vec_of_t.size()) ssout << to_string_matrix(vec_of_t[0]);
			for (size_t i = 1; i < vec_of_t.size(); ++i) ssout << ", " << to_string_matrix(vec_of_t[i]);
			ssout << " ]";
			return ssout.str();
		}

		static std::string vector_to_str_string(const std::vector< std::string >& vec_of_t)
		{
			std::stringstream ssout;
			ssout << "[ ";
			if (vec_of_t.size()) ssout << "\"" << vec_of_t[0] << "\"";
			for (size_t i = 1; i < vec_of_t.size(); ++i) ssout << ", " << "\"" << vec_of_t[i] << "\"";
			ssout << " ]";
			return ssout.str();
		}

		static std::string variant_to_str(const Variant& value)
		{
			Eigen::IOFormat matrix_to_json_array(Eigen::FullPrecision, Eigen::DontAlignCols, ", ", ", ", "[", "]", "[", "]");
			switch (value.get_type())
			{
			case VariantType::VR_NONE:     return "null";                              break;
			case VariantType::VR_BOOL:     return value.get<bool>() ? "true" : "false"; break;
			case VariantType::VR_CHAR:     return std::to_string(value.get<char>());  break;
			case VariantType::VR_SHORT:    return std::to_string(value.get<short>()); break;
			case VariantType::VR_INT:      return std::to_string(value.get<int>());   break;
			case VariantType::VR_LONG:     return std::to_string(value.get<long>());  break;
			case VariantType::VR_LONGLONG: return std::to_string(value.get<long long>()); break;
			case VariantType::VR_UCHAR:    return std::to_string(value.get<unsigned char>());  break;
			case VariantType::VR_USHORT:   return std::to_string(value.get<unsigned short>()); break;
			case VariantType::VR_UINT:     return std::to_string(value.get<unsigned int>()); break;
			case VariantType::VR_ULONG:    return std::to_string(value.get<unsigned long>()); break;
			case VariantType::VR_ULONGLONG:return std::to_string(value.get<unsigned long long>()); break;
			case VariantType::VR_FLOAT:    return std::to_string(value.get<float>());  break;
			case VariantType::VR_DOUBLE:   return std::to_string(value.get<double>());  break;
			case VariantType::VR_LONG_DOUBLE:   return std::to_string(value.get<long double>());  break;

			case VariantType::VR_FLOAT_MATRIX:        return to_string_matrix<MatrixF>(value.get<MatrixF>()); break;
			case VariantType::VR_DOUBLE_MATRIX:       return to_string_matrix<MatrixD>(value.get<MatrixD>()); break;
			case VariantType::VR_LONG_DOUBLE_MATRIX:  return to_string_matrix<MatrixLD>(value.get<MatrixLD>()); break;

				//case VariantType::VR_INDIVIDUAL:         (*this) = ref.get<Individual>(); break;
				//case VariantType::VR_POPULATION:         (*this) = ref.get<Population>(); break;

			case VariantType::VR_STD_VECTOR_INT:               return vector_to_str<int>(value.get< std::vector<int> >()); break;
			case VariantType::VR_STD_VECTOR_FLOAT:             return vector_to_str<float>(value.get< std::vector<float> >()); break;
			case VariantType::VR_STD_VECTOR_DOUBLE:            return vector_to_str<double>(value.get< std::vector<double> >()); break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE:       return vector_to_str<long double>(value.get< std::vector<long double> >()); break;
			case VariantType::VR_STD_VECTOR_FLOAT_MATRIX:      return vector_to_str_matrix<MatrixF>(value.get< std::vector<MatrixF> >()); break;
			case VariantType::VR_STD_VECTOR_DOUBLE_MATRIX:     return vector_to_str_matrix<MatrixD>(value.get< std::vector<MatrixD> >()); break;
			case VariantType::VR_STD_VECTOR_LONG_DOUBLE_MATRIX:return vector_to_str_matrix<MatrixLD>(value.get< std::vector<MatrixLD> >()); break;

			case VariantType::VR_C_STRING:
			case VariantType::VR_STD_STRING:           return "\"" + value.get< std::string >() + "\""; break;
			case VariantType::VR_STD_VECTOR_STRING:    return vector_to_str_string(value.get< std::vector<std::string> >()); break;

			case VariantType::VR_PTR:				   return std::to_string(value.get<size_t>()); break;
			default: return ""; break;
			}
		}
		
	};
	REGISTERED_SERIALIZE_OUTPUT(CSVSerializeOutput, ".csv")
}