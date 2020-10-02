#ifndef _LABELED_EXAMPLE_H
#define _LABELED_EXAMPLE_H

#include <memory>
#include <vector>

class Input;

class LabeledExample {
public:
    LabeledExample(std::shared_ptr<Input> features, double l);
    const Input& features() const;
    double label() const;
private:
    std::shared_ptr<Input> _features;
    double _label;
};

#endif
