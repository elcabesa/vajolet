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

#include <memory>
#include "position.h"
#include "searchLimits.h"

class timeManagement
{
public:
	explicit timeManagement( SearchLimits& limits ):_firstIterationFinished(false), _idLoopIterationFinished(false), _idLoopFailLow(false), _idLoopFailOver(false), _resolution(0), _stop(false), _limits(limits){}
	virtual ~timeManagement(){}

	void notifyIterationHasBeenFinished();
	void notifyFailLow();
	void notifyFailOver();
	void stop();

	unsigned int getResolution() const;
	virtual bool isSearchFinished() const = 0;

	virtual bool stateMachineStep( const long long int time, const unsigned long long visitedNodes ) = 0;
	
	static std::unique_ptr<timeManagement> create (SearchLimits& limits, const eNextMove nm);
	

protected:
	void _clearIdLoopIterationFinished();
	bool _isSearchInFailLowOverState() const;
	bool _hasFirstIterationFinished() const;
	bool _isIdLoopIterationFinished() const;

	bool _firstIterationFinished;
	bool _idLoopIterationFinished;
	bool _idLoopFailLow;
	bool _idLoopFailOver;
	
	unsigned int _resolution;

	bool _stop;
	const SearchLimits& _limits;
};

#endif
