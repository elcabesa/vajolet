#ifndef _LAYER_H
#define _LAYER_H

#include <iostream>
#include <fstream>
#include "dense.h"

class Layer {
public:
    Layer(const unsigned int inputSize, const unsigned int outputSize, const double stdDev);
    virtual ~Layer();
    
    unsigned int getInputSize() const;
    unsigned int getOutputSize() const;
    double getOutput(unsigned int i) const;
    const Input& output() const;
    void printOutput() const;
    
    virtual void propagate(const Input& input) = 0;
    virtual void incrementalPropagate(const Input& input) = 0;
    
    virtual bool deserialize(std::ifstream& ss) = 0;

protected:
    unsigned int _inputSize;
    unsigned int _outputSize;
    DenseInput _output;
    const double _stdDev;
    
};

#endif  
