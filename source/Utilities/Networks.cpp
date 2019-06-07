#include "Denn/Utilities/Networks.h"
#include "Denn/NeuralNetwork/Layer.h"

namespace Denn
{
	static bool is_valid_type_name(const std::string& type, const char* ptr)
	{
		return ((!type.size() && (std::isalpha(*ptr) || *ptr == '_'))	//start case
			||  (type.size() &&  (std::isalnum(*ptr) || *ptr == '_')));//all case

	}

	NNFromStringOut  get_network_from_string
	(
		const std::string& network_string,
		const int in_shape1D,
		const int out_shape1D
	)
	{
		//parser state
		enum State
		{
			S_READ_TYPE,
			S_READ_SHAPE,
			S_READ_INPUTS,
			S_FINALIZE
		};
		//source ptr
		const char* ptr = network_string.data();
		//name layer
		State state{ S_READ_TYPE };
		//net
		NeuralNetwork nn;
		std::string err;
		//layer info
		bool set_out_as_input = false;
		bool first = true;
		std::string type;
		std::vector<float> shape;
		std::vector<float> inputs;
		//temp layer input
		Shape in_shape;
		Inputs in_inputs;
		//ptr
		while (*ptr || state == S_FINALIZE)
		{
			//jump space
			while (std::isspace(*ptr)) ++ptr;
			//change parse state
			if (state == S_READ_TYPE)
			{
				while (is_valid_type_name(type, ptr))
				{
					type += *ptr++;
				}
			    //jump space
			    while (std::isspace(*ptr)) ++ptr;
				//test case: "type(ARGS)" or "type VOID"
				//all lower case
				std::transform(type.begin(), type.end(), type.begin(), ::tolower);
				//test
				if (!LayerFactory::exists(type))
				{
					err += type + " is an invalid layer name";
					return std::make_tuple(nn, err, false);
				}
				//activation not need inputs
				if (LayerFactory::description(type)->m_shape_type == SHAPE_ACTIVATION)
				{
				    if (*ptr=='[')
				    {
						++ptr; //jump [
						//activation not need a shape
				    	state = S_READ_INPUTS; 
						continue;
				    }
					//go to finalize
				    state = S_FINALIZE;
					continue;
				}
				else
				{
				    if (*ptr!='[')
				    {
				        err += type + " needs params";
				        return std::make_tuple(nn, err, false);
				    }
					++ptr; //jump [
				    state = S_READ_SHAPE;
					continue;
				}
			}
			else if (state == S_READ_SHAPE)
			{
				if (std::isdigit(*ptr))
				{
					//parse
					char *outptr = nullptr;
					shape.push_back(std::strtod(ptr, &outptr));
					if (ptr == outptr)
					{
						err += std::string(ptr-2, 5) + " is not valid shape";
						return std::make_tuple(nn, err, false);
					}
					//next
					ptr = outptr; continue;
				}
				else if (*ptr == ',')
				{
					state = S_READ_INPUTS;
					//next
					++ptr; continue;
				}
				else if (*ptr == ']')
				{
					//parser has read the inputs and not shapes
					inputs = shape;
					//void shape
					shape.clear();
					//go to end state
					state = S_FINALIZE;
					//next
					++ptr; continue;
				}
			}
			else if (state == S_READ_INPUTS)
			{
				if (std::isdigit(*ptr))
				{
					//parse
					char *outptr = nullptr;
					inputs.push_back(std::strtod(ptr, &outptr));
					if (ptr == outptr)
					{
						err += std::string(ptr - 2, 5) + " is not valid input";
						return std::make_tuple(nn, err, false);
					}
					//next
					ptr = outptr; continue;
				}
				else if (*ptr == ']')
				{
					state = S_FINALIZE;
					//next
					++ptr; continue;
				}
			}
			else if (state == S_FINALIZE)
			{
				//get if is the last
				while (std::isspace(*ptr)) ++ptr;
				//get layer info
				auto* description = LayerFactory::description(type);
				//test shape
				if (first)
				{
					switch (shape.size())
					{
					case 1: 
						if (!(description->shape_type() & SHAPE_1D))
						{
							err += "Shape for this layer must to be 1D";
							return std::make_tuple(nn, err, false);
						}
						in_shape = Shape(shape[0]);
					break;
					case 2:
						if (!(description->shape_type() & SHAPE_2D))
						{
							err += "Shape for this layer must to be 2D";
							return std::make_tuple(nn, err, false);
						}
						in_shape = Shape(shape[0],shape[1]);
					break;
					case 3:
						if (!(description->shape_type() & SHAPE_3D))
						{
							err += "Shape for this layer must to be 3D";
							return std::make_tuple(nn, err, false);
						}
						in_shape = Shape(shape[0], shape[1], shape[2]);
					break;
					//0 input, default
					case 0:
						if ((0 < in_shape1D) 
						&&  first 
						&&  (description->shape_type() & SHAPE_1D))
						{
							in_shape = Shape(in_shape1D);
							break;
						}
					default:
						err += "Shape must to be 1D/2D/3D";
						return std::make_tuple(nn, err, false);
					break;
					}
				}
				else
				{
					in_shape = nn[nn.size() - 1].out_size();
				}
				//except activation layer
				if(!(description->shape_type() & SHAPE_ACTIVATION))
				{
					if(set_out_as_input)
					{
						err += "Only last layer can omit output info";
						return std::make_tuple(nn, err, false);
					}
					else if(inputs.size() <= 0) //last layer can omitted output
					{
						inputs.push_back(out_shape1D);
						set_out_as_input = true;
					}
				}
				//test inputs
				if (inputs.size() < description->inputs().min()
				||  inputs.size() > description->inputs().max())
				{
					err += "Inputs for this layer are not invalid"
						   " (min: " + std::to_string(description->inputs().min()) + 
						   ", max: " + std::to_string(description->inputs().max()) + ")";
					return std::make_tuple(nn, err, false);
				}
				in_inputs = inputs;
				//new layer
				auto layer = LayerFactory::create(type, in_shape, in_inputs);
				//add
				nn.add_layer(layer);
				//ok
				first = false;
				type.clear();
				inputs.clear();
				shape.clear();
				//in clear
				in_shape = Shape();
				in_inputs.clear();
				//restart
				state = S_READ_TYPE;
			}
		}
		//ok
		return std::make_tuple(nn, err, true);
	}


