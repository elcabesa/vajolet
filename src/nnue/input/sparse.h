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

#ifndef _SPARSE_H
#define _SPARSE_H

#include <utility>
#include <vector>

#include "input.h"

class SparseInput: public Input {
public:

    SparseInput(const unsigned int size);
    ~SparseInput();

    void print() const;
    const double& get(unsigned int index) const;
    void set(unsigned int index, double v);
    unsigned int getElementNumber() const;
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    //TODO manage 
    double _zeroInput = 0.0;
    std::vector<std::pair<unsigned int, double>> _in;
};

#endif   
