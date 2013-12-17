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

#include <fstream>
#include <string>
#include <iostream>

U64 polyglotKey(const Position& pos);

class PolyglotBook : private std::ifstream {
public:
	PolyglotBook();
	~PolyglotBook();
	Move probe(const Position& pos, const std::string& fName, bool pickBest);
private:
	template<typename T> PolyglotBook& operator>>(T& n);

	bool open(const std::string& fName);
	size_t find_first(U64 key);

	std::string fileName;

};


#endif /* BOOK_H_ */
