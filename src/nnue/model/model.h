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
    void randomizeParams();
    void printParams();
    const Input& forwardPass(const Input& input, bool verbose = false);
    double calcLoss(const LabeledExample& le);
    double calcAvgLoss(const std::vector<std::shared_ptr<LabeledExample>>& input);
    void calcLossGradient(const LabeledExample& le);
    void calcTotalLossGradient(const std::vector<std::shared_ptr<LabeledExample>>& input);  
    double getAvgLoss() const;
    void serialize(std::ofstream& ss) const;
    bool deserialize(std::ifstream& ss);
    Layer& getLayer(unsigned int index);
    unsigned int getLayerCount();
    
private:
    std::vector<std::unique_ptr<Layer>> _layers;
    Cost cost;
    double _totalLoss;
    double _avgLoss;
};

#endif
