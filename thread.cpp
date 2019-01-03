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
	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
	timeManagement _timeMan;

	std::unique_ptr<UciOutput> _UOI;

	volatile static bool _quit;
	volatile static bool _startThink;
	
	volatile static bool _searcherReady;
	volatile static bool _timerReady;

	long long _lastHasfullMessage;
	
	std::thread _searcher;
	std::thread _timer;
	std::mutex _searchMutex;

	std::condition_variable _searchCond;
	std::condition_variable _timerCond;

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

volatile bool my_thread::impl::_searcherReady = false;
volatile bool my_thread::impl::_timerReady = false;

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

bool my_thread::impl::_isReady(){ return _searcherReady && _timerReady; }

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
		_timerReady = true;
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
	_src.stopSearch();
}

void my_thread::impl::_searchThread()
{
	std::mutex mutex;
	while (!_quit)
	{

		std::unique_lock<std::mutex> lk(mutex);
		_searcherReady =  true;
		_startThink = false;
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		if(!_quit)
		{
			_limits.checkInfiniteSearch();
			_timeMan.initNewSearch( _src.getPosition().getNextTurn() );
			_src.resetStopCondition();
			_st.resetStartTimers();
			_timerCond.notify_one();
			_src.manageNewSearch();
			
		}
		lk.unlock();
	}
}

bool my_thread::impl::_initThreads()
{
	auto startTime = std::chrono::steady_clock::now();
	
	_timer = std::thread(&my_thread::impl::_timerThread, this);
	_searcher = std::thread(&my_thread::impl::_searchThread, this);
	_src.stopSearch();
	// wait initialization
	for( auto delta = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime ); delta.count() < 1000; delta = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime ) )
	{
		if( _isReady() ) return true;
	}
	return false;
	
	
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
