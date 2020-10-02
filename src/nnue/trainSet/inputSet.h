#ifndef _INPUT_SET_H
#define _INPUT_SET_H

#include <memory>
#include <vector>

class LabeledExample;

class InputSet {
public:
    InputSet();
    virtual ~InputSet();
    
    virtual void generate() = 0;
    virtual const std::vector<std::shared_ptr<LabeledExample>>& validationSet() const = 0;
    virtual const std::vector<std::shared_ptr<LabeledExample>>& batch()const = 0;
    virtual void printStatistics() const = 0;
};

#endif 
