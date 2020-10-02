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
#ifndef SEARCH_IMPL_H_
#define SEARCH_IMPL_H_

#include "game.h"
#include "searchLogger.h"
#include "multiPVmanager.h"
#include "nnue.h"
#include "parameters.h"
#include "position.h"
#include "pvLineFollower.h"
#include "rootMovesToBeSearched.h"
#include "search.h"
#include "searcher.h"
#include "searchData.h"
#include "searchTimer.h"
#include "timeManagement.h"
#include "transposition.h"


class ttEntry;
class UciOutput;

class Search::impl
{
public:
	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	impl(SearchTimer& st, SearchLimits& sl, transpositionTable& tt, std::unique_ptr<UciOutput> UOI = UciOutput::create()):_UOI(std::move(UOI)), _pos(&_nnue), _sl(sl), _st(st), _tt(tt) {}

	impl(const impl& other) = delete;
	impl& operator=(const impl& other) = delete;

	void stopSearch() {
		for(auto &s : _searchers)
		{
			s.stopSearch();
		}
	}

	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;
	void showLine(){ if(_searchers.size() >0) {_searchers[0].showLine();}}
	SearchResult manageNewSearch(timeManagement & tm);
	Position& getPosition();
	void setUOI( UciOutput::type UOI ); // todo remove??
	SearchParameters& getSearchParameters() {return _sp;};
	bool setNnue(bool use, std::string path);

private:
	std::vector<Searcher> _searchers;


	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------
	std::unique_ptr<UciOutput> _UOI;
    NNUE _nnue;
	Position _pos;

	SearchLimits& _sl; // todo limits belong to threads
	SearchTimer& _st;
	transpositionTable& _tt;
	Game _game;
	SearchParameters _sp;
    rootMovesToBeSearched _rootMovesToBeSearched;
	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------
    SearchResult _go(timeManagement & tm, /*int depth = 1,*/ Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline pvToBeFollowed = PVline());
	SearchResult _manageQsearch(timeManagement & tm);
    void _generateRootMovesList(const std::list<Move>& ml);
    void _filterRootMovesByTablebase();
    void _waitStopPondering() const;
    Move _getPonderMoveFromBook(const Move& bookMove );
    Move _getPonderMoveFromHash( const Move& bookMove );
};

#endif 
