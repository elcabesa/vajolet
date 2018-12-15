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

#include <condition_variable>
#include <thread>

#include "search.h"
#include "searchLimits.h"
#include "searchTimer.h"
#include "timeManagement.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"


/*********************************************
* implementation
**********************************************/
class my_thread::impl
{
private:
	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
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
	void _printTimeDependentOutput( long long int time );
public:
	impl();
	~impl();
	void startThinking( const Position& p, SearchLimits& l);
	void stopThinking();
	void ponderHit();
	void stopPonder();
	timeManagement& getTimeMan();
	
	void quitThreads();
};

volatile bool my_thread::impl::_quit = false;
volatile bool my_thread::impl::_startThink= false;

my_thread::impl::impl(): _src(_st, _limits), _timeMan(_limits)
{
	_UOI = UciOutput::create();
	_initThreads();
}

my_thread::impl::~impl()
{
	quitThreads();
}

timeManagement& my_thread::impl::getTimeMan(){ return _timeMan; }

void my_thread::impl::_printTimeDependentOutput(long long int time) {

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

void my_thread::impl::_timerThread()
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

void my_thread::impl::_searchThread()
{
	std::mutex mutex;
	while (!_quit)
	{

		std::unique_lock<std::mutex> lk(mutex);
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		if(!_quit)
		{
			_limits.checkInfiniteSearch();
			_timeMan.initNewSearch( _src.getPosition().getNextTurn() );
			_src.resetStopCondition();
			_st.resetStartTimers();
			_timerCond.notify_one();
			_src.manageNewSearch();
			_startThink = false;
		}
		lk.unlock();
	}
}

void my_thread::impl::_initThreads()
{
	_timer = std::thread(&my_thread::impl::_timerThread, this);
	_searcher = std::thread(&my_thread::impl::_searchThread, this);
	_src.stopSearch();
}

inline void my_thread::impl::quitThreads()
{
	_quit = true;
	_searchCond.notify_one();
	_timerCond.notify_one();
	_timer.join();
	_searcher.join();
}

inline void my_thread::impl::startThinking( const Position& p, SearchLimits& l)
{
	_src.stopSearch();
	_lastHasfullMessage = 0;

	while(_startThink){}

	if(!_startThink)
	{
		std::lock_guard<std::mutex> lk(_searchMutex);
		_limits = l;
		_src.getPosition() = p;
		_startThink = true;
		_searchCond.notify_one();
	}
}

inline void my_thread::impl::stopThinking()
{
	_timeMan.stop();
	stopPonder();
}

inline void my_thread::impl::ponderHit()
{
	_st.resetPonderTimer();
	stopPonder();
}

inline void my_thread::impl::stopPonder(){ _limits.ponder = false;}



/*********************************************
* my_thread class
**********************************************/

my_thread::my_thread(): pimpl{std::make_unique<impl>()}{}

my_thread::~my_thread() = default;

void my_thread::quitThreads(){ pimpl->quitThreads();}

void my_thread::stopThinking() { pimpl->stopThinking();}

void my_thread::ponderHit() { pimpl->ponderHit();}

inline void my_thread::stopPonder(){ pimpl->stopPonder(); }

timeManagement& my_thread::getTimeMan(){ return pimpl->getTimeMan(); }

void my_thread::startThinking( const Position& p, SearchLimits& l){	pimpl->startThinking( p, l); }
