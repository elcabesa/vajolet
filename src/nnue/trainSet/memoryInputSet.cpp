#include <iostream>
#include <random>

#include "memoryInputSet.h"
#include "labeledExample.h"
#include "dense.h"

MemoryInputSet::MemoryInputSet(){}
MemoryInputSet::~MemoryInputSet(){}

void MemoryInputSet::generate() {
    std::cout<<"create input "<<std::flush;
    const int SIZE = 10;
    const int BATCH_SIZE = 1000;
    double TRAIN_SET_PERC = 0.9;
    
    std::vector<std::shared_ptr<LabeledExample>> _trainSet;
    _verificationSet.clear();
    _batches.clear();
    
    for(unsigned int x1 = 0; x1<SIZE; ++x1) {
        for(unsigned int x2 = 0; x2<SIZE; ++x2) {
            for(unsigned int x3 = 0; x3<SIZE; ++x3) {
                for(unsigned int x4 = 0; x4<SIZE; ++x4) {
                    std::vector<double> inVec = {double(x1), double(x2),double(x3),double(x4)};
                    std::shared_ptr<Input> in(new DenseInput(inVec));
                    std::shared_ptr<LabeledExample> le(new LabeledExample(std::move(in),function(x1,x2,x3,x4)));
                    _trainSet.push_back(std::move(le));
                }
            }
        }
    }

    std::cout<<"DONE"<<std::endl;
    std::cout<<"input size "<<_trainSet.size()<<std::endl;
    
    std::cout<<"shuffle input "<<std::flush;
    std::random_device rng;
    std::shuffle(std::begin(_trainSet), std::end(_trainSet), rng);
    std::cout<<"DONE"<<std::endl;
    
    std::cout<<"split Set"<<std::flush;
    std::size_t t1 = _trainSet.size() * TRAIN_SET_PERC;
    
    _verificationSet.assign(_trainSet.begin() + t1, _trainSet.end());
    _trainSet.erase(_trainSet.begin() + t1, _trainSet.end() );

    
    std::cout<<"DONE"<<std::endl;
    
    
    std::cout<<"split in batches"<<std::flush;
    auto start = _trainSet.begin();
    auto end = _trainSet.begin() + std::min(BATCH_SIZE, (int)_trainSet.size());
    std::vector<std::shared_ptr<LabeledExample>> batch(start, end);
    _batches.push_back(batch);
    while(end != _trainSet.end()) {
        start += std::min(BATCH_SIZE, (int)std::distance(start,_trainSet.end()));
        end += std::min(BATCH_SIZE, (int)std::distance(end,_trainSet.end()));
        std::vector<std::shared_ptr<LabeledExample>> batch(start, end);
        _batches.push_back(batch);
        
    }
    std::cout<<"DONE"<<std::endl;
    
    /*std::cout<<"trainSet"<<std::endl;
    for(auto & el: _trainSet){
        el->features().print();
        std::cout<<el->label()<<std::endl;
    }
    
    std::cout<<"_verificationSet"<<std::endl;
    for(auto & el: _verificationSet){
        el->features().print();
        std::cout<<el->label()<<std::endl;
    }
    
    std::cout<<"batches"<<std::endl;
    for(auto & b: _batches){
        std::cout<<"----batch---"<<std::endl;
        for(auto & el: b){
            el->features().print();
            std::cout<<el->label()<<std::endl;
        }
    }*/

}
    
double MemoryInputSet::function(double x1, double x2, double x3, double x4) {
    return x1 + x2*x2 - 0.2*x3*x4;
}


const std::vector<std::shared_ptr<LabeledExample>>& MemoryInputSet::validationSet() const {
    return _verificationSet;
}

const std::vector<std::shared_ptr<LabeledExample>>& MemoryInputSet::batch()const {
    if(_n >= _batches.size()) {
        _n = 0;
    }
    return _batches[_n++];
}

void MemoryInputSet::printStatistics() const {
    std::cout<<"statistics:"<<std::endl;
}
