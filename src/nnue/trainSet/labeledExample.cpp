#include "labeledExample.h"
#include "input.h"

LabeledExample::LabeledExample(std::shared_ptr<Input> features, double l):_features(std::move(features)), _label(l) {}

const Input& LabeledExample::features() const {
    return *_features;
}
double LabeledExample::label() const {
    return _label / 10000;
}
