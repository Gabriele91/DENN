# DENN: Differential Evolution for Neural Networks

DENN is a framework designed and developed to train Deep Neural Networks 
using the Differential Evolution as the optimizer.

# How to use

First of all you have to generate the dataset file, in order to do that, execution the following commands:

    cd datasets
    python3 build_*
    cd ..

Then compile the DENN framework, running  the following command:

    make release
    
Finally, choose one of the templates and run it, for instance:  

    Release/DENN-float template/JADE_NN_MNIST.config

## History
The first implementation of this algorithm was in [2017](https://github.com/Gabriele91/DENN-Tensorflow/) . Then we have developed a standard-alone implementation: [DENN-LITE](https://github.com/Gabriele91/DENN-LITE/), on which we have implemented the JADE, ADE, SHADE, and other DE variants, applied on classification and [algorithm learning problems](https://github.com/Gabriele91/DENN-LITE/tree/nram).
Finally, this repository is the code used for the [MDPI publication](https://www.mdpi.com/2227-7390/8/1/69), on which an analysis of that algorithm applied to many problems is presented. Also, at WIVACE 2019, a coevolution version (CoDENN) was presented, and the source couse is avaliable as [branch of this repository](https://github.com/Gabriele91/DENN/tree/coevolution_dynamic).

## Requirements
* C++14 Compiler
* CMake
* Python 3
    
### Dependencies    
* C++:
    * zlib
* Python:
    * numpy
    * tqdm
    * pandas

# Citation
If you use this code for your research, please cite our paper.

    @article{baioletti2020differential,
    title={Differential Evolution for Neural Networks Optimization},
    author={Baioletti, Marco and Di Bari, Gabriele and Milani, Alfredo and Poggioni, Valentina},
    journal={Mathematics},
    volume={8},
    number={1},
    pages={69},
    year={2020},
    publisher={Multidisciplinary Digital Publishing Institute}
    }
