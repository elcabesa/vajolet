#include <algorithm>

#include "linear.h"

linearActivation::linearActivation() {
    
}

linearActivation::~linearActivation() {
    
}

double linearActivation::propagate(double input) const {
    return input;
}

double linearActivation::derivate(double) const {
    return 1;
}

const std::string linearActivation::getType() const {
    return "Linear";
}
