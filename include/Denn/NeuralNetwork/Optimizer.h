#pragma once
#include "Denn/Config.h"
#include <unordered_map>

namespace Denn
{

class Optimizer
{
public:
	Optimizer(Random* random=nullptr) : m_random(random) {}
	//update
	virtual void update(AlignedMapColVector w, ConstAlignedMapColVector& dw) const= 0;
	/////////////////////////////////////////////////////////////////////////
	//Random engine
	Random*& random()       { return m_random; }
	Random*  random() const { return m_random; }
protected:
	mutable Random* m_random{nullptr};
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
	
	SGD(  
		Scalar lrate = Scalar(0.01)
	  , Scalar decay = Scalar(0)
	  , Scalar momentum = Scalar(0)
	  , bool nesterov = false
	  , Random* random = nullptr
	);

	virtual void update(AlignedMapColVector w, ConstAlignedMapColVector& dw)  const override;
};

}