#include "Denn/Evaluation.h"
#include "Denn/Parameters.h"
#include "Denn/CostFunction.h"

namespace Denn
{
	class CrossEntropy : public Evaluation
	{
	public:
        //methods
        virtual bool minimize() const { return true; }
        virtual Scalar operator () (const NeuralNetwork& network, const DataSet& dataset)
        {
			const int n = dataset.features().cols();
			const Scalar eps = 1e-8;
			const Matrix& pred = network.feedforward(dataset.features());
			const Matrix& target = dataset.labels();
			Scalar loss = -(target.array().cwiseProduct((pred.array() + eps).log())).sum() / Scalar(n);
			return loss;
        }
    };
    REGISTERED_EVALUATION(CrossEntropy,"cross_entropy")

	class CrossEntropyLogistic : public Evaluation
	{
	public:
        //methods
        virtual bool minimize() const { return true; }
        virtual Scalar operator () (const NeuralNetwork& network, const DataSet& dataset)
        {
			Matrix output = network.feedforward(dataset.features());
			return CostFunction::cross_entropy_logistic_by_cols(
					  dataset.labels()
					, output
			).mean();
        }
    };
    REGISTERED_EVALUATION(CrossEntropyLogistic,"cross_entropy_logistic")

    
}