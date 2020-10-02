#include <cmath>
#include <iostream>

#include "cost.h"

Cost::Cost() {}

Cost::~Cost() {}

double Cost::calc(const double out, const double label) const {
    //std::cout<<"LABEL "<<label<<" OUT "<<out<<std::endl;
    return std::pow((out - label), 2.0) / 2.0;
}

double Cost::derivate(const double out, const double label) const {
    return out - label;
}


