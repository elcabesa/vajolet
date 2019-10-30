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

#include "position.h"
#include "selfplay.h"
#include "searchLimits.h"
#include "thread.h"
#include "vajo_io.h"

void SelfPlay::playGame() {
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	SearchLimits sl;
	sl.setDepth(15);
	
	my_thread::getInstance().startThinking(p, sl);
	my_thread::getInstance().finished().wait(lock);
	sync_cout<<"end"<<sync_endl;
	my_thread::getInstance().startThinking(p, sl);
	my_thread::getInstance().finished().wait(lock);
	
}
