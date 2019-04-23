#include "Denn/Parameters.h"
#include "Denn/Utilities/String.h"
#include "Denn/Utilities/Networks.h"
namespace Denn
{
        //default int
    template < typename T >
    inline std::vector< T > json_jarray_to_vector(JsonArray& jarray)
    {
        std::vector< T > out; out.resize(jarray.size());
        for (size_t i = 0; i != jarray.size(); ++i) out[i] = (T)jarray[i].number();
        return out;
    }
    //others type
    template <>
    inline std::vector< std::string > json_jarray_to_vector< std::string >(JsonArray& jarray)
    {
        std::vector< std::string > out; out.resize(jarray.size());
        for (size_t i = 0; i != jarray.size(); ++i) out[i] = jarray[i].string();
        return out;
    }
    
    static inline bool conf_is_space(char c)
    {
        return c == ' ' || (c >= '\t' && c <= '\r');
    }    

    static bool conf_skip_line_space(const char*& source)
    {
        bool a_space_is_skipped = false;
        while (conf_is_space(*source) && *source != '\n')
        {
            //to true
            a_space_is_skipped = true;
            //jump
            ++source;
            //exit
            if (!*source) break;
        }
        return a_space_is_skipped;
    }

    //name
	static std::string conf_name(const char*& source)
	{
		//init
		std::string out;
		//first
		if (std::isalpha(*source) || *source == '_' || *source == '-' )
		{
			out += (*source);
			++source;
		}
		//parse name
		while ( std::isalnum(*source) || *source == '_' || *source == '-' || *source == '.')
		{
			out += (*source);
			++source;
		}
		//end
		return out;
	}

    static inline bool args_is_digit_or_point(char c)
    {
        return c == '.'  || (c >= '0' && c <= '9');
    }

    static JsonValue json_arg_magic_cast(const std::string& value)
    {
        auto vtrim = str_trim(value);
        if(vtrim.size())
        {
                 if(vtrim == "true")  return {true};
            else if(vtrim == "false") return {false};
            else if(args_is_digit_or_point(vtrim[0])) return {std::stod(vtrim.data())};
        }
        return {value};
    }

    static bool conf_parse_cline_args
    (
          JsonObject& arguments
        , int nargs
        , const char **vargs
    )
    {
        	enum State
			{
			  START
			, NAME
			, EQUAL
			, VALUE
			};
			//Parse state
			State state{ START };
			//values
			std::string name;
			std::string value;
			//parsing
			for (int i = 0; i < nargs; ++i)
			{
				//ptr
				const char* ptr = vargs[i];
				//parse
				while (*ptr)
				{
					switch (state)
					{
					case START:
						//begin
						conf_skip_line_space(ptr);
						name.clear();
						value.clear();
						state = NAME;
					break;
					case NAME:
						name = conf_name(ptr);
						if (!name.size())
						{
							std::cerr << "Name argument is not valid" << std::endl;
							return false;
						}
						state = EQUAL;
					break;
					case EQUAL:
						if (*ptr != '=')
						{
							std::cerr << "\'=\' is not found" << std::endl;
							return false;
						}
						++ptr;
						state = VALUE;
					break;
					case VALUE:
						while(*ptr) value += (*ptr++);
						if (!value.size())
						{
							std::cerr << "Value argument is not valid" << std::endl;
							return false;
						}
						state = START;
						//add value
                        arguments.at(name) = json_arg_magic_cast(value) ;
					break;
					default:
						return false;
					break;
					}
				}

			}
    }

