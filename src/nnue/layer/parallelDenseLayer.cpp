#include <cassert>
#include <iostream>
#include <random>

#include "activation.h"
#include "parallelDenseLayer.h"
#include "denseLayer.h"
#include "input.h"
#include "parallelSparse.h"

ParallelDenseLayer::ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, std::shared_ptr<Activation> act, const double stdDev):
    Layer{number * inputSize, number * outputSize, stdDev}, _layerInputSize(inputSize), _layerOutputSize(outputSize)
    
{
    for(unsigned int n = 0 ; n < number; ++n){
        _parallelLayers.emplace_back(DenseLayer(_layerInputSize, _layerOutputSize, act, _stdDev));
    }    
}

ParallelDenseLayer::~ParallelDenseLayer() {}

std::vector<double>& ParallelDenseLayer::bias() {return _parallelLayers[0/0].bias();}
std::vector<double>& ParallelDenseLayer::weight() {return _parallelLayers[0/0].bias();}

DenseLayer& ParallelDenseLayer::getLayer(unsigned int i) {
    assert(i<_number);
    return _parallelLayers[i];
}


unsigned int ParallelDenseLayer::_calcBiasIndex(const unsigned int layer, const unsigned int offset) const {
    assert(offset < _layerOutputSize);
    assert(layer < _number);
    unsigned int x = layer * _layerOutputSize + offset;
    assert(x < _outputSize);
    return x;
}


void ParallelDenseLayer::propagate(const Input& input) {
    unsigned int n= 0;
    _output.clear();
    for(auto& l: _parallelLayers) {
        
        const ParalledSparseInput psi(input, n, _layerInputSize);
        l.propagate(psi);
        
        // copy back output
        auto& out = l.output();
        unsigned int num = out.getElementNumber();
        for(unsigned int o = 0; o < num; ++o){
            auto el = out.getElementFromIndex(o);
            _output.set(_calcBiasIndex(n, el.first), el.second); 
        }
        ++n;
    }
    
}




bool ParallelDenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE PARALLEL DENSE LAYER"<<std::endl;
    if(ss.get() != '{') {std::cout<<"ParallelDenseLayer missing {"<<std::endl;return false;}
    
    for(auto& l :_parallelLayers) {
        if(!l.deserialize(ss)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    }
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
