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

#include "command.h"
#include "clock.h"
#include "selfplay.h"

#include "searchResult.h"
#include "thread.h"
#include "vajo_io.h"

SelfPlay::SelfPlay() : _p(Position::pawnHash::off), _c(_time, _increment) {
	_p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	_sl.setWTime(_c.getWhiteTime());
	_sl.setBTime(_c.getBlackTime());
	
	_sl.setWInc(_increment * 1000);
	_sl.setBInc(_increment * 1000);
}

void SelfPlay::playGame() {
	// init locks
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	
	// init vajolet search & time management
	my_thread &thr = my_thread::getInstance();
	thr.setMute(true);

	int i = 0;
	_c.start();
	while (!_isGameFinished()) {
		// set time limit
		_sl.setWTime(_c.getWhiteTime());
		_sl.setBTime(_c.getBlackTime());
		
		int count = i/2;
		if( i%2 == 0) {
			std::cout<< count + 1<<"."<<std::endl;
		}
		
		// do the search
		auto begin = std::chrono::high_resolution_clock::now();
		thr.startThinking(_p, _sl);
		thr.finished().wait(lock);
		auto end = std::chrono::high_resolution_clock::now();
		
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		
		auto res = thr.getResult();
		
		_c.switchTurn();
		std::cout<<UciManager::displayMove(_p, res.PV.getMove(0))<<"("<<ms<<") depth: " <<res.depth<<" score: "<<((i%2 == 0) ? res.Res: -res.Res) <<" white_time: "<<_c.getWhiteTime()<<" black_time: "<<_c.getBlackTime()<<std::endl;
		_p.doMove(res.PV.getMove(0));
		
		++i;
	}
}

bool SelfPlay::_isGameFinished() {
	// checkmate
	if (_p.isCheckMate()) {
		std::cout<<std::endl<<"CHECKMATE"<<std::endl;
		return true;
	}
	
	// patta ripetizione
	// num mosse
	if (_p.isDraw(true)) {
		std::cout<<std::endl<<"DRAW"<<std::endl;
		return true;
	}
	
	// stallo
	if (_p.isStaleMate()) {
		std::cout<<std::endl<<"STALEMATE"<<std::endl;
		return true;
	}
	
	// early stop
	// near 0 for x moves after ply y
	// abs(v) > x  for y moves
	
	return false;
}
