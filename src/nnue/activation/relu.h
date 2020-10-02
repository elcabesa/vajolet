#ifndef _RELU_H
#define _RELU_H

#include "activation.h"

class reluActivation: public Activation{
public:
    reluActivation();
    ~reluActivation();
    
    double propagate(double input) const;
    double derivate(double input) const;
    const std::string getType() const;
    static constexpr double alpha = 1e-5;
}; 


#endif 
