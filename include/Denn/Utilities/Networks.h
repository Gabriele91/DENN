#pragma once
#include "Denn/Core/Json.h"
#include "Denn/NeuralNetwork/NeuralNetwork.h"

namespace Denn
{
	//output get_network_from_string -> NN & errors
	using NNFromStringOut = std::tuple< NeuralNetwork, std::string, bool >;
	using TestNNStringOut = std::tuple< std::string, bool >;
	//build a network from string
	NNFromStringOut  get_network_from_string
	(
		const std::string& network_string,
		const int intpus = -1,
		const int output = -1
	);
	//test a network from string
	TestNNStringOut  get_network_from_string_test(const std::string& network_string);
	//build a string from network
	std::string  get_string_from_network
	(
		const NeuralNetwork& nn
	);
	//nn struct to json
	JsonObject nn_struct_to_json_object(const NeuralNetwork& nn);
	//nn to json
	JsonObject nn_to_json_object(const NeuralNetwork& nn);
	//nn from json
	NeuralNetwork nn_from_json_object(const JsonObject& nn);
	//nn weights to json
	JsonArray nn_weights_to_json_array(const NeuralNetwork& nn);
	//nn weights from json
	bool nn_weights_from_json_object(NeuralNetwork& nn, const JsonArray& w);
}