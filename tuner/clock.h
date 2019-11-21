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
#ifndef CLOCK_H_
#define CLOCK_H_

#include <chrono>

class Clock {
	using _time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
public:
	enum class turn {
		white,
		black
	};
	
	Clock(float time, float increment);
	
	void reset();
	int getWhiteTime() const;
	int getBlackTime() const;
	bool isWhiteTurn() const;
	bool isBlackTurn() const;
	turn getTurn() const;
	void start();
	void stop();
	void switchTurn();
private:
	turn _turn;
	const int _time;
	const int _increment;
	int _whiteTime;
	int _blackTime;
	_time_t _begin;
	
	void _updateClock();
	void _switchTurn();
};

#endif /* CLOCK_H_ */