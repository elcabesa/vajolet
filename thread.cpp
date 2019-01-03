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

#include "io.h"
#include "position.h"
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

	const unsigned int _initTimeout = 1;
	
	enum threadStatus{
		initalizing,
		ready,
		running
	};
	
	volatile threadStatus _timerStatus = initalizing;
	volatile threadStatus _searchStatus = initalizing;
		
	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
	timeManagement _timeMan;

	std::unique_ptr<UciOutput> _UOI;

	volatile static bool _quit;
	volatile static bool _startThink;
	
	long long _lastHasfullMessage;
	
	std::thread _searcher;
	std::thread _timer;
	
	std::mutex _initMutex;
	std::mutex _waitNewStartMutex;
	
	std::condition_variable _initCV;
	std::condition_variable _searchCond;
	std::condition_variable _timerCond;
	std::condition_variable _runCV;

	bool _initThreads();

	void _timerThread();
	void _searchThread();
	void _printTimeDependentOutput( long long int time );
	bool _isReady();
	void _stopPonder();
	void _quitThreads();
public:
	impl();
	~impl();
	void startThinking( const Position& p, SearchLimits& l);
	void stopThinking();
	void ponderHit();
	timeManagement& getTimeMan();
};

volatile bool my_thread::impl::_quit = false;
volatile bool my_thread::impl::_startThink = false;


my_thread::impl::impl(): _src(_st, _limits), _timeMan(_limits)
{
	_UOI = UciOutput::create();
	if( !_initThreads() )
	{
		sync_cout<<"unable to initialize threads, exiting..."<<sync_endl;
		exit(-1);
	}
}

my_thread::impl::~impl()
{
	_quitThreads();
}

bool my_thread::impl::_isReady(){ return _searchStatus == ready && _timerStatus== ready; }

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
		_timerStatus = ready;
		_runCV.notify_one();
		_timerCond.wait(lk, [&]{return (_startThink && !_timeMan.isSearchFinished() ) || _quit;} );
		_timerStatus = running;

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
	_src.stopSearch();
}

void my_thread::impl::_searchThread()
{
	std::mutex mutex;
	while (!_quit)
	{

		std::unique_lock<std::mutex> lk(mutex);
		_searchStatus = ready;
		_runCV.notify_one();
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		_searchStatus = running;
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

bool my_thread::impl::_initThreads()
{	
	_timer = std::thread(&my_thread::impl::_timerThread, this);
	_searcher = std::thread(&my_thread::impl::_searchThread, this);
	_src.stopSearch();
	// wait initialization
	
	std::unique_lock<std::mutex> lck(_initMutex);
	// wait initialization
	return _initCV.wait_for( lck, std::chrono::seconds( _initTimeout ), [&]{ return _isReady();});
}

inline void my_thread::impl::_quitThreads()
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

	std::unique_lock<std::mutex> lck(_waitNewStartMutex);
	_runCV.wait( lck, [&]{ return _startThink == false && _timerStatus == ready && _searchStatus == ready; } );

	_limits = l;
	_src.getPosition() = p;
	_startThink = true;
	_searchCond.notify_one();
	
}

inline void my_thread::impl::stopThinking()
{
	_timeMan.stop();
	_stopPonder();
}

inline void my_thread::impl::ponderHit()
{
	_st.resetPonderTimer();
	_stopPonder();
}

inline void my_thread::impl::_stopPonder(){ _limits.ponder = false;}



/*********************************************
* my_thread class
**********************************************/

my_thread::my_thread(): pimpl{std::make_unique<impl>()}{}

my_thread::~my_thread() = default;

void my_thread::stopThinking() { pimpl->stopThinking();}

void my_thread::ponderHit() { pimpl->ponderHit();}

timeManagement& my_thread::getTimeMan(){ return pimpl->getTimeMan(); }

void my_thread::startThinking( const Position& p, SearchLimits& l){	pimpl->startThinking( p, l); }
