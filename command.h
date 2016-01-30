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

#ifndef COMMAND_H_
#define COMMAND_H_
#include <string>
#include <list>
#include <algorithm>
#include "position.h"
#include "move.h"
//--------------------------------------------------------------------
//	function prototype
//--------------------------------------------------------------------
const char PIECE_NAMES_FEN[] = {' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};
void uciLoop(void);

std::string displayUci(const Move & m);
std::string displayMove(const Position& pos,const Move & m);
void printCurrMoveNumber(unsigned int moveNumber, const Move &m, unsigned long long visitedNodes, long long int time);
void showCurrLine(const Position & pos, unsigned int ply);
void printPVs(unsigned int count);
void printPV(Score res, unsigned int depth, unsigned int seldepth, Score alpha, Score beta, long long time, unsigned int count, std::list<Move>& PV, unsigned long long nodes);



#endif /* COMMAND_H_ */
