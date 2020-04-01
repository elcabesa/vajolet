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
#ifndef BOOK_H_
#define BOOK_H_

#include <list>
#include <mutex>
#include <string>

#include "move.h"
#include "PGNGameCollection.h"

class Move;
class Position;

class Book {
public:
	Book(const std::string& file);
	std::list<Move> getLine();
private:
	pgn::GameCollection _games;
	Move _moveFromPgn(const Position& pos,const  std::string& str);
	pgn::GameCollection::iterator _itr;
	std::mutex _mtx;
	
};

#endif /* CLOCK_H_ */