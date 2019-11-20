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

namespace pgn { class Game;};

class Tournament {
public:
	Tournament();
	void play();

private:
	void _saveGamePgn(const pgn::Game& g);
	void _createNewTournamentPgn();

	static const std::string _pgnName;
};

#endif /* TOURNAMENT_H_ */