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
#ifndef TOURNAMENT_H_
#define TOURNAMENT_H_

#include <string>
#include "player.h"
#include "tunerPars.h"
#include "hashKey.h"

class Book;
class BinSaver;
class Tournament {
public:
	Tournament(bool& stop, Player& _p1, Player& _p2, Book &b, BinSaver * const fs = nullptr,  bool verbose = false);
	void play();
	bool isFinished() const { return _finished;}

private:
	Player &_p1, &_p2;
	Book& _book;
	BinSaver * const _fs;
	bool _verbose;
	bool& _stop;
	bool _finished= false;
#define ALREADYSEEN_SIZE (50000)
	tKey alreadySeen[ALREADYSEEN_SIZE];


};

#endif /* TOURNAMENT_H_ */
