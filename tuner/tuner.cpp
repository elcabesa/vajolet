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

#include <csignal>
#include <iostream>

#include "libchess.h"
#include "player.h"
#include "thread.h"
#include "tournament.h"
#include "transposition.h"

void signalHandler(int signum)
{
	my_thread::getInstance().stopThinking();
	exit(signum);
}

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	std::cout <<"Vajolet tuner"<< std::endl;
}

int main() {
	
	signal(SIGINT, signalHandler); 
#ifdef SIGBREAK	
	signal(SIGBREAK, signalHandler); 
#endif
#ifdef SIGHUP		
	signal(SIGHUP, signalHandler);  
#endif
	
	//pgn::GameCollection gc;
	printStartInfo();
	//----------------------------------
	//	init global data
	//----------------------------------
	libChessInit();
	transpositionTable::getInstance().setSize(1);
	
	
	Player p1("p1");
	Player p2("p2");
	p1.getSearchParameters().razorMargin = 20000;
	p1.getSearchParameters().razorMarginDepth = 0;
	p1.getSearchParameters().razorMarginCut = 0;
	
	
	//----------------------------------
	//	play tournament
	//----------------------------------
	for(int round  = 1; round <=TunerParameters::roundNumber; ++round) {
		Tournament t("tournament.pgn", p1, p2);
		auto res = t.play();
		std::cout<<"Tournament Result: "<<static_cast<int>(res)<<std::endl;
	}
	
	return 0;
}