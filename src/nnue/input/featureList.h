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


#ifndef _FEATURE_LIST_H
#define _FEATURE_LIST_H

#include <array>

class FeatureList {
public:
    void clear();
    void add(unsigned int f);
    unsigned int get(unsigned int index) const;
    unsigned int size() const;
private:

    // TODO minimize size
    // TODO save max element stored
    std::array<unsigned int, 100> _list;
    unsigned int _pos = 0;
};

#endif