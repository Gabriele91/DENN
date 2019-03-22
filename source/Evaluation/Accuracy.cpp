#include "Denn/Evaluation.h"
#include "Denn/Parameters.h"
#include "Denn/NeuralNetwork.h"
#include "Denn/DataSet/DataSet.h"

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
            const Matrix& y = dataset.labels();
            //output
            Scalar output = Scalar(0.0);
            //values
            Matrix::Index  max_index_x, max_index_y;
            //max-max
            for (Matrix::Index j = 0; j < x.cols(); ++j)
            {
                x.col(j).maxCoeff(&max_index_x);
                y.col(j).maxCoeff(&max_index_y);
                output += Scalar(max_index_x == max_index_y);
            }
            //
            return output / Scalar(x.cols());
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
            const Matrix& y = dataset.labels();
            //output
            Scalar output = Scalar(0.0);
            //values
            Matrix::Index  max_index_x, max_index_y;
            //max-max
            for (Matrix::Index j = 0; j < x.cols(); ++j)
            {
                x.col(j).maxCoeff(&max_index_x);
                y.col(j).maxCoeff(&max_index_y);
                output += Scalar(max_index_x == max_index_y);
            }
            //
            return -output / Scalar(x.cols());
        }
		
    };
	REGISTERED_EVALUATION(InverseAccuracy,"inverse_accuracy")
}