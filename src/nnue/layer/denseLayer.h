#ifndef _DENSE_LAYER_H
#define _DENSE_LAYER_H

#include <cassert>
#include <memory>
#include <vector>
#include <set>

#include "layer.h"

class Activation;

class DenseLayer: public Layer {
public:
    DenseLayer(const unsigned int inputSize, const unsigned int outputSize, Activation& act, std::vector<double>* bias, std::vector<double>* weight,const double stdDev = 0.0);
    ~DenseLayer();
    
    void propagate(const Input& input);

    unsigned int _calcWeightIndex(const unsigned int i, const unsigned int o) const;

    bool deserialize(std::ifstream& ss);
    
private:
    std::vector<double>* _bias;
    std::vector<double>* _weight;
   
    std::vector<double> _netOutput;
    
    Activation& _act;
    
    //std::set<unsigned int> _activeFeature;
    
    
    void calcNetOut(const Input& input);
    void calcOut();
};

#endif  
