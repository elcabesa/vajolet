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

#ifndef _INPUT_H
#define _INPUT_H

#include <utility>

class Input {
public:
    
    Input(const unsigned int size);
    virtual ~Input();
    
    unsigned int size() const;
    
    virtual void print() const = 0;
    
    virtual const double& get(unsigned int index) const = 0;
    virtual void set(unsigned int index, double v) = 0;
    
    virtual unsigned int getElementNumber() const = 0;
    
    virtual const std::pair<unsigned int, double> getElementFromIndex(unsigned int index) const = 0;
    
    virtual void clear() = 0;
protected:
    unsigned int _size;
};

#endif  
