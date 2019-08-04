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
#ifndef GAME_H_
#define GAME_H_

#include <vector>

#include "hashKey.h"
#include "move.h"
#include "pvLine.h"
#include "score.h"

class Position;

class Game
{
public:
	struct GamePosition
	{
		HashKey key;
		Move m;
		PVline PV;
		Score alpha;
		Score beta;
		unsigned int depth;
	};

	Game(): _isChess960(false){}
	void CreateNewGame(bool isChess960);
	void insertNewMoves(Position &pos);
	void savePV(PVline PV,unsigned int depth, Score alpha, Score beta);
	/*void printGamesInfo();*/

	bool isNewGame(const Position &pos) const;
	/*bool isPonderRight() const;*/
	/*GamePosition getNewSearchParameters() const;*/

private:
	std::vector<GamePosition> _positions;
	bool _isChess960;
public:

};

#endif