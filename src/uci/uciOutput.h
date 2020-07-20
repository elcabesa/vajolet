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

#ifndef UCIOUTPUT_H_
#define UCIOUTPUT_H_

#include <memory>
#include <vector>

#include "bitBoardIndex.h"
#include "score.h"
#include "tSquare.h"

class Move;
class Position;
class PVline;
class rootMove;

class UciOutput
{
public:
	/* factory method */
	enum class type
	{
		standard,	// standard output
		mute		// no output
	};
	static std::unique_ptr<UciOutput> create(const UciOutput::type t  = UciOutput::type::standard);
	
	enum class PVbound
	{
		lowerbound,
		upperbound,
		normal
	};
	
	virtual void setDepth(const unsigned int depth) final;
	virtual void setPVlineIndex(const unsigned int PVlineIndex) final;

	// destructor
	virtual ~UciOutput(){};
	
	// virtual output methods
	virtual void printPVs(std::vector<rootMove>& rm, bool ischess960, const long long time, int maxLinePrint = -1) const = 0;
	virtual void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound = PVbound::normal, const int depth = -1, const int count = -1) const = 0;
	virtual void printPV(const Move& m, bool isChess960 = false) final;
	virtual void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const = 0;
	virtual void showCurrLine(const Position& pos, const unsigned int ply) const = 0;
	virtual void printDepth() const = 0;
	virtual void printScore(const signed int cp) const = 0;
	virtual void printBestMove(const Move& bm, const Move& ponder, bool isChess960) const = 0;
	virtual void printGeneralInfo(const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const = 0;
	
	static bool reduceVerbosity;
	
	static std::string displayUci(const Move& m, const bool chess960);
	static std::string displayMove(const Position& pos, const Move& m);
	static char getPieceName(const bitboardIndex idx);
	type getType() const { return _type;}
	
protected:
	unsigned int _depth;
	unsigned int _PVlineIndex;
	type _type;
	
	static char _printFileOf( const tSquare& sq ) { return char( 'a' + getFileOf( sq ) ); }
	static char _printRankOf( const tSquare& sq ) { return char( '1' + getRankOf( sq ) ); }
	
	static const char _PIECE_NAMES_FEN[];
};


#endif