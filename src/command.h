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


#include <memory>
#include <string>
#include <vector>

#include "bitBoardIndex.h"
#include "score.h"
//--------------------------------------------------------------------
//	forward declaration
//--------------------------------------------------------------------
class Move;
class Position;
class rootMove;
class PVline;


//--------------------------------------------------------------------
//	function prototype
//--------------------------------------------------------------------
class UciManager
{
private:
	explicit UciManager();
	~UciManager();
	UciManager(const UciManager&) = delete;
	UciManager& operator=(const UciManager&) = delete;
	UciManager(const UciManager&&) = delete;
	UciManager& operator=(const UciManager&&) = delete;
	
	class impl;
	std::unique_ptr<impl> pimpl;
public:

	static UciManager& getInstance()
	{
		static UciManager instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}
	void uciLoop(void);
	static char getPieceName(const bitboardIndex idx);
	static std::string displayUci(const Move& m, const bool chess960);
	static std::string displayMove(const Position& pos, const Move& m);
};

/******************************************
UCI output library
*******************************************/
class UciOutput
{
public:
	/* factory method */
	enum type
	{
		standard,	// standard output
		mute		// no output
	};

	enum PVbound
	{
		lowerbound,
		upperbound,
		normal
	};

	static std::unique_ptr<UciOutput> create( const UciOutput::type t  = UciOutput::standard );
	
	virtual void setDepth( const unsigned int depth ) final;
	virtual void setPVlineIndex( const unsigned int PVlineIndex ) final;


	// destructor
	virtual ~UciOutput(){};
	
	// virtual output methods
	virtual void printPVs(std::vector<rootMove>& rm, bool ischess960, int maxLinePrint = -1) const = 0;
	virtual void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, const PVbound bound = normal, const int depth = -1, const int count = -1, bool ischess960 = false) const = 0;
	virtual void printPV( const Move& m, bool isChess960 = false) final;
	virtual void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const = 0;
	virtual void showCurrLine(const Position & pos, const unsigned int ply) const = 0;
	virtual void printDepth() const = 0;
	virtual void printScore(const signed int cp) const = 0;
	virtual void printBestMove( const Move& bm, const Move& ponder, bool isChess960 ) const = 0;
	virtual void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const = 0;
	
protected:
	unsigned int _depth;
	unsigned int _PVlineIndex;

};

#endif /* COMMAND_H_ */
