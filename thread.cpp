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

#include "book.h"
#include "command.h"
#include "movegen.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"

void timeManagerInit(const Position& pos, searchLimits& lim, timeManagementStruct& timeMan)
{
	timeMan.FirstIterationFinished = false;
	if((!lim.btime && !lim.wtime) && !lim.moveTime)
	{
		lim.infinite = true;
		timeMan.resolution = 100;
	}
	else if(lim.moveTime)
	{
		timeMan.maxAllocatedTime = lim.moveTime+1;
		timeMan.allocatedTime = lim.moveTime;
		timeMan.minSearchTime =lim.moveTime;
		timeMan.resolution = std::min((long long int)100, timeMan.allocatedTime/100);
	}
	else
	{
		unsigned int time;
		unsigned int increment;

		if(pos.getNextTurn())
		{
			time = lim.btime;
			increment = lim.binc;
		}
		else
		{
			time = lim.wtime;
			increment = lim.winc;
		}



		if(lim.movesToGo > 0)
		{
			timeMan.allocatedTime = time / lim.movesToGo;
			timeMan.maxAllocatedTime = 10.0 * timeMan.allocatedTime;
			timeMan.maxAllocatedTime = std::min(10.0 * timeMan.allocatedTime, 0.8 * time);
			timeMan.maxAllocatedTime = std::max(timeMan.maxAllocatedTime,timeMan.allocatedTime);
		}
		else
		{
			timeMan.allocatedTime = time / 35.0 + increment * 0.98;
			timeMan.maxAllocatedTime= 10 * timeMan.allocatedTime;
		}

		timeMan.resolution = std::min((long long int)100, timeMan.allocatedTime/100);
		timeMan.allocatedTime = std::min( (long long int)timeMan.allocatedTime ,(long long int)( time - 2 * timeMan.resolution));
		timeMan.minSearchTime = timeMan.allocatedTime * 0.3;
		long long buffer = std::max( 2 * timeMan.resolution, 200u );
		timeMan.allocatedTime = std::min( (long long int)timeMan.allocatedTime ,(long long int)(time - buffer ));
		timeMan.maxAllocatedTime = std::min( (long long int)timeMan.maxAllocatedTime ,(long long int)(time - buffer ));
	}



	timeMan.singularRootMoveCount = 0;
	timeMan.idLoopIterationFinished = false;
	timeMan.idLoopAlpha = false;
	timeMan.idLoopBeta = false;

}


volatile bool my_thread::quit = false;
volatile bool my_thread::startThink = false;


timeManagementStruct my_thread::timeMan;


long long my_thread::lastHasfullMessage;


my_thread * my_thread::pInstance;
std::mutex  my_thread::_mutex;


