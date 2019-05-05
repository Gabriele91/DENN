#include "Denn/NeuralNetwork/Optimizer.h"

namespace Denn
{

	SGD::SGD
	(
	  Scalar lrate 
	, Scalar decay 
	, Scalar momentum 
	, bool nesterov
	, Random* random
	)
	: Optimizer(random)
	, m_lrate(lrate)
	, m_decay(decay)
	, m_momentum(momentum)
	, m_nesterov(nesterov)
	{
	}

	void SGD::update(AlignedMapColVector w, ConstAlignedMapColVector& dw)  const
	{
		//old gradient vector
		ColVector& v = m_v_map[dw.data()];
		//size
		if (v.size() == 0)
		{
			v.resize(dw.size());
			v.setZero();
		}
		// update v
		v = m_momentum * v + (dw + m_decay * w);
		// update w
		if (m_nesterov)
			w -= m_lrate * (m_momentum * v + (dw + m_decay * w));
		else
			w -= m_lrate * v;
	}
}