#ifndef _SPARSE_H
#define _SPARSE_H

#include <utility>
#include <vector>

#include "input.h"

class SparseInput: public Input {
public:

    SparseInput(const unsigned int size);
    ~SparseInput();

    void print() const;
    const double& get(unsigned int index) const;
    void set(unsigned int index, double v);
    unsigned int getElementNumber() const;
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    //TODO manage 
    double _zeroInput = 0.0;
    std::vector<std::pair<unsigned int, double>> _in;
};

#endif   
