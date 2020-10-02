#ifndef _PARALLEL_DENSE_LAYER_H
#define _PARALLEL_DENSE_LAYER_H

#include <memory>
#include <vector>

#include "layer.h"
#include "denseLayer.h"

class Activation;

class ParallelDenseLayer: public Layer {
public:
    ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, Activation& act, std::vector<std::vector<double>*> biases, std::vector<std::vector<double>*> weights, const double stdDev = 0.0);
    ~ParallelDenseLayer();
    
    void propagate(const Input& input);
    
    DenseLayer& getLayer(unsigned int);

    unsigned int _calcBiasIndex(const unsigned int layer, const unsigned int offset) const;
    
    bool deserialize(std::ifstream& ss);
    
private:
    std::vector<DenseLayer> _parallelLayers;
    const unsigned int _layerInputSize;
    const unsigned int _layerOutputSize;

};

#endif  
