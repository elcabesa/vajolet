#include <cassert>
#include <iostream>
#include <random>

#include "activation.h"
#include "denseLayer.h"
#include "input.h"

DenseLayer::DenseLayer(const unsigned int inputSize, const unsigned int outputSize, std::shared_ptr<Activation> act, std::vector<double>* bias, std::vector<double>* weight,const double stdDev):
    Layer{inputSize, outputSize, stdDev},
    _bias(bias),
    _weight(weight),
    _act(std::move(act))
    
{
    _bias->resize(outputSize, 0.0);
    _weight->resize(outputSize * inputSize, 1.0);
}

DenseLayer::~DenseLayer() {}

void DenseLayer::calcNetOut(const Input& input) {
    assert(input.size() == _inputSize);
    _netOutput = *_bias;
    unsigned int num = input.getElementNumber();
    for(unsigned int idx = 0; idx < num; ++idx) {
        auto& el = input.getElementFromIndex(idx);
        for(unsigned int o = 0; o < _outputSize; ++o) {
            _netOutput[o] += el.second * (*_weight)[_calcWeightIndex(el.first,o)];
        }
    }  
}

void DenseLayer::calcOut() {
    for(unsigned int o=0; o < _outputSize; ++o) {
        _output.set(o, _act->propagate(_netOutput[o]));
    }
}

void DenseLayer::propagate(const Input& input) {
    calcNetOut(input);
    calcOut();
}

unsigned int DenseLayer::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o + i * _outputSize;

}

bool DenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE DENSE LAYER"<<std::endl;
    union un{
        double d;
        char c[8];
    }u;
    if(ss.get() != '{') {std::cout<<"DenseLayer missing {"<<std::endl;return false;}
    for( auto & b: *_bias) {
        ss.read(u.c, 8);
        b = u.d;
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    for( auto & w: *_weight) {
        ss.read(u.c, 8);
        w = u.d;
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '}') {std::cout<<"DenseLayer missing }"<<std::endl;return false;} 
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
