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
#include "command.h"




void timeManagerInit(const Position& pos, searchLimits& lim, timeManagementStruct& timeMan)
{
	timeMan.FirstIterationFinished = false;
	if((!lim.btime && !lim.wtime) && !lim.moveTime)
	{
		lim.infinite = true;
	}
	if(lim.moveTime)
	{
		timeMan.maxAllocatedTime = lim.moveTime;
		timeMan.allocatedTime = lim.moveTime;
		timeMan.minSearchTime =lim.moveTime;
		timeMan.resolution = std::min((long long int)100, timeMan.allocatedTime/100);
	}
	else
	{

		if(pos.getNextTurn())
		{
			if(lim.movesToGo > 0)
			{
				timeMan.allocatedTime = lim.btime/lim.movesToGo;
				timeMan.maxAllocatedTime= 2 * timeMan.allocatedTime;
			}else
			{
				timeMan.allocatedTime = lim.btime/40.0+lim.binc*0.8;
				timeMan.maxAllocatedTime= 2 * timeMan.allocatedTime;
			}

			timeMan.resolution = std::min((long long int)100, timeMan.allocatedTime/100);
			timeMan.allocatedTime = std::min( (long long int)timeMan.allocatedTime ,(long long int)(lim.btime - 2 * timeMan.resolution));
			timeMan.minSearchTime =timeMan.allocatedTime*0.3;
			timeMan.maxAllocatedTime = std::min( (long long int)timeMan.maxAllocatedTime ,(long long int)(lim.btime - 2 * timeMan.resolution));
		}
		else
		{
			if(lim.movesToGo > 0)
			{
				timeMan.allocatedTime = lim.wtime/lim.movesToGo;
				timeMan.maxAllocatedTime= 2 * timeMan.allocatedTime;
			}else
			{
				timeMan.allocatedTime = lim.wtime/40.0+lim.winc*0.8;
				timeMan.maxAllocatedTime= 2 * timeMan.allocatedTime;
			}
			timeMan.resolution = std::min((long long int)100, timeMan.allocatedTime/100);
			timeMan.allocatedTime = std::min( (long long int)timeMan.allocatedTime ,(long long int)(lim.wtime - 2 * timeMan.resolution));
			timeMan.minSearchTime =timeMan.allocatedTime*0.3;
			timeMan.maxAllocatedTime = std::min( (long long int)timeMan.maxAllocatedTime ,(long long int)(lim.wtime - 2 * timeMan.resolution));
		}



	}

	if(lim.infinite)
	{
		timeMan.resolution = 100;
	}
	timeMan.singularRootMoveCount = 0;
	timeMan.idLoopIterationFinished = false;
	timeMan.idLoopAlpha = false;
	timeMan.idLoopBeta = false;

	//sync_cout<<"info debug resolution "<<timeMan.resolution<<sync_endl;
	//sync_cout<<"info debug allocatedTime "<<timeMan.allocatedTime<<sync_endl;

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
	unsigned long long oldThbits = 0;
	std::mutex mutex;
	while (!quit)
	{

		std::unique_lock<std::mutex> lk(mutex);

		timerCond.wait(lk, [&]{return (startThink && src.stop==false ) || quit;} );
		if (!quit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(timeMan.resolution));
			long long int time = src.getElapsedTime();

			if(timeMan.idLoopIterationFinished)
			{
				timeMan.FirstIterationFinished = true;
			}
			if(!src.stop &&time >= timeMan.allocatedTime && ( timeMan.idLoopAlpha || timeMan.idLoopBeta ) )
			{
				timeMan.allocatedTime = timeMan.maxAllocatedTime;
				//sync_cout<<"info debug EXTEND TIME: "<<timeMan.allocatedTime<<sync_endl;
			}
			if(!src.stop && timeMan.maxAllocatedTime == timeMan.allocatedTime && ( timeMan.idLoopIterationFinished ) && !(src.limits.infinite || src.limits.ponder) )
			{
				src.stop = true;
				//sync_cout<<"info debug FINISHED ITERATION IN EXTEND TIME "<<time<<sync_endl;
			}

			if(!src.stop && time >= timeMan.allocatedTime && timeMan.FirstIterationFinished && !(src.limits.infinite || src.limits.ponder))
			{
				//sync_cout<<"info debug TIME EXPIRED "<<time<< " >= "<<timeMan.allocatedTime<<sync_endl;
				src.stop = true;
			}
			if(!src.stop && timeMan.idLoopIterationFinished && time >= timeMan.allocatedTime*0.7 && !(src.limits.infinite || src.limits.ponder))
			{
				//sync_cout<<"info debug STOP BECAUSE WE WILL PROBABLY NOT BE ABLE TO FINISH THE NEXT ITERATION "<<time<<":"<<timeMan.allocatedTime<<sync_endl;
				src.stop = true;
			}
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			if(time - lastHasfullMessage > 1000)
			{

				lastHasfullMessage = time;
				unsigned int fullness = TT.getFullness();
				if( fullness != oldFullness)
				{
					sync_cout<<"info hashfull " << fullness << sync_endl;
				}
				if(src.showCurrentLine)
				{
					src.showLine = true;
				}
				oldFullness = fullness;

				unsigned long long int thbits = src.tbHits;
				if( thbits != oldThbits)
				{
					sync_cout<<"info tbhits " << thbits << sync_endl;
				}
				oldThbits = thbits;

			}

#endif



			if(timeMan.idLoopIterationFinished && time >= timeMan.minSearchTime && !(src.limits.infinite || src.limits.ponder))
			{
				if(timeMan.singularRootMoveCount >=20)
				{
					//sync_cout<<"info debug EARLY STOP"<<sync_endl;
					src.stop = true;
				}
			}


			if(src.limits.nodes && timeMan.FirstIterationFinished && src.getVisitedNodes() > src.limits.nodes)
			{
				src.stop = true;
			}
			if(src.limits.moveTime && timeMan.FirstIterationFinished && time>=src.limits.moveTime)
			{
				src.stop = true;
			}
			if(src.stop)
			{
				sync_cout<<"info hashfull " << TT.getFullness() << sync_endl;
				sync_cout<<"info tbhits " << src.tbHits << sync_endl;
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
			src.resetStartTime();
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

	if( game.isNewGame(src.pos))
	{
		game.CreateNewGame();

	}
	game.insertNewMoves(src.pos);


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
			sync_cout << "info pv " << displayUci(m) << sync_endl;
			while(src.limits.ponder){}
			sync_cout << "bestmove " << displayUci(m);

			src.pos.doMove(m);
			const ttEntry* const tte = TT.probe(src.pos.getKey());
			src.pos.undoMove();

			if(tte && ( m.packed = (tte->getPackedMove())))
			{
				std::cout<<" ponder "<<displayUci(m);
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
			sync_cout << "info pv " << displayUci(bookM) << sync_endl;
			while( (src.limits.infinite && !src.stop) || src.limits.ponder){}
			sync_cout<<"bestmove "<< displayUci(bookM) << sync_endl;
			return;
		}
	}
	startThinkResult res;
	/*if(game.isPonderRight())
	{
		sync_cout<<"info debug PONDER RIGHT"<<sync_endl;
	}else
	{
		sync_cout<<"info debug PONDER WRONG"<<sync_endl;
	}*/
	res = src.startThinking();



	std::list<Move> PV = res.PV;

	while(src.limits.ponder)
	{
	}

	//-----------------------------
	// print out the choosen line
	//-----------------------------
	sync_cout << "bestmove " << displayUci( PV.front() );

	if(PV.size() > 1)
	{
		std::list<Move>::iterator it = PV.begin();
		std::advance(it, 1);
		std::cout<<" ponder "<<displayUci(*it);
	}
	else
	{
		src.pos.doMove( PV.front() );
		const ttEntry* const tte = TT.probe(src.pos.getKey());
		src.pos.undoMove();

		Move m;
		if(tte && ( m.packed = tte->getPackedMove()))
		{
			std::cout << " ponder " << displayUci(m);
		}

	}

	std::cout<<sync_endl;
	game.savePV(PV, res.depth, res.alpha, res.beta);


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


