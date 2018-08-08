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
#include "parameters.h"




//---------------------------------------------
//	local global constant
//---------------------------------------------
const char PIECE_NAMES_FEN[] = {' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};
static bool reduceVerbosity = false;

const static std::string StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
/*! \brief array of char to create the fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/



//---------------------------------------------
//	function implementation
//---------------------------------------------

char getPieceName( const unsigned int idx )
{
	assert( idx < Position::lastBitboard );
	return PIECE_NAMES_FEN[ idx ];
}

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printUciInfo(void)
{
	sync_cout << "id name " << PROGRAM_NAME << " " << VERSION PRE_RELEASE << sync_endl;
	sync_cout << "id author Marco Belli" << sync_endl;
	sync_cout << "option name Hash type spin default 1 min 1 max 65535" << sync_endl;
	sync_cout << "option name Threads type spin default 1 min 1 max 128" << sync_endl;
	sync_cout << "option name MultiPV type spin default 1 min 1 max 500" << sync_endl;
	sync_cout << "option name Ponder type check default true" << sync_endl;
	sync_cout << "option name OwnBook type check default true" <<sync_endl;
	sync_cout << "option name BestMoveBook type check default false"<<sync_endl;
	sync_cout << "option name UCI_EngineAbout type string default " << PROGRAM_NAME << " " << VERSION PRE_RELEASE<< " by Marco Belli (build date: " <<__DATE__ <<" "<< __TIME__<<")"<<sync_endl;
	sync_cout << "option name UCI_ShowCurrLine type check default false" << sync_endl;
	sync_cout << "option name SyzygyPath type string default <empty>" << sync_endl;
	sync_cout << "option name SyzygyProbeDepth type spin default 1 min 1 max 100" << sync_endl;
	sync_cout << "option name ClearHash type button" << sync_endl;
	sync_cout << "option name PerftUseHash type check default false" << sync_endl;
	sync_cout << "option name reduceVerbosity type check default false" << sync_endl;

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

	unsigned long long elapsed = Search::getTime();
	unsigned long long res = pos.perft(n);
	elapsed = Search::getTime() - elapsed;

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
	
	if( token != "name" )
	{
		sync_cout << "info string malformed command"<< sync_endl;
		return;
		
	}
	

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
	
	if( value.empty() && (name != "SyzygyPath" && name != "UCI_EngineAbout" && name != "ClearHash")) // sygyzy path is allowed to have an empty value
	{
		sync_cout << "info string malformed command"<< sync_endl;
		return;
		
	}

	if(name == "Hash")
	{
		int hash = 1;
		try
		{
			hash = std::stoi(value);
		}
		catch(...)
		{
			hash = 1;
		}
		hash = std::min(hash,65535);
		unsigned long elements = transpositionTable::getInstance().setSize(hash);
		sync_cout<<"info string hash table allocated, "<<elements<<" elements ("<<hash<<"MB)"<<sync_endl;

	}
	else if(name == "ClearHash")
	{
		transpositionTable::getInstance().clear();
	}
	else if(name == "PerftUseHash")
	{
		if(value=="true")
		{
			Position::perftUseHash = true;
		}
		else
		{
			Position::perftUseHash = false;
		}
	}
	else if(name == "reduceVerbosity")
	{
		if(value=="true")
		{
			reduceVerbosity = true;
		}
		else
		{
			reduceVerbosity = false;
		}
	}
	else if(name == "Threads")
	{
		try
		{
			int i = std::stoi(value);
			Search::threads = (i<=128)?(i>0?i:1):128;
			sync_cout<<"info string Threads number set to "<<Search::threads<<sync_endl;
		}
		catch(...){}
	}
	else if(name == "MultiPV")
	{
		try
		{
			int i = std::stoi(value);
			Search::multiPVLines = i<500 ? (i>0 ? i : 1) : 500;
			sync_cout<<"info string MultiPv Lines set to "<<Search::multiPVLines<<sync_endl;
		}
		catch(...){}	
	}
	else if(name == "OwnBook")
	{
		if(value=="true")
		{
			Search::useOwnBook = true;
			sync_cout<<"info string OwnBook option set to true"<<sync_endl;
		}
		else{
			Search::useOwnBook = false;
			sync_cout<<"info string OwnBook option set to false"<<sync_endl;
		}
	}
	else if(name == "BestMoveBook")
	{
		if(value == "true")
		{
			Search::bestMoveBook = true;
			sync_cout<<"info string BestMoveBook option set to true"<<sync_endl;
		}
		else
		{
			Search::bestMoveBook = false;
			sync_cout<<"info string BestMoveBook option set to false"<<sync_endl;
		}
	}
	else if(name == "UCI_ShowCurrLine")
	{
		if(value == "true")
		{
			Search::showCurrentLine = true;
			sync_cout<<"info string UCI_ShowCurrLine option set to true"<<sync_endl;
		}
		else
		{
			Search::showCurrentLine = false;
			sync_cout<<"info string UCI_ShowCurrLine option set to false"<<sync_endl;
		}
	}
	else if(name == "Ponder")
	{
		sync_cout<<"info string Ponder option set to "<<value<<sync_endl;
	}
	else if(name == "SyzygyPath")
	{
		Search::SyzygyPath = value;
		tb_init(Search::SyzygyPath.c_str());
		sync_cout<<"info string TB_LARGEST = "<<TB_LARGEST<<sync_endl;
	}
	else if(name == "SyzygyProbeDepth")
	{
		try
		{
			Search::SyzygyProbeDepth = std::max(std::stoi(value),0) ;
		}
		catch(...)
		{
			Search::SyzygyProbeDepth = 1;
		}
		sync_cout<<"info string SyzygyProbeDepth option set to "<<Search::SyzygyProbeDepth<<sync_endl;
	}
	else if(name == "Syzygy50MoveRule")
	{
		if(value == "true")
		{
			Search::Syzygy50MoveRule = true;
			sync_cout<<"info string Syzygy50MoveRule option set to true"<<sync_endl;
		}
		else
		{
			Search::Syzygy50MoveRule = false;
			sync_cout<<"info string Syzygy50MoveRule option set to false"<<sync_endl;
		}
	}
	else if(name == "UCI_EngineAbout")
	{
		sync_cout<< PROGRAM_NAME << " " << VERSION PRE_RELEASE<< " by Marco Belli (build date: " <<__DATE__ <<" "<< __TIME__<<")"<<sync_endl;
	}
	else
	{
		sync_cout << "info string No such option: " << name << sync_endl;
	}

}


void setvalue(std::istringstream& is)
{
	std::string token, name, value;

	is >> name;

	is >> value;

	if(name =="KingAttackWeights0")
	{
		KingAttackWeights[0] = stoi(value);
	}
	else if(name =="KingAttackWeights1")
	{
		KingAttackWeights[1] = stoi(value);
	}
	else if(name =="KingAttackWeights2")
	{
		KingAttackWeights[2] = stoi(value);
	}
	else if(name =="KingAttackWeights3")
	{
		KingAttackWeights[3] = stoi(value);
	}
	else if(name =="kingShieldBonus")
	{
		kingShieldBonus[0] = stoi(value);
	}
	else if(name =="kingFarShieldBonus")
	{
		kingFarShieldBonus[0] = stoi(value);
	}
	else if(name =="kingStormBonus0")
	{
		kingStormBonus[0] = stoi(value);
	}
	else if(name =="kingStormBonus1")
	{
		kingStormBonus[1] = stoi(value);
	}
	else if(name =="kingStormBonus2")
	{
		kingStormBonus[2] = stoi(value);
	}
	else if(name =="kingSafetyBonus0")
	{
		kingSafetyBonus[0] = stoi(value);
	}
	else if(name =="kingSafetyBonus1")
	{
		kingSafetyBonus[1] = stoi(value);
	}
	else if(name =="kingSafetyPars10")
	{
		kingSafetyPars1[0] = stoi(value);
	}
	else if(name =="kingSafetyPars11")
	{
		kingSafetyPars1[1] = stoi(value);
	}
	else if(name =="kingSafetyPars12")
	{
		kingSafetyPars1[2] = stoi(value);
	}
	else if(name =="kingSafetyPars13")
	{
		kingSafetyPars1[3] = stoi(value);
	}
	else if(name =="kingSafetyPars20")
	{
		kingSafetyPars2[0] = stoi(value);
	}
	else if(name =="kingSafetyPars21")
	{
		kingSafetyPars2[1] = stoi(value);
	}
	else if(name =="kingSafetyPars22")
	{
		kingSafetyPars2[2] = stoi(value);
	}
	else if(name =="kingSafetyPars23")
	{
		kingSafetyPars2[3] = stoi(value);
	}
}


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

		token.clear();
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
		else if(token == "setvalue")
		{
			setvalue(is);
		}
		else if (token == "eval")
		{
			Score s = pos.eval<true>();
			sync_cout << "Eval:" <<  s / 10000.0 << sync_endl;
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
			int n = 1;
			try
			{
				n = std::stoi(token);
			}
			catch(...)
			{
				n = 1;
			}
			n = std::max(n,1);
			doPerft(n, pos);
		}
		else if (token == "divide" && (is>>token))
		{
			int n = 1;
			try
			{
				n = std::stoi(token);
			}
			catch(...)
			{
				n = 1;
			}
			
			unsigned long long res = pos.divide(n);
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

	bool capture = pos.isCaptureMove(m);
	bool check = pos.moveGivesCheck(m);
	bool doubleCheck = pos.moveGivesDoubleCheck(m);
	unsigned int legalMoves;
	Position::bitboardIndex piece = pos.getPieceAt((tSquare)m.bit.from);
	bool pawnMove = pos.isPawn(piece);
	bool isPromotion = m.isPromotionMove();
	bool isEnPassant = m.isEnPassantMove();
	bool isCastle = m.isCastleMove();

	bool disambigusFlag = false;
	bool fileFlag = false;
	bool rankFlag = false;


	// calc legalmoves
	{
		Position p = pos;
		p.doMove(m);
		Movegen mg(p);
		legalMoves = mg.getNumberOfLegalMoves();
		p.undoMove();
	}


	{
		Movegen mg(pos);
		unsigned int lm = mg.getNumberOfLegalMoves();

		// calc disambigus data
		for (unsigned int i = 0; i < lm; i++)
		{
			Move mm = mg.getMoveFromMoveList(i);
			if( pos.getPieceAt((tSquare)mm.bit.from) == piece && (mm.bit.to == m.bit.to) && (mm.bit.from != m.bit.from))
			{
				disambigusFlag = true;
				if(FILES[mm.bit.from] == FILES[m.bit.from])
				{
					rankFlag = true;
				}
				if(RANKS[mm.bit.from] == RANKS[m.bit.from])
				{
					fileFlag = true;
				}

			}

		}
		if( disambigusFlag && !rankFlag && !fileFlag)
		{
			fileFlag = true;
		}
	}


	// castle move
	if (isCastle)
	{
		bool kingSide = m.bit.to > m.bit.from;
		if( kingSide)
		{
			s = "O-O";
		}
		else
		{
			s = "O-O-O";
		}
		return s;

	}


	// 1 ) use the name of the piece if it's not a pawn move
	if( !pawnMove )
	{
		s+=PIECE_NAMES_FEN[piece % Position::separationBitmap];
	}
	if( fileFlag )
	{
		s += char('a'+FILES[ m.bit.from ]);
	}
	if( rankFlag )
	{
		s += char('1'+RANKS[ m.bit.from ]);
	}


	//capture motation
	if (capture)
	{
		if(pawnMove)
		{
			s+=char('a'+FILES[m.bit.from]);
		}
		// capture add x before to square
		s+="x";
	}
	// to square
	s += char('a'+FILES[ m.bit.to ]);
	s += char('1'+RANKS[ m.bit.to ]);
	// add en passant info
	if ( isEnPassant )
	{
		s+="e.p.";
	}
	//promotion add promotion to
	if(isPromotion)
	{
		s += "=";
		s += PIECE_NAMES_FEN[m.bit.promotion + Position::whiteQueens];
	}
	// add check information
	if( check  )
	{
		if( legalMoves > 0 )
		{
			if( doubleCheck )
			{
				s+="++";
			}
			else
			{
				s+="+";
			}
		}
		else
		{
			s+="#";
		}
	}
	return s;

}





/*****************************
uci concrete output definitions
******************************/
class UciMuteOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm) const;
	void printPV(const Score res, const unsigned int depth, const unsigned int seldepth, const Score alpha, const Score beta, const long long time, const unsigned int count, std::list<Move>& PV, const unsigned long long nodes) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth(const unsigned int depth) const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move m, const Move ponder = Move(0) ) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};

class UciStandardOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm) const;
	void printPV(const Score res, const unsigned int depth, const unsigned int seldepth, const Score alpha, const Score beta, const long long time, const unsigned int count, std::list<Move>& PV, const unsigned long long nodes) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth(const unsigned int depth) const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move m, const Move ponder = Move(0)) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};


/*****************************
uci standard output implementation
******************************/
void UciStandardOutput::printPVs(std::vector<rootMove>& rmList) const
{

	int i= 0;
	for ( auto & rm : rmList )
	{
		if(rm.nodes)
		{
			printPV(rm.score, rm.depth, rm.maxPlyReached, -SCORE_INFINITE, SCORE_INFINITE, rm.time, i, rm.PV, rm.nodes );
		}
		i++;
	}
}

void UciStandardOutput::printPV(const Score res, const unsigned int depth, const unsigned int seldepth, const Score alpha, const Score beta, const long long time, const unsigned int count, std::list<Move>& PV, const unsigned long long nodes) const
{

	sync_cout<<"info multipv "<< (count+1) << " depth "<< (depth) <<" seldepth "<< seldepth <<" score ";

	if(abs(res) >SCORE_MATE_IN_MAX_PLY)
	{
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else
	{
		//int satRes = std::min(res,SCORE_MAX_OUTPUT_VALUE);
		//satRes = std::max(satRes,SCORE_MIN_OUTPUT_VALUE);
		std::cout << "cp "<< (int)((float)res/100.0);
	}

	std::cout << (res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");

	std::cout << " nodes " << nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout << " nps " << (unsigned int)((double)nodes*1000/(double)time) << " time " << (long long int)(time);
#endif

	std::cout << " pv ";
	for_each( PV.begin(), PV.end(), [&](Move &m){std::cout<<displayUci(m)<<" ";});
	std::cout<<sync_endl;
}

void UciStandardOutput::printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const
{
	sync_cout << "info currmovenumber " << moveNumber << " currmove " << displayUci(m) << " nodes " << visitedNodes <<
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			" time " << time <<
#endif
	sync_endl;
}

void UciStandardOutput::showCurrLine(const Position & pos, const unsigned int ply) const
{
	sync_cout << "info currline";
	unsigned int start = pos.getStateSize() - ply;

	for (unsigned int i = start; i<= start+ply/2; i++) // show only half of the search line
	{
		std::cout << " " << displayUci(pos.getState(i).currentMove);
	}
	std::cout << sync_endl;

}
void UciStandardOutput::printDepth(const unsigned int depth) const
{
	sync_cout<<"info depth "<<depth<<sync_endl;
}

void UciStandardOutput::printScore(const signed int cp) const
{
	sync_cout<<"info score cp "<<cp<<sync_endl;
}

void UciStandardOutput::printBestMove( const Move bm, const Move ponder ) const
{
	sync_cout<<"bestmove "<< displayUci(bm);
	if( ponder.packed != 0 )
	{
		std::cout<<" ponder " << displayUci(ponder);
	}
	std::cout<< sync_endl;
}

void UciStandardOutput::printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const
{
	if( !reduceVerbosity )
	{
		long long int localtime = std::max(time,1ll);
		sync_cout<<"info hashfull " << fullness << " tbhits " << thbits << " nodes " << nodes <<" time "<< time << " nps " << (unsigned int)((double)nodes*1000/(double)localtime) << sync_endl;
	}
}

/*****************************
uci Mute output implementation
******************************/
void UciMuteOutput::printPVs(std::vector<rootMove>&) const{}
void UciMuteOutput::printPV(const Score ,const unsigned int , const unsigned int ,const Score , const Score , const long long, const unsigned int, std::list<Move>&, const unsigned long long ) const{}
void UciMuteOutput::printCurrMoveNumber(const unsigned int, const Move& , const unsigned long long , const long long int ) const {}
void UciMuteOutput::showCurrLine(const Position & , const unsigned int ) const{}
void UciMuteOutput::printDepth(const unsigned int ) const{}
void UciMuteOutput::printScore(const signed int ) const{}
void UciMuteOutput::printBestMove( const Move, const Move ) const{}
void UciMuteOutput::printGeneralInfo( const unsigned int , const unsigned long long int , const unsigned long long int , const long long int ) const{}



/*****************************
uci output factory method implementation
******************************/
std::unique_ptr<UciOutput> UciOutput::create( const UciOutput::type t )
{
	if( t == standard)
	{
		return std::make_unique<UciStandardOutput>();
	}
	else/* if(t == mute)*/
	{
		return std::make_unique<UciMuteOutput>();
	}
}