    Parameters::ReturnType Parameters::from_json
    (
          const std::string& source
        , int nargs
        , const char **vargs
    )
    {
        //init json parse 
        Json json_input(source);
        //parse error
        if (json_input.errors().size())
        {
            std::cerr << "json\'s errors:" << std::endl;
            std::cerr << json_input.errors() << std::endl;
            return FAIL;
        }
        //test: is a object
        if (!json_input.document().is_object())
        {
            std::cerr << "json not valid" << std::endl;
            return FAIL;
        }
        //find arguments
        auto& jobject = json_input.document().object();
        auto jargs_it = jobject.find("arguments");
        if (jargs_it == jobject.end())
        {
            std::cerr << "arguments not found" << std::endl;
            return FAIL;
        }
        //test, arguments is a object
        auto& jargs_value = jargs_it->second;
        if (!jargs_value.is_object())
        {
            std::cerr << "arguments isn't a json object" << std::endl;
            return FAIL;
        }
        //ref to args
        auto& jargs = jargs_value.object();
        //overwrite 
        conf_parse_cline_args(jargs, nargs, vargs);
        //start
        for (auto& arg : jargs)
        {
            //read
            auto& key = arg.first;
            auto& value = arg.second;
            //set
            for (auto& info : m_params_info)
            {
                if (info.has_an_associated_variable()
                && (info.m_associated_variable->name() == key))
                {
                    //get
                    auto* associated_variable = (GenericReadOnly*)info.m_associated_variable;
                    auto type = associated_variable->type();
                    //set
                    switch (value.type())
                    {
                    case JsonValue::Type::IS_NULL:
                        break;
                    case JsonValue::Type::IS_BOOL:
                         if (type == static_variant_type<bool>())
                         {
                            associated_variable->set(value.boolean());
                         }
                         else
                         {
                             std::cerr << "the argument \'" << info.m_associated_variable->name() << "\' has an unsupported type value (bool)" << std::endl;
                             return FAIL;
                         }
                    case JsonValue::Type::IS_NUMBER:
                        switch (type)
                        {
                            case VariantType::VR_NONE: /**/ break;
                            case VariantType::VR_BOOL:         associated_variable->set<bool>(value.number()); break;
                            case VariantType::VR_CHAR:         associated_variable->set<char>(value.number()); break;
                            case VariantType::VR_SHORT:        associated_variable->set<short>(value.number()); break;
                            case VariantType::VR_INT:          associated_variable->set<int>(value.number()); break;
                            case VariantType::VR_LONG:         associated_variable->set<long>(value.number()); break;
                            case VariantType::VR_LONGLONG:     associated_variable->set<long long>(value.number()); break;
                            case VariantType::VR_UCHAR:        associated_variable->set<unsigned char>(value.number()); break;
                            case VariantType::VR_USHORT:       associated_variable->set<unsigned short>(value.number()); break;
                            case VariantType::VR_UINT:         associated_variable->set<unsigned int>(value.number()); break;
                            case VariantType::VR_ULONG:        associated_variable->set<unsigned long>(value.number()); break;
                            case VariantType::VR_ULONGLONG:    associated_variable->set<unsigned long long>(value.number()); break;
                            case VariantType::VR_FLOAT:        associated_variable->set<float>(value.number()); break;
                            case VariantType::VR_DOUBLE:       associated_variable->set<double>(value.number()); break;
                            case VariantType::VR_LONG_DOUBLE:  associated_variable->set<long double>(value.number()); break;
                            default:
                            {
                                std::cerr << "the argument \'" << info.m_associated_variable->name() << "\' has an unsupported type value (number)" << std::endl;
                                return FAIL;
                            }
                        }
                    break;
                    case JsonValue::Type::IS_STRING:
                        if (type == static_variant_type<std::string>())
                        {
                            associated_variable->set(value.string());
                        }
                        else if (type == static_variant_type<char>())
                        {
                            associated_variable->set(value.string()[0]);
                        }
                        else
                        {
                            std::cerr << "the argument \'" << info.m_associated_variable->name() << "\' has an unsupported type value (string)" << std::endl;
                            return FAIL;
                        }
                    break;
                    //Array int/string
                    case JsonValue::Type::IS_ARRAY:
                    {
                        auto& jarray = value.array();
                        switch (type)
                        {
                        case Denn::VR_STD_VECTOR_SHORT:
                            associated_variable->set(json_jarray_to_vector<short>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_INT:
                            associated_variable->set(json_jarray_to_vector<int>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_LONG:
                            associated_variable->set(json_jarray_to_vector<long>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_LONGLONG:
                            associated_variable->set(json_jarray_to_vector<long long>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_USHORT:
                            associated_variable->set(json_jarray_to_vector<unsigned short>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_UINT:
                            associated_variable->set(json_jarray_to_vector<unsigned int>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_ULONG:
                            associated_variable->set(json_jarray_to_vector<unsigned long>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_ULONGLONG:
                            associated_variable->set(json_jarray_to_vector<unsigned long long>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_FLOAT:
                            associated_variable->set(json_jarray_to_vector<float>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_DOUBLE:
                            associated_variable->set(json_jarray_to_vector<double>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_LONG_DOUBLE:
                            associated_variable->set(json_jarray_to_vector<long double>(jarray));
                            break;
                        case Denn::VR_STD_VECTOR_STRING:
                            associated_variable->set(json_jarray_to_vector<std::string>(jarray));
                            break;
                        default:
                        {
                            std::cerr << "the argument \'" << info.m_associated_variable->name() << "\' has an unsupported array value (array)" << std::endl;
                            return FAIL;
                        }
                        break;
                        }
                    }
                    break;
                    default:
                    {
                        std::cerr << "the argument \'" << info.m_associated_variable->name() << "\' has an unsupported value (object)" << std::endl;
                        return FAIL;
                    }
                    break;
                    }
                }
            }
        }
        //find network weights
        jargs_it = jobject.find("network");
        //test
        if (jargs_it != jobject.end() && jargs_it->second.is_array())
        {
            //parsing
            auto& jnn = jargs_it->second.array();
            //size test
            if(!jnn.size())
            {
                return SUCCESS; //void weights
            }
            //Array Matrix
            MatrixList network_weights;
            //for each layers
            for(auto& jlayer : jnn)
            for(auto& jmatrix : jlayer.array())
            {
                if(jmatrix.size())
                {
                    Matrix wdata(jmatrix.size(), jmatrix[0].size());
                    for(int row = 0; row!=wdata.rows() ; ++row)
                    for(int col = 0; col!=wdata.cols() ; ++col)
                    {
                        wdata(row,col) = jmatrix[row].array()[col].number();
                    }
                    network_weights.push_back(wdata);
                }
                else  network_weights.push_back(Matrix());
            }
            m_network_weights.set(network_weights);

        }
        return SUCCESS;
    }
}