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

#include "command.h"
#include "position.h"
#include "search.h"
#include "searchLimits.h"
#include "searchTimer.h"


class timeManagement
{
public:
	enum searchState
	{
		wait,
		infiniteSearch,
		fixedTimeSearch,
		standardSearchPonder,
		standardSearch,
		standardSearchExtendedTime,
		searchFinished
	};

	timeManagement( SearchLimits& limits ):_limits(limits){}

	void initNewSearch( SearchLimits& lim, const Position::eNextMove nm );

	void notifyIterationHasBeenFinished();
	void notifyFailLow();
	void notifyFailOver();

	unsigned int getResolution() const;

	void stop();
	
	bool isSearchFinished() const;
	
	void chooseSearchType( enum searchState s);
	bool stateMachineStep( const long long int time, const unsigned long long visitedNodes );
	

private:
	void _resetSearchvariables();
	void _clearIdLoopIterationFinished();
	bool _isSearchInFailLowOverState() const;
	bool _hasFirstIterationFinished() const;
	bool _isIdLoopIterationFinished() const;

	bool _firstIterationFinished;
	bool _idLoopIterationFinished;
	bool _idLoopFailLow;
	bool _idLoopFailOver;
	
	long long _allocatedTime;
	long long _minSearchTime;
	long long _maxAllocatedTime;
	unsigned int _resolution;

	bool _stop;

	searchState _searchState;
	const SearchLimits& _limits;

};


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
private:
	std::vector<GamePosition> positions;
public:

	void CreateNewGame(void)
	{
		positions.clear();
	}

	void insertNewMoves(Position &pos)
	{
		unsigned int actualPosition = positions.size();
		for(unsigned int i = actualPosition; i < pos.getStateSize(); i++)// todo usare iteratore dello stato
		{
			GamePosition p;
			p.key = pos.getState(i).key;
			p.m =  pos.getState(i).currentMove;
			positions.push_back(p);
		}
	}

	void savePV(PVline PV,unsigned int depth, Score alpha, Score beta)
	{
		positions.back().PV = PV;
		positions.back().depth = depth;
		positions.back().alpha = alpha;
		positions.back().beta = beta;
	}


	void printGamesInfo()
	{
		for(auto p : positions)
		{
			if( p.m != NOMOVE)
			{
				std::cout<<"Move: "<<displayUci(p.m)<<"  PV:";
				for( auto m : p.PV )
				{
					std::cout<<displayUci(m)<<" ";
				}

			}
			std::cout<<std::endl;
		}

	}

	~Game()
	{
		//printGamesInfo();
	}
	bool isNewGame(Position &pos)
	{
		if( positions.size() == 0 || pos.getStateSize() < positions.size())
		{
			//printGamesInfo();
			return true;
		}

		unsigned int n = 0;
		for(auto p : positions)
		{
			if(pos.getState(n).key != p.key)
			{
				//printGamesInfo();
				return true;
			}
			n++;

		}
		return false;
	}

	bool isPonderRight(void)
	{
		if( positions.size() > 2)
		{
			GamePosition previous =*(positions.end()-3);
			if(previous.PV.size()>=1 && previous.PV.getMove(1) == positions.back().m)
			{
				return true;
			}

		}
		return false;
	}

	GamePosition getNewSearchParameters(void)
	{
		GamePosition previous =*(positions.end()-3);
		return previous;
	}


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

	timeManagement timeMan;

};




#endif /* THREAD_H_ */
