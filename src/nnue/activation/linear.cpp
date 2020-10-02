#include <algorithm>

#include "linear.h"

linearActivation::linearActivation() {
    
}

linearActivation::~linearActivation() {
    
}

double linearActivation::propagate(double input) const {
    return input;
}

const std::string linearActivation::getType() const {
    return "Linear";
}
