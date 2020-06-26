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

#include "command.h"
#include "movepicker.h"
#include "position.h"
#include "pvLine.h"
#include "rootMove.h"
#include "uciOutput.h"
#include "vajo_io.h"

/*****************************
uci concrete output definitions
******************************/
class UciMuteOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm, bool ischess960, int maxLinePrint = -1) const override;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound = PVbound::normal, const int depth = -1, const int count = -1) const override;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const override;
	void showCurrLine(const Position & pos, const unsigned int ply) const override;
	void printDepth() const override;
	void printScore(const signed int cp) const override;
	void printBestMove( const Move& m, const Move& ponder, bool isChess960) const override;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const override;
	UciMuteOutput() {
		_type = type::mute;
	}
};


class UciStandardOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm, bool ischess960, int maxLinePrint = -1) const override;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound = PVbound::normal, const int depth = -1, const int count = -1) const override;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const override;
	void showCurrLine(const Position & pos, const unsigned int ply) const override;
	void printDepth() const override;
	void printScore(const signed int cp) const override;
	void printBestMove( const Move& m, const Move& ponder, bool isChess960) const override;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const override;
	UciStandardOutput() {
		_type = type::standard;
	}
};


/*****************************
uci standard output implementation
******************************/

void UciStandardOutput::printPVs(std::vector<rootMove>& rmList, bool ischess960, int maxLinePrint) const
{
	if(maxLinePrint == -1) {
		maxLinePrint = rmList.size();
	} else {
		maxLinePrint = std::min(rmList.size(), (size_t)maxLinePrint); 
	}
	for(int i = 0; i < maxLinePrint; ++i)
	{
		auto rm = rmList[i];
		printPV(rm.score, rm.maxPlyReached, rm.time, rm.PV, rm.nodes, ischess960, PVbound::normal, rm.depth, i);
	}
}

void UciStandardOutput::printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound, const int depth, const int count) const
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

	std::cout << (bound == PVbound::lowerbound ? " lowerbound" : bound == PVbound::upperbound ? " upperbound" : "");

	std::cout << " nodes " << nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout << " nps " << (unsigned int)((double)nodes*1000/(double)time) << " time " << (long long int)(time);
#endif

	std::cout << " pv ";
	for_each( PV.begin(), PV.end(), [&](Move &m){std::cout<<displayUci(m, ischess960)<<" ";});
	std::cout<<sync_endl;
}

