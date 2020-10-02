#ifndef _MEMORY_INPUT_SET_H
#define _MEMORY_INPUT_SET_H
#include "inputSet.h"


class MemoryInputSet: public InputSet {
public:
    MemoryInputSet();
    ~MemoryInputSet();
    
    void generate();
    
    const std::vector<std::shared_ptr<LabeledExample>>& validationSet() const;
    const std::vector<std::shared_ptr<LabeledExample>>& batch()const;
    void printStatistics() const;

    
private:
    double function(double x1, double x2, double x3, double x4);
    std::vector<std::shared_ptr<LabeledExample>> _verificationSet;
    std::vector<std::vector<std::shared_ptr<LabeledExample>>> _batches;
    mutable unsigned int _n =0;
};

#endif 
