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
#ifndef SEARCHER_H_
#define SEARCHER_H_

#include <mutex>
#include "searchLogger.h"
#include "multiPVmanager.h"
#include "parameters.h"
#include "position.h"
#include "pvLineFollower.h"
#include "rootMovesToBeSearched.h"
#include "search.h"
#include "searchData.h"
#include "searchTimer.h"
#include "timeManagement.h"
#include "transposition.h"


class logWriter;
class ttEntry;
class UciOutput;

class Searcher
{
public:
	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	Searcher(Search::impl& father, SearchTimer& st, SearchLimits& sl, transpositionTable& tt, SearchParameters& sp, timeManagement& tm, rootMovesToBeSearched(rm), PVline pvToBeFollowed, UciOutput::type UOI);
    Searcher(const Searcher& other);

    void showLine();
    void stopSearch();

    unsigned long long getVisitedNodes() const { return _visitedNodes; };
	unsigned long long getTbHits() const { return _tbHits; };
    bool isFinished() const {return _finished;}



    void searchManager(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove);
	Score performQsearch();

private:
	//--------------------------------------------------------
	// private enum definition
	//--------------------------------------------------------
	enum class nodeType
	{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	};

	//--------------------------------------------------------
	// private static members
	//--------------------------------------------------------
	//TODO move elsewhere?
	static constexpr int ONE_PLY = 16;
	static constexpr int ONE_PLY_SHIFT = 4;
	static std::mutex _mutex;

	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------
	std::unique_ptr<UciOutput> _UOI;
    std::unique_ptr<logWriter> _lw;

    PVlineFollower _pvLineFollower;
    MultiPVManager _multiPVmanager;
    rootMovesToBeSearched _rootMovesToBeSearched;

    SearchData _sd;
	Position _pos;
    
    bool _showLine = false;

	bool _validIteration = false;
	Score _expectedValue = 0;
	eNextMove _initialTurn = whiteTurn;
	
	unsigned long long _visitedNodes = 0;
	unsigned long long _tbHits = 0;
	unsigned int _maxPlyReached = 0;

    const SearchParameters &_sp;
	const SearchLimits& _sl;
	const SearchTimer& _st;
	transpositionTable& _tt;
    timeManagement& _tm;
    Search::impl& _father;

	volatile bool _stop = false;
    volatile bool _finished = false;
	
	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------

    void _resetStopCondition();
    void _cleanMemoryBeforeStartingNewSearch();

    void _idLoop(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, bool masterThread = false );
    void _excludeRootMoves( std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, bool masterThread);

    template<bool log>rootMove _aspirationWindow(const int depth, Score alpha, Score beta, const bool masterThread);

    template<nodeType type, bool log>Score _alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,PVline& pvLine);
    template<nodeType type, bool log>Score _qsearch(unsigned int ply,int depth,Score alpha,Score beta, PVline& pvLine);

	signed int _razorMargin(unsigned int depth,bool cut) const {return _sp.razorMargin + depth * _sp.razorMarginDepth + cut * _sp.razorMarginCut; }
	Score _futility(int depth, bool improving);
	Score _getDrawValue() const;
	void _updateCounterMove(const Move& m);
	void _updateNodeStatistics(const unsigned int ply);
	bool _manageDraw(const bool PVnode, PVline& pvLine);
	void _showCurrenLine(const unsigned int ply, const int depth);
	bool _MateDistancePruning(const unsigned int ply, Score& alpha, Score& beta) const;
	void _appendTTmoveIfLegal(const Move& ttm, PVline& pvLine) const;
	bool _canUseTTeValue(const bool PVnode, const Score beta, const Score ttValue, const ttEntry * const tte, short int depth) const;
	const HashKey _getSearchKey(const bool excludedMove = -false) const;

	using tableBaseRes = struct{ ttType TTtype; Score value;};
	tableBaseRes _checkTablebase(const unsigned int ply, const int depth);
	
#ifdef DEBUG_EVAL_SIMMETRY
	void _testSimmetry() const;
#endif

};

#endif
