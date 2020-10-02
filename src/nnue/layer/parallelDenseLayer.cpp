#include <cassert>
#include <iostream>
#include <random>

#include "activation.h"
#include "parallelDenseLayer.h"
#include "denseLayer.h"
#include "input.h"
#include "parallelSparse.h"

ParallelDenseLayer::ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, std::shared_ptr<Activation> act, const double stdDev):
    Layer{number * inputSize, number * outputSize, stdDev}, _number(number), _layerInputSize(inputSize), _layerOutputSize(outputSize), _layerWeightNumber(_layerInputSize * _layerOutputSize)
    
{
    _bias.resize(_outputSize, 0.0);
    _weight.resize(number * _layerWeightNumber, 1.0);
    
    
    for(unsigned int n = 0 ; n < number; ++n){
        _parallelLayers.emplace_back(DenseLayer(_layerInputSize, _layerOutputSize, act, _stdDev));
    }
    consolidateResult();
    
}

ParallelDenseLayer::~ParallelDenseLayer() {}

std::vector<double>& ParallelDenseLayer::bias() {return _bias;}
std::vector<double>& ParallelDenseLayer::weight() {return _weight;}

void ParallelDenseLayer::consolidateResult() {
    unsigned int n = 0;
    for(auto& l: _parallelLayers) {
        auto & b = l.bias();
        for(unsigned int o = 0; o < _layerOutputSize; ++o) {
            b[o] = _bias[_calcBiasIndex(n, o)];
        }
        
        auto & w = l.weight();
        for(unsigned int wi = 0; wi < _layerWeightNumber; ++wi) {
            w[wi] = _weight[_calcWeightIndex(n, wi)];
        }
        
        ++n;
    }
}

double ParallelDenseLayer::getBiasSumGradient(unsigned int index) const{
    unsigned int layerNum = index / _layerOutputSize;
    assert(layerNum<_number);
    return _parallelLayers[layerNum].getBiasSumGradient(index % _layerOutputSize);
}
double ParallelDenseLayer::getWeightSumGradient(unsigned int index) const {
    unsigned int layerNum = index / _layerWeightNumber;
    assert(layerNum<_number);
    return _parallelLayers[layerNum].getWeightSumGradient(index % _layerWeightNumber);
}

unsigned int ParallelDenseLayer::_calcWeightIndex(const unsigned int layer, const unsigned int offset) const {
    assert(offset < _layerWeightNumber);
    assert(layer < _number);
    unsigned int x = layer * _layerWeightNumber + offset;
    assert(x < _weight.size());
    return x;
}

unsigned int ParallelDenseLayer::_calcBiasIndex(const unsigned int layer, const unsigned int offset) const {
    assert(offset < _layerOutputSize);
    assert(layer < _number);
    unsigned int x = layer * _layerOutputSize + offset;
    assert(x < _bias.size());
    return x;
}

void ParallelDenseLayer::randomizeParams() {    
    unsigned int n = 0;
    for(auto& l: _parallelLayers) {
        l.randomizeParams();
        
        auto & b = l.bias();
        for(unsigned int o = 0; o < _layerOutputSize; ++o) {
            _bias[_calcBiasIndex(n, o)] = b[o];
        }
        
        auto & w = l.weight();
        for(unsigned int wi = 0; wi < _layerWeightNumber; ++wi) {
            _weight[_calcWeightIndex(n, wi)] = w[wi];
        }
        ++n;
    }    
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

void ParallelDenseLayer::printParams() const {
    for(auto& l: _parallelLayers) {
        l.printParams();
    }

}

std::vector<double> ParallelDenseLayer::backPropHelper() const {
    std::vector<double> ret;
    for(auto& l: _parallelLayers) {
        auto r = l.backPropHelper();
        ret.insert(ret.end(), r.begin(), r.end());
    }
    return ret;    
}

void ParallelDenseLayer::resetSum() {
    for(auto& l: _parallelLayers) {
        l.resetSum();
    } 
}

void ParallelDenseLayer::accumulateGradients(const Input& input) {
    assert(input.size() == _inputSize);
    unsigned int n= 0;
    for(auto& l: _parallelLayers) {
        const ParalledSparseInput psi(input, n , _layerInputSize);
        l.accumulateGradients(psi);
        ++n;
    }
}

void ParallelDenseLayer::backwardCalcBias(const std::vector<double>& h) {
    assert(h.size() == _outputSize);
    unsigned int n = 0;
    for(auto& l: _parallelLayers) {
        const std::vector<double> in(h.begin() + _layerOutputSize * n, h.begin() + _layerOutputSize * (n + 1));
        l.backwardCalcBias(in);
        ++n;
    }
}


void ParallelDenseLayer::backwardCalcWeight(const Input& input) {
    unsigned int n= 0;
    for(auto& l: _parallelLayers) {
        const ParalledSparseInput psi(input, n , _layerInputSize);
        l.backwardCalcWeight(psi);
        ++n;
    }
}


void ParallelDenseLayer::serialize(std::ofstream& ss) const{
    ss << "{";
    for(auto& l: _parallelLayers) {
        l.serialize(ss);
    }
    ss << "}"<<std::endl;
}

bool ParallelDenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE PARALLEL DENSE LAYER"<<std::endl;
    if(ss.get() != '{') {std::cout<<"ParallelDenseLayer missing {"<<std::endl;return false;}
    
    unsigned int n = 0;
    for(auto& l :_parallelLayers) {
        if(!l.deserialize(ss)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
        
        // todo this code is duplicated with the code of randomize params
        auto & b = l.bias();
        for(unsigned int o = 0; o < _layerOutputSize; ++o) {
            _bias[_calcBiasIndex(n, o)] = b[o];
        }
        
        auto & w = l.weight();
        for(unsigned int wi = 0; wi < _layerWeightNumber; ++wi) {
            _weight[_calcWeightIndex(n, wi)] = w[wi];
        }
        ++n;
    }
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
