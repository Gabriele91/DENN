#pragma once
#include "Denn/NeuralNetwork.h"

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
}