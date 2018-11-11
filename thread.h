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

#ifndef THREAD_H_
#define THREAD_H_

#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "position.h"
#include "timeManagement.h"

class Searc;
class SearchLimits;
class SearchTimer;


class Game
{
public:
	struct GamePosition
	{
		uint64_t key;
		Move m;
		PVline PV;
		Score alpha;
		Score beta;
		unsigned int depth;
	};

	void CreateNewGame();
	void insertNewMoves(Position &pos);
	void savePV(PVline PV,unsigned int depth, Score alpha, Score beta);
	void printGamesInfo();

	bool isNewGame(const Position &pos) const;
	bool isPonderRight() const;
	GamePosition getNewSearchParameters() const;

private:
	std::vector<GamePosition> _positions;
public:

};

class my_thread
{
private:
	my_thread();
	~my_thread();

	static my_thread * pInstance;
	static std::mutex _mutex;

	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
	Game _game;
	timeManagement _timeMan;

	std::unique_ptr<UciOutput> _UOI;

	volatile static bool _quit;
	volatile static bool _startThink;

	long long _lastHasfullMessage;


	std::thread _timer;
	std::thread _searcher;
	std::mutex _searchMutex;

	std::condition_variable _searchCond;
	std::condition_variable _timerCond;

	void _initThreads();

	void _timerThread();
	void _searchThread();
	void _manageNewSearch();
	Move _getPonderMoveFromHash( const Move bestMove );
	Move _getPonderMoveFromBook( const Move bookMove );
	void _waitStopPondering() const;
	void _printTimeDependentOutput( long long int time );



public :

	static my_thread& getInstance()
	{
		if (!pInstance)
		{
			std::lock_guard<std::mutex> lock(_mutex);

			if (!pInstance)
			{
				my_thread * temp = new my_thread;
			    pInstance = temp;
			}
		}

		return *pInstance;
	}

	void quitThreads();
	void startThinking(Position * p, SearchLimits& l);
	void stopPonder();
	void stopThinking();
	void ponderHit();
	timeManagement& getTimeMan(){ return _timeMan;}



};




#endif /* THREAD_H_ */
