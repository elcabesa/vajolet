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

class Book;
class EpdSaver;

namespace pgn { class Game;}

enum class TournamentResult {
	p1Won = 1,
	p2Won = -1,
	draw = 0
};

class Tournament {
public:
	Tournament(const std::string& pgnName, const std::string& debugName, Player& _p1, Player& _p2, Book &b, EpdSaver * const fs = nullptr,  bool verbose = false);
	TournamentResult play();

private:
	void _saveGamePgn(const pgn::Game& g);
	void _createNewTournamentPgn();
	void _updateResults(const pgn::Game& g, Player& _p1, Player& _p2);

	const std::string _pgnName;
	const std::string _debugName;
	Player &_p1, &_p2;
	Book& _book;
	EpdSaver * const _fs;
	bool _verbose;
};

#endif /* TOURNAMENT_H_ */