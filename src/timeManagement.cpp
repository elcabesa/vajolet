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

#include "timeManagement.h"

/***************************************************************
infinite search or node limited
****************************************************************/
class InfiniteSearchTimeManagement: public timeManagement{
public:
	InfiniteSearchTimeManagement(SearchLimits& limits);
	bool stateMachineStep(const long long int time, const unsigned long long visitedNodes);
	bool isSearchFinished() const;
private:
	bool _searchFinished;
};

InfiniteSearchTimeManagement::InfiniteSearchTimeManagement(SearchLimits& limits):timeManagement(limits), _searchFinished(false) {
	_resolution = 100;
}

bool InfiniteSearchTimeManagement::isSearchFinished() const {
	return _searchFinished;
}

bool InfiniteSearchTimeManagement::stateMachineStep(const long long int, const unsigned long long visitedNodes) {
	if(
		_stop
		|| ( _limits.isNodeLimitedSearch() && _hasFirstIterationFinished() && visitedNodes >= _limits.getNodeLimit() )
	)
	{
		_searchFinished = true;
		return true;
	}
	return false;
}

/***************************************************************
fixed time limited search
****************************************************************/
class FixedTimeManagement: public timeManagement{
public:
	FixedTimeManagement(SearchLimits& limits);
	bool stateMachineStep(const long long int time, const unsigned long long visitedNodes);
	bool isSearchFinished() const;
private:
	bool _searchFinished;
	long long _allocatedTime;
};

FixedTimeManagement::FixedTimeManagement(SearchLimits& limits):timeManagement(limits), _searchFinished(false) {
	_allocatedTime = _limits.getMoveTime();
	_resolution = std::min((long long int)100, _allocatedTime / 100 );
}

bool FixedTimeManagement::isSearchFinished() const {
	return _searchFinished;
}

bool FixedTimeManagement::stateMachineStep(const long long int time, const unsigned long long) {
	if(
		_stop
		|| ( time >= _allocatedTime && _hasFirstIterationFinished() )
	)
	{
		_searchFinished = true;
		return true;
	}
	return false;
}

/***************************************************************
standard time search
****************************************************************/
class NormalTimeManagement: public timeManagement{
public:
	NormalTimeManagement(SearchLimits& limits, const eNextMove nm);
	bool stateMachineStep(const long long int time, const unsigned long long visitedNodes);
	bool isSearchFinished() const;
private:
	long long _allocatedTime;
	long long _minSearchTime;
	long long _maxAllocatedTime;
	
	enum class searchState
	{
		_standardSearchPonder,
		_standardSearch,
		_standardSearchExtendedTime,
		_searchFinished
	};
	
	searchState _searchState;
};

NormalTimeManagement::NormalTimeManagement(SearchLimits& limits, const eNextMove nm):timeManagement(limits), _allocatedTime(0), _minSearchTime(0), _maxAllocatedTime(0), _searchState(searchState::_standardSearch) {
	
	long long time;
	long long increment;

	if( nm == blackTurn )
	{
		time = _limits.getBTime();
		increment = _limits.getBInc();
	}
	else
	{
		time = _limits.getWTime();
		increment = _limits.getWInc();
	}

	if( _limits.getMovesToGo() > 0 )
	{
		_allocatedTime = time / _limits.getMovesToGo();
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
	
	_maxAllocatedTime = std::max(0ll, _maxAllocatedTime);
	_minSearchTime = std::max(0ll, _minSearchTime);
	_allocatedTime = std::max(0ll, _allocatedTime);
	
	_searchState = _limits.isPondering() ? searchState::_standardSearchPonder : searchState::_standardSearch;
}

bool NormalTimeManagement::isSearchFinished() const {
	return _searchState == searchState::_searchFinished;
}

bool NormalTimeManagement::stateMachineStep(const long long int time, const unsigned long long) {
	bool stopSearch = false;
	
	switch( _searchState )
	{
	case searchState::_standardSearch:

		//sync_cout<<"standardSearch"<<sync_endl;
		if(
				_stop
				|| ( _isIdLoopIterationFinished() && time >= _minSearchTime && time >= _allocatedTime * 0.7 )
		)
		{
			_searchState = searchState::_searchFinished;
			stopSearch = true;
		}
		else if( time >= _allocatedTime )
		{
			if( _isSearchInFailLowOverState() )
			{
				_searchState = searchState::_standardSearchExtendedTime;
			}
			else if( _hasFirstIterationFinished() )
			{
				_searchState = searchState::_searchFinished;
				stopSearch = true;
			}
		}
		break;

	case searchState::_standardSearchPonder:
		//sync_cout<<"standardSearchPonder"<<sync_endl;
		if( _stop )
		{
			_searchState = searchState::_searchFinished;
			stopSearch = true;
		}
		else if( !_limits.isPondering() )
		{
			_searchState = searchState::_standardSearch;
		}
		break;

	case searchState::_standardSearchExtendedTime:
		//sync_cout<<"standardSearchExtendedTime"<<sync_endl;
		if(
				_stop
				|| ( time >= _maxAllocatedTime && _hasFirstIterationFinished() )
				|| _isIdLoopIterationFinished()
		)
		{
			_searchState = searchState::_searchFinished;
			stopSearch = true;
		}
		break;

	case searchState::_searchFinished:
		//sync_cout<<"searchFinished"<<sync_endl;
	default:
		stopSearch = true;
		break;
	}

	_clearIdLoopIterationFinished();
	
	return stopSearch;
}

/***************************************************************
FACTORY
****************************************************************/
std::unique_ptr<timeManagement> timeManagement::create (SearchLimits& limits, const eNextMove nm)
{
	if (limits.isInfiniteSearch()) {
		return std::make_unique<InfiniteSearchTimeManagement>(limits);
	}
	if (limits.isMoveTimeSearch()) {
		return std::make_unique<FixedTimeManagement>(limits);
	}
	return std::make_unique<NormalTimeManagement>(limits, nm);
	
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


void timeManagement::stop()
{
	_stop = true;
}

unsigned int timeManagement::getResolution() const
{
	return _resolution;
}

