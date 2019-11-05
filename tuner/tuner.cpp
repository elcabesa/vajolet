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

#include <iostream>
#include <fstream>

#include "libchess.h"
#include "PGNGameCollection.h"
#include "selfplay.h"
#include "thread.h"
#include "transposition.h"
#include "vajo_io.h"

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
	sync_cout<<"Vajolet tuner"<<sync_endl;
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
	
	for( int i = 0; i < 100; ++i) { 
		SelfPlay s;
		auto g = s.playGame(i + 1);
		std::ofstream myfile;
		myfile.open ("tournament.pgn", std::fstream::app);	
		myfile<<g;
		myfile.close();
	}
	return 0;
}