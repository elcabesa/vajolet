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


#include <list>
#include <vector>

#include "command.h"
#include "history.h"
#include "move.h"
#include "position.h"
#include "searchLimits.h"
#include "searchTimer.h"

class PVline : private std::list<Move>
{
public:
	inline unsigned int size() const
	{
		return std::list<Move>::size();
	}
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
	
	inline const Move& getMove( unsigned int n ) const
	{
		if( size() > n )
		{
			auto it = begin();
			std::advance(it, n);
			return *it;
		}
		else
		{
			return NOMOVE;
		}
	}
	
	using std::list<Move>::iterator;
	using std::list<Move>::begin;
	using std::list<Move>::end;
	PVline( unsigned int n, const Move m ) : std::list<Move>(n,m){}
	PVline(){}
	
};

class startThinkResult
{
public:
	Score alpha;
	Score beta;
	unsigned int depth;
	PVline PV;
	Score Res;
	startThinkResult( Score Alpha, Score Beta, unsigned int Depth, PVline pv, Score res ): alpha(Alpha), beta(Beta), depth(Depth), PV(pv), Res(res){}
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
		PV.reset();
	}
	
	rootMove( const Move& m, PVline& pv, Score s, unsigned int maxPly, unsigned int d, unsigned long long n, long long int t) : score{s}, PV{pv}, firstMove{m}, maxPlyReached{maxPly}, depth{d}, nodes{n}, time{t} {}
};


class SearchData
{
public:
	struct Sd
	{
	public:
		Move excludeMove;
		bool skipNullMove;
		Move killers[2];
		Score staticEval;
		bool inCheck;
	} story[800];
	CounterMove counterMoves;
	CaptureHistory captureHistory;
	History history;

	void clearKillers(unsigned int ply);
	void cleanData(void);
	void saveKillers(unsigned int ply, Move& m);

	const History& getHistory()const {return history;}
	const CaptureHistory& getCaptureHistory()const {return captureHistory;}
	const CounterMove& getCounterMove()const {return  counterMoves;}
	const Move& getKillers(unsigned int ply,unsigned int n) const { return story[ply].killers[n]; }
};


class Search
{
public:

	//--------------------------------------------------------
	// public members
	//--------------------------------------------------------
	Position pos;


	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------
	static void initSearchParameters(void);

	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
public:

	Search( SearchTimer& st, SearchLimits& sl, std::unique_ptr<UciOutput> UOI = UciOutput::create( ) ):_UOI(std::move(UOI)), _sl(sl), _st(st){}
	Search( const Search& other ):_UOI(UciOutput::create()), _sl(other._sl), _st(other._st){ /* todo fare la copia*/}
	Search& operator=(const Search& other)
	{
		// todo fare una copia fatta bene
		_sl = other._sl;
		_st = other._st;
		_UOI= UciOutput::create();
		return * this;
	}

	startThinkResult startThinking(int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline pvToBeFollowed = {} );

	void stopSearch(){ stop = true;}
	void resetStopCondition(){ stop = false;}
	bool isNotStopped(){ return stop == false; }

	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;
	void showLine(){ _showLine= true;}

private:
	//--------------------------------------------------------
	// private enum definition
	//--------------------------------------------------------
	enum nodeType
	{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	};

	//--------------------------------------------------------
	// private static members
	//--------------------------------------------------------
	static const int ONE_PLY = 16;
	static const int ONE_PLY_SHIFT = 4;
	static const unsigned int LmrLimit = 32;
	static Score futilityMargin[7];
	static unsigned int FutilityMoveCounts[2][16];
	static Score PVreduction[2][LmrLimit*ONE_PLY][64];
	static Score nonPVreduction[2][LmrLimit*ONE_PLY][64];
	static std::vector<Move> rootMoves;

	static Score mateIn(int ply) { return SCORE_MATE - ply; }
	static Score matedIn(int ply) { return SCORE_MATED + ply; }

	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------
	std::unique_ptr<UciOutput> _UOI;

	int globalReduction;
	bool validIteration = false;
	Score ExpectedValue = 0;
	unsigned int multiPVcounter = 0;
	bool followPV;
	PVline pvLineToFollow;
	Position::eNextMove initialNextMove;
	bool _showLine = false;


	SearchData sd;

	unsigned long long visitedNodes;
	unsigned long long tbHits;
	unsigned int maxPlyReached;

	std::vector<rootMove> rootMovesSearched;

	SearchLimits& _sl; // todo limits belong to threads
	SearchTimer& _st;


	volatile bool stop = false;

	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------
	void cleanMemoryBeforeStartingNewSearch(void);
	void generateRootMovesList( std::vector<Move>& rm, std::list<Move>& ml);
	void filterRootMovesByTablebase( std::vector<Move>& rm );
	startThinkResult manageQsearch(void);
	
	void enableFollowPv();
	void disableFollowPv();
	void manageLineToBefollowed(unsigned int ply, Move& ttMove);


	signed int razorMargin(unsigned int depth,bool cut) const { return 20000+depth*78+cut*20000; }

	template<nodeType type>Score qsearch(unsigned int ply,int depth,Score alpha,Score beta, PVline& pvLine);
	template<nodeType type>Score alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,PVline& pvLine);

	void idLoop(rootMove& bestMove, int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, bool masterThread = false);

	void setUOI( std::unique_ptr<UciOutput> UOI );
	static Score futility(int depth, bool improving );
	

};

#endif /* SEARCH_H_ */
