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
class TxtSaver;
class HashKey;

class SelfPlay {
	
public:
	SelfPlay(Player& white, Player& black, Book& b, tKey* alreadySeenPosition, TxtSaver * const fs = nullptr);
	void playGame();
private:	
	bool _isGameFinished(Score res);
	Position _p;
	Clock _c;
	SearchLimits _sl;
	Player& _white;
	Player& _black;
	Book& _book;
	TxtSaver * const _fs;
	unsigned int winCount = 0;
	unsigned int drawCount = 0;
	const int pawnValue = 10000;
	const int wonGame = 20 * pawnValue;
	const int drawGame = 0.04 * pawnValue;
	tKey* _alreadySeenPosition;
};

#endif /* SELFPLAY_H_ */
