#ifndef _DENSE_LAYER_H
#define _DENSE_LAYER_H

#include <memory>
#include <vector>

#include "layer.h"

class Activation;

class DenseLayer: public Layer {
public:
    DenseLayer(const unsigned int inputSize, const unsigned int outputSize, std::shared_ptr<Activation> act, const double stdDev = 0.0);
    ~DenseLayer();
    
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
    
    unsigned int _calcWeightIndex(const unsigned int i, const unsigned int o) const;
    
    void serialize(std::ofstream& ss) const;
    bool deserialize(std::ifstream& ss);
    
private:
    std::vector<double> _bias;
    std::vector<double> _weight;
    std::vector<double> _biasGradient;
    std::vector<double> _weightGradient;
    std::vector<double> _biasSumGradient;
    std::vector<double> _weightSumGradient;
    std::vector<double> _netOutput;
    std::shared_ptr<Activation> _act;
    
    
    void calcNetOut(const Input& input);
    void calcOut();
};

#endif  
