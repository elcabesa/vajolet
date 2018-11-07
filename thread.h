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


class timeManagementStruct
{
public:
	volatile long long allocatedTime;
	volatile long long minSearchTime;
	volatile long long maxAllocatedTime;
	volatile unsigned int resolution;
	
	volatile bool extendedTime;
	
	void resetIterationInformations();
	void notifyIterationHasBeenFinished();
	void notifyFailLow();
	void notifyFailOver();
	void clearIdLoopIterationFinished();
	
	bool isSearchInFailLowOverState() const;
	bool hasFirstIterationFinished() const;
	bool isIdLoopIterationFinished() const;
	

private:
	bool FirstIterationFinished;
	bool idLoopIterationFinished;
	bool idLoopFailLow;
	bool idLoopFailOver;

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

	SearchLimits limits; // todo limits belong to threads

	std::unique_ptr<UciOutput> _UOI;
	my_thread():src(st, limits)
	{
		_UOI = UciOutput::create();
		initThreads();
		game.CreateNewGame();
	};

	static my_thread * pInstance;


	volatile static bool quit;
	volatile static bool startThink;

	SearchTimer st;
	std::thread timer;
	std::thread searcher;
	std::mutex searchMutex;
	std::condition_variable searchCond;
	std::condition_variable timerCond;
	Search src;

	static long long lastHasfullMessage;

	Game game;
	void initThreads();

	void timerThread();
	void searchThread();
	void manageNewSearch();
	Move getPonderMoveFromHash(const Move bestMove );
	Move getPonderMoveFromBook(const Move bookMove );
	void waitStopPondering() const;
public :


	void quitThreads();

	static std::mutex  _mutex;

	static my_thread* getInstance()
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

		return pInstance;
	}

	static timeManagementStruct timeMan;

	~my_thread()
	{
		quitThreads();
	}
	void startThinking(Position * p, SearchLimits& l)
	{
		src.stopSearch();
		lastHasfullMessage = 0;

		while(startThink){}

		if(!startThink)
		{
			std::lock_guard<std::mutex> lk(searchMutex);
			limits = l;
			src.pos = *p;
			startThink = true;
			searchCond.notify_one();
		}
	}

	void stopPonder(){ limits.ponder = false;}

	void stopThinking()
	{
		src.stopSearch();
		stopPonder();
	}

	void ponderHit()
	{
		st.resetPonderTimer();
		stopPonder();
	}

};




#endif /* THREAD_H_ */
