#include "Denn/NeuralNetwork.h"

namespace Denn
{	
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
			add_layer(nn[i].copy()->get_ptr());
		}
	}
	NeuralNetwork& NeuralNetwork::operator= (const NeuralNetwork & nn)
	{
		//alloc
		m_layers.clear();
		//copy all layers
		for (size_t i = 0; i != nn.size(); ++i)
		{
			add_layer(nn[i].copy()->get_ptr());
		}
		//self return
		return *this;
	}	
	/////////////////////////////////////////////////////////////////////////
	void NeuralNetwork::add_layer(const Layer::SPtr& layer)
	{
		m_layers.push_back(layer->copy());
		m_layers.back()->network() = this;
	}
	/////////////////////////////////////////////////////////////////////////
	const Matrix& NeuralNetwork::predict(const Matrix& input) const
	{
		//no layer?
		denn_assert(m_layers.size());
		//input layer
		m_layers[0]->predict(input);
		//hidden layers
		for (size_t i = 1; i < size(); ++i)
		{
			m_layers[i]->predict(m_layers[i-1]->ff_output());
		}
		//return
		return m_layers[size()-1]->ff_output();
	}	
	const Matrix& NeuralNetwork::feedforward(const Matrix& input, Random* random) const
	{
		//no layer?
		denn_assert(m_layers.size());
		//set random engine (dropout)
		m_random = random;
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
		feedforward(input, opt.random());
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

	//self operatos
	NeuralNetwork& NeuralNetwork::operator += (NeuralNetwork& right)
	{
		for(size_t l=0;l < size(); ++l)
		for(size_t m=0;m < m_layers[l]->size(); ++m)
		{
			(*this)[l][m] += right[l][m];
		}
		return (*this);
	}
	NeuralNetwork& NeuralNetwork::operator -= (NeuralNetwork& right)
	{
		for(size_t l=0;l < size(); ++l)
		for(size_t m=0;m < m_layers[l]->size(); ++m)
		{
			(*this)[l][m] -= right[l][m];
		}
		return (*this);
	}
	NeuralNetwork& NeuralNetwork::operator *= (NeuralNetwork& right)
	{
		for(size_t l=0;l < size(); ++l)
		for(size_t m=0;m < m_layers[l]->size(); ++m)
		{
			(*this)[l][m] *= right[l][m];
		}
		return (*this);
	}
	NeuralNetwork& NeuralNetwork::operator /= (NeuralNetwork& right)
	{
		for(size_t l=0;l < size(); ++l)
		for(size_t m=0;m < m_layers[l]->size(); ++m)
		{
			(*this)[l][m] *= right[l][m];
		}
		return (*this);
	}

	//operatos
	NeuralNetwork operator + (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		tmp += right;
		return tmp;
	}

	NeuralNetwork operator - (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		tmp -= right;
		return tmp;
	}

	NeuralNetwork operator * (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		tmp *= right;
		return tmp;
	}

	NeuralNetwork operator / (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		tmp /= right;
		return tmp;
	}

	//logic operatos
	NeuralNetwork operator < (NeuralNetwork& left, NeuralNetwork& right) 
	{
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			auto r_array = right[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) = Scalar(l_array(a) < r_array(a));
		}
		return tmp;
	}

	NeuralNetwork operator > (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			auto r_array = right[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) = Scalar(l_array(a) > r_array(a));
		}
		return tmp;
	}

	NeuralNetwork operator == (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			auto r_array = right[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) == r_array(a));
		}
		return tmp;
	}

	NeuralNetwork operator <= (NeuralNetwork& left, NeuralNetwork& right) 
	{
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			auto r_array = right[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) <= r_array(a));
		}
		return tmp;
	}

	NeuralNetwork operator >= (NeuralNetwork& left, NeuralNetwork& right) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			auto r_array = right[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) >= r_array(a));
		}
		return tmp;
	}

	//logics vs float
	
	NeuralNetwork operator < (NeuralNetwork& left, Scalar value) 
	{
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) = Scalar(l_array(a) < value);
		}
		return tmp;
	}

	NeuralNetwork operator > (NeuralNetwork& left, Scalar value) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) = Scalar(l_array(a) > value);
		}
		return tmp;
	}

	NeuralNetwork operator == (NeuralNetwork& left, Scalar value) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) == value);
		}
		return tmp;
	}

	NeuralNetwork operator <= (NeuralNetwork& left, Scalar value) 
	{
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) <= value);
		}
		return tmp;
	}

	NeuralNetwork operator >= (NeuralNetwork& left, Scalar value) 
	{ 
		NeuralNetwork tmp(left);  
		for(size_t l=0;l < tmp.size(); ++l)
		for(size_t m=0;m < tmp[l].size(); ++m)
		{
			auto t_array = tmp[l][m].array();
			auto l_array = left[l][m].array();
			for(size_t a=0; a < t_array.size(); ++a)
				t_array(a) =  Scalar(l_array(a) >= value);
		}
		return tmp;
	}
}
