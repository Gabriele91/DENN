#include "Denn/Evaluation.h"
#include "Denn/Parameters.h"
#include "Denn/CostFunction.h"

namespace Denn
{
	class Accuracy : public Evaluation
	{
	public:
        //methods
        virtual bool minimize() const { return false; }
        virtual Scalar operator () (const NeuralNetwork& network, const DataSet& dataset)
        {
			const Matrix& x = network.feedforward(dataset.features());
            return CostFunction::accuracy_cols(x,dataset.labels());
        }
		
    };
    REGISTERED_EVALUATION(Accuracy,"accuracy")
    
	class InverseAccuracy : public Evaluation
	{
	public:
        //methods
        virtual bool minimize() const { return true; }
        virtual Scalar operator () (const NeuralNetwork& network, const DataSet& dataset)
        {
			const Matrix& x = network.feedforward(dataset.features());
            return CostFunction::inverse_accuracy_cols(x,dataset.labels());
        }
		
    };
	REGISTERED_EVALUATION(InverseAccuracy,"inverse_accuracy")
}