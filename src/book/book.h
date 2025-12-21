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

#include <memory>

//forward declaration
class Move;
class Position;

class PolyglotBook
{
private:
	PolyglotBook(const PolyglotBook&) = delete;
	PolyglotBook& operator=(const PolyglotBook&) = delete;
	PolyglotBook(const PolyglotBook&&) = delete;
	PolyglotBook& operator=(const PolyglotBook&&) = delete;
	
	class impl;
	std::unique_ptr<impl> pimpl;
public:
	explicit PolyglotBook();
	~PolyglotBook();
	Move probe(const Position& pos, bool pickBest);

};

#endif /* BOOK_H_ */
