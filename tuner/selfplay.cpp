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
#include "position.h"
#include "selfplay.h"
#include "searchLimits.h"
#include "searchResult.h"
#include "thread.h"
#include "vajo_io.h"

void SelfPlay::playGame() {
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	
	my_thread::getInstance().setMute(true);
	
	
	Position p(Position::pawnHash::off);
	
	SearchLimits sl;
	sl.setDepth(15);
	
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	int i = 0;
	while( !isGameFinished(p)) {
		my_thread::getInstance().startThinking(p, sl);
		my_thread::getInstance().finished().wait(lock);
		auto res = my_thread::getInstance().getResult();
		
		int count = i/2;
		if( i%2 == 0) {
			std::cout<< count <<" "<<UciManager::displayMove(p, res.PV.getMove(0));
		} else {
			std::cout<< " "<<UciManager::displayMove(p, res.PV.getMove(0))<<std::endl;
		}
		
		p.doMove(res.PV.getMove(0));
		++i;
	}
}

bool SelfPlay::isGameFinished(const Position& p) {
	// checkmate
	if (p.isCheckMate()) {
		std::cout<<std::endl<<"CHECKMATE"<<std::endl;
		return true;
	}
	
	// patta ripetizione
	// num mosse
	if (p.isDraw(true)) {
		std::cout<<std::endl<<"DRAW"<<std::endl;
		return true;
	}
	
	// stallo
	if (p.isStaleMate()) {
		std::cout<<std::endl<<"STALEMATE"<<std::endl;
		return true;
	}
	
	// early stop
	// near 0 for x moves after ply y
	// abs(v) > x  for y moves
	
	return false;
}
