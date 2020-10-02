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
#include <fstream>
#include <string>

#include "book.h"
#include "fenSaver.h"
#include "player.h"
#include "position.h"
#include "tournament.h"

#include "libchess.h"
#include "vajo_io.h"

void signalHandler(int signum)
{
	exit(signum);
}

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	std::cout <<"Vajolet fen generator"<< std::endl;
}

int main() {
	
	signal(SIGINT, signalHandler); 
#ifdef SIGBREAK	
	signal(SIGBREAK, signalHandler); 
#endif
#ifdef SIGHUP		
	signal(SIGHUP, signalHandler);  
#endif

	printStartInfo();
	//----------------------------------
	//	init global data
	//----------------------------------
	libChessInit();

	//----------------------------------
	//	setup hyperparameters
	//----------------------------------
	/*TunerParameters::gameNumber = 9e6;
	TunerParameters::gameTime = 10;
	TunerParameters::gameTimeIncrement = 0.1;

	FenSaver fs(5);
	Book book("book.pgn");
	Player p1("p1");
	Player p2("p2");
	Tournament t("tournament.pgn", "tournament.txt",p1, p2, book, &fs, true);
	auto res = t.play();
	sync_cout<<"Tournament Result: "<<static_cast<int>(res)<<sync_endl;*/
    
    FenSaver fs(1);
    Position pos;
    std::string line;
    std::ifstream myfile ("quiet.epd");
    if (myfile.is_open())
    {
        //unsigned int x = 0;
        while ( getline (myfile,line) )
        {
            //std::cout << (++x) << std::endl;
            fs.save(pos.setupFromFen(line));
        }
        myfile.close();
    }
    else std::cout << "Unable to open file"<<std::endl;

	return 0;
}
