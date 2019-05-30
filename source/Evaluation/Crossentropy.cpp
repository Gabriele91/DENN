#include "Denn/Evaluation.h"
#include "Denn/Parameters.h"
#include "Denn/DataSet/DataSet.h"
#include "Denn//NeuralNetwork/NeuralNetwork.h"

namespace Denn
{
	class CrossEntropy : public DefaultEvaluation
	{
	public:
        //methods
        virtual bool minimize() const { return true; }
        virtual Scalar operator () (const Matrix& pred, const DataSet& dataset)
        {
			const int n = dataset.features().cols();
			const Scalar eps = SCALAR_EPS;
			const Matrix& target = dataset.labels();
			Scalar loss = -(target.array().cwiseProduct((pred.array() + eps).log())).sum() / Scalar(n);
			return loss;
        }
    };
    REGISTERED_EVALUATION(CrossEntropy,"cross_entropy")

	class BinaryCrossEntropy : public DefaultEvaluation
	{
	public:
        //methods
        virtual bool minimize() const { return true; }
        virtual Scalar operator () (const Matrix& pred, const DataSet& dataset)
        {
			const int n = dataset.features().cols();
			const Matrix& target = dataset.labels();
			auto pred_clamp = pred.array().unaryExpr([](Scalar y)->Scalar{ 
				const Scalar eps = SCALAR_EPS;
				return clamp<Scalar>(y,eps,1 - eps); 
			});
			return  -(target.array() * pred_clamp.log() + (1 - target.array()) * (1 - pred_clamp).log()).sum() / Scalar(n);
        }
    };
    REGISTERED_EVALUATION(BinaryCrossEntropy,"binary_cross_entropy")
    
}