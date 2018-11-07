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

#include "io.h"

void timeManagement::_resetSearchvariables()
{
	_firstIterationFinished = false;
	_idLoopIterationFinished = false;
	_idLoopFailLow = false;
	_idLoopFailOver = false;	
	_stop = false;
}

void timeManagement::notifyIterationHasBeenFinished()
{
	_firstIterationFinished = true;
	_idLoopIterationFinished = true;
	_idLoopFailLow = false;
	_idLoopFailOver = false;	
}

void timeManagement::notifyFailLow()
{
	_idLoopFailLow = true;
	_idLoopFailOver = false;	
}

void timeManagement::notifyFailOver()
{
	_idLoopFailLow = false;
	_idLoopFailOver = true;	
}

void timeManagement::_clearIdLoopIterationFinished()
{
	_idLoopIterationFinished = false;
}

bool timeManagement::_isSearchInFailLowOverState() const
{
	return _idLoopFailLow || _idLoopFailOver;
}

bool timeManagement::_hasFirstIterationFinished() const
{
	return _firstIterationFinished;
}

bool timeManagement::_isIdLoopIterationFinished() const
{
	return _idLoopIterationFinished;
}


void timeManagement::stop()
{
	_stop = true;
}

bool timeManagement::isSearchFinished() const
{
	return _searchState == searchFinished;
}


void timeManagement::chooseSearchType( enum searchState s)
{
	_searchState = s;
}

void timeManagement::initNewSearch( SearchLimits& lim )
{
	if((!lim.btime && !lim.wtime) && !lim.moveTime)
	{
		lim.infinite = true;
		resolution = 100;
		chooseSearchType( timeManagement::infiniteSearch);
	}
	else if(lim.moveTime)
	{
		maxAllocatedTime = lim.moveTime;
		allocatedTime = lim.moveTime;
		minSearchTime = lim.moveTime;
		resolution = std::min((long long int)100, allocatedTime / 100 );
		chooseSearchType( timeManagement::fixedTimeSearch);
	}
	else
	{
		unsigned int time;
		unsigned int increment;

		if(_src.pos.getNextTurn())
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
			allocatedTime = time / lim.movesToGo;
			maxAllocatedTime = 10.0 * allocatedTime;
			maxAllocatedTime = std::min( 10.0 * allocatedTime, 0.8 * time);
			maxAllocatedTime = std::max( maxAllocatedTime, allocatedTime );
		}
		else
		{
			allocatedTime = time / 35.0 + increment * 0.98;
			maxAllocatedTime = 10 * allocatedTime;
		}

		resolution = std::min( (long long int)100, allocatedTime / 100 );
		allocatedTime = std::min( (long long int)allocatedTime ,(long long int)( time - 2 * resolution ) );
		minSearchTime = allocatedTime * 0.3;
		long long buffer = std::max( 2 * resolution, 200u );
		allocatedTime = std::min( (long long int)allocatedTime, (long long int)( time - buffer ) );
		maxAllocatedTime = std::min( (long long int)maxAllocatedTime, (long long int)( time - buffer ) );

		chooseSearchType( _limits.ponder == true ? timeManagement::standardSearchPonder : timeManagement::standardSearch );
	}

	_resetSearchvariables();

}

void timeManagement::stateMachineStep( long long int time )
{
	switch( _searchState )
	{
	case wait:
		//sync_cout<<"wait"<<sync_endl;
		break;

	case infiniteSearch:

		//sync_cout<<"infiniteSearch"<<sync_endl;
		if(
				_stop
				|| ( _limits.nodes && _hasFirstIterationFinished() && _src.getVisitedNodes() > _limits.nodes )
				|| ( _limits.moveTime && _hasFirstIterationFinished() && time >= _limits.moveTime )
		)
		{
			_searchState = searchFinished;
			_src.stopSearch();
		}
		break;

	case fixedTimeSearch:

		//sync_cout<<"fixedTimeSearch"<<sync_endl;
		if(
				_stop
				|| ( time >= allocatedTime && _hasFirstIterationFinished() )
		)
		{
			_searchState = searchFinished;
			_src.stopSearch();
		}
		break;

	case standardSearch:

		//sync_cout<<"standardSearch"<<sync_endl;
		if(
				_stop
				|| ( time >= allocatedTime && _hasFirstIterationFinished() )
				|| ( _isIdLoopIterationFinished() && time >= minSearchTime && time >= allocatedTime * 0.7 )
		)
		{
			_searchState = searchFinished;
			_src.stopSearch();
		}
		else if( time >= allocatedTime && _isSearchInFailLowOverState() )
		{
			_searchState = standardSearchExtendedTime;
		}
		break;

	case standardSearchPonder:
		//sync_cout<<"standardSearchPonder"<<sync_endl;
		if( _stop )
		{
			_searchState = searchFinished;
			_src.stopSearch();
		}
		else if( _limits.ponder == false )
		{
			_searchState = standardSearch;
		}
		break;

	case standardSearchExtendedTime:
		//sync_cout<<"standardSearchExtendedTime"<<sync_endl;
		if(
				_stop
				|| ( time >= maxAllocatedTime && _hasFirstIterationFinished() )
				|| _isIdLoopIterationFinished()
		)
		{
			_searchState = searchFinished;
			_src.stopSearch();
		}
		break;

	case searchFinished:
		//sync_cout<<"searchFinished"<<sync_endl;
	default:
		break;
	}

	_clearIdLoopIterationFinished();
}


volatile bool my_thread::quit = false;
volatile bool my_thread::startThink = false;

my_thread * my_thread::pInstance;
std::mutex  my_thread::_mutex;

void my_thread::printTimeDependentOutput(long long int time) {

	if( time - lastHasfullMessage > 1000 )
	{
		lastHasfullMessage = time;

		_UOI->printGeneralInfo(transpositionTable::getInstance().getFullness(),	src.getTbHits(), src.getVisitedNodes(), time);

		if(uciParameters::showCurrentLine)
		{
			src.showLine();
		}
	}
}

void my_thread::timerThread()
{
	std::mutex mutex;
	while (!quit)
	{
		std::unique_lock<std::mutex> lk(mutex);

		timerCond.wait(lk, [&]{return (startThink && !timeMan.isSearchFinished() ) || quit;} );

		if (!quit)
		{

			long long int time = st.getClockTime();
			
			timeMan.stateMachineStep( time );


#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			printTimeDependentOutput( time );
#endif


			std::this_thread::sleep_for(std::chrono::milliseconds( timeMan.resolution ));
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
			timeMan.initNewSearch( limits );
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
	
	if( legalMoves == 1 && !limits.infinite)
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
	if( uciParameters::useOwnBook && !limits.infinite )
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
	while(limits.ponder){}
}


