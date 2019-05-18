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
#include <algorithm>
#include <iomanip>

#include "benchmark.h"
#include "command.h"
#include "vajo_io.h"
#include "movepicker.h"
#include "parameters.h"
#include "perft.h"
#include "position.h"
#include "pvLine.h"
#include "rootMove.h"
#include "searchTimer.h"
#include "searchLimits.h"
// todo remove
#include "syzygy/syzygy.h"
#include "syzygy2/tbprobe.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"
#include "vajolet.h"
#include "version.h"

/*****************************
uci concrete output definitions
******************************/
class UciMuteOutput: public UciOutput
{
public:
	void printPVs(std::vector<rootMove>& rm, bool ischess960, int maxLinePrint = -1) const;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound = normal, const int depth = -1, const int count = -1) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth() const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move& m, const Move& ponder, bool isChess960) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};


class UciStandardOutput: public UciOutput
{
public:
	static bool reduceVerbosity;
	void printPVs(std::vector<rootMove>& rm, bool ischess960, int maxLinePrint = -1) const;
	void printPV(const Score res, const unsigned int seldepth, const long long time, PVline& PV, const unsigned long long nodes, bool ischess960, const PVbound bound = normal, const int depth = -1, const int count = -1) const;
	void printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const;
	void showCurrLine(const Position & pos, const unsigned int ply) const;
	void printDepth() const;
	void printScore(const signed int cp) const;
	void printBestMove( const Move& m, const Move& ponder, bool isChess960) const;
	void printGeneralInfo( const unsigned int fullness, const unsigned long long int thbits, const unsigned long long int nodes, const long long int time) const;
};

class UciManager::impl
{
public:
	explicit impl();
	~impl();
	
