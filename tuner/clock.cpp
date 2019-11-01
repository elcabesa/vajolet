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
#include <mutex>

#include "clock.h"

Clock::Clock(float time, float increment):_turn(turn::white), _time(std::max(int(time*1000),0)), _increment(std::max(int(increment*1000), 0)), _whiteTime(_time), _blackTime(_time) {}

void Clock::reset() {
	_turn = turn::white;
	_whiteTime = _time;
	_blackTime = _time;
}

int Clock::getWhiteTime() const { return _whiteTime; }
int Clock::getBlackTime() const { return _blackTime; }

void Clock::switchTurn() {
	_updateClock();
	_switchTurn();
}

void Clock::start() {
	_begin = std::chrono::high_resolution_clock::now();
}

void Clock::stop() {
	_updateClock();
}

void Clock::_updateClock() {
	_time_t now = std::chrono::high_resolution_clock::now();
	auto dur = now - _begin;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	_begin = now;
	
	if ( _turn == turn::white ) {
		_whiteTime += -ms + _increment;
	} else {
		_blackTime += -ms + _increment;
	}
}

void Clock::_switchTurn() {
	if ( _turn == turn::white ) {
		_turn = turn::black;
	} else {
		_turn = turn::white;
	}
}
