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
#ifndef SEARCH_DATA_H_
#define SEARCH_DATA_H_

#include "move.h"
#include "history.h"

class SearchData
{
private:
	static const unsigned int STORY_LENGTH = 800;
	
	struct Sd
	{
		static const unsigned int KILLER_SIZE = 2;
		Move excludeMove = Move::NOMOVE;
		bool skipNullMove = false;
		Move killers[KILLER_SIZE] = { Move::NOMOVE, Move::NOMOVE};
		Score staticEval = 0;
		bool inCheck = false;
	} _story[STORY_LENGTH];
	
	CounterMove _counterMoves;
	CaptureHistory _captureHistory;
	History _history;
public:

	void clearKillers(unsigned int ply);
	void cleanData(void);
	void saveKillers(unsigned int ply, const Move& m);
	void setExcludedMove(unsigned int ply, const Move& m);
	const Move& getExcludedMove(unsigned int ply);
	
	History& getHistory(){return _history;}
	CaptureHistory& getCaptureHistory(){return _captureHistory;}
	CounterMove& getCounterMove(){return _counterMoves;}
	const History& getHistory()const {return _history;}
	const CaptureHistory& getCaptureHistory()const {return _captureHistory;}
	const CounterMove& getCounterMove()const {return _counterMoves;}
	const Move& getKillers(unsigned int ply, unsigned int n) const {
		assert(ply < STORY_LENGTH);
		assert(killer < KILLER_SIZE);
		return _story[ply].killers[n];
	}
	bool skipNullMove(unsigned int ply) const {
		assert(ply < STORY_LENGTH); 
		return _story[ply].skipNullMove;
	}
	
	void setSkipNullMove(unsigned int ply, bool val) {
		assert(ply < STORY_LENGTH); 
		_story[ply].skipNullMove = val;
	}
	
	void setStaticEval(unsigned int ply, Score s) {
		assert(ply < STORY_LENGTH);
		_story[ply].staticEval = s;
	}
	
	Score getStaticEval(unsigned int ply) const {
		assert(ply < STORY_LENGTH);
		return _story[ply].staticEval;
	}
	
	void setInCheck(unsigned int ply, bool b) {
		assert(ply < STORY_LENGTH);
		_story[ply].inCheck = b;
	}
	
	bool getInCheck(unsigned int ply) const {
		assert(ply < STORY_LENGTH);
		return _story[ply].inCheck;
	}
};

extern const SearchData _defaultSearchData;

#endif