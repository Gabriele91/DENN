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
	/////////////////////////////////////////////////////////////////////////
	//add a layer
	void add_layer(const Layer::SPtr& layer);
	//template for each kind of layer
	template < typename DerivateLayer >
	void add_layer(const DerivateLayer& layer)
	{
		add_layer(std::static_pointer_cast<Layer>(std::make_shared<DerivateLayer>(layer)));
	}
	//template for variant args of layers add into network
	template < typename ...Layers >
	void add_layer(const Layer& layer, Layers ...layers)
	{
		add_layer(layer);
		add_layer(layers...);
	}
	/////////////////////////////////////////////////////////////////////////
	const Matrix& predict(const Matrix& input) const;
	const Matrix& feedforward(const Matrix&, Random* random = nullptr) const;
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
	//no 0 values
	void no_0_weights();
	/////////////////////////////////////////////////////////////////////////
	size_t size() const;

	Layer& operator [] (size_t i);

	const Layer& operator [] (size_t i) const;

	LayerIterator begin();

	LayerIterator end();

	LayerConstIterator begin() const;

	LayerConstIterator end() const;
	/////////////////////////////////////////////////////////////////////////
	//Random engine
	Random*& random()       { return m_random; }
	Random*  random() const { return m_random; }
	/////////////////////////////////////////////////////////////////////////
	NeuralNetwork&  operator += (NeuralNetwork& right);
	NeuralNetwork&  operator -= (NeuralNetwork& right);
	NeuralNetwork&  operator *= (NeuralNetwork& right);
	NeuralNetwork&  operator /= (NeuralNetwork& right);
	
protected:
	//layer list
	LayerList m_layers;
	//ref to random engine
	mutable Random* m_random{nullptr};
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

//math operations
extern NeuralNetwork operator + (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator - (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator * (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator / (NeuralNetwork& left, NeuralNetwork& right);

//logic operatos
extern NeuralNetwork operator < (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator > (NeuralNetwork& left, NeuralNetwork& right); 
extern NeuralNetwork operator == (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator <= (NeuralNetwork& left, NeuralNetwork& right);
extern NeuralNetwork operator >= (NeuralNetwork& left, NeuralNetwork& right);


}
