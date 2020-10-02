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
#include <mutex>

#include "vajo_io.h"
#include "position.h"
#include "search.h"
#include "searchLimits.h"
#include "searchResult.h"
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
	
	enum class threadStatus
	{
		initalizing,
		ready,
		running
	};
	
	volatile threadStatus _searchStatus = threadStatus::initalizing;
	volatile threadStatus _timerStatus = threadStatus::initalizing;
	
	std::thread _searcher;
	std::thread _timer;
	
	std::mutex _sMutex;
	std::mutex _tMutex;
	std::mutex _gMutex;
	
	std::condition_variable _searchCond;
	std::condition_variable _timerCond;
	std::condition_variable _finishedSearch;
	
	volatile bool _quit = false;
	volatile bool _startThink = false;
		
	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
	transpositionTable _tt;
	std::unique_ptr<timeManagement> _timeMan;
	std::unique_ptr<UciOutput> _UOI;
	long long _lastHasfullMessageTime;
	SearchResult _srcRes;


	bool _initThreads();
	void _quitThreads();
	
	void _timerThread();
	void _searchThread();
	void _printTimeDependentOutput( long long int time );
	void _stopPonder();

public:
	explicit impl();
	~impl();
	void startThinking(const Position& p, SearchLimits& l);
	void stopThinking();
	void ponderHit();
	timeManagement& getTimeMan();
	const SearchResult& getResult() const;
	void setMute(bool mute);
	SearchParameters& getSearchParameters();
	const SearchResult& synchronousSearch(const Position& p, SearchLimits& l);
	bool isSearchRunning() const;
	transpositionTable& getTT();
};

