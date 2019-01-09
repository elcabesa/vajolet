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

#ifndef SEARCH_TIMER_H_
#define SEARCH_TIMER_H_

#include <chrono>

class SearchTimer
{
	std::chrono::time_point<std::chrono::steady_clock> startTime;
	std::chrono::time_point<std::chrono::steady_clock> ponderTime;
public:
	static std::chrono::time_point<std::chrono::steady_clock> getTime(){ return std::chrono::steady_clock::now(); }

	long long int getElapsedTime() const { return (std::chrono::duration_cast<std::chrono::milliseconds>( getTime() - startTime )).count(); }
	long long int getClockTime() const { return (std::chrono::duration_cast<std::chrono::milliseconds>( getTime() - ponderTime )).count(); }
	void resetStartTimers(){ ponderTime = startTime = getTime(); }
	void resetPonderTimer(){ ponderTime = getTime(); }

	explicit SearchTimer(const SearchTimer &other):startTime(other.startTime), ponderTime(other.ponderTime){};
	SearchTimer():startTime(getTime()), ponderTime(getTime()){};
};

#endif
