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

#ifndef _DENSE_H
#define _DENSE_H

#include <vector>

#include "input.h"

class DenseInput: public Input {
public:
    DenseInput(const std::vector<double> v);
    DenseInput(const unsigned int size);
    ~DenseInput();
    void print() const;
    const double& get(unsigned int index) const;
    void set(unsigned int index, double v);
    unsigned int getElementNumber() const;
    const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const;
    void clear();
private:
    mutable std::pair<unsigned int, double> tempReply;
    std::vector<double> _in;
};

#endif   
