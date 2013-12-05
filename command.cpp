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

//---------------------------------------------
//	include
//---------------------------------------------
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iterator>
#include "vajolet.h"
#include "command.h"
#include "io.h"
#include "position.h"
#include "movegen.h"
#include "move.h"
#include "search.h"
#include "thread.h"
#include "transposition.h"




//---------------------------------------------
//	local global constant
//---------------------------------------------

const static char* StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


//---------------------------------------------
//	function implementation
//---------------------------------------------

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printUciInfo(void){
	sync_cout<< "id name "<<PROGRAM_NAME<<" "<<VERSION<<std::endl;
	std::cout<<"id author Belli Marco"<<std::endl;
	std::cout<<"option name Hash type spin default 1 min 1 max 4096"<<sync_endl;
	std::cout<<"uciok"<<sync_endl;
}


/*	\brief get an input string and convert it to a valid move;
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
Move moveFromUci(Position& pos, std::string& str) {

	// idea from stockfish, we generate all the legal moves and return the legal moves with the same UCI string
	Move m;
	m.packed=0;
	Movegen mg(pos,m);

	while( (m=mg.getNextMove()).packed){
		if(str==pos.displayUci(m)){
			return m;
		}
	}
	// move not found
	m.packed=0;
	return m; // m è gia una NOMOVE
}


/*	\brief handle position command
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static position(std::istringstream& is, Position & pos){
	std::string token, fen;
	is >> token;
	if (token == "startpos")
	{
		fen = StartFEN;
		is >> token; // Consume "moves" token if any
	}
	else if (token == "fen")
		while (is >> token && token != "moves")
			fen += token + " ";
	else
		return;
	pos.setupFromFen(fen);

	Move m;
	// Parse move list (if any)
	while (is >> token && (m = moveFromUci(pos, token)).packed != 0)
	{
		pos.doMove(m);
	}
}

/*	\brief handle perft command
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void static doPerft(unsigned int n, Position & pos){
	std::string token;
	unsigned long elapsed = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
	unsigned long long res=pos.perft(n);
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count()-elapsed;
	sync_cout<<"Perft "<<n<<" leaf nodes: "<<res<<sync_endl;
	sync_cout<<elapsed<<"ms "<<((double)res)/elapsed<<" kN/s"<<sync_endl;
}


void static go(std::istringstream& is, Position & pos, my_thread & thr) {

	searcLimits limits;
	std::string token;


    while (is >> token)
    {
        if (token == "searchmoves")
            while (is >> token){
                limits.searchMoves.push_back(moveFromUci(pos,token));
            }
        else if (token == "wtime")     is >> limits.wtime;
        else if (token == "btime")     is >> limits.btime;
        else if (token == "winc")      is >> limits.winc;
        else if (token == "binc")      is >> limits.binc;
        else if (token == "movestogo") is >> limits.movesToGo;
        else if (token == "depth")     is >> limits.depth;
        else if (token == "nodes")     is >> limits.nodes;
        else if (token == "movetime")  is >> limits.moveTime;
        else if (token == "mate")      is >> limits.mate;
        else if (token == "infinite")  limits.infinite = true;
        else if (token == "ponder")    limits.ponder = true;
    }

    thr.startTinking(&pos,limits);
}




void setoption(std::istringstream& is) {
	std::string token, name, value;

	is >> token; // Consume "name" token

	// Read option name (can contain spaces)
	while (is >> token && token != "value"){
		name += std::string(" ", !name.empty()) + token;
	}
	// Read option value (can contain spaces)
	while (is >> token){
		value += std::string(" ", !value.empty()) + token;
	}

	if(name =="Hash"){
		TT.setSize(stoi(value));
	}
	else{
		sync_cout << "No such option: " << name << sync_endl;
	}

}


/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void uciLoop(){
	my_thread thr;
	Position pos;
	pos.setupFromFen(StartFEN);
	std::string token, cmd;

	do{
		if (!std::getline(std::cin, cmd)) // Block here waiting for input
			cmd = "quit";
		std::istringstream is(cmd);

		is >> std::skipws >> token;


		if(token== "uci"){
			printUciInfo();
		}
		else if (token =="quit" || token =="stop" || token =="ponderhit"){
			thr.stopThinking();
		}
		else if (token =="ucinewgame"){
		}
		else if (token =="d"){
			pos.display();
		}
		else if (token =="position"){
			position(is,pos);
		}
		else if(token=="setoption"){
			setoption(is);
		}
		else if (token =="eval"){
			sync_cout<<"Eval:" <<pos.eval()/10000.0<<sync_endl;
			sync_cout<<"gamePhase:" <<pos.getGamePhase()/65535.0*100<<"%"<<sync_endl;
		}
		else if (token =="isready"){
			sync_cout<<"readyok"<<sync_endl;
		}
		else if (token =="perft" && (is>>token)){
			doPerft(stoi(token), pos);

		}
		else if (token =="divide" && (is>>token)){
			unsigned long res=pos.divide(stoi(token));
			sync_cout<<"divide Res="<<res<<sync_endl;
		}
		else if (token =="go"){
			go(is,pos,thr);

		}
		else{
			sync_cout<<"unknown command"<<sync_endl;
		}
	}while(token!="quit");
}
