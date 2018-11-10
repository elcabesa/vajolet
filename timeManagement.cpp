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

void timeManagement::_resetSearchVariables()
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


void timeManagement::stop()
{
	_stop = true;
}

bool timeManagement::isSearchFinished() const
{
	return _searchState == searchFinished;
}

unsigned int timeManagement::getResolution() const
{
	return _resolution;
}


void timeManagement::_chooseSearchType( enum searchState s )
{
	_searchState = s;
}

void timeManagement::initNewSearch( const Position::eNextMove nm )
{
	// todo move thos controls in command or limits class... here we only have to read it
	if( _limits.infinite )
	{
		_resolution = 100;
		_chooseSearchType( timeManagement::infiniteSearch );
	}
	else if(_limits.moveTime)
	{
		_allocatedTime = _limits.moveTime;
		_resolution = std::min((long long int)100, _allocatedTime / 100 );
		_chooseSearchType( timeManagement::fixedTimeSearch);
	}
	else
	{
		unsigned int time;
		unsigned int increment;

		if( nm == Position::blackTurn )
		{
			time = _limits.btime;
			increment = _limits.binc;
		}
		else
		{
			time = _limits.wtime;
			increment = _limits.winc;
		}

		if( _limits.movesToGo > 0 )
		{
			_allocatedTime = time / _limits.movesToGo;
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

		_chooseSearchType( _limits.ponder == true ? timeManagement::standardSearchPonder : timeManagement::standardSearch );
	}

	_resetSearchVariables();

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
				|| ( _limits.nodes && _hasFirstIterationFinished() && visitedNodes >= _limits.nodes )
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
				|| ( _isIdLoopIterationFinished() && time >= _minSearchTime && time >= _allocatedTime * 0.7 )
		)
		{
			_searchState = searchFinished;
			stopSearch = true;
		}
		else if( time >= _allocatedTime )
		{
			if( _isSearchInFailLowOverState() )
			{
				_searchState = standardSearchExtendedTime;
			}
			else if( _hasFirstIterationFinished() )
			{
				_searchState = searchFinished;
				stopSearch = true;
			}
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
		stopSearch = true;
		break;
	}

	_clearIdLoopIterationFinished();
	
	return stopSearch;
}

