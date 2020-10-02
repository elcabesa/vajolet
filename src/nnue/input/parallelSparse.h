#ifndef _PARALLEL_SPARSE_H
#define _PARALLEL_SPARSE_H

#include "input.h"

class ParalledSparseInput: public Input {
public:
    ParalledSparseInput(const Input& si, unsigned int number, unsigned int size);
    ~ParalledSparseInput();
    
    void print() const;
    
    const double& get(unsigned int index) const;
    /*double& get(unsigned int index);*/
    void set(unsigned int index, double v); // not allowed
    
    unsigned int getElementNumber() const;
    
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    const Input& _si;
    const unsigned int _number;
    mutable std::pair<unsigned int, double> tempReply;
    mutable unsigned int _elementNumber;
    mutable std::vector<std::pair<unsigned int, double>> _elements;
};

#endif   