my_thread::impl::impl(): _src(_st, _limits, _tt), _timeMan(timeManagement::create(_limits, _src.getPosition().getNextTurn())), _UOI(UciOutput::create()), _srcRes(0, 0, 0, PVline(), 0) 
{
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

timeManagement& my_thread::impl::getTimeMan(){ return *_timeMan; }

void my_thread::impl::_printTimeDependentOutput(long long int time) {

	if( time - _lastHasfullMessageTime > 1000 )
	{
		_lastHasfullMessageTime = time;

		_UOI->printGeneralInfo(_tt.getFullness(),	_src.getTbHits(), _src.getVisitedNodes(), time);

		if(uciParameters::showCurrentLine)
		{
			_src.showLine();
		}
	}
}

void my_thread::impl::_timerThread()
{
	std::unique_lock<std::mutex> lk(_tMutex);
	
	while (!_quit)
	{
		_timerStatus = threadStatus::ready;
		_timerCond.notify_all();
		
		_timerCond.wait(lk, [&]{return (_startThink && !_timeMan->isSearchFinished() ) || _quit;} );
		
		_timerStatus = threadStatus::running;
		_timerCond.notify_all();

		if (!_quit)
		{
			long long int time = _st.getClockTime();
			
			bool stop = _timeMan->stateMachineStep( time, _src.getVisitedNodes() );
			if( stop )
			{
				_src.stopSearch();
			}

#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			_printTimeDependentOutput( time );
#endif
			std::this_thread::sleep_for(std::chrono::milliseconds( _timeMan->getResolution() ));
		}
	}
	_src.stopSearch();
}

void my_thread::impl::_searchThread()
{
	std::unique_lock<std::mutex> lk(_sMutex);
	
	while (!_quit)
	{
		_searchStatus = threadStatus::ready;
		_searchCond.notify_all();
		
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		
		_searchStatus = threadStatus::running;
		_searchCond.notify_all();
		
		if(!_quit)
		{
			_limits.checkInfiniteSearch();
			_limits.manageSkillLevel();
			_timeMan = timeManagement::create(_limits, _src.getPosition().getNextTurn());
			_st.resetTimers();
			_timerCond.notify_all();
			_srcRes = _src.manageNewSearch(getTimeMan());
			_startThink = false;
		}
		_finishedSearch.notify_all();
	}
}

bool my_thread::impl::_initThreads()
{
	std::lock( _tMutex, _sMutex );
	std::unique_lock<std::mutex> lckt(_tMutex, std::adopt_lock);
	std::unique_lock<std::mutex> lcks(_sMutex, std::adopt_lock);
	_timer = std::thread(&my_thread::impl::_timerThread, this);
	_searcher = std::thread(&my_thread::impl::_searchThread, this);
	_src.stopSearch();
	
	// wait initialization
	if( !_timerCond.wait_for( lckt, std::chrono::seconds( _initTimeout ), [&]{ return _timerStatus == threadStatus::ready;} ) ) return false;
	if( !_searchCond.wait_for( lcks, std::chrono::seconds( _initTimeout ), [&]{ return _searchStatus == threadStatus::ready;} ) ) return false;

	return true;
}

inline void my_thread::impl::_quitThreads()
{
	std::lock( _tMutex, _sMutex );
	std::unique_lock<std::mutex> lks(_sMutex, std::adopt_lock);
	std::unique_lock<std::mutex> lkt(_tMutex, std::adopt_lock);
	_quit = true;
	lks.unlock();
	lkt.unlock();

	_searchCond.notify_all();
	_timerCond.notify_all();
	_timer.join();
	_searcher.join();
}

inline void my_thread::impl::startThinking( const Position& p, SearchLimits& l)
{
	_src.stopSearch();
	_lastHasfullMessageTime = 0;
	std::lock(_tMutex, _sMutex);
	std::unique_lock<std::mutex> lcks(_sMutex, std::adopt_lock);
	_searchCond.wait( lcks, [&]{ return _searchStatus == threadStatus::ready; } );
	std::unique_lock<std::mutex> lckt(_tMutex, std::adopt_lock);
	_timerCond.wait( lckt, [&]{ return _timerStatus == threadStatus::ready; } );

	_limits = l;
	_src.getPosition() = p;
	_startThink = true;
	_searchCond.notify_all();
}

inline void my_thread::impl::stopThinking()
{
	_timeMan->stop();
	_stopPonder();
}

inline void my_thread::impl::ponderHit()
{
	_st.resetClockTimer();
	_stopPonder();
}

inline void my_thread::impl::_stopPonder(){ _limits.setPonder(false);}

const SearchResult& my_thread::impl::getResult() const {
	return _srcRes;
}

void my_thread::impl::setMute(bool mute) {
	if (mute) {
		_UOI = UciOutput::create(UciOutput::type::mute);
		_src.setUOI(UciOutput::type::mute);
	} else {
		_UOI = UciOutput::create(UciOutput::type::standard);
		_src.setUOI(UciOutput::type::standard);
	}
}

SearchParameters& my_thread::impl::getSearchParameters() { return _src.getSearchParameters(); }


const SearchResult& my_thread::impl::synchronousSearch(const Position& p, SearchLimits& l) {
	startThinking(p, l);

	std::unique_lock<std::mutex> lk(_gMutex);
	_finishedSearch.wait(lk, [&]{return _startThink == false;});

	std::lock(_tMutex, _sMutex);
	std::unique_lock<std::mutex> lcks(_sMutex, std::adopt_lock);
	_searchCond.wait( lcks, [&]{ return _searchStatus == threadStatus::ready; } );
	std::unique_lock<std::mutex> lckt(_tMutex, std::adopt_lock);
	_timerCond.wait( lckt, [&]{ return _timerStatus == threadStatus::ready; } );

	return getResult();
}

bool my_thread::impl::isSearchRunning() const {
	return _searchStatus == threadStatus::running;
}

transpositionTable& my_thread::impl::getTT() {return _tt;}

/*********************************************
* my_thread class
**********************************************/

my_thread::my_thread(): pimpl{std::make_unique<impl>()}{}

my_thread::~my_thread() = default;

void my_thread::stopThinking() { pimpl->stopThinking();}

void my_thread::ponderHit() { pimpl->ponderHit();}

timeManagement& my_thread::getTimeMan(){ return pimpl->getTimeMan(); }

void my_thread::startThinking(const Position& p, SearchLimits& l){ pimpl->startThinking( p, l); }

void my_thread::setMute(bool mute) { pimpl->setMute(mute); }

SearchParameters& my_thread::getSearchParameters() { return pimpl->getSearchParameters(); }

const SearchResult& my_thread::synchronousSearch(const Position& p, SearchLimits& l) { return pimpl->synchronousSearch(p, l); }

bool my_thread::isSearchRunning() const { return pimpl->isSearchRunning(); }

transpositionTable& my_thread::getTT() { return pimpl->getTT();}
