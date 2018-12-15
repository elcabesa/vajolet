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

#include <memory>

#include "command.h"

#include "history.h"
#include "pvLine.h"
#include "score.h"

class Position;
class SearchTimer;
class SearchLimits;

// may be moved to another file.. used by search and benchmark
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

// may be moved to another file.. used by search and command to print
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
	bool operator==(const Move& m) const { return firstMove == m; }
	bool operator==(const rootMove& m) const { return firstMove == m.firstMove; }
	//Move toBeExcludedMove = Move::NOMOVE;

	rootMove(const Move& m) : firstMove{m}
	{
		PV.clear();
	}
	
	rootMove( const Move& m, PVline& pv, Score s, unsigned int maxPly, unsigned int d, unsigned long long n, long long int t) : score{s}, PV{pv}, firstMove{m}, maxPlyReached{maxPly}, depth{d}, nodes{n}, time{t} {}
};

// may be moved to another file.. used by search and movePicker to order
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
	// public static methods
	//--------------------------------------------------------
	static void initSearchParameters();

	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	Search( SearchTimer& st, SearchLimits& sl, std::unique_ptr<UciOutput> UOI = UciOutput::create( ) );
	~Search();

	Search( const Search& other ) = delete;
	Search& operator=(const Search& other) = delete;
	Search(Search&&) =delete;
	Search& operator=(Search&&) = delete;

	startThinkResult startThinking(int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline pvToBeFollowed = {} );

	void stopSearch();
	void resetStopCondition();

	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;
	void showLine();
	void manageNewSearch();
	Position& getPosition();

private:
	class impl;
	std::unique_ptr<impl> pimpl;
};

#endif /* SEARCH_H_ */
