#include "Denn/Core/Dump.h"
#include "Denn/Evaluation.h"
#include "Denn/Parameters.h"
#include <iostream>

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
		#if 1
        {
			const int n = dataset.features().cols();
			const Matrix& target = dataset.labels();
			auto pred_clamp = pred.array().unaryExpr([](Scalar y)->Scalar{ 
				const Scalar eps = SCALAR_EPS;
				return clamp<Scalar>(y,eps,1 - eps); 
			});
			return  -(target.array() * pred_clamp.log() + (1 - target.array()) * (1 - pred_clamp).log()).sum() / Scalar(n);
        }
		#elif 0
        {
			const Scalar eps = SCALAR_EPS;
			const int n = dataset.features().cols();
			const Matrix& target = dataset.labels();
			return  -(target.array() * (pred.array() + eps).log() +
					  (1-target.array()) *  (1 - pred.array() + eps).log()).sum() / Scalar(n);
        }
		#else
        {
			const int n = dataset.features().cols();
			const Scalar eps = SCALAR_EPS;
			const Matrix& target = dataset.labels();
			std::cout << "- - - - - - - - - - - - - - - - - " << std::endl;
			std::cout << Denn::Dump::json_matrix(pred) << std::endl;
			std::cout << Denn::Dump::json_matrix(target) << std::endl;
			std::cout << "- - - - - - - - - - - - - - - - - " << std::endl;
			Scalar loss = -(target.array().cwiseProduct((pred.array() + eps).log())).sum() / Scalar(n);
			return loss;
        }
		#endif
    };
    REGISTERED_EVALUATION(BinaryCrossEntropy,"binary_cross_entropy")

    
}