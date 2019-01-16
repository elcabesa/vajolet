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

#include "benchmark.h"
#include "command.h"
#include "io.h"
#include "movepicker.h"
#include "parameters.h"
#include "position.h"
#include "pvLine.h"
#include "rootMove.h"
#include "searchTimer.h"
#include "searchLimits.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"
#include "vajolet.h"
#include "version.h"



//---------------------------------------------
//	local global constant
//---------------------------------------------
const char PIECE_NAMES_FEN[] = {' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};


const static std::string StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
/*! \brief array of char to create the fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/

/*****************************
uci concrete output definitions
******************************/
class UciMuteOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm) const;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, const PVbound bound = normal, const int depth = -1, const int count = -1) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth() const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move m, const Move& ponder) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};


class UciStandardOutput: public UciOutput
{
public:
	static bool reduceVerbosity;
	void printPVs(std::vector<rootMove>& rm) const;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, const PVbound bound = normal, const int depth = -1, const int count = -1) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth() const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move m, const Move& ponder) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};


//---------------------------------------------
//	function implementation
//---------------------------------------------

char getPieceName( const bitboardIndex idx )
{
	assert( isValidPiece( idx ) || idx == empty);
	return PIECE_NAMES_FEN[ idx ];
}

std::string getProgramNameAndVersion()
{
	std::string s = programName;
	s += " ";
	s += version;
	s += preRelease;
	return s;
}

