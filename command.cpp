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
#include "benchmark.h"




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
	std::cout<<"option name MultiPV type spin default 1 min 1 max 500"<<sync_endl;
	std::cout<<"option name Ponder type check default true"<<sync_endl;
	std::cout<<"option name OwnBook type check default true"<<std::endl;
	std::cout<<"option name BestMoveBook type check default false"<<std::endl;
	std::cout<<"option name UCI_EngineAbout type string default VajoletII by Marco Belli"<<sync_endl;
	std::cout<<"option name UCI_ShowCurrLine type check default false"<<sync_endl;
	std::cout<<"option name UCI_LimitStrength type check default false"<<sync_endl;
	std::cout<<"option name UCI_Elo type spin default 3000 min 1000 max 3000"<<sync_endl;
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
	m=0;
	Movegen mg(pos,m);

	while( (m=mg.getNextMove()).packed){
		if(str==pos.displayUci(m)){
			return m;
		}
	}
	// move not found
	m=0;
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
	while (is >> token && ((m = moveFromUci(pos, token)).packed != 0))
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
	searchLimits limits;
	std::string token;


    while (is >> token)
    {
        if (token == "searchmoves")
            while (is >> token){
            	Move m;
            	m=moveFromUci(pos,token);
            	if(m.packed){
            		limits.searchMoves.push_back(moveFromUci(pos,token));
            	}
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
	else if(name =="MultiPV"){
		int i=stoi(value);
		search::multiPVLines=i<500?(i>0?i:1):500;
	}
	else if(name =="OwnBook"){
		if(value=="true"){
			search::useOwnBook=true;
		}
		else{
			search::useOwnBook=false;
		}
	}
	else if(name =="BestMoveBook"){
		if(value=="true"){
			search::bestMoveBook=true;
		}
		else{
			search::bestMoveBook=false;
		}
	}
	else if(name =="UCI_ShowCurrLine"){
		if(value=="true"){
			search::showCurrentLine=true;
		}
		else{
			search::showCurrentLine=false;
		}
	}
	else if(name =="UCI_LimitStrength"){
		if(value=="true"){
			search::limitStrength=1;
		}
		else{
			search::limitStrength=0;
		}
	}
	else if(name =="UCI_Elo"){
			int i=stoi(value);
			search::eloStrenght=i<3000?(i>1000?i:1000):3000;
		}
	else{
		sync_cout << "No such option: " << name << sync_endl;
	}

}


void setvalue(std::istringstream& is) {
	std::string token, name, value;

	is >> name;

	is >> value;

	if(name =="queenMG"){
		Position::pieceValue[Position::whiteQueens].insert(0,stoi(value));
		Position::pieceValue[Position::blackQueens]=-Position::pieceValue[Position::whiteQueens];
		Position::initPstValues();
	}else if(name =="queenEG"){
		Position::pieceValue[Position::whiteQueens].insert(1,stoi(value));
		Position::pieceValue[Position::blackQueens]=-Position::pieceValue[Position::whiteQueens];
		Position::initPstValues();
	}else if(name =="rookMG"){
		Position::pieceValue[Position::whiteRooks].insert(0,stoi(value));
		Position::pieceValue[Position::blackRooks]=-Position::pieceValue[Position::whiteRooks];
		Position::initPstValues();
	}else if(name =="rookEG"){
		Position::pieceValue[Position::whiteRooks].insert(1,stoi(value));
		Position::pieceValue[Position::blackRooks]=-Position::pieceValue[Position::whiteRooks];
		Position::initPstValues();
	}else if(name =="bishopMG"){
		Position::pieceValue[Position::whiteBishops].insert(0,stoi(value));
		Position::pieceValue[Position::blackBishops]=-Position::pieceValue[Position::whiteBishops];
		Position::initPstValues();
	}else if(name =="bishopEG"){
		Position::pieceValue[Position::whiteBishops].insert(1,stoi(value));
		Position::pieceValue[Position::blackBishops]=-Position::pieceValue[Position::whiteBishops];
		Position::initPstValues();
	}else if(name =="knightMG"){
		Position::pieceValue[Position::whiteKnights].insert(0,stoi(value));
		Position::pieceValue[Position::blackKnights]=-Position::pieceValue[Position::whiteKnights];
		Position::initPstValues();
	}else if(name =="knightEG"){
		Position::pieceValue[Position::whiteKnights].insert(1,stoi(value));
		Position::pieceValue[Position::blackKnights]=-Position::pieceValue[Position::whiteKnights];
		Position::initPstValues();
	}else if(name =="pawnsEG"){
		Position::pieceValue[Position::whitePawns].insert(1,stoi(value));
		Position::pieceValue[Position::blackPawns]=-Position::pieceValue[Position::whitePawns];
		Position::initPstValues();
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
		//sync_cout<<"ricevuto comando:"<<cmd<<sync_endl;

		is >> std::skipws >> token;


		if(token== "uci"){
			printUciInfo();
		}
		else if (token =="quit" || token =="stop"){
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
		else if(token=="setvalue"){
			setvalue(is);
		}
		else if (token =="eval"){
			pawnTable pawnHashTable;
			sync_cout<<"Eval:" <<pos.eval<true>(pawnHashTable)/10000.0<<sync_endl;
			sync_cout<<"gamePhase:" <<pos.getGamePhase()/65536.0*100<<"%"<<sync_endl;
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
		else if (token =="bench"){
			benchmark();
		}
		else if (token == "ponderhit"){
			thr.ponderHit();
		}
		else{
			sync_cout<<"unknown command"<<sync_endl;
		}
	}while(token!="quit");
}
