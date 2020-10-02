#ifndef _SPARSE_H
#define _SPARSE_H

#include <map>

#include "input.h"

class SparseInput: public Input {
public:
    
    SparseInput(unsigned int size, const std::map<unsigned int, double> v);
    SparseInput(const std::vector<double> v);
    SparseInput(unsigned int size, const std::vector<unsigned int> v);
    SparseInput(const unsigned int size);
    ~SparseInput();

    void print() const;
    const double& get(unsigned int index) const;
    /*double& get(unsigned int index);*/
    void set(unsigned int index, double v);
    unsigned int getElementNumber() const;
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    //TODO manage 
    double _zeroInput = 0.0;
    std::map<unsigned int, double> _in;
};

#endif   
