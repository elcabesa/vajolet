#ifndef _MODEL_H
#define _MODEL_H

#include <memory>
#include <vector>

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
    
    const Input& forwardPass(const Input& input, bool verbose = false);
    const Input& incrementalPass(const Input& input, bool verbose = false);
    
    bool deserialize(std::ifstream& ss);
    
    void clear();

private:
    std::vector<std::unique_ptr<Layer>> _layers;
};

#endif
