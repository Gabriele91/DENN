#include "Denn/Layer/Activations.h"
#include "Denn/Utilities/Math.h"

namespace Denn
{
    
    //////////////////////////////////////////////////////////////////////////////////////////////////    
    Sigmoid::Sigmoid(const Shape& shape, const Inputs& inputs) : ActivationLayer("sigmoid", shape, shape) {}
    Layer::SPtr Sigmoid::copy() const
    {
        return std::static_pointer_cast<Layer>(std::make_shared<Sigmoid>(*this));
    }
    const Matrix& Sigmoid::feedforward(const Matrix& bottom) 
    {
        m_top.resize(bottom.rows(),bottom.cols());
        m_top.noalias() = (Scalar(1.0) / (Scalar(1.0) + (-bottom).array().exp())).matrix();
        //return feed
        return m_top;
    }
    const Matrix& Sigmoid::backpropagate(const Matrix& bottom, const Matrix& grad) 
    {
        CODE_BACKPROPAGATION(
            // d(L)/d(z_i) = d(L)/d(a_i) * d(a_i)/d(z_i)
            // d(a_i)/d(z_i) = a_i * (1-a_i)
            Matrix da_dz = m_top.array().cwiseProduct(Scalar(1.0) - m_top.array());
            m_grad_bottom = grad.cwiseProduct(da_dz);
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////    
    ReLU::ReLU(const Shape& shape, const Inputs& inputs) : ActivationLayer("relu", shape, shape) {}
    Layer::SPtr ReLU::copy() const
    {
        return std::static_pointer_cast<Layer>(std::make_shared<ReLU>(*this));
    }
    const Matrix& ReLU::feedforward(const Matrix& bottom)
    {  
        // a = z*(z>0)
        m_top.resize(bottom.rows(),bottom.cols());
        m_top.noalias() = bottom.cwiseMax(0.0);
        //return feed
        return m_top;
    }
    const Matrix& ReLU::backpropagate(const Matrix& bottom, const Matrix& grad)
    {
        CODE_BACKPROPAGATION(
            Matrix positive = (bottom.array() > 0.0).cast<Scalar>();
            m_grad_bottom = grad.cwiseProduct(positive);
        
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////    
    LeakyReLU::LeakyReLU(const Shape& shape, const Inputs& inputs) : ActivationLayer("leaky_relu", shape, shape)
    {
        m_alpha = inputs.size() ? inputs[0] : 0.01;
    }
    Layer::SPtr LeakyReLU::copy() const
    {
        return std::static_pointer_cast<Layer>(std::make_shared<LeakyReLU>(*this));
    }
    const Matrix& LeakyReLU::feedforward(const Matrix& bottom)
    {  
        // a = max(0,x) − αmax(0,−x)
        m_top.resize(bottom.rows(),bottom.cols());
        m_top.noalias() = bottom.unaryExpr([this](Scalar x) -> Scalar {
            return Activation::leaky_relu(x,m_alpha);
        });
        //return feed
        return m_top;
    }
    const Matrix& LeakyReLU::backpropagate(const Matrix& bottom, const Matrix& grad)
    {
        CODE_BACKPROPAGATION(
            Matrix derivate = bottom.array().unaryExpr([this](Scalar x) -> Scalar {
                return Activation::leaky_relu_derivative(x,m_alpha);
            });
            m_grad_bottom = grad.cwiseProduct(derivate);
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////    
    Softmax::Softmax(const Shape& shape, const Inputs& inputs) : ActivationLayer("softmax", shape, shape) {}
    Layer::SPtr Softmax::copy() const
    {
        return std::static_pointer_cast<Layer>(std::make_shared<Softmax>(*this));
    }
    const Matrix& Softmax::feedforward(const Matrix& bottom)
    { 
        // a = exp(z) / \sum{ exp(z) }
        m_top = (bottom.rowwise() - bottom.colwise().maxCoeff()).array().exp();
        RowArray z_exp_sum = m_top.colwise().sum();  // \sum{ exp(z) }
        m_top.array().rowwise() /= z_exp_sum;
        //return feed
        return m_top;
    }
    const Matrix& Softmax::backpropagate(const Matrix& bottom, const Matrix& grad)
    {
        CODE_BACKPROPAGATION(
            // d(L)/d(z_i) = \sum{ d(L)/d(a_j) * d(a_j)/d(z_i) }
            // = \sum_(i!=j){ d(L)/d(a_j) * d(a_j)/d(z_i) } + d(L)/d(a_i) * d(a_i)/d(z_i)
            // = a_i * ( d(L)/d(a_i) - \sum{a_j * d(L)/d(a_j)} )
            RowArray temp_sum = m_top.cwiseProduct(grad).colwise().sum();
            m_grad_bottom = m_top.array().cwiseProduct(grad.array().rowwise() - temp_sum);
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////
}