void my_thread::timerThread()
{
	std::mutex mutex;
	while (!quit)
	{

		std::unique_lock<std::mutex> lk(mutex);

		timerCond.wait(lk, [&]{return (startThink && src.isNotStopped() ) || quit;} );
		if (!quit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(timeMan.resolution));
			long long int time = st.getClockTime();

			if(timeMan.idLoopIterationFinished)
			{
				timeMan.FirstIterationFinished = true;
			}
			if(src.isNotStopped() && time >= timeMan.allocatedTime && ( timeMan.idLoopAlpha || timeMan.idLoopBeta ) )
			{
				timeMan.allocatedTime = timeMan.maxAllocatedTime;
			}
			if(src.isNotStopped() && timeMan.maxAllocatedTime == timeMan.allocatedTime /*&& time >= timeMan.allocatedTime */&& ( timeMan.idLoopIterationFinished ) && !(src.limits.infinite || src.limits.ponder) )
			{
				src.stopSearch();
			}

			if(src.isNotStopped() && time >= timeMan.allocatedTime && timeMan.FirstIterationFinished && !(src.limits.infinite || src.limits.ponder))
			{
				src.stopSearch();
			}
			if(src.isNotStopped() && timeMan.idLoopIterationFinished && time >= timeMan.minSearchTime && time >= timeMan.allocatedTime*0.7 && !(src.limits.infinite || src.limits.ponder))
			{
				src.stopSearch();
			}
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			if(time - lastHasfullMessage > 1000)
			{
				lastHasfullMessage = time;

				_UOI->printGeneralInfo( transpositionTable::getInstance().getFullness(), src.getTbHits(), src.getVisitedNodes(), time);

				if( uciParameters::showCurrentLine )
				{
					src.showLine();
				}

			}

#endif

			if(timeMan.idLoopIterationFinished && time >= timeMan.minSearchTime && !(src.limits.infinite || src.limits.ponder))
			{
				if(timeMan.singularRootMoveCount >=20)
				{
					src.stopSearch();
				}
			}


			if(src.limits.nodes && timeMan.FirstIterationFinished && src.getVisitedNodes() > src.limits.nodes)
			{
				src.stopSearch();
			}
			if(src.limits.moveTime && timeMan.FirstIterationFinished && time>=src.limits.moveTime)
			{
				src.stopSearch();
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
			src.resetStopCondition();
			st.resetStartTimers();
			timerCond.notify_one();
			src.resetStopCondition();
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
		PVline PV( 1, Move(0) );
		_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
		
		waitStopPondering();

		_UOI->printBestMove( Move(0) );

		return;
	}
	
	if( legalMoves == 1 && !src.limits.infinite)
	{
		
		Move bestMove = mg.getMoveFromMoveList(0);
		
		PVline PV( 1, bestMove );
		_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
		
		waitStopPondering();
		
		Move ponderMove = getPonderMoveFromHash( bestMove );
		
		_UOI->printBestMove(bestMove, ponderMove);

		return;

	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if( uciParameters::useOwnBook && !src.limits.infinite )
	{
		PolyglotBook pol;
		Move bookM = pol.probe(src.pos, uciParameters::bestMoveBook);
		if(bookM.packed)
		{
			PVline PV( 1, bookM );
			
			_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
			
			waitStopPondering();
			
			Move ponderMove = getPonderMoveFromBook( bookM );
			
			_UOI->printBestMove(bookM, ponderMove);
			
			return;
		}
	}
	
/*	if( game.isPonderRight() )
	{
		Game::GamePosition gp = game.getNewSearchParameters();

		PVline newPV;
		std::copy( gp.PV.begin(), gp.PV.end(), std::back_inserter( newPV ) );
		
		newPV.resize(gp.depth/2 + 1);
		newPV.pop_front();
		newPV.pop_front();
		res = src.startThinking( gp.depth/2 + 1, gp.alpha, gp.beta, newPV );
	}
	else
*/	
	startThinkResult res = src.startThinking( );
	
	PVline PV = res.PV;

	waitStopPondering();

	//-----------------------------
	// print out the choosen line
	//-----------------------------

	_UOI->printGeneralInfo( transpositionTable::getInstance().getFullness(), src.getTbHits(), src.getVisitedNodes(), st.getElapsedTime());
	
	Move bestMove = PV.getMove(0);
	Move ponderMove = PV.getMove(1);
	if( ponderMove == NOMOVE )
	{
		ponderMove = getPonderMoveFromHash( bestMove );
	}
	
	_UOI->printBestMove( bestMove, ponderMove );

	game.savePV(PV, res.depth, res.alpha, res.beta);

}



void my_thread::initThreads()
{
	timer = std::thread(&my_thread::timerThread, this);
	searcher = std::thread(&my_thread::searchThread, this);
	src.stopSearch();
}

void my_thread::quitThreads()
{
	quit = true;
	searchCond.notify_one();
	timerCond.notify_one();
	timer.join();
	searcher.join();
}

Move my_thread::getPonderMoveFromHash(const Move bestMove )
{
	Move ponderMove(0);
	src.pos.doMove( bestMove );
	
	const ttEntry* const tte = transpositionTable::getInstance().probe(src.pos.getKey());
	
	Move m;
	m.packed = tte->getPackedMove();
	if( src.pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	src.pos.undoMove();
	
	return ponderMove;
}

Move my_thread::getPonderMoveFromBook(const Move bookMove )
{
	Move ponderMove(0);
	src.pos.doMove( bookMove );
	PolyglotBook pol;
	Move m = pol.probe(src.pos, uciParameters::bestMoveBook);
	
	if( src.pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	src.pos.undoMove();
	
	return ponderMove;
}

void my_thread::waitStopPondering() const
{
	while(src.limits.ponder){}
}


