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
#include "command.h"
#include "movegen.h"
#include "movepicker.h"
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
			_src.manageNewSearch();
			_startThink = false;
		}
		lk.unlock();
	}
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
}

my_thread::~my_thread()
{
	quitThreads();
}

void my_thread::startThinking( const Position& p, SearchLimits& l)
{
	_src.stopSearch();
	_lastHasfullMessage = 0;

	while(_startThink){}

	if(!_startThink)
	{
		std::lock_guard<std::mutex> lk(_searchMutex);
		_limits = l;
		_src.pos = p;
		_startThink = true;
		_searchCond.notify_one();
	}
}