	TestNNStringOut  get_network_from_string_test(const std::string& network_string)
	{
		auto value = get_network_from_string(network_string, 1, 1);
		return { std::get<1>(value), std::get<2>(value) };
	}

	std::string  get_string_from_network
	(
		const NeuralNetwork& nn
	)
	{
		//out string
		std::string out;
		//for each layers
		for (size_t i = 0; i < nn.size(); ++i)
		{
			const Layer& layer = nn[i];
			//type shape
			std::string type = layer.name();
			std::string shape;
			std::string inputs;
			//get layer info
			auto* description = LayerFactory::description(type);
			//shape
			if (!i)
			{
				if (description->shape_type() & SHAPE_3D)
					shape =
					std::to_string(layer.in_size().width()) + " " +
					std::to_string(layer.in_size().height()) + " " +
					std::to_string(layer.in_size().channels());
				else if (description->shape_type() & SHAPE_2D)
					shape =
					std::to_string(layer.in_size().width()) + " " +
					std::to_string(layer.in_size().height());
				else
					shape = std::to_string(layer.in_size().width());

			}
			//inputs
			for (int in : layer.inputs()) inputs += " " + std::to_string(in);
			// activation case
			if (LayerFactory::description(type)->m_shape_type == SHAPE_ACTIVATION)
			{
				if(inputs.size())
					out += type + "[" + inputs + "]";
				else
					out += type;
			}
			// default case
			else 
			{
				out += (i ? " " : "") + type + "[" + shape + "," + inputs + "]";
			}
			//endline
			out += "    ";
		}
		return out;
	}

	//nn to json
	JsonObject nn_to_json_object(const NeuralNetwork& nn)
	{
		JsonObject jout;
		if(nn.size())
		{
			//input layer is a FC?
			if(nn[0].name() == "fully_connected")
				jout["input"]  = nn[0].in_size().width();
			//search the last fully_connected layer 
			//TODO test the intermedie layers (have to be ACTIVATION type)
			for(int i = nn.size()-1; i > 0; --i)
				if(nn[i].name() == "fully_connected")
					jout["output"] = nn[nn.size()-1].out_size().width();
			//weights
			jout["layout"] = get_string_from_network(nn);
			jout["weights"] = nn_weights_to_json_array(nn);
		}
		//return
		return jout;
	}

	//nn from json
	NeuralNetwork nn_from_json_object(const JsonObject& jnn)
	{
		NeuralNetwork nnout;
		//test
		int in1D = 1;
		int out1D = 1;
		//test
		auto input_it = jnn.find("input");
		auto out_it = jnn.find("ouput");
		//iterator test
		if( input_it != jnn.end() && input_it->second.is_number())
			in1D = input_it->second.number();
		if( out_it != jnn.end() && out_it->second.is_number())
			out1D = out_it->second.number();
		//its
		auto layout_it = jnn.find("layout");
		auto weights_it = jnn.find("weights");
		//test
		if( layout_it == jnn.end() || !layout_it->second.is_string())
			return nnout;
		//test
		if( weights_it == jnn.end() || !weights_it->second.is_array())
			return nnout;
		//parsing
		auto status_nn = get_network_from_string(layout_it->second.string(), in1D, out1D);
		//test
		if(!std::get<2>(status_nn))
			return nnout;
		else 
			nnout = std::get<0>(status_nn);
		//weights
		if(!nn_weights_from_json_object(nnout, weights_it->second.array()))
			return nnout; //return void NN?
		//ok
		return nnout;
	}

	//nn weights to json
	JsonArray nn_weights_to_json_array(const NeuralNetwork& nn)
	{
		//save w layer
		JsonArray wl;
		//serialize net
		for (size_t i = 0; i != nn.size(); ++i)
		{
			JsonArray wm;
			for (size_t m = 0; m != nn[i].size(); ++m)
				wm.push_back(json_array_from_matrix(nn[i][m]));
			wl.push_back(wm);
		}
		return wl;
	}

	//nn weights from json
	bool nn_weights_from_json_object(NeuralNetwork& nn, const JsonArray& w)
	{
		if(nn.size() !=  w.size()) return false;
		//deserialize net
		for (size_t i = 0; i != nn.size(); ++i)
		{
			//test
			if(!w[i].is_array() || nn[i].size() !=  w[i].array().size())
			 	return false;
			//deserialize
			for (size_t m = 0; m != nn[i].size(); ++m)
			{
				nn[i][m] = matrix_from_json_array(w[i].array());
			}
		}
		return true;
	}
	
	//to json
	JsonObject to_json_objects(const NeuralNetwork& nn)
	{
		JsonObject jout;
		//get layout
		jout["layout"] = get_string_from_network(nn);
		//save w layer
		JsonArray wl;
		//serialize net
		for (size_t i = 0; i != nn.size(); ++i)
		{
			JsonArray wm;
			for (size_t m = 0; m != nn[i].size(); ++m)
				wm.push_back(json_array_from_matrix(nn[i][m]));
			wl.push_back(wm);
		}
		jout["weights"] = wl;
		return jout;
	}
	//from json
	NeuralNetwork from_json(const JsonObject& nn);
}