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

#ifndef TIME_MANAGEMENT_H_
#define TIME_MANAGEMENT_H_

#include "position.h"
#include "searchLimits.h"

class timeManagement
{
public:
	enum searchState
	{
		wait,
		infiniteSearch,
		fixedTimeSearch,
		standardSearchPonder,
		standardSearch,
		standardSearchExtendedTime,
		searchFinished
	};

	timeManagement( SearchLimits& limits ):_limits(limits){}

	void initNewSearch( const Position::eNextMove nm );

	void notifyIterationHasBeenFinished();
	void notifyFailLow();
	void notifyFailOver();

	unsigned int getResolution() const;

	void stop();
	
	bool isSearchFinished() const;
	
	void chooseSearchType( enum searchState s);
	bool stateMachineStep( const long long int time, const unsigned long long visitedNodes );
	

private:
	void _resetSearchvariables();
	void _clearIdLoopIterationFinished();
	bool _isSearchInFailLowOverState() const;
	bool _hasFirstIterationFinished() const;
	bool _isIdLoopIterationFinished() const;

	bool _firstIterationFinished;
	bool _idLoopIterationFinished;
	bool _idLoopFailLow;
	bool _idLoopFailOver;
	
	long long _allocatedTime;
	long long _minSearchTime;
	long long _maxAllocatedTime;
	unsigned int _resolution;

	bool _stop;

	searchState _searchState;
	SearchLimits& _limits;

};

#endif