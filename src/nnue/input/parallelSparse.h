/*
	This file is part of Vajolet.
	Copyright (C) 2013-2018 Marco Belli
	
    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _PARALLEL_SPARSE_H
#define _PARALLEL_SPARSE_H

#include <vector>
#include "input.h"

class ParalledSparseInput: public Input {
public:
    ParalledSparseInput(const Input& si, unsigned int number, unsigned int size);
    ~ParalledSparseInput();
    
    void print() const;
    
    const double& get(unsigned int index) const;
    void set(unsigned int index, double v); // not allowed
    
    unsigned int getElementNumber() const;
    
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    const Input& _si;
    const unsigned int _number;
    mutable std::pair<unsigned int, double> tempReply;
    mutable unsigned int _elementNumber;
    mutable std::vector<std::pair<unsigned int, double>> _elements;
};

#endif   
