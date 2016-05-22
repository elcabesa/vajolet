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
#include "syzygy/tbprobe.h"




//---------------------------------------------
//	local global constant
//---------------------------------------------

const static std::string StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
/*! \brief array of char to create the fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/



//---------------------------------------------
//	function implementation
//---------------------------------------------

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printUciInfo(void)
{
	sync_cout << "id name " << PROGRAM_NAME << " " << VERSION << sync_endl;
	sync_cout << "id author Belli Marco" << sync_endl;
	sync_cout << "option name Hash type spin default 1 min 1 max 65535" << sync_endl;
	sync_cout << "option name Threads type spin default 1 min 1 max 128" << sync_endl;
	sync_cout << "option name MultiPV type spin default 1 min 1 max 500" << sync_endl;
	sync_cout << "option name Ponder type check default true" << sync_endl;
	sync_cout << "option name OwnBook type check default true" <<sync_endl;
	sync_cout << "option name BestMoveBook type check default false"<<sync_endl;
	sync_cout << "option name UCI_EngineAbout type string default VajoletII by Marco Belli" << sync_endl;
	sync_cout << "option name UCI_ShowCurrLine type check default false" << sync_endl;
	sync_cout << "option name UCI_LimitStrength type check default false" << sync_endl;
	sync_cout << "option name UCI_Elo type spin default 3000 min 1000 max 3000" << sync_endl;
	sync_cout << "option name SyzygyPath type string default <empty>" << sync_endl;
	sync_cout << "option name SyzygyProbeDepth type spin default 1 min 1 max 100" << sync_endl;

	sync_cout << "uciok" << sync_endl;
}


/*	\brief get an input string and convert it to a valid move;
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
Move moveFromUci(const Position& pos,const  std::string& str)
{

	// idea from stockfish, we generate all the legal moves and return the legal moves with the same UCI string
	Move m;
	Movegen mg(pos);
	while( (m = mg.getNextMove()).packed)
	{
		if(str == displayUci(m))
		{
			return m;
		}
	}
	// move not found
	return Movegen::NOMOVE;
}


/*	\brief handle position command
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static position(std::istringstream& is, Position & pos)
{
	std::string token, fen;
	is >> token;
	if (token == "startpos")
	{
		fen = StartFEN;
		is >> token; // Consume "moves" token if any
	}
	else if (token == "fen")
	{
		while (is >> token && token != "moves")
		{
			fen += token + " ";
		}
	}
	else
	{
		return;
	}

	pos.setupFromFen(fen);

	Move m;
	// Parse move list (if any)
	while (is >> token && ((m = moveFromUci(pos, token)) != Movegen::NOMOVE))
	{
		pos.doMove(m);
	}
}

/*	\brief handle perft command
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void static doPerft(const unsigned int n, Position & pos)
{

	unsigned long long elapsed = search::getTime();
	unsigned long long res = pos.perft(n);
	elapsed = search::getTime() - elapsed;

	sync_cout << "Perft " << n << " leaf nodes: " << res << sync_endl;
	sync_cout << elapsed << "ms " << ((double)res) / (double)elapsed << " kN/s" << sync_endl;
}


void static go(std::istringstream& is, Position & pos, my_thread * thr)
{
	searchLimits limits;
	std::string token;
	bool searchMovesCommand = false;


    while (is >> token)
    {
        if (token == "searchmoves")		{searchMovesCommand = true;}
        else if (token == "wtime")		{is >> limits.wtime;searchMovesCommand = false;}
        else if (token == "btime")		{is >> limits.btime;searchMovesCommand = false;}
        else if (token == "winc")		{is >> limits.winc;searchMovesCommand = false;}
        else if (token == "binc")		{is >> limits.binc;searchMovesCommand = false;}
        else if (token == "movestogo")	{is >> limits.movesToGo;searchMovesCommand = false;}
        else if (token == "depth")		{is >> limits.depth;searchMovesCommand = false;}
        else if (token == "nodes")		{is >> limits.nodes;searchMovesCommand = false;}
        else if (token == "movetime")	{is >> limits.moveTime;searchMovesCommand = false;}
        else if (token == "mate")		{is >> limits.mate;searchMovesCommand = false;}
        else if (token == "infinite")	{limits.infinite = true;searchMovesCommand = false;}
        else if (token == "ponder")		{limits.ponder = true;searchMovesCommand = false;}
        else if (searchMovesCommand == true)
        {
        	Move m = moveFromUci(pos, token);
			if(m.packed)
			{
				sync_cout << "mossa " << token << sync_endl;
				limits.searchMoves.push_back( moveFromUci(pos, token) );
			}
        }
    }
    thr->startThinking(&pos,limits);
}




void setoption(std::istringstream& is)
{
	std::string token, name, value;

	is >> token; // Consume "name" token

	// Read option name (can contain spaces)
	while (is >> token && token != "value")
	{
		name += std::string(" ", !name.empty()) + token;
	}
	// Read option value (can contain spaces)
	while (is >> token)
	{
		value += std::string(" ", !value.empty()) + token;
	}

	if(name == "Hash")
	{
		int hash = stoi(value);
		hash = std::min(hash,65536);
		TT.setSize(hash);
	}
	else if(name == "Threads")
	{
		int i = stoi(value);
		search::threads = (i<=128)?(i>0?i:1):128;
	}
	else if(name == "MultiPV")
	{
		int i = stoi(value);
		search::multiPVLines = i<500 ? (i>0 ? i : 1) : 500;
	}
	else if(name == "OwnBook")
	{
		if(value=="true")
		{
			search::useOwnBook = true;
		}
		else{
			search::useOwnBook = false;
		}
	}
	else if(name == "BestMoveBook")
	{
		if(value == "true")
		{
			search::bestMoveBook = true;
		}
		else
		{
			search::bestMoveBook = false;
		}
	}
	else if(name == "UCI_ShowCurrLine")
	{
		if(value == "true"){
			search::showCurrentLine = true;
		}
		else
		{
			search::showCurrentLine = false;
		}
	}
	else if(name == "UCI_LimitStrength")
	{
		if(value == "true")
		{
			search::limitStrength = 1;
		}
		else
		{
			search::limitStrength = 0;
		}
	}
	else if(name == "UCI_Elo")
	{
		int i = stoi(value);
		search::eloStrenght = i<3000 ?( i>1000 ? i : 1000) : 3000;
	}
	else if(name == "Ponder")
	{
	}
	else if(name == "SyzygyPath")
	{
		search::SyzygyPath = value;
		tb_init(search::SyzygyPath.c_str());
		sync_cout<<"info string TB_LARGEST = "<<TB_LARGEST<<sync_endl;
	}
	else if(name == "SyzygyProbeDepth")
	{
		search::SyzygyProbeDepth = stoi(value);
	}
	else if(name == "Syzygy50MoveRule")
	{
		if(value == "true")
		{
			search::Syzygy50MoveRule = true;
		}
		else
		{
			search::Syzygy50MoveRule = false;
		}
	}
	else
	{
		sync_cout << "No such option: " << name << sync_endl;
	}

}

/*
void setvalue(std::istringstream& is)
{
	std::string token, name, value;

	is >> name;

	is >> value;

	if(name =="queenMG")
	{
		Position::pieceValue[Position::whiteQueens].insert(0,stoi(value));
		Position::pieceValue[Position::blackQueens].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="queenEG")
	{
		Position::pieceValue[Position::whiteQueens].insert(1,stoi(value));
		Position::pieceValue[Position::blackQueens].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="rookMG")
	{
		Position::pieceValue[Position::whiteRooks].insert(0,stoi(value));
		Position::pieceValue[Position::blackRooks].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="rookEG")
	{
		Position::pieceValue[Position::whiteRooks].insert(1,stoi(value));
		Position::pieceValue[Position::blackRooks].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="bishopMG")
	{
		Position::pieceValue[Position::whiteBishops].insert(0,stoi(value));
		Position::pieceValue[Position::blackBishops].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="bishopEG")
	{
		Position::pieceValue[Position::whiteBishops].insert(1,stoi(value));
		Position::pieceValue[Position::blackBishops].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="knightMG")
	{
		Position::pieceValue[Position::whiteKnights].insert(0,stoi(value));
		Position::pieceValue[Position::blackKnights].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="knightEG")
	{
		Position::pieceValue[Position::whiteKnights].insert(1,stoi(value));
		Position::pieceValue[Position::blackKnights].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="pawnsEG")
	{
		Position::pieceValue[Position::whitePawns].insert(1,stoi(value));
		Position::pieceValue[Position::blackPawns].insert(1,stoi(value));
		Position::initPstValues();
	}
}*/


