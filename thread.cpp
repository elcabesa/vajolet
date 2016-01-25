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

#include "thread.h"
#include "io.h"
#include "search.h"
#include "movegen.h"
#include "book.h"




void timeManagerInit(const Position& pos, searchLimits& lim, timeManagementStruct& timeMan)
{
	if((!lim.btime && !lim.wtime) && !lim.moveTime)
	{
		lim.infinite = true;
	}
	if(lim.moveTime)
	{
		timeMan.allocatedTime = lim.moveTime;
		timeMan.maxSearchTime = lim.moveTime;
		timeMan.minSearchTime = lim.moveTime;
		timeMan.resolution = std::min((unsigned long int)100, timeMan.allocatedTime/100);
	}
	else
	{
		if(pos.getNextTurn())
		{
			if(lim.movesToGo > 0)
			{
				timeMan.allocatedTime = (long unsigned int)std::min((lim.btime*4.0)/lim.movesToGo, lim.btime*0.8);
				timeMan.maxSearchTime = timeMan.allocatedTime;
			}else
			{
				timeMan.allocatedTime = (long unsigned int)((float)(lim.btime)/10.0);
				timeMan.maxSearchTime = timeMan.allocatedTime;
			}
		}
		else
		{
			if(lim.movesToGo > 0)
			{
				timeMan.allocatedTime = (long unsigned int)std::min((lim.wtime*4.0)/lim.movesToGo, lim.wtime*0.8);
				timeMan.maxSearchTime = timeMan.allocatedTime;
			}else
			{
				timeMan.allocatedTime = (long unsigned int)((float)(lim.wtime)/10.0);
				timeMan.maxSearchTime = timeMan.allocatedTime;
			}
		}

		timeMan.minSearchTime = (long unsigned int)(timeMan.allocatedTime*0.1);
		timeMan.resolution = std::min((unsigned long int)100, timeMan.allocatedTime/100);
	}

	if(lim.infinite)
	{
		timeMan.resolution = 100;
	}
	timeMan.singularRootMoveCount = 0;
	timeMan.idLoopIterationFinished = false;

}


volatile bool my_thread::quit = false;
volatile bool my_thread::startThink = false;


timeManagementStruct my_thread::timeMan;


long long my_thread::lastHasfullMessage;


my_thread * my_thread::pInstance;
std::mutex  my_thread::_mutex;


void my_thread::timerThread()
{
	unsigned int oldFullness = 0;
	std::mutex mutex;
	while (!quit)
	{

		std::unique_lock<std::mutex> lk(mutex);

		timerCond.wait(lk, [&]{return (startThink && src.stop==false ) || quit;} );
		if (!quit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(timeMan.resolution));
			long long int time = src.getElapsedTime();
			if(time >= timeMan.allocatedTime && !(src.limits.infinite || src.limits.ponder))
			{
				src.stop = true;
			}
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			if(time - lastHasfullMessage > 1000)
			{
				unsigned int fullness = TT.getFullness();
				lastHasfullMessage = time;
				if( fullness != oldFullness)
				{
					sync_cout<<"info hashfull " << fullness << sync_endl;
				}
				if(src.showCurrentLine)
				{
					src.showLine = true;
				}
				oldFullness = fullness;
			}
#endif

			if(timeMan.idLoopIterationFinished && time >= timeMan.allocatedTime*0.7 && !(src.limits.infinite || src.limits.ponder))
			{
				src.stop = true;
			}

			if(timeMan.idLoopIterationFinished && time >= timeMan.minSearchTime && !(src.limits.infinite || src.limits.ponder))
			{
				if(timeMan.singularRootMoveCount >=1)
				{
					src.stop = true;
				}
			}


			if(src.limits.nodes && src.getVisitedNodes() > src.limits.nodes)
			{
				src.stop = true;
			}
			if(src.limits.moveTime && time>=src.limits.moveTime)
			{
				src.stop = true;
			}
			timeMan.idLoopIterationFinished = false;
		}
		lk.unlock();
	}
}

void my_thread::searchThread()
{
	std::mutex mutex;
	while (!quit)
	{

		std::unique_lock<std::mutex> lk(mutex);
		searchCond.wait(lk, [&]{return startThink||quit;} );
		if(!quit)
		{
			timeManagerInit(src.pos, src.limits, timeMan);
			src.stop = false;
			timerCond.notify_one();
			src.stop = false;
			manageNewSearch();
			startThink = false;
		}
		lk.unlock();
	}
}

void my_thread::manageNewSearch()
{
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/

	Movegen mg(src.pos);
	unsigned int legalMoves = mg.getNumberOfLegalMoves();

	if(legalMoves == 0)
	{
		while((src.limits.infinite && !src.stop) || src.limits.ponder){}

		sync_cout<<"info depth 0 score cp 0"<<sync_endl;
		sync_cout<<"bestmove 0000"<<sync_endl;

		return;
	}
	else if( legalMoves == 1 )
	{
		if(!src.limits.infinite)
		{
			Move m = mg.getFirstMove();
			sync_cout << "info pv " << src.pos.displayUci(m) << sync_endl;
			while(src.limits.ponder){}
			sync_cout << "bestmove " << src.pos.displayUci(m);

			src.pos.doMove(m);
			const ttEntry* const tte = TT.probe(src.pos.getKey());
			src.pos.undoMove();

			if(tte && ( m.packed = (tte->getPackedMove())))
			{
				std::cout<<" ponder "<<src.pos.displayUci(m);
			}
			std::cout<<sync_endl;

			return;
		}
	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if(search::useOwnBook && !src.limits.infinite )
	{
		PolyglotBook pol;
		Move bookM = pol.probe(src.pos, search::bestMoveBook);
		if(bookM.packed)
		{
			sync_cout << "info pv " << src.pos.displayUci(bookM) << sync_endl;
			while( (src.limits.infinite && !src.stop) || src.limits.ponder){}
			sync_cout<<"bestmove "<< src.pos.displayUci(bookM) << sync_endl;
			return;
		}
	}
	std::list<Move> PV = src.startThinking();

	while(src.limits.ponder)
	{
	}

	//-----------------------------
	// print out the choosen line
	//-----------------------------
	sync_cout << "bestmove " << src.pos.displayUci( PV.front() );

	if(PV.size() > 1)
	{
		std::list<Move>::iterator it = PV.begin();
		std::advance(it, 1);
		std::cout<<" ponder "<<src.pos.displayUci(*it);
	}
	else
	{
		src.pos.doMove( PV.front() );
		const ttEntry* const tte = TT.probe(src.pos.getKey());
		src.pos.undoMove();

		Move m;
		if(tte && ( m.packed = tte->getPackedMove()))
		{
			std::cout << " ponder " << src.pos.displayUci(m);
		}

	}
	std::cout<<sync_endl;


}



void my_thread::initThreads()
{
	timer = std::thread(&my_thread::timerThread, this);
	searcher = std::thread(&my_thread::searchThread, this);
	src.stop = true;
}

void my_thread::quitThreads()
{
	quit = true;
	searchCond.notify_one();
	timerCond.notify_one();
	timer.join();
	searcher.join();
}


