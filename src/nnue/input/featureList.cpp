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
#include <cassert>
#include "featureList.h"

void FeatureList::clear() {
	_pos = 0;
}

void FeatureList::add(unsigned int f) {
    assert(_pos<_size);
	// search in remove
	_list[_pos++] = f;

}

unsigned int FeatureList::get(unsigned int index) const {
    assert(index<_size);
    assert(index<_pos);
	return _list[index];
}

unsigned int FeatureList::size() const {
	return _pos;
}