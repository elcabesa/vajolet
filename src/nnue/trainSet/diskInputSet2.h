#ifndef _DISK_INPUT_SET2_H
#define _DISK_INPUT_SET2_H

#include <fstream>
#include <string>
#include "inputSet.h"
#include "sparse.h"

class DiskInputSet2: public InputSet {
public:
    DiskInputSet2(const std::string path, unsigned int inputSize, unsigned int batchsize = 30);
    ~DiskInputSet2();
    
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

    mutable std::vector<std::shared_ptr<LabeledExample>> _validationSet;
    mutable std::vector<std::shared_ptr<LabeledExample>> _batch;
    
    unsigned int _inputSize;
    unsigned int _batchsize;
    
    mutable std::ifstream _ss;
    
    const std::vector<std::shared_ptr<LabeledExample>>& _readFile(unsigned int index) const;
    LabeledExample _parseLine(std::ifstream& ss, bool& finish) const;
    std::shared_ptr<Input> _getFeatures(std::ifstream& ss) const;
    double _getLabel(std::ifstream& ss) const;

};

#endif 
