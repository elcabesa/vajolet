#ifndef _MODEL_H
#define _MODEL_H

#include <memory>
#include <vector>

#include "cost.h"
#include "layer.h"

class LabeledExample;
class Input;
class InputSet;


class Model {
public:
    Model();
    
    void addLayer(std::unique_ptr<Layer> l);
    Layer& getLayer(unsigned int index);
    unsigned int getLayerCount();
    
    void randomizeParams();
    void printParams();
    void printParamsStats();
    
    const Input& forwardPass(const Input& input, bool verbose = false);
    
    // for reference, avg loss is also calculated by calcTotalLossGradient
    double calcLoss(const LabeledExample& le, bool verbose = false);
    double calcAvgLoss(const std::vector<std::shared_ptr<LabeledExample>>& input, bool verbose = false);
    
    void calcLossGradient(const LabeledExample& le);
    void calcTotalLossGradient(const std::vector<std::shared_ptr<LabeledExample>>& input);  
    double getAvgLoss() const;
    
    void serialize(std::ofstream& ss) const;
    bool deserialize(std::ifstream& ss);
    
    void clear();

private:
    std::vector<std::unique_ptr<Layer>> _layers;
    Cost cost;
    double _totalLoss;
    double _avgLoss;
};

#endif
