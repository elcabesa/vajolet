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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "vajolet.h"
#include "command.h"
#include "io.h"
#include "position.h"

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void printUciInfo(void){
	sync_cout<< "id name "<<PROGRAM_NAME<<" "<<VERSION<<std::endl<<
			"id author Belli Marco"<<std::endl<< "uciok"<<sync_endl;
}



/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void uciLoop(){
	Position pos;
	std::string token, cmd;

	Move m1,m2,m3,m4,m5,m6,m7;
	//pos.display();
	m1.from=E2;
	m1.to=E4;
	pos.doMove(m1);
	//pos.display();
	m2.from=E7;
	m2.to=E5;
	pos.doMove(m2);
	//pos.display();
	m3.from=G1;
	m3.to=F3;
	pos.doMove(m3);
	//pos.display();
	m4.from=B8;
	m4.to=C6;
	pos.doMove(m4);
	//pos.display();
	m5.from=F1;
	m5.to=C4;
	pos.doMove(m5);
	//pos.display();
	m6.from=F8;
	m6.to=C5;
	pos.doMove(m6);
	//pos.display();
	m7.from=E1;
	m7.to=G1;
	m7.flags= Move::fcastle;
	pos.doMove(m7);
	//pos.display();
	pos.undoMove(m7);
	pos.undoMove(m6);
	pos.undoMove(m5);
	pos.undoMove(m4);
	pos.undoMove(m3);
	pos.undoMove(m2);
	pos.undoMove(m1);
	//pos.display();


	do{
		if (!std::getline(std::cin, cmd)) // Block here waiting for input
			cmd = "quit";
		std::istringstream is(cmd);

		is >> std::skipws >> token;

		if(token== "uci"){
			printUciInfo();
		}
		else if (token =="quit" || token =="stop" || token =="ponderhit"){
		}
		else if (token =="ucinewgame"){
		}
		else if (token =="d"){
			pos.display();
		}
		else if (token =="position"){
		}
		else if (token =="isready"){
			sync_cout<<"readyok"<<sync_endl;
		}
		else{
			sync_cout<<"unknown command"<<sync_endl;
		}
	}while(token!="quit");
}