/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void uciLoop()
{
	my_thread *thr = my_thread::getInstance();
	Position pos;
	pos.setupFromFen(StartFEN);
	std::string token, cmd;

	do
	{
		assert(thr);
		if (!std::getline(std::cin, cmd)) // Block here waiting for input
			cmd = "quit";
		std::istringstream is(cmd);

		is >> std::skipws >> token;


		if(token== "uci")
		{
			printUciInfo();
		}
		else if (token == "quit" || token == "stop")
		{
			thr->stopThinking();
		}
		else if (token == "ucinewgame")
		{
		}
		else if (token == "d")
		{
			pos.display();
		}
		else if (token == "position")
		{
			position(is,pos);
		}
		else if(token == "setoption")
		{
			setoption(is);
		}
		/*else if(token == "setvalue")
		{
			setvalue(is);
		}*/
		else if (token == "eval")
		{
			sync_cout << "Eval:" << pos.eval<true>() / 10000.0 << sync_endl;
			sync_cout << "gamePhase:"  << pos.getGamePhase()/65536.0*100 << "%" << sync_endl;
#ifdef DEBUG_EVAL_SIMMETRY

			Position ppp;
			ppp.setupFromFen(pos.getSymmetricFen());
			ppp.display();
			sync_cout << "Eval:"  << ppp.eval<true>() / 10000.0 << sync_endl;
			sync_cout << "gamePhase:" << ppp.getGamePhase()/65536.0*100 << "%" << sync_endl;

#endif

		}
		else if (token == "isready")
		{
			sync_cout << "readyok" << sync_endl;
		}
		else if (token == "perft" && (is>>token))
		{
			doPerft(stoi(token), pos);
		}
		else if (token == "divide" && (is>>token))
		{
			unsigned long long res = pos.divide(stoi(token));
			sync_cout << "divide Res= " << res << sync_endl;
		}
		else if (token == "go")
		{
			go(is, pos, thr);
		}
		else if (token == "bench")
		{
			benchmark();
		}
		else if (token == "ponderhit")
		{
			thr->ponderHit();
		}
		else
		{
			sync_cout << "unknown command" << sync_endl;
		}
	}while(token!="quit");

	thr->quitThreads();
}

