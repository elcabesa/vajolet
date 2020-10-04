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


#ifndef _DIFFERENTIAL_LIST_H
#define _DIFFERENTIAL_LIST_H

#include <array>

class SparseInput;

class DifferentialList {
public:
    void clear();
    void add(unsigned int f);
    void remove(unsigned int f);
    unsigned int size() const;
    unsigned int addList(unsigned int) const;
    unsigned int removeList(unsigned int) const;
    unsigned int addSize() const;
    unsigned int removeSize() const;
private:
    static constexpr unsigned int _size = 500;
    // TODO minimize size
    // TODO save max element stored
    std::array<unsigned int, _size> _addList;
    std::array<unsigned int, _size> _removeList;
    unsigned int _addPos = 0;
    unsigned int _removePos = 0;

};

#endif