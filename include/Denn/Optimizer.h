#pragma once
#include "Config.h"
#include <unordered_map>

namespace Denn
{

class Optimizer
{
public:
	virtual void update(AlignedMapColVector w, ConstAlignedMapColVector& dw) const= 0;
};

class SGD : public Optimizer
{
protected:

	Scalar m_lrate;
	Scalar m_decay;
	Scalar m_momentum;  
	bool   m_nesterov; 
	// velocity
	mutable std::unordered_map<const Scalar*, ColVector> m_v_map;

public:
	
	SGD(  Scalar lrate = Scalar(0.01)
		, Scalar decay = Scalar(0)
		, Scalar momentum = Scalar(0)
		, bool nesterov = false);

	virtual void update(AlignedMapColVector w, ConstAlignedMapColVector& dw)  const override;
};

}