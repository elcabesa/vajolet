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
#ifndef SELFPLAY_H_
#define SELFPLAY_H_

#include <string>
#include "clock.h"
#include "position.h"
#include "searchLimits.h"

namespace pgn { class Game;}
class Book;
class Player;
class EpdSaver;

class SelfPlay {
	
public:
	SelfPlay(Player& white, Player& black, Book& b, EpdSaver * const fs = nullptr);
	pgn::Game playGame(unsigned int round);
private:	
	bool _isGameFinished(Score res);
	std::string _getGameResult(Score res);
	void _addGameTags(pgn::Game& g, int round);
	void _addGameResult(pgn::Game& g, const std::string & s);
	Position _p;
	Clock _c;
	SearchLimits _sl;
	Player& _white;
	Player& _black;
	Book& _book;
	EpdSaver * const _fs;
};

#endif /* SELFPLAY_H_ */