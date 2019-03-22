#include "Denn/NeuralNetwork.h"

namespace Denn
{	
	///////////////////////////////////////////////////////////////////////////
	//return ptr
	NeuralNetwork::SPtr NeuralNetwork::get_ptr()
	{
		return this->shared_from_this();
	}
	NeuralNetwork::SPtr NeuralNetwork::copy() const
	{
		auto copy = std::make_shared<NeuralNetwork>();		
		for (size_t i = 0; i != size(); ++i)
		{
			copy->m_layers.push_back((*this)[i].copy());
		}
		return copy;
	}
	////////////////////////////////////////////////////////////////
	//  default constructor 
	NeuralNetwork::NeuralNetwork()
	{
	}
	//  default copy constructor  and assignment operator
	NeuralNetwork::NeuralNetwork(const NeuralNetwork& nn)
	{
		//alloc
		m_layers.clear();
		//copy all layers
		for (size_t i = 0; i != nn.size(); ++i)
		{
			m_layers.push_back(nn[i].copy()->get_ptr());
		}
	}
	NeuralNetwork& NeuralNetwork::operator= (const NeuralNetwork & nn)
	{
		//alloc
		m_layers.clear();
		//copy all layers
		for (size_t i = 0; i != nn.size(); ++i)
		{
			m_layers.push_back(nn[i].copy()->get_ptr());
		}
		//self return
		return *this;
	}
	/////////////////////////////////////////////////////////////////////////
	const Matrix& NeuralNetwork::feedforward(const Matrix& input) const
	{
		//no layer?
		denn_assert(m_layers.size());
		//input layer
		m_layers[0]->feedforward(input);
		//hidden layers
		for (size_t i = 1; i < size(); ++i)
		{
			m_layers[i]->feedforward(m_layers[i-1]->ff_output());
		}
		//return
		return m_layers[size()-1]->ff_output();
	}	
	void NeuralNetwork::backpropagate(const Matrix& input, const Matrix& target, OutputLoss oltype)
	{
		//ptrs
		Layer::SPtr first_layer = m_layers[0];
		Layer::SPtr last_layer = m_layers[size() - 1];

		// Let output layer compute back-propagation data
		// MSE
		Matrix eval;
		switch (oltype)
		{
		default:
		case Denn::NeuralNetwork::MSE:
		{
			const Matrix& pred = last_layer->ff_output();
			const int nobs = pred.cols();
			const int nclass = pred.rows();
			eval.resize(nclass, nobs);
			eval = ((pred.array() - target.array()) * Scalar(2)) / Scalar(nobs);
		}
		break;
		case Denn::NeuralNetwork::MULTICLASS_CROSS_ENTROPY:
		{
			// Compute the derivative of the input of this layer
			Scalar eps = 1e-8;
			const Matrix& pred = last_layer->ff_output();
			const int nobs = pred.cols();
			const int nclass = pred.rows();
			// backward: d(L)/d(p_i) = -y_i/p_i/n
			eval.resize(nclass, nobs);
			eval = -target.array().cwiseQuotient(pred.array() + eps) / nobs;
		}
		break;
		case Denn::NeuralNetwork::BINARY_CROSS_ENTROPY:
		{
			// Compute the derivative of the input of this layer
			// L = -y * log(phat) - (1 - y) * log(1 - phat)
			// in = phat
			// d(L) / d(in)= -y / phat + (1 - y) / (1 - phat), y is either 0 or 1
			const Matrix& pred = last_layer->ff_output();
			const int nobs = pred.cols();
			const int nclass = pred.rows();
			eval.resize(nclass, nobs);
			eval = (target.array() < Scalar(0.5)).select((Scalar(1) - pred.array()).cwiseInverse(), -pred.cwiseInverse());
		}
		break;
		}

		// If there is only one hidden layer, "prev_layer_data" will be the input data
		if (size() == 1)
		{
			first_layer->backpropagate(input, eval);
			return;
		}

		// Compute gradients for the last hidden layer
		last_layer->backpropagate(m_layers[size() - 2]->ff_output(), eval);

		// Compute gradients for all the hidden layers except for the first one and the last one
		for (int i = size() - 2; i > 0; i--)
		{
			m_layers[i]->backpropagate(m_layers[i - 1]->ff_output(), m_layers[i + 1]->bp_output());
		}

		// Compute gradients for the first layer
		first_layer->backpropagate(input, m_layers[1]->bp_output());
	}
	void NeuralNetwork::fit(const Matrix& input, const Matrix& output, 
							const Optimizer& opt, OutputLoss type)
	{
		//->
		feedforward(input);
		//<-
		backpropagate(input, output, type);
		//update
		for (size_t i = 0; i < size(); ++i)
			m_layers[i]->update(opt);
	}
	/////////////////////////////////////////////////////////////////////////
	//no 0 
	void NeuralNetwork::no_0_weights()
	{
		for (auto& layer  : *this)
		for (auto  matrix : *layer)
		{
			matrix.noalias() = matrix.unaryExpr([](Scalar weight) -> Scalar
			{ 			
				const Scalar eps = SCALAR_EPS;
				while(std::abs(weight) <= eps) weight += std::copysign(eps, weight);
				return weight;
			});
		}
	}
	/////////////////////////////////////////////////////////////////////////
	size_t NeuralNetwork::size_of_all_layers() const
	{
		size_t sum = 0;
		for(size_t l = 0; l != size(); ++l) sum += (*this)[l].size();
		return sum;
	}

	size_t NeuralNetwork::size() const
	{
		return m_layers.size();
	}

	Layer& NeuralNetwork::operator [] (size_t i)
	{
		return *(m_layers[i].get());
	}

	const Layer& NeuralNetwork::operator [] (size_t i) const
	{
		return *(m_layers[i].get());
	}

	NeuralNetwork::LayerIterator NeuralNetwork::begin()
	{
		return m_layers.begin();
	}

	NeuralNetwork::LayerIterator NeuralNetwork::end()
	{
		return m_layers.end();
	}

	NeuralNetwork::LayerConstIterator NeuralNetwork::begin() const
	{
		return m_layers.begin();
	}

	NeuralNetwork::LayerConstIterator NeuralNetwork::end() const
	{
		return m_layers.end();
	}
}
