#ifndef _LINEAR_H
#define _LINEAR_H

#include "activation.h"

class linearActivation: public Activation{
public:
    linearActivation();
    ~linearActivation();
    
    double propagate(double input) const;
    double derivate(double input) const;
    const std::string getType() const;
}; 


#endif 