/*	\brief print the start message
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printStartMessage(void)
{
	sync_cout << "id name " << getProgramNameAndVersion() << sync_endl;
}

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printUciInfo(void)
{
	sync_cout << "id name " << getProgramNameAndVersion() << sync_endl;
	sync_cout << "id author Marco Belli" << sync_endl;
	sync_cout << "option name Hash type spin default 1 min 1 max 65535" << sync_endl;
	sync_cout << "option name Threads type spin default 1 min 1 max 128" << sync_endl;
	sync_cout << "option name MultiPV type spin default 1 min 1 max 500" << sync_endl;
	sync_cout << "option name Ponder type check default true" << sync_endl;
	sync_cout << "option name OwnBook type check default true" <<sync_endl;
	sync_cout << "option name BestMoveBook type check default false"<<sync_endl;
	sync_cout << "option name UCI_EngineAbout type string default " << getProgramNameAndVersion() << " by Marco Belli (build date: " <<__DATE__ <<" "<< __TIME__<<")"<<sync_endl;
	sync_cout << "option name UCI_ShowCurrLine type check default false" << sync_endl;
	sync_cout << "option name SyzygyPath type string default <empty>" << sync_endl;
	sync_cout << "option name SyzygyProbeDepth type spin default 1 min 1 max 100" << sync_endl;
	sync_cout << "option name Syzygy50MoveRule type check default true" << sync_endl;
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
	MovePicker mp(pos);
	while( ( m = mp.getNextMove() ) )
	{
		if(str == displayUci(m))
		{
			return m;
		}
	}
	// move not found
	return Move::NOMOVE;
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
	while( is >> token && (m = moveFromUci(pos, token) ) )
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

	SearchTimer st;

	unsigned long long res = pos.perft(n);

	long long int totalTime = std::max( st.getElapsedTime(), 1ll) ;

	sync_cout << "Perft " << n << " leaf nodes: " << res << sync_endl;
	sync_cout << totalTime << "ms " << ((double)res) / (double)totalTime << " kN/s" << sync_endl;
}


void static go(std::istringstream& is, Position & pos, my_thread & thr)
{
	SearchLimits limits;
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
			if( m )
			{
				limits.searchMoves.push_back( m );
			}
        }
    }
    thr.startThinking( pos, limits );
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
			UciStandardOutput::reduceVerbosity = true;
		}
		else
		{
			UciStandardOutput::reduceVerbosity = false;
		}
	}
	else if(name == "Threads")
	{
		try
		{
			int i = std::stoi(value);
			uciParameters::threads = (i<=128)?(i>0?i:1):128;
			sync_cout<<"info string Threads number set to "<<uciParameters::threads<<sync_endl;
		}
		catch(...){}
	}
	else if(name == "MultiPV")
	{
		try
		{
			int i = std::stoi(value);
			uciParameters::multiPVLines = i<500 ? (i>0 ? i : 1) : 500;
			sync_cout<<"info string MultiPv Lines set to "<<uciParameters::multiPVLines<<sync_endl;
		}
		catch(...){}	
	}
	else if(name == "OwnBook")
	{
		if(value=="true")
		{
			uciParameters::useOwnBook = true;
			sync_cout<<"info string OwnBook option set to true"<<sync_endl;
		}
		else{
			uciParameters::useOwnBook = false;
			sync_cout<<"info string OwnBook option set to false"<<sync_endl;
		}
	}
	else if(name == "BestMoveBook")
	{
		if(value == "true")
		{
			uciParameters::bestMoveBook = true;
			sync_cout<<"info string BestMoveBook option set to true"<<sync_endl;
		}
		else
		{
			uciParameters::bestMoveBook = false;
			sync_cout<<"info string BestMoveBook option set to false"<<sync_endl;
		}
	}
	else if(name == "UCI_ShowCurrLine")
	{
		if(value == "true")
		{
			uciParameters::showCurrentLine = true;
			sync_cout<<"info string UCI_ShowCurrLine option set to true"<<sync_endl;
		}
		else
		{
			uciParameters::showCurrentLine = false;
			sync_cout<<"info string UCI_ShowCurrLine option set to false"<<sync_endl;
		}
	}
	else if(name == "Ponder")
	{
		sync_cout<<"info string Ponder option set to "<<value<<sync_endl;
	}
	else if(name == "SyzygyPath")
	{
		uciParameters::SyzygyPath = value;
		tb_init(uciParameters::SyzygyPath.c_str());
		sync_cout<<"info string TB_LARGEST = "<<TB_LARGEST<<sync_endl;
	}
	else if(name == "SyzygyProbeDepth")
	{
		try
		{
			uciParameters::SyzygyProbeDepth = std::max(std::stoi(value),0) ;
		}
		catch(...)
		{
			uciParameters::SyzygyProbeDepth = 1;
		}
		sync_cout<<"info string SyzygyProbeDepth option set to "<<uciParameters::SyzygyProbeDepth<<sync_endl;
	}
	else if(name == "Syzygy50MoveRule")
	{
		if(value == "true")
		{
			uciParameters::Syzygy50MoveRule = true;
			sync_cout<<"info string Syzygy50MoveRule option set to true"<<sync_endl;
		}
		else
		{
			uciParameters::Syzygy50MoveRule = false;
			sync_cout<<"info string Syzygy50MoveRule option set to false"<<sync_endl;
		}
	}
	else if(name == "UCI_EngineAbout")
	{
		sync_cout<< programName << " " << version << preRelease << " by Marco Belli (build date: " <<__DATE__ <<" "<< __TIME__<<")"<<sync_endl;
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
	printStartMessage();
	my_thread &thr = my_thread::getInstance();
	Position pos;
	pos.setupFromFen(StartFEN);
	std::string token, cmd;

	do
	{
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
			thr.stopThinking();
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
			sync_cout << "gamePhase:"  << pos.getGamePhase( pos.getActualState() )/65536.0*100 << "%" << sync_endl;

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
			thr.ponderHit();
		}
		else
		{
			sync_cout << "unknown command" << sync_endl;
		}
	}while(token!="quit");
}

char printFileOf( const tSquare& sq )
{
	return char( 'a' + getFileOf( sq ) );
}

char printRankOf( const tSquare& sq )
{
	return char( '1' + getRankOf( sq ) );
}

/*! \brief return the uci string for a given move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
std::string displayUci(const Move & m){


	std::string s;

	if( !m )
	{
		s = "0000";
		return s;
	}

	//from
	s += printFileOf( m.getFrom() );
	s += printRankOf( m.getFrom() );


	//to
	s += printFileOf( m.getTo() );
	s += printRankOf( m.getTo() );
	//promotion
	if( m.isPromotionMove() )
	{
		s += tolower( getPieceName( m.getPromotedPiece() ) );
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
	bitboardIndex piece = pos.getPieceAt( m.getFrom() );
	bool pawnMove = isPawn(piece);
	bool isPromotion = m.isPromotionMove();
	bool isEnPassant = m.isEnPassantMove();
	bool isCastle = m.isCastleMove();

	bool disambigusFlag = false;
	bool fileFlag = false;
	bool rankFlag = false;


	// calc legalmoves
	{
		Position p(pos);
		p.doMove(m);
		legalMoves = p.getNumberOfLegalMoves();
		p.undoMove();
	}


	{
		// calc disambigus data
		Move mm;
		MovePicker mp( pos );
		while ( ( mm = mp.getNextMove() ) )
		{
			if( pos.getPieceAt( mm.getFrom() ) == piece 
				&& ( mm.getTo() == m.getTo() ) 
				&& ( mm.getFrom() != m.getFrom() )
			)
			{
				disambigusFlag = true;
				if( getFileOf( mm.getFrom() ) == getFileOf( m.getFrom() ) )
				{
					rankFlag = true;
				}
				if( getRankOf( mm.getFrom() ) == getRankOf( m.getFrom() ) )
				{
					fileFlag = true;
				}
			}
		}
		if( disambigusFlag && !rankFlag && !fileFlag )
		{
			fileFlag = true;
		}
	}


	// castle move
	if (isCastle)
	{
		if( m.isKingSideCastle() )
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
		s+= getPieceName( getPieceType( piece ) );
	}
	if( fileFlag )
	{
		s += printFileOf( m.getFrom() );
	}
	if( rankFlag )
	{
		s += printRankOf( m.getFrom() );
	}


	//capture motation
	if (capture)
	{
		if(pawnMove)
		{
			s += printFileOf( m.getFrom() );
		}
		// capture add x before to square
		s+="x";
	}
	// to square
	s += printFileOf( m.getTo() );
	s += printRankOf( m.getTo() );
	// add en passant info
	if ( isEnPassant )
	{
		s+="e.p.";
	}
	//promotion add promotion to
	if(isPromotion)
	{
		s += "=";
		s +=  getPieceName( m.getPromotedPiece() );
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
uci standard output implementation
******************************/

