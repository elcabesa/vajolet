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
#include <memory>
#include "vajolet.h"
#include "move.h"
//--------------------------------------------------------------------
//	forward declaration
//--------------------------------------------------------------------
struct Move;
class Position;

//--------------------------------------------------------------------
//	function prototype
//--------------------------------------------------------------------

void uciLoop(void);
char getPieceName( const unsigned int idx );

std::string displayUci(const Move & m);
std::string displayMove(const Position& pos,const Move & m);




class UciOutputInterface
{
public:
	enum type
	{
		standardUciOutput,
		noUciOutput
	};
	virtual ~UciOutputInterface(){};
	virtual void printPVs(const unsigned int count) const = 0;
	virtual void printPV(const Score res, const unsigned int depth, const unsigned int seldepth, const Score alpha, const Score beta, const long long time, const unsigned int count, std::list<Move>& PV, const unsigned long long nodes) const = 0;
	virtual void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const = 0;
	virtual void showCurrLine(const Position & pos, const unsigned int ply) const = 0;
	virtual void printDepth(const unsigned int depth) const = 0;
	virtual void printScore(const signed int cp) const = 0;
	virtual void printBestMove( const Move bm, const Move ponder = Move(0)  ) const = 0;
	virtual void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const = 0;
	static std::shared_ptr<UciOutputInterface> factory( const UciOutputInterface::type t  = UciOutputInterface::standardUciOutput );
	
	
};





#endif /* COMMAND_H_ */
