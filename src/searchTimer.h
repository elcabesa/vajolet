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

#include <cstdint>
#include <chrono>

class SearchTimer
{
	std::chrono::time_point<std::chrono::steady_clock> _startTime;
	std::chrono::time_point<std::chrono::steady_clock> _ponderTime;

	static auto _getTime(){return std::chrono::steady_clock::now();}
public:
	explicit SearchTimer():_startTime(_getTime()), _ponderTime(_startTime) {};
	explicit SearchTimer(const SearchTimer &other) = delete;
	explicit SearchTimer(const SearchTimer &&other) = delete;
	SearchTimer& operator=(const SearchTimer& other ) {_startTime = other._startTime; _ponderTime = other._ponderTime; return *this;}
	SearchTimer& operator=(const SearchTimer&&) = delete;

	int64_t getElapsedTime() const {return (std::chrono::duration_cast<std::chrono::milliseconds>(_getTime() - _startTime )).count();}
	int64_t getClockTime()   const {return (std::chrono::duration_cast<std::chrono::milliseconds>(_getTime() - _ponderTime)).count();}
	void resetTimers() {_ponderTime = _startTime = _getTime();}
	void resetClockTimer() {_ponderTime = _getTime();}
};

#endif