bool UciStandardOutput::reduceVerbosity = false;

void UciStandardOutput::printPVs(std::vector<rootMove>& rmList) const
{

	int i= 0;
	for ( auto & rm : rmList )
	{
		printPV(rm.score, rm.maxPlyReached, rm.time, rm.PV, rm.nodes, normal, rm.depth, i++ );
	}
}

void UciStandardOutput::printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, const PVbound bound, const int depth, const int count) const
{

	int localDepth = depth == -1? _depth : depth;
	int PVlineIndex = (count == -1 ? _PVlineIndex : count ) + 1 ;
	sync_cout<<"info multipv "<< PVlineIndex << " depth "<< localDepth <<" seldepth "<< seldepth <<" score ";

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

	std::cout << (bound == lowerbound ? " lowerbound" : bound == upperbound ? " upperbound" : "");

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
		std::cout << " " << displayUci(pos.getState(i).getCurrentMove());
	}
	std::cout << sync_endl;

}
void UciStandardOutput::printDepth() const
{
	sync_cout<<"info depth "<<_depth<<sync_endl;
}

void UciStandardOutput::printScore(const signed int cp) const
{
	sync_cout<<"info score cp "<<cp<<sync_endl;
}

void UciStandardOutput::printBestMove( const Move bm, const Move& ponder ) const
{
	sync_cout<<"bestmove "<< displayUci(bm);
	if( ponder )
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
void UciMuteOutput::printPV(const Score, const unsigned int, const long long, PVline&, const unsigned long long, const PVbound, const int, const int) const{}
void UciMuteOutput::printCurrMoveNumber(const unsigned int, const Move& , const unsigned long long , const long long int ) const {}
void UciMuteOutput::showCurrLine(const Position & , const unsigned int ) const{}
void UciMuteOutput::printDepth() const
{
	sync_cout<<"info depth "<<_depth<<sync_endl;
}
void UciMuteOutput::printScore(const signed int ) const{}
void UciMuteOutput::printBestMove( const Move, const Move& ) const{}
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
	else// if(t == mute)
	{
		return std::make_unique<UciMuteOutput>();
	}
}

void UciOutput::setDepth( const unsigned int depth )
{
	_depth = depth;
}

void UciOutput::setPVlineIndex( const unsigned int PVlineIndex )
{
	_PVlineIndex = PVlineIndex;
}

void UciOutput::printPV( const Move& m )
{
	PVline PV( 1, m );
	printPV(0, 0, 0, PV, 0, normal, 0, 0);
}