	void uciLoop();
	static char getPieceName(const bitboardIndex idx);
	static std::string displayUci(const Move& m, const bool chess960);
	static std::string displayMove(const Position& pos, const Move& m);
	
private:
	/*! \brief array of char to create the fen string
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	
	static void setTTSize(unsigned int size)
	{
		unsigned long elements = transpositionTable::getInstance().setSize(size);
		sync_cout<<"info string hash table allocated, "<<elements<<" elements ("<<size<<"MB)"<<sync_endl;
	}
	static void clearHash() {transpositionTable::getInstance().clear();}
	static void setTTPath( std::string s ) {
		tb_init(s.c_str());
		auto&  szg = Syzygy::getInstance();
		szg.setPath(s);
		sync_cout<<"info string syzygy path set to "<<s<<sync_endl;
		sync_cout<<"info string "<<szg.getSize()<<" tables found"<<sync_endl;
	}
	std::string unusedVersion;
	unsigned int unusedSize;
	static const char _PIECE_NAMES_FEN[];
	static const std::string _StartFEN;
	
	static char _printFileOf( const tSquare& sq ) { return char( 'a' + getFileOf( sq ) ); }
	static char _printRankOf( const tSquare& sq ) { return char( '1' + getRankOf( sq ) ); }

	
	class UciOption
	{
	public:
		virtual bool setValue( std::string v, bool verbose = true) = 0;
		virtual std::string print() const =0;
		virtual ~UciOption(){}
		bool operator==(const std::string& rhs){ return _name == rhs; }
	protected:
		UciOption( std::string name):_name(name){}	
		const std::string _name;
	};

	class StringUciOption final: public UciOption
	{
	public:
		StringUciOption( std::string name, std::string& value, void (*callbackFunc)(std::string), const std::string defVal):UciOption(name),_defaultValue(defVal),_value(value), _callbackFunc(callbackFunc){ setValue(_defaultValue, false); }
		std::string print() const{
			std::string s = "option name ";
			s += _name;
			s += " type string default ";
			s += _defaultValue;
			return s;
		};	
		bool setValue( std::string s, bool verbose = true)
		{
			_value = s;
			if(_callbackFunc)
			{
				_callbackFunc(_value);
			}
			if(verbose)
			{
				sync_cout<<"info string "<<_name<<" set to "<<_value<<sync_endl;
			}
			return true;
		}
	private:
		const std::string _defaultValue;
		std::string& _value;
		void (*_callbackFunc)(std::string);
	};

	class SpinUciOption final: public UciOption
	{
	public:
		SpinUciOption( std::string name, unsigned int& value, void (*callbackFunc)(unsigned int), const unsigned int defVal, const unsigned int minVal, const int unsigned maxVal):
			UciOption(name),
			_defValue(defVal),
			_minValue(minVal),
			_maxValue(maxVal),
			_value(value),
			_callbackFunc(callbackFunc)
		{ 
			setValue( std::to_string(_defValue), false );
		}
		std::string print() const{
			std::string s = "option name ";
			s += _name;
			s += " type spin default ";
			s += std::to_string(_defValue);
			s += " min ";
			s += std::to_string(_minValue);
			s += " max ";
			s += std::to_string(_maxValue);
			return s;
		};

		bool setValue( std::string s, bool verbose = true)
		{
			int value;
			try
			{
				value = std::stoi(s);
			}
			catch(...)
			{
				return false;
			}
			if( value < (int)_minValue ) { value = _minValue; }
			if( value > (int)_maxValue ) { value = _maxValue; }
			
			_value = value;
			
			if( _callbackFunc )
			{
				_callbackFunc(_value);
			}
			if(verbose)
			{
				sync_cout<<"info string "<<_name<<" set to "<<_value<<sync_endl;
			}
			return true;
		}		
	private:
		const unsigned int _defValue;
		const unsigned int _minValue;
		const unsigned int _maxValue;
		unsigned int & _value;
		void (*_callbackFunc)(unsigned int);
		
	};

	class CheckUciOption final: public UciOption
	{
	public:
		CheckUciOption( std::string name, bool& value, const bool defVal):UciOption(name),_defaultValue(defVal), _value(value)
		{
			setValue( _defaultValue ? "true" : "false", false );
		}
		std::string print() const{
			std::string s = "option name ";
			s += _name;
			s += " type check default ";
			s += ( _defaultValue ? "true" : "false" );
			return s;
		};	
		bool setValue( std::string s, bool verbose = true)
		{
			if( s == "true" )
			{
				_value = true;
				if(verbose)
				{
					sync_cout<<"info string "<<_name<<" set to "<<s<<sync_endl;
				}
				
			}
			else if( s == "false" )
			{
				_value = false;
				if(verbose)
				{
					sync_cout<<"info string "<<_name<<" set to "<<s<<sync_endl;
				}
			}
			else
			{
				sync_cout<<"info string error setting "<<_name<<sync_endl;
				return false;
			}
			return true;
		}
	private:
		const bool _defaultValue;
		bool& _value;
	};

	class ButtonUciOption final: public UciOption
	{
	public:
		ButtonUciOption( std::string name, void (*callbackFunc)()):UciOption(name), _callbackFunc(callbackFunc){}
		std::string print() const{
			std::string s = "option name ";
			s += _name;
			s += " type button";
			return s;
		};
		bool setValue( std::string, bool verbose = true ){if(_callbackFunc){_callbackFunc();} (void)verbose; return true;}
	private:
		void (*_callbackFunc)();
	};
	
	std::list<std::unique_ptr<UciOption>> _optionList;
	
	std::string _getProgramNameAndVersion();
	void _printNameAndVersionMessage(void);
	void _printUciInfo(void);
	Move _moveFromUci(const Position& pos,const  std::string& str);
	void _position(std::istringstream& is);
	void _doPerft(const unsigned int n);
	void _go(std::istringstream& is);
	void _setoption(std::istringstream& is);
	
	Position _pos;
};

const char UciManager::impl::_PIECE_NAMES_FEN[] = {' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};
const std::string UciManager::impl::_StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

UciManager::impl::impl()
{
	std::cout.rdbuf()->pubsetbuf( nullptr, 0 );
	std::cin.rdbuf()->pubsetbuf( nullptr, 0 );
	
	_optionList.emplace_back( new SpinUciOption("Hash",unusedSize, setTTSize, 1, 1, 65535));
	_optionList.emplace_back( new SpinUciOption("Threads", uciParameters::threads, nullptr, 1, 1, 128));
	_optionList.emplace_back( new SpinUciOption("MultiPV", uciParameters::multiPVLines, nullptr, 1, 1, 500));
	_optionList.emplace_back( new CheckUciOption("Ponder", uciParameters::Ponder, true));
	_optionList.emplace_back( new CheckUciOption("OwnBook", uciParameters::useOwnBook, true));
	_optionList.emplace_back( new CheckUciOption("BestMoveBook", uciParameters::bestMoveBook, false));
	_optionList.emplace_back( new StringUciOption("UCI_EngineAbout", unusedVersion, nullptr, _getProgramNameAndVersion() + " by Marco Belli (build date: " + __DATE__ + " " + __TIME__ + ")"));
	_optionList.emplace_back( new CheckUciOption("UCI_ShowCurrLine", uciParameters::showCurrentLine, false));
	_optionList.emplace_back( new StringUciOption("SyzygyPath", uciParameters::SyzygyPath, setTTPath, "<empty>"));
	_optionList.emplace_back( new SpinUciOption("SyzygyProbeDepth", uciParameters::SyzygyProbeDepth, nullptr, 1, 1, 100));
	_optionList.emplace_back( new CheckUciOption("Syzygy50MoveRule", uciParameters::Syzygy50MoveRule, true));
	_optionList.emplace_back( new ButtonUciOption("ClearHash", clearHash));
	_optionList.emplace_back( new CheckUciOption("PerftUseHash", Perft::perftUseHash, false));
	_optionList.emplace_back( new CheckUciOption("reduceVerbosity", UciStandardOutput::reduceVerbosity, false));
	_optionList.emplace_back( new CheckUciOption("UCI_Chess960", uciParameters::Chess960, false));
	
	_pos.setupFromFen(_StartFEN);
}

UciManager::impl::~impl()
{}

char UciManager::impl::getPieceName(const bitboardIndex idx){
	assert( isValidPiece( idx ) || idx == empty);
	return _PIECE_NAMES_FEN[ idx ];
}

std::string UciManager::impl::displayUci(const Move& m, const bool chess960)
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

std::string UciManager::impl::displayMove(const Position& pos, const Move& m)
{
	std::string s;

	const bool capture = pos.isCaptureMove(m);
	const bool check = pos.moveGivesCheck(m);
	const bool doubleCheck = pos.moveGivesDoubleCheck(m);
	unsigned int legalReplies;
	const bitboardIndex piece = pos.getPieceAt( m.getFrom() );
	const bool pawnMove = isPawn(piece);
	const bool isPromotion = m.isPromotionMove();
	const bool isEnPassant = m.isEnPassantMove();
	const bool isCastle = m.isCastleMove();

	bool disambigusFlag = false;
	bool fileFlag = false;
	bool rankFlag = false;


	// calc legal reply to this move
	{
		Position p(pos);
		p.doMove(m);
		legalReplies = p.getNumberOfLegalMoves();
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
	}
	// add check information
	if( check )
	{
		if( legalReplies > 0 )
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




//---------------------------------------------
//	function implementation
//---------------------------------------------


std::string UciManager::impl::_getProgramNameAndVersion()
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
void UciManager::impl::_printNameAndVersionMessage(void)
{
	sync_cout << "id name " << _getProgramNameAndVersion() << sync_endl;
}

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void UciManager::impl::_printUciInfo(void)
{
	_printNameAndVersionMessage();
	sync_cout << "id author Marco Belli" << sync_endl;
	for( const auto& opt: _optionList ) sync_cout << opt->print()<<sync_endl;
	sync_cout << "uciok" << sync_endl;
}


/*	\brief get an input string and convert it to a valid move;
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
Move UciManager::impl::_moveFromUci(const Position& pos,const  std::string& str)
{

	// idea from stockfish, we generate all the legal moves and return the legal moves with the same UCI string
	Move m;
	MovePicker mp(pos);
	while( ( m = mp.getNextMove() ) )
	{
		if(str == displayUci(m, pos.isChess960()))
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
void UciManager::impl::_position(std::istringstream& is)
{
	std::string token, fen;
	is >> token;
	if (token == "startpos")
	{
		fen = _StartFEN;
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
	// todo parse fen!! invalid fen shall be rejected and Vajolet shall not crash
	_pos.setupFromFen(fen);

	Move m;
	// Parse move list (if any)
	while( is >> token && (m = _moveFromUci(_pos, token) ) )
	{
		_pos.doMove(m);
	}
}

/*	\brief handle perft command
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void UciManager::impl::_doPerft(const unsigned int n)
{
	SearchTimer st;
	
	unsigned long long res = Perft(_pos).perft(n);

	long long int totalTime = std::max( st.getElapsedTime(), 1ll) ;

	sync_cout << "Perft " << n << " leaf nodes: " << res << sync_endl;
	sync_cout << totalTime << "ms " << ((double)res) / (double)totalTime << " kN/s" << sync_endl;
}

void UciManager::impl::_go(std::istringstream& is)
{
	SearchLimits limits;
	std::string token;
	bool searchMovesCommand = false;


    while (is >> token)
    {
        if (token == "searchmoves")		{searchMovesCommand = true;}
        else if (token == "wtime")		{ long long int x; is >> x; limits.setWTime(x);     searchMovesCommand = false;}
        else if (token == "btime")		{ long long int x; is >> x; limits.setBTime(x);     searchMovesCommand = false;}
        else if (token == "winc")		{ long long int x; is >> x; limits.setWInc(x);      searchMovesCommand = false;}
        else if (token == "binc")		{ long long int x; is >> x; limits.setBInc(x);      searchMovesCommand = false;}
        else if (token == "movestogo")	{ long long int x; is >> x; limits.setMovesToGo(x); searchMovesCommand = false;}
        else if (token == "depth")		{ int x;           is >> x; limits.setDepth(x);     searchMovesCommand = false;}
        else if (token == "nodes")		{ unsigned int x;  is >> x; limits.setNodeLimit(x); searchMovesCommand = false;}
        else if (token == "movetime")	{ long long int x; is >> x; limits.setMoveTime(x);  searchMovesCommand = false;}
        else if (token == "mate")		{ long long int x; is >> x; limits.setMate(x);      searchMovesCommand = false;}
        else if (token == "infinite")	{ limits.setInfiniteSearch();                       searchMovesCommand = false;}
        else if (token == "ponder")		{ limits.setPonder(true);                           searchMovesCommand = false;}
        else if (searchMovesCommand == true)
        {
			if( Move m = _moveFromUci(_pos, token); m != Move::NOMOVE )
			{
				limits.moveListInsert(m);
			}
        }
    }
    my_thread::getInstance().startThinking( _pos, limits );
}

void UciManager::impl::_setoption(std::istringstream& is)
{
	std::string token, name, value;

	///////////////////////////////////////////////
	// PARSE INPUT
	///////////////////////////////////////////////
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
	
	///////////////////////////////////////////////
	// FIND OPTION
	///////////////////////////////////////////////
	
	// find the  in the list
	auto it = std::find_if(_optionList.begin(), _optionList.end(), [&](std::unique_ptr<UciOption>& p) { return *p == name;});
	if( it == _optionList.end() )
	{
		sync_cout << "info string No such option: " << name << sync_endl;
		return;
	}
	
	///////////////////////////////////////////////
	// EXECUTE ACTION
	///////////////////////////////////////////////
	
	auto &op = *it;
	if( !op->setValue(value) )
	{
		sync_cout << "info string malformed command"<< sync_endl;
		return;
	}
}


/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void UciManager::impl::uciLoop()
{
	// todo manage command parsin with a list?
	_printNameAndVersionMessage();
	my_thread &thr = my_thread::getInstance();
	
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
			_printUciInfo();
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
			_pos.display();
		}
		else if (token == "position")
		{
			_position(is);
		}
		else if(token == "setoption")
		{
			_setoption(is);
		}
		else if (token == "eval")
		{
			Score s = _pos.eval<true>();
			sync_cout << "Eval:" <<  s / 10000.0 << sync_endl;
			sync_cout << "gamePhase:"  << _pos.getGamePhase( _pos.getActualState() )/65536.0*100 << "%" << sync_endl;

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
			_doPerft(n);
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
			
			unsigned long long res = Perft(_pos).divide(n);
			sync_cout << "divide Res= " << res << sync_endl;
		}
		else if (token == "go")
		{
			_go(is);
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



UciManager::UciManager(): pimpl{std::make_unique<impl>()}{}

UciManager::~UciManager() = default;

void UciManager::uciLoop() { pimpl->uciLoop();}
char UciManager::getPieceName( const bitboardIndex idx ) { return impl::getPieceName(idx);}
std::string UciManager::displayUci(const Move& m, const bool chess960) { return  impl::displayUci(m, chess960);}
std::string UciManager::displayMove( const Position& pos, const Move& m ) {  return impl::displayMove(pos, m);}

/*****************************
uci standard output implementation
******************************/

bool UciStandardOutput::reduceVerbosity = false;

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
		printPV(rm.score, rm.maxPlyReached, rm.time, rm.PV, rm.nodes, ischess960,  normal, rm.depth, i);
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

