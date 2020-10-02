#ifndef _ACTIVATION_H
#define _ACTIVATION_H

#include <memory>
#include <string>

class Activation {    
public:
    
   
    Activation();
    virtual ~Activation();
    
    virtual double propagate(double input) const = 0;
    virtual double derivate(double input) const = 0;
    virtual const std::string getType() const = 0;
};

class ActivationFactory {
public:
    enum class type
    {
        linear,
        relu
    };
    static std::unique_ptr<Activation> create(const ActivationFactory::type t  = ActivationFactory::type::linear);
};

#endif 
