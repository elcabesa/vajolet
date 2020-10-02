#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "diskInputSet.h"
#include "labeledExample.h"
#include "dense.h"

DiskInputSet::DiskInputSet(const std::string path, unsigned int inputSize): _path(path), _n(0), _inputSize(inputSize){}
DiskInputSet::~DiskInputSet(){}

void DiskInputSet::generate() {
    for(const auto& entry : std::filesystem::directory_iterator(_path)) {
        std::string path = entry.path();
        if (path.find("testset") != std::string::npos) {
            _testSets.push_back(path);
            //std::cout<<path<<std::endl;
        }
        if (path.find("validationset") != std::string::npos) {
            _verificationSet = path;
            //std::cout<<path<<std::endl;
        }
    }
}


const std::vector<std::shared_ptr<LabeledExample>>& DiskInputSet::validationSet() const {
    _validationSet.clear();
    //std::cout<<_verificationSet<<std::endl;
    std::ifstream ss(_verificationSet);
    if(ss.is_open())
    {
        bool finish = false;
        while(!finish){
            auto ex = parseLine(ss, finish);
            if(finish) {
                //std::cout<<"FINITO"<<std::endl;
                break;
            }
            _validationSet.push_back(std::make_shared<LabeledExample>(ex));
            //std::cout<<"LABELED EXAMPLE"<<std::endl;
            //ex.features().print();
            //std::cout<<ex.label()<<std::endl;
        }
        //std::cout<<"----------"<<std::endl;
        ss.close();
    }
    
    return _validationSet;
    
    
}

const std::vector<std::shared_ptr<LabeledExample>>& DiskInputSet::batch()const {
    
    if(_n >= _testSets.size()) {
        _n = 0;
        std::random_shuffle(_testSets.begin(), _testSets.end());
    }
    
    return readFile(_n++);
}

const std::vector<std::shared_ptr<LabeledExample>>& DiskInputSet::readFile(unsigned int index) const {
    _batch.clear();
    std::ifstream ss(_testSets[index]);
    if(ss.is_open())
    {
        bool finish = false;
        while(!finish){
            auto ex = parseLine(ss, finish);
            if(finish) {
                //std::cout<<"FINITO"<<std::endl;
                break;
            }
            _batch.push_back(std::make_shared<LabeledExample>(ex));
            //std::cout<<"LABELED EXAMPLE"<<std::endl;
            //ex.features().print();
            //std::cout<<ex.label()<<std::endl;
        }
        //std::cout<<"----------"<<std::endl;
        ss.close();
    }
    ++_n;
    //std::cout<<"batch size "<<_batch.size()<<std::endl;
    return _batch;
}

LabeledExample DiskInputSet::parseLine(std::ifstream& ss, bool& finish) const {
    std::vector<unsigned int> inVec = {};
    std::shared_ptr<Input> in(new SparseInput(_inputSize, inVec));
    LabeledExample empty(in,0);
    char temp = ss.get();
    if(ss.eof()) {finish = true; return empty;} 
    if(temp != '{') {std::cout<<"missing {"<<std::endl;finish = true; return empty;}
    auto features = getFeatures(ss);
    if(ss.get() != '{') {std::cout<<"missing {"<<std::endl;finish = true; return empty;}
    double label = getLabel(ss);
    if(ss.get() != '\n') {std::cout<<"missing CR"<<std::endl;finish = true; return empty;}
    
    return LabeledExample(features, label);
}

std::shared_ptr<Input> DiskInputSet::getFeatures(std::ifstream& ss) const {
    std::string x = "";
    std::vector<unsigned int> inVec = {};
    char temp;
    do {
        temp = ss.get();
        while(temp != ',' && temp != '}') {
            x += temp;
            temp = ss.get();
        }
        if(x != "") {
        //std::cout<<x<<std::endl;
            assert((unsigned int)std::stoi(x)<_inputSize);
            inVec.push_back(std::stoi(x));
            
        }
        x= "";
    }while(temp != '}');
    std::shared_ptr<Input> feature(new SparseInput(_inputSize, inVec));
    return feature;
}

double DiskInputSet::getLabel(std::ifstream& ss) const {
    std::string x;
    char temp = ss.get();
    while(temp != '}') {
        x += temp;
        temp = ss.get();
    }
    return std::stod(x);
}

void DiskInputSet::printStatistics() const {
    std::cout<<"Statistics:"<<std::endl;
    
    std::cout<<"input size: "<<_inputSize<<std::endl;
    std::cout<<"file number: "<<_testSets.size()<<std::endl;
    /*for(auto& in: _testSets) {
        std::cout<< in <<" ";
    }*/
    
    std::vector<unsigned long int> featureCount(_inputSize, 0);
    for(unsigned int in = 0; in < _testSets.size(); ++in) {
        //std::cout<<"read file "<<in<<std::endl;
        auto & set = readFile(in);
        for(auto& features: set) {
            const auto& feat =features->features();
            unsigned int num = feat.getElementNumber();
            for(unsigned int o = 0; o < num; ++o){
                auto el = feat.getElementFromIndex(o);
                ++(featureCount[el.first]); 
                //std::cout<<featureCount[el.first]<<std::endl;
            }
        }
    }
    
    int featCount  = std::count_if(featureCount.begin(), featureCount.end(), [](unsigned long int i){return i != 0;});
    std::cout<<"active features ("<<featCount<<"/"<<_inputSize<<")"<<std::endl;
    
    unsigned long int total = std::accumulate(featureCount.begin(), featureCount.end(), 0.0); 
    std::cout<<"total features "<<total<<std::endl;
    std::cout<<"average count "<<total/ double(_inputSize)<<std::endl;
    
    auto max = std::max_element(featureCount.begin(), featureCount.end());
    std::cout<<"max count "<<*max<<std::endl;
    std::cout<<"max at position "<<std::distance(featureCount.begin(), max)<<std::endl;
    std::cout<<std::endl;
    
    /*for(unsigned int i = 0; i< featureCount.size(); ++i) {
        std::cout<<i<<": "<<featureCount[i]<<std::endl;
    }*/
    /*{
        const unsigned long int max =100;
        for( int n = 0; n<max; ++n) {
            std::cout<<"["<<n<<"]: "<<std::count_if(featureCount.begin(), featureCount.end(), [n](unsigned long int i){return i == n;})<<std::endl;
        }
        std::cout<<"[other]: "<<std::count_if(featureCount.begin(), featureCount.end(), [max](unsigned long int i){return i >=max ;})<<std::endl;
    }*/
}
