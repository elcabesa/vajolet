#ifndef _DENSE_H
#define _DENSE_H

#include "input.h"

class DenseInput: public Input {
public:
    DenseInput(const std::vector<double> v);
    DenseInput(const unsigned int size);
    ~DenseInput();
    void print() const;
    const double& get(unsigned int index) const;
    /*double& get(unsigned int index);*/
    void set(unsigned int index, double v);
    unsigned int getElementNumber() const;
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    mutable std::pair<unsigned int, double> tempReply;
    std::vector<double> _in;
};

#endif   
