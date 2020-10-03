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

#include "differentialList.h"
#include "sparse.h"


void DifferentialList::clear() {
	_addPos = 0;
	_removePos = 0;
}

void DifferentialList::serialize(SparseInput& s, unsigned int offset) const{
	for(unsigned int i = 0; i < _addPos; ++i) {
		s.set(_addList[i] + offset, 1.0);
	}
	for(unsigned int i = 0; i < _removePos; ++i) {
		s.set(_removeList[i] + offset, -1.0);
	}
}

void DifferentialList::add(unsigned int f) {
	// search in remove
	for(unsigned int i = 0; i < _removePos; ++i) {
		if(_removeList[i] == f) {
			// found remove it
			for(unsigned int x = i; x < _removePos; ++x) {
				_removeList[x] = _removeList[x+1];
			}
			--_removePos;
			return;
		}
	}
	_addList[_addPos++] = f;

}
void DifferentialList::remove(unsigned int f) {
	// search in remove
	for(unsigned int i = 0; i < _addPos; ++i) {
		if(_addList[i] == f) {
			// found remove it
			for(unsigned int x = i; x < _addPos; ++x) {
				_addList[x] = _addList[x+1];
			}
			--_addPos;
			return;
		}
	}
	_removeList[_removePos++] = f;
}

unsigned int DifferentialList::size() const {
	return _addPos + _removePos;
}

unsigned int DifferentialList::addList(unsigned int i) const {
	return _addList[i];
}

unsigned int DifferentialList::removeList(unsigned int i) const {
	return _removeList[i];
}

unsigned int DifferentialList::addSize() const {
	return _addPos;
}

unsigned int DifferentialList::removeSize() const {
	return _removePos;
}