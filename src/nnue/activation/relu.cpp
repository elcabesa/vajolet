#include <algorithm>

#include "relu.h"

reluActivation::reluActivation() {
    
}

reluActivation::~reluActivation() {
    
}

double reluActivation::propagate(double input) const {
    return std::max(input, alpha * input);
}

double reluActivation::derivate(double input) const {
    return input >=0 ? 1 : alpha;
}

const std::string reluActivation::getType() const {
    return "Relu";
}