	std::cout << (bound == lowerbound ? " lowerbound" : bound == upperbound ? " upperbound" : "");

	std::cout << " nodes " << nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout << " nps " << (unsigned int)((double)nodes*1000/(double)time) << " time " << (long long int)(time);
#endif

	std::cout << " pv ";
	for_each( PV.begin(), PV.end(), [&](Move &m){std::cout<<UciManager::displayUci(m, ischess960)<<" ";});
	std::cout<<sync_endl;
}

void UciStandardOutput::printCurrMoveNumber(const unsigned int moveNumber, const Move &m, const unsigned long long visitedNodes, const long long int time, bool isChess960) const
{
	sync_cout << "info currmovenumber " << moveNumber << " currmove " << UciManager::displayUci(m, isChess960) << " nodes " << visitedNodes <<
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
		std::cout << " " << UciManager::displayUci(pos.getState(i).getCurrentMove(), pos.isChess960());
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
	sync_cout<<"bestmove "<< UciManager::displayUci(bm, isChess960);
	if( ponder )
	{
		std::cout<<" ponder " << UciManager::displayUci(ponder, isChess960);
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

void UciOutput::printPV( const Move& m, bool ischess960)
{
	PVline PV;
	PV.set(m);
	printPV(0, 0, 0, PV, 0,ischess960, normal, 0, 0);
}
