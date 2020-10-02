#ifndef _DISK_INPUT_SET_H
#define _DISK_INPUT_SET_H

#include <fstream>
#include <string>
#include "inputSet.h"
#include "sparse.h"

class DiskInputSet: public InputSet {
public:
    DiskInputSet(const std::string path, unsigned int inputSize);
    ~DiskInputSet();
    
    void generate();
    
    //todo return not a reference?
    const std::vector<std::shared_ptr<LabeledExample>>& validationSet() const;
    const std::vector<std::shared_ptr<LabeledExample>>& batch()const;
    void printStatistics() const;

    
private:
    std::string _path;
    mutable std::vector<std::string> _testSets;
    std::string _verificationSet;
    mutable unsigned int _n;

    LabeledExample parseLine(std::ifstream& ss, bool& finish) const;
    std::shared_ptr<Input> getFeatures(std::ifstream& ss) const;
    double getLabel(std::ifstream& ss) const;
    
    
    mutable std::vector<std::shared_ptr<LabeledExample>> _validationSet;
    mutable std::vector<std::shared_ptr<LabeledExample>> _batch;
    
    unsigned int _inputSize;
    
    const std::vector<std::shared_ptr<LabeledExample>>& readFile(unsigned int index) const;
    
    
};

#endif 
