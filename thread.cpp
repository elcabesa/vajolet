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
#include "search.h"
#include "searchLimits.h"
#include "searchTimer.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"


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

		_timerCond.wait(lk, [&]{return (_startThink && !_timeMan.isSearchFinished() ) || _quit;} );

		if (!_quit)
		{

			long long int time = _st.getClockTime();
			
			bool stop = _timeMan.stateMachineStep( time, _src.getVisitedNodes() );
			if( stop )
			{
				_src.stopSearch();
			}


#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			_printTimeDependentOutput( time );
#endif


			std::this_thread::sleep_for(std::chrono::milliseconds( _timeMan.getResolution() ));
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
			_limits.checkInfiniteSearch();
			_timeMan.initNewSearch( _src.pos.getNextTurn() );
			_src.resetStopCondition();
			_st.resetStartTimers();
			_timerCond.notify_one();
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


	Movegen mg( _src.pos );
	unsigned int legalMoves = mg.getNumberOfLegalMoves();

	if(legalMoves == 0)
	{
		PVline PV( 1, Move(0) );
		_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
		
		_waitStopPondering();

		_UOI->printBestMove( Move(0) );

		return;
	}
	
	if( legalMoves == 1 && !_limits.infinite )
	{
		Move bestMove = mg.getNextMove();
		
		PVline PV( 1, bestMove );
		_UOI->printPV( 0, 0, 0, -1, 1, 0, 0, PV, 0 );
		
		_waitStopPondering();
		
		Move ponderMove = _getPonderMoveFromHash( bestMove );
		
		_UOI->printBestMove( bestMove, ponderMove );

		return;

	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if( uciParameters::useOwnBook && !_limits.infinite )
	{
		PolyglotBook pol;
		Move bookM = pol.probe(_src.pos, uciParameters::bestMoveBook);
		if( bookM )
		{
			PVline PV( 1, bookM );
			
			_UOI->printPV(0, 0, 0, -1, 1, 0, 0, PV, 0);
			
			_waitStopPondering();
			
			Move ponderMove = _getPonderMoveFromBook( bookM );
			
			_UOI->printBestMove(bookM, ponderMove);
			
			return;
		}
	}
	
	//if( game.isPonderRight() )
	//{
	//	Game::GamePosition gp = game.getNewSearchParameters();
	//
	//	PVline newPV;
	//	std::copy( gp.PV.begin(), gp.PV.end(), std::back_inserter( newPV ) );
	//	
	//	newPV.resize(gp.depth/2 + 1);
	//	newPV.pop_front();
	//	newPV.pop_front();
	//	res = src.startThinking( gp.depth/2 + 1, gp.alpha, gp.beta, newPV );
	//}
	//else
	
	startThinkResult res = _src.startThinking( );
	
	PVline PV = res.PV;

	_waitStopPondering();

	//-----------------------------
	// print out the choosen line
	//-----------------------------

	_UOI->printGeneralInfo( transpositionTable::getInstance().getFullness(), _src.getTbHits(), _src.getVisitedNodes(), _st.getElapsedTime());
	
	Move bestMove = PV.getMove(0);
	Move ponderMove = PV.getMove(1);
	if( !ponderMove )
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
	
	Move m( tte->getPackedMove() );
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
	_timeMan.stop();
	stopPonder();
}

void my_thread::ponderHit()
{
	_st.resetPonderTimer();
	stopPonder();
}

my_thread::my_thread():_src(_st, _limits), _timeMan(_limits)
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



void Game::CreateNewGame(void)
{
	_positions.clear();
}

void Game::insertNewMoves(Position &pos)
{
	unsigned int actualPosition = _positions.size();
	for(unsigned int i = actualPosition; i < pos.getStateSize(); i++)// todo usare iteratore dello stato
	{
		GamePosition p;
		p.key = pos.getState(i).getKey();
		p.m = pos.getState(i).getCurrentMove();
		_positions.push_back(p);
	}
}

void Game::savePV(PVline PV,unsigned int depth, Score alpha, Score beta)
{
	_positions.back().PV = PV;
	_positions.back().depth = depth;
	_positions.back().alpha = alpha;
	_positions.back().beta = beta;
}


void Game::printGamesInfo()
{
	for(auto p : _positions)
	{
		if( p.m )
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

bool Game::isNewGame(const Position &pos) const
{
	if( _positions.size() == 0 || pos.getStateSize() < _positions.size())
	{
		//printGamesInfo();
		return true;
	}

	unsigned int n = 0;
	for(auto p : _positions)
	{
		if(pos.getState(n).getKey() != p.key)
		{
			//printGamesInfo();
			return true;
		}
		n++;

	}
	return false;
}

bool Game:: isPonderRight() const
{
	if( _positions.size() > 2)
	{
		GamePosition previous =*(_positions.end()-3);
		if(previous.PV.size()>=1 && previous.PV.getMove(1) == _positions.back().m)
		{
			return true;
		}

	}
	return false;
}

Game::GamePosition Game::getNewSearchParameters() const
{
	GamePosition previous =*(_positions.end()-3);
	return previous;
}
