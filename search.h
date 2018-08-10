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
#ifndef SEARCH_H_
#define SEARCH_H_

#include <chrono>
#include <vector>
#include <list>
#include <cmath>
#include <string>
#include "vajolet.h"
#include "position.h"
#include "move.h"
#include "history.h"
#include "eval.h"
#include "command.h"

class PVline : public std::list<Move> 
{
public:
	inline void reset()
	{
		clear();
	}
	
	inline void appendNewPvLine( Move bestMove, PVline childPV )
	{
		clear();
		emplace_back( bestMove );
		splice( end(), childPV );
	}
	
	inline void appendNewMove( Move move )
	{
		clear();
		emplace_back( move );
	}
};

class startThinkResult
{
public:
	Score alpha;
	Score beta;
	unsigned int depth;
	std::list<Move> PV;
	Score Res;
	startThinkResult( Score Alpha, Score Beta, unsigned int Depth, std::list<Move> pv, Score res ): alpha(Alpha), beta(Beta), depth(Depth), PV(pv), Res(res){}
};

class searchLimits
{
public:
	volatile bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,nodes,mate,moveTime;
	int depth;
	std::list<Move> searchMoves;
	searchLimits()
	{
		ponder = false;
		infinite = false;
		wtime = 0;
		btime = 0;
		winc = 0;
		binc = 0;
		movesToGo = 0;
		depth = -1;
		nodes = 0;
		mate = 0;
		moveTime = 0;
	}

};

class rootMove
{
public:
	Score score = -SCORE_INFINITE;
	PVline PV;
	Move firstMove;
	unsigned int maxPlyReached = 0u;
	unsigned int depth = 0u;
	unsigned long long nodes = 0u;
	long long int time = 0ll;
	bool operator<(const rootMove& m) const { return score > m.score; } // Ascending sort
	bool operator==(const Move& m) const { return firstMove.packed == m.packed; }

	rootMove(Move & m) : firstMove{m}
	{
		PV.clear();
	}
	
	rootMove(Move& m, PVline& pv, Score s, unsigned int maxPly, unsigned int d, unsigned long long n, long long int t) : score{s}, PV{pv}, firstMove{m}, maxPlyReached{maxPly}, depth{d}, nodes{n}, time{t} {}
};




class searchData
{
public:
	Move excludeMove;
	bool skipNullMove;
	Move killers[2];
};


class Search
{
private:
	std::unique_ptr<UciOutput> _UOI;
	bool followPV;
	static int globalReduction;
	static const unsigned int LmrLimit = 32;
	static Score futility[8];
	static Score futilityMargin[7];
	static unsigned int FutilityMoveCounts[16];
	static Score PVreduction[LmrLimit*ONE_PLY][64];
	static Score nonPVreduction[LmrLimit*ONE_PLY][64];

	static Score mateIn(int ply) { return SCORE_MATE - ply; }
	static Score matedIn(int ply) { return SCORE_MATED + ply; }
	bool validIteration = false;
	Score ExpectedValue = 0;

	unsigned int multiPVcounter = 0;
	History history;
	CaptureHistory captureHistory;
	CounterMove counterMoves;

	searchData sd[STATE_INFO_LENGTH];
	
	void cleanMemoryBeforeStartingNewSearch(void);
	void generateRootMovesList( std::vector<Move>& rm, std::list<Move>& ml);
	void filterRootMovesByTablebase( std::vector<Move>& rm );
	startThinkResult manageQsearch(void);
	
	void cleanData(void)
	{
		std::memset(sd, 0, sizeof(sd));
	}

	void saveKillers(unsigned int ply, Move& m)
	{
		Move * const tempKillers =  sd[ply].killers;
		if(tempKillers[0] != m)
		{
			tempKillers[1] = tempKillers[0];
			tempKillers[0] = m;
		}

	}

	void clearKillers(unsigned int ply)
	{
		Move * const tempKillers =  sd[ply].killers;

		tempKillers[1] = 0;
		tempKillers[0] = 0;
	}

	signed int razorMargin(unsigned int depth,bool cut) const { return 20000+depth*78+cut*20000; }

	enum nodeType
	{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	} ;


	template<nodeType type>Score qsearch(unsigned int ply,int depth,Score alpha,Score beta, PVline& PV);
	template<nodeType type>Score alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,PVline& PV);

	unsigned long long visitedNodes;
	unsigned long long tbHits;

	unsigned int maxPlyReached;

//	void reloadPv(unsigned int i);
//	void verifyPv(std::list<Move> &newPV, Score res);

public:

	static std::vector<Move> rootMoves;
	std::vector<rootMove> rootMovesSearched;
	PVline PV;
	searchLimits limits;
	Position pos;

	const History& getHistory()const {return history;}
	const CaptureHistory& getCaptureHistory()const {return captureHistory;}
	const CounterMove& getCounterMove()const {return  counterMoves;}



	static unsigned int threads;
	static unsigned int multiPVLines;
	static bool useOwnBook;
	static bool bestMoveBook;
	static bool showCurrentLine;
	static std::string SyzygyPath;
	static unsigned int SyzygyProbeDepth;
	static bool Syzygy50MoveRule;
	volatile bool showLine = false;

	static void initLMRreduction(void)
	{
		for (unsigned int d = 1; d < LmrLimit*ONE_PLY; d++)
			for (int mc = 1; mc < 64; mc++)
			{
				double    PVRed = -1.5 + 0.33*log(double(d)) * log(double(mc));
				double nonPVRed = -1.2 + 0.37*log(double(d)) * log(double(mc));
				PVreduction[d][mc] = (Score)(PVRed >= 1.0 ? floor(PVRed * int(ONE_PLY)) : 0);
				nonPVreduction[d][mc] = (Score)(nonPVRed >= 1.0 ? floor(nonPVRed * int(ONE_PLY)) : 0);
			}
	};

	void stopPonder(){ limits.ponder = false;}
	volatile bool stop = false;


	const Move&  getKillers(unsigned int ply,unsigned int n) const { return sd[ply].killers[n]; }


	startThinkResult startThinking(int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline PV= {} );
	
	rootMove idLoop(int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline PV= {} );
	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;

private:
	// gestione timer
	long long int startTime;
	long long int ponderTime;
public:
	static long long int getTime(){ return std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count(); }
	long long int getElapsedTime() const { return getTime() - startTime; }
	long long int getClockTime() const { return getTime() - ponderTime; }
	void resetStartTime(){ startTime = getTime(); }
	void resetPonderTime(){ ponderTime = getTime(); }
	
	Search( std::unique_ptr<UciOutput> UOI = UciOutput::create( ) ):_UOI(std::move(UOI)){}
	UciOutput& getUOI(){ return *_UOI;}
	void setUOI( std::unique_ptr<UciOutput> UOI )
	{
		// manage output syncronization
		sync_cout;
		_UOI = std::move(UOI);
		std::cout<<sync_noNewLineEndl;
	}
	

};

extern Search defaultSearch;

#endif /* SEARCH_H_ */
