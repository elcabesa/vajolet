#ifndef _INPUT_H
#define _INPUT_H

#include <vector>
#include <utility>

class Input {
public:
    
    Input(const unsigned int size);
    virtual ~Input();
    
    unsigned int size() const;
    
    virtual void print() const = 0;
    
    virtual const double& get(unsigned int index) const = 0;
    /*virtual double& get(unsigned int index) = 0;*/
    virtual void set(unsigned int index, double v) = 0;
    
    virtual unsigned int getElementNumber() const = 0;
    
    virtual const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const = 0;
    
    virtual void clear() = 0;
protected:
    unsigned int _size;
};

#endif  
