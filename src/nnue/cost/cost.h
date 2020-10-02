#ifndef _COST_H
#define _COST_H


class Cost {
public:
    Cost();
    ~Cost();    
    
    double calc(const double out, const double label) const;
    double derivate(const double out, const double label) const;
};

#endif  
