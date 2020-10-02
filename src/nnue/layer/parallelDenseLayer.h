#ifndef _PARALLEL_DENSE_LAYER_H
#define _PARALLEL_DENSE_LAYER_H

#include <memory>
#include <vector>

#include "layer.h"
#include "denseLayer.h"

class Activation;

class ParallelDenseLayer: public Layer {
public:
    ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, std::shared_ptr<Activation> act, const double stdDev = 0.0);
    ~ParallelDenseLayer();
    
    void propagate(const Input& input);
    void printParams() const;
    void randomizeParams();
    void backwardCalcBias(const std::vector<double>& h);
    void backwardCalcWeight(const Input& input);
    std::vector<double> backPropHelper() const;
    
    void resetSum();
    void accumulateGradients(const Input& input);
    
    std::vector<double>& bias();
    std::vector<double>& weight();
    
    void consolidateResult();
    
    double getBiasSumGradient(unsigned int index) const;
    double getWeightSumGradient(unsigned int index) const;
    
    unsigned int _calcWeightIndex(const unsigned int layer, const unsigned int offset) const;
    unsigned int _calcBiasIndex(const unsigned int layer, const unsigned int offset) const;
    
    void serialize(std::ofstream& ss) const;
    bool deserialize(std::ifstream& ss);
    
private:
    std::vector<double> _bias;
    std::vector<double> _weight;
    std::vector<DenseLayer> _parallelLayers;
    const unsigned int _number;
    const unsigned int _layerInputSize;
    const unsigned int _layerOutputSize;
    const unsigned int _layerWeightNumber;

};

#endif  
