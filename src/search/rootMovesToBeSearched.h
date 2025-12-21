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
#ifndef ROOT_MOVES_TO_BE_SEARCHED_H_
#define ROOT_MOVES_TO_BE_SEARCHED_H_

#include <list>
#include "move.h"

class rootMovesToBeSearched
{
public:
	void fill(const std::list<Move>& ml);
	void fill(MoveList<MAX_MOVE_PER_POSITION> ml);
	size_t size() const;
	const Move& getMove(unsigned int pos) const;
	void print() const;
	bool contain(const Move& m) const;
	const std::vector<Move>& getAll() const {return _rm;}
	void remove(const Move& m);
private:
	std::vector<Move> _rm;
	
};

#endif /* SEARCH_H_ */