void UciStandardOutput::printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const
{
	sync_cout << "info currmovenumber " << moveNumber << " currmove " << displayUci(m, isChess960) << " nodes " << visitedNodes <<
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
		std::cout << " " << displayUci(pos.getState(i).getCurrentMove(), pos.isChess960());
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

void UciStandardOutput::printBestMove( const Move& bm, const Move& ponder, bool isChess960 ) const
{
	sync_cout<<"bestmove "<< displayUci(bm, isChess960);
	if( ponder )
	{
		std::cout<<" ponder " << displayUci(ponder, isChess960);
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
void UciMuteOutput::printPVs(std::vector<rootMove>&, bool , int ) const{}
void UciMuteOutput::printPV(const Score, const unsigned int, const long long, PVline&, const unsigned long long, bool, const PVbound, const int, const int) const{}
void UciMuteOutput::printCurrMoveNumber(const unsigned int, const Move& , const unsigned long long , const long long int, bool ) const {}
void UciMuteOutput::showCurrLine(const Position & , const unsigned int ) const{}
void UciMuteOutput::printDepth() const{}
void UciMuteOutput::printScore(const signed int ) const{}
void UciMuteOutput::printBestMove( const Move&, const Move&, bool ) const{}
void UciMuteOutput::printGeneralInfo( const unsigned int , const unsigned long long int , const unsigned long long int , const long long int ) const{}



/*****************************
uci output factory method implementation
******************************/
std::unique_ptr<UciOutput> UciOutput::create( const UciOutput::type t )
{
	if( t == type::standard)
	{
		return std::make_unique<UciStandardOutput>();
	}
	else// if(t == type::mute)
	{
		return std::make_unique<UciMuteOutput>();
	}
}

bool UciOutput::reduceVerbosity = false;

void UciOutput::setDepth( const unsigned int depth )
{
	_depth = depth;
}

void UciOutput::setPVlineIndex( const unsigned int PVlineIndex )
{
	_PVlineIndex = PVlineIndex;
}

void UciOutput::printPV( const Move& m, bool ischess960)
{
	PVline PV;
	PV.set(m);
	printPV(0, 0, 0, PV, 0,ischess960, PVbound::normal, 0, 0);
}

char UciOutput::getPieceName(const bitboardIndex idx){
	assert( isValidPiece( idx ) || idx == empty);
	return _PIECE_NAMES_FEN[ idx ];
}

std::string UciOutput::displayUci(const Move& m, const bool chess960)
{
	std::string s;

	if( !m )
	{
		s = "0000";
		return s;
	}
	
	auto from = m.getFrom();
	//from
	s += _printFileOf(from);
	s += _printRankOf(from);

	auto to = m.getTo();
	if (m.isCastleMove() && !chess960)
	{
		to = getSquare(m.isKingSideCastle() ? FILEG : FILEC, getRankOf(from));
	}

	//to
	s += _printFileOf(to);
	s += _printRankOf(to);
	//promotion
	if( m.isPromotionMove() )
	{
		s += tolower( getPieceName( m.getPromotedPiece() ) );
	}
	return s;
}

std::string UciOutput::displayMove(const Position& pos, const Move& m)
{
	std::string s;

	const bool capture = pos.isCaptureMove(m);
	const bool check = pos.moveGivesCheck(m);
	//const bool doubleCheck = pos.moveGivesDoubleCheck(m);
	unsigned int checkMate;
	const bitboardIndex piece = pos.getPieceAt( m.getFrom() );
	const bool pawnMove = isPawn(piece);
	const bool isPromotion = m.isPromotionMove();
	/*const bool isEnPassant = m.isEnPassantMove();*/
	const bool isCastle = m.isCastleMove();

	bool fileFlag = false;
	bool rankFlag = false;


	// calc checkmate
	{
		Position p(pos, Position::pawnHash::off);
		p.doMove(m);
		checkMate = p.isCheckMate();
	}


	{
		// calc disambigus data
		bool disambigusFlag = false;
		Move mm;
		MovePicker mp( pos );
		while ( ( mm = mp.getNextMove() ) )
		{
			if( pos.getPieceAt( mm.getFrom() ) == piece 
				&& !pawnMove
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
	} else {


		// 1 ) use the name of the piece if it's not a pawn move
		if( !pawnMove )
		{
			s+= getPieceName( getPieceType( piece ) );
		}
		if( fileFlag )
		{
			s += _printFileOf( m.getFrom() );
		}
		if( rankFlag )
		{
			s += _printRankOf( m.getFrom() );
		}


		//capture motation
		if (capture)
		{
			if(pawnMove)
			{
				s += _printFileOf( m.getFrom() );
			}
			// capture add x before to square
			s+="x";
		}
		// to square
		s += _printFileOf( m.getTo() );
		s += _printRankOf( m.getTo() );
		// add en passant info
		/*if ( isEnPassant )
		{
			s+="e.p.";
		}*/
		//promotion add promotion to
		if(isPromotion)
		{
			s += "=";
			s +=  getPieceName( m.getPromotedPiece() );
		}
	}
	// add check information
	if( check )
	{
		if( checkMate )
		{
			s+="#";
		}
		else 
		{
			/*if( doubleCheck )
			{
				s+="++";
			}
			else*/
			{
				s+="+";
			}
		}
	}
	return s;
}


const char UciOutput::_PIECE_NAMES_FEN[] = {' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};