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

inline void timeManagement::_clearIdLoopIterationFinished()
{
	_idLoopIterationFinished = false;
}

inline bool timeManagement::_isSearchInFailLowOverState() const
{
	return _idLoopFailLow || _idLoopFailOver;
}

inline bool timeManagement::_hasFirstIterationFinished() const
{
	return _firstIterationFinished;
}

inline bool timeManagement::_isIdLoopIterationFinished() const
{
	return _idLoopIterationFinished;
}


inline void timeManagement::stop()
{
	_stop = true;
}

inline bool timeManagement::isSearchFinished() const
{
	return _searchState == searchFinished;
}

inline unsigned int timeManagement::getResolution() const
{
	return _resolution;
}


void timeManagement::chooseSearchType( enum searchState s )
{
	_searchState = s;
}

void timeManagement::initNewSearch( SearchLimits& lim, const Position::eNextMove nm )
{
	if((!lim.btime && !lim.wtime) && !lim.moveTime)
	{
		lim.infinite = true;
		_resolution = 100;
		chooseSearchType( timeManagement::infiniteSearch );
	}
	else if(lim.moveTime)
	{
		_maxAllocatedTime = lim.moveTime;
		_allocatedTime = lim.moveTime;
		_minSearchTime = lim.moveTime;
		_resolution = std::min((long long int)100, _allocatedTime / 100 );
		chooseSearchType( timeManagement::fixedTimeSearch);
	}
	else
	{
		unsigned int time;
		unsigned int increment;

		if( nm == Position::blackTurn )
		{
			time = lim.btime;
			increment = lim.binc;
		}
		else
		{
			time = lim.wtime;
			increment = lim.winc;
		}

		if( lim.movesToGo > 0 )
		{
			_allocatedTime = time / lim.movesToGo;
			_maxAllocatedTime = 10.0 * _allocatedTime;
			_maxAllocatedTime = std::min( 10.0 * _allocatedTime, 0.8 * time);
			_maxAllocatedTime = std::max( _maxAllocatedTime, _allocatedTime );
		}
		else
		{
			_allocatedTime = time / 35.0 + increment * 0.98;
			_maxAllocatedTime = 10 * _allocatedTime;
		}

		_resolution = std::min( (long long int)100, _allocatedTime / 100 );
		_allocatedTime = std::min( (long long int)_allocatedTime ,(long long int)( time - 2 * _resolution ) );
		_minSearchTime = _allocatedTime * 0.3;
		long long buffer = std::max( 2 * _resolution, 200u );
		_allocatedTime = std::min( (long long int)_allocatedTime, (long long int)( time - buffer ) );
		_maxAllocatedTime = std::min( (long long int)_maxAllocatedTime, (long long int)( time - buffer ) );

		chooseSearchType( _limits.ponder == true ? timeManagement::standardSearchPonder : timeManagement::standardSearch );
	}

	_resetSearchvariables();

}

bool timeManagement::stateMachineStep( const long long int time, const unsigned long long visitedNodes )
{
	bool stopSearch = false;
	
	switch( _searchState )
	{
	case wait:
		//sync_cout<<"wait"<<sync_endl;
		break;

	case infiniteSearch:

		//sync_cout<<"infiniteSearch"<<sync_endl;
		if(
				_stop
				|| ( _limits.nodes && _hasFirstIterationFinished() && visitedNodes > _limits.nodes )
				|| ( _limits.moveTime && _hasFirstIterationFinished() && time >= _limits.moveTime )
		)
		{
			_searchState = searchFinished;
			stopSearch = true;
		}
		break;

	case fixedTimeSearch:

		//sync_cout<<"fixedTimeSearch"<<sync_endl;
		if(
				_stop
				|| ( time >= _allocatedTime && _hasFirstIterationFinished() )
		)
		{
			_searchState = searchFinished;
			stopSearch = true;
		}
		break;

	case standardSearch:

		//sync_cout<<"standardSearch"<<sync_endl;
		if(
				_stop
				|| ( time >= _allocatedTime && _hasFirstIterationFinished() )
				|| ( _isIdLoopIterationFinished() && time >= _minSearchTime && time >= _allocatedTime * 0.7 )
		)
		{
			_searchState = searchFinished;
			stopSearch = true;
		}
		else if( time >= _allocatedTime && _isSearchInFailLowOverState() )
		{
			_searchState = standardSearchExtendedTime;
		}
		break;

	case standardSearchPonder:
		//sync_cout<<"standardSearchPonder"<<sync_endl;
		if( _stop )
		{
			_searchState = searchFinished;
			stopSearch = true;
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
				|| ( time >= _maxAllocatedTime && _hasFirstIterationFinished() )
				|| _isIdLoopIterationFinished()
		)
		{
			_searchState = searchFinished;
			stopSearch = true;
		}
		break;

	case searchFinished:
		//sync_cout<<"searchFinished"<<sync_endl;
	default:
		break;
	}

	_clearIdLoopIterationFinished();
	
	return stopSearch;
}


volatile bool my_thread::_quit = false;
volatile bool my_thread::_startThink = false;

my_thread * my_thread::pInstance;
std::mutex  my_thread::_mutex;

void my_thread::_printTimeDependentOutput(long long int time) {

	if( time - _lastHasfullMessage > 1000 )
	{
		_lastHasfullMessage = time;

		_UOI->printGeneralInfo(transpositionTable::getInstance().getFullness(),	_src.getTbHits(), _src.getVisitedNodes(), time);

		if(uciParameters::showCurrentLine)
		{
			_src.showLine();
		}
	}
}

void my_thread::_timerThread()
{
	std::mutex mutex;
	while (!_quit)
	{
		std::unique_lock<std::mutex> lk(mutex);

		_timerCond.wait(lk, [&]{return (_startThink && !timeMan.isSearchFinished() ) || _quit;} );

		if (!_quit)
		{

			long long int time = _st.getClockTime();
			
			bool stop = timeMan.stateMachineStep( time, _src.getVisitedNodes() );
			if( stop )
			{
				_src.stopSearch();
			}


#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			_printTimeDependentOutput( time );
#endif


			std::this_thread::sleep_for(std::chrono::milliseconds( timeMan.getResolution() ));
		}
		lk.unlock();
	}
}

void my_thread::_searchThread()
{
	std::mutex mutex;
	while (!_quit)
	{

		std::unique_lock<std::mutex> lk(mutex);
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		if(!_quit)
		{
			timeMan.initNewSearch( _limits, _src.pos.getNextTurn() );
			_src.resetStopCondition();
			_st.resetStartTimers();
			_timerCond.notify_one();
			_src.resetStopCondition();
			_manageNewSearch();
			_startThink = false;
		}
		lk.unlock();
	}
}

void my_thread::_manageNewSearch()
{


	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/

	if( _game.isNewGame(_src.pos))
	{
		_game.CreateNewGame();

	}
	_game.insertNewMoves(_src.pos);


	Movegen mg(_src.pos);
	unsigned int legalMoves = mg.getNumberOfLegalMoves();

	if(legalMoves == 0)
	{
		PVline PV( 1, Move(0) );
		_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
		
		_waitStopPondering();

		_UOI->printBestMove( Move(0) );

		return;
	}
	
	if( legalMoves == 1 && !_limits.infinite)
	{
		
		Move bestMove = mg.getMoveFromMoveList(0);
		
		PVline PV( 1, bestMove );
		_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
		
		_waitStopPondering();
		
		Move ponderMove = _getPonderMoveFromHash( bestMove );
		
		_UOI->printBestMove(bestMove, ponderMove);

		return;

	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if( uciParameters::useOwnBook && !_limits.infinite )
	{
		PolyglotBook pol;
		Move bookM = pol.probe(_src.pos, uciParameters::bestMoveBook);
		if(bookM.packed)
		{
			PVline PV( 1, bookM );
			
			_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
			
			_waitStopPondering();
			
			Move ponderMove = _getPonderMoveFromBook( bookM );
			
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
	startThinkResult res = _src.startThinking( );
	
	PVline PV = res.PV;

	_waitStopPondering();

	//-----------------------------
	// print out the choosen line
	//-----------------------------

	_UOI->printGeneralInfo( transpositionTable::getInstance().getFullness(), _src.getTbHits(), _src.getVisitedNodes(), _st.getElapsedTime());
	
	Move bestMove = PV.getMove(0);
	Move ponderMove = PV.getMove(1);
	if( ponderMove == NOMOVE )
	{
		ponderMove = _getPonderMoveFromHash( bestMove );
	}
	
	_UOI->printBestMove( bestMove, ponderMove );

	_game.savePV(PV, res.depth, res.alpha, res.beta);

}



void my_thread::_initThreads()
{
	_timer = std::thread(&my_thread::_timerThread, this);
	_searcher = std::thread(&my_thread::_searchThread, this);
	_src.stopSearch();
}

void my_thread::quitThreads()
{
	_quit = true;
	_searchCond.notify_one();
	_timerCond.notify_one();
	_timer.join();
	_searcher.join();
}

Move my_thread::_getPonderMoveFromHash(const Move bestMove )
{
	Move ponderMove(0);
	_src.pos.doMove( bestMove );
	
	const ttEntry* const tte = transpositionTable::getInstance().probe(_src.pos.getKey());
	
	Move m;
	m.packed = tte->getPackedMove();
	if( _src.pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_src.pos.undoMove();
	
	return ponderMove;
}

Move my_thread::_getPonderMoveFromBook(const Move bookMove )
{
	Move ponderMove(0);
	_src.pos.doMove( bookMove );
	PolyglotBook pol;
	Move m = pol.probe(_src.pos, uciParameters::bestMoveBook);
	
	if( _src.pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_src.pos.undoMove();
	
	return ponderMove;
}

void my_thread::_waitStopPondering() const
{
	while(_limits.ponder){}
}


inline void my_thread::stopPonder(){ _limits.ponder = false;}

void my_thread::stopThinking()
{
	timeMan.stop();
	stopPonder();
}

void my_thread::ponderHit()
{
	_st.resetPonderTimer();
	stopPonder();
}

my_thread::my_thread():_src(_st, _limits), timeMan(_limits)
{
	_UOI = UciOutput::create();
	_initThreads();
	_game.CreateNewGame();
}

my_thread::~my_thread()
{
	quitThreads();
}

void my_thread::startThinking(Position * p, SearchLimits& l)
{
	_src.stopSearch();
	_lastHasfullMessage = 0;

	while(_startThink){}

	if(!_startThink)
	{
		std::lock_guard<std::mutex> lk(_searchMutex);
		_limits = l;
		_src.pos = *p;
		_startThink = true;
		_searchCond.notify_one();
	}
}