/*! \brief return the uci string for a given move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
std::string displayUci(const Move & m){


	std::string s;

	if(m.packed==0)
	{
		s = "0000";
		return s;
	}

	//from
	s += char('a'+FILES[m.bit.from]);
	s += char('1'+RANKS[m.bit.from]);


	//to
	s += char('a'+FILES[m.bit.to]);
	s += char('1'+RANKS[m.bit.to]);
	//promotion
	if(m.bit.flags == Move::fpromotion)
	{
		s += PIECE_NAMES_FEN[m.bit.promotion+Position::blackQueens];
	}
	return s;

}

/*! \brief return the uci string for a given move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
std::string displayMove(const Position& pos, const Move & m)
{

	std::string s;

	bool capture = (bitSet((tSquare)m.bit.to) & pos.getTheirBitmap(Position::Pieces));

	if( !pos.isPawn(pos.getPieceAt( (tSquare)m.bit.from)) )
	{
		s+=PIECE_NAMES_FEN[pos.getPieceAt((tSquare)m.bit.from) % Position::separationBitmap];
	}
	if(capture && pos.isPawn(pos.getPieceAt((tSquare)m.bit.from)))
	{
		s+=char('a'+FILES[m.bit.from]);
	}
	if(capture)
	{
		s+="x";
	}




	//to
	s += char('a'+FILES[ m.bit.to ]);
	s += char('1'+RANKS[ m.bit.to ]);
	if( pos.moveGivesCheck(m) )
	{
		s+="+";
	}
	//promotion
	if(m.bit.flags == Move::fpromotion)
	{
		s += "=";
		s += PIECE_NAMES_FEN[m.bit.promotion + Position::whiteQueens];
	}
	return s;

}

void printCurrMoveNumber(unsigned int moveNumber, const Move &m, unsigned long long visitedNodes, long long int time)
{
	sync_cout << "info currmovenumber " << moveNumber << " currmove " << displayUci(m) << " nodes " << visitedNodes <<
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			" time " << time <<
#endif
	sync_endl;
}

void showCurrLine(const Position & pos, unsigned int ply)
{
	sync_cout << "info currline";
	unsigned int start = pos.getStateIndex()-ply + 1;

	for (unsigned int i = start; i<= start+ply/2; i++) // show only half of the search line
	{
		std::cout << " " << displayUci(pos.getState(i).currentMove);
	}
	std::cout << sync_endl;

}


void printPVs(unsigned int count)
{

	int i= 0;
	std::for_each(search::rootMoves.begin(),std::next(search::rootMoves.begin(), count), [&](rootMove& rm)
	{
		if(rm.nodes)
		{
			printPV(rm.score, rm.depth, rm.maxPlyReached, -SCORE_INFINITE, SCORE_INFINITE, rm.time, i, rm.PV, rm.nodes,search::tbHits);
		}
		i++;
	});
}

void printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, long long time,unsigned int count,std::list<Move>& PV,unsigned long long nodes, unsigned long long tbHits)
{

	sync_cout<<"info multipv "<< (count+1) << " depth "<< (depth) <<" seldepth "<< seldepth <<" score ";

	if(abs(res) >SCORE_MATE_IN_MAX_PLY)
	{
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else
	{
		int satRes = std::min(res,SCORE_MAX_OUTPUT_VALUE);
		satRes = std::max(satRes,SCORE_MIN_OUTPUT_VALUE);
		std::cout << "cp "<< (int)((float)satRes/100.0);
	}

	std::cout << (res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");

	std::cout << " nodes " << nodes <<" tbhits "<<tbHits;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout << " nps " << (unsigned int)((double)nodes*1000/(double)time) << " time " << (long long int)(time);
#endif

	std::cout << " pv ";
	for_each( PV.begin(), PV.end(), [&](Move &m){std::cout<<displayUci(m)<<" ";});
	std::cout<<sync_endl;
}
