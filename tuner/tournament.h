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

namespace pgn { class Game;};

enum class TournamentResult {
	p1Won,
	p2Won,
	draw
};

class Tournament {
public:
	Tournament(const std::string& pgnName, const Player& _p1, const Player& _p2);
	TournamentResult play();

private:
	void _saveGamePgn(const pgn::Game& g);
	void _createNewTournamentPgn();
	void _updateResults(const pgn::Game& g, Player& _p1, Player& _p2);

	const std::string _pgnName;
	Player _p1, _p2;
};

#endif /* TOURNAMENT_H_ */