/*
	This file is part of Vajolet.

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

#include <iostream>
#include <vector>

#include "moveList.h"
#include "rootMovesToBeSearched.h"
#include "uciOutput.h"
#include "vajolet.h"
#include "vajo_io.h"




void rootMovesToBeSearched::remove(const Move& m) {
	if (auto it = std::find(_rm.begin(), _rm.end(), m); it != _rm.end()) {
		_rm.erase(it);
	}
}

bool rootMovesToBeSearched::contain(const Move& m) const {
	return std::find(_rm.begin(), _rm.end(), m) != _rm.end();
}

void rootMovesToBeSearched::print() const {
	unsigned int i = 0;
	sync_cout;
	std::cout<<"move list"<<std::endl;
	for(auto m: _rm)
	{
		++i;
		std::cout<<i<<": "<<UciOutput::displayUci(m, false)<<std::endl;
		
	}
	std::cout<<sync_endl;
}

const Move& rootMovesToBeSearched::getMove(unsigned int pos) const {
	if (pos < size()) {
		return _rm[pos];
	}
	return Move::NOMOVE;
}

size_t rootMovesToBeSearched::size() const {
	return _rm.size();
}

void rootMovesToBeSearched::fill(const std::list<Move>& ml) {
	_rm.clear();
	for_each(ml.begin(), ml.end(), [&](const Move &m){_rm.emplace_back(m);});
}

void rootMovesToBeSearched::fill(MoveList<MAX_MOVE_PER_POSITION> ml) {
	_rm.clear();
	std::for_each(ml.begin(), ml.end(), [&](const Move &m){_rm.emplace_back(m);});
}

