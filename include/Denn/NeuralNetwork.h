#pragma once
#include "Config.h"
#include "Layer.h"

namespace Denn
{
class NeuralNetwork
{
public:
	////////////////////////////////////////////////////////////////
	using Scalar			 = Denn::Scalar;
	using LayerList			 = std::vector < Layer::SPtr >;
	using LayerIterator		 = typename LayerList::iterator;
	using LayerConstIterator = typename LayerList::const_iterator;
	////////////////////////////////////////////////////////////////
	enum OutputLoss
	{
		MSE,
		MULTICLASS_CROSS_ENTROPY,
		BINARY_CROSS_ENTROPY
	};
	////////////////////////////////////////////////////////////////
	//  default constructor 
	NeuralNetwork();
	//  default copy constructor  and assignment operator
	NeuralNetwork(const NeuralNetwork& nn);
	NeuralNetwork& operator= (const NeuralNetwork & nn);
	////////////////////////////////////////////////////////////////
	// add layers
	template < class ...Layers >
	NeuralNetwork(Layers ...layers)
	{
		add_layer(layers...);
	}
	template < typename ...Layers >
	void add_layer(const Layer& layer, Layers ...layers)
	{
		add_layer(layer);
		add_layer(layers...);
	}
	/////////////////////////////////////////////////////////////////////////
	void add_layer(const Layer::SPtr& layer)
	{
		m_layers.push_back(layer->copy());
	}
	template < typename DerivateLayer >
	void add_layer(const DerivateLayer& layer)
	{
		m_layers.push_back(std::static_pointer_cast<Layer>(std::make_shared<DerivateLayer>(layer)));
	}
	/////////////////////////////////////////////////////////////////////////
	const Matrix& feedforward(const Matrix& input) const;
	void backpropagate
	(
		const Matrix& input, 
		const Matrix& output,
		OutputLoss type = MULTICLASS_CROSS_ENTROPY
	);
	void fit
	(
		const Matrix& input, 
		const Matrix& output,
		const Optimizer& opt = SGD(),
		OutputLoss type = MULTICLASS_CROSS_ENTROPY
	);
	/////////////////////////////////////////////////////////////////////////
	size_t size() const;

	Layer& operator [] (size_t i);

	const Layer& operator [] (size_t i) const;

	LayerIterator begin();

	LayerIterator end();

	LayerConstIterator begin() const;

	LayerConstIterator end() const;
	/////////////////////////////////////////////////////////////////////////
protected:

	LayerList m_layers;

};

template <>
inline NeuralNetwork::Scalar distance_pow2<const NeuralNetwork>(const NeuralNetwork& a, const NeuralNetwork& b)
{
	//bad case
	if(a.size()!=b.size()) return std::numeric_limits<Scalar>::infinity();
	//value
	Scalar dpow2 = 0.0;
	//sum
	for(size_t i = 0; i!=a.size() ; ++i) dpow2 += distance_pow2(a[i],b[i]);
	//return 
	return dpow2;
} 


}
