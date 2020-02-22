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
#include <list>

#include "benchmark.h"
#include "command.h"
#include "movepicker.h"
#include "perft.h"
#include "position.h"
#include "searchTimer.h"
#include "searchLimits.h"
#include "syzygy/syzygy.h"
#include "tSquare.h"
#include "thread.h"
#include "transposition.h"
#include "uciOutput.h"
#include "uciParameters.h"
#include "vajolet.h"
#include "vajo_io.h"
#include "version.h"

class UciManager::impl
{
public:
	explicit impl();
	~impl();
	
	void uciLoop(std::istream& is);

	
private:
	/*! \brief array of char to create the fen string
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	
	static void setTTSize(unsigned int size)
	{
		uint64_t elements = transpositionTable::getInstance().setSize(size);
		sync_cout<<"info string hash table allocated, "<<elements<<" elements ("<<size<<"MB)"<<sync_endl;
	}
	static void clearHash() {transpositionTable::getInstance().clear();}
	static void setTTPath( std::string s ) {
		auto&  szg = Syzygy::getInstance();
		szg.setPath(s);
		sync_cout<<"info string "<<szg.getSize()<<" tables found"<<sync_endl;
	}
	std::string unusedVersion;
	unsigned int unusedSize;
	
	static const std::string _StartFEN;

	
	class UciOption
	{
	public:
		virtual bool setValue( const std::string& v, bool verbose = true) = 0;
		virtual std::string print() const =0;
		virtual ~UciOption(){}
		bool operator==(const std::string& rhs){ return _name == rhs; }
	protected:
		UciOption(const std::string& name):_name(name){}	
		const std::string _name;
	};

	class StringUciOption final: public UciOption
	{
	public:
		StringUciOption( const std::string& name, std::string& value, void (*callbackFunc)(std::string), const std::string& defVal):UciOption(name),_defaultValue(defVal),_value(value), _callbackFunc(callbackFunc){ setValue(_defaultValue, false); }
		std::string print() const override{
			std::string s = "option name ";
			s += _name;
			s += " type string default ";
			s += _defaultValue;
			return s;
		};	
		bool setValue( const std::string& s, bool verbose = true) override
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
		SpinUciOption( const std::string& name, unsigned int& value, void (*callbackFunc)(unsigned int), const unsigned int defVal, const unsigned int minVal, const int unsigned maxVal):
			UciOption(name),
			_defValue(defVal),
			_minValue(minVal),
			_maxValue(maxVal),
			_value(value),
			_callbackFunc(callbackFunc)
		{ 
			setValue( std::to_string(_defValue), false );
		}
		std::string print() const override{
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

		bool setValue( const std::string& s, bool verbose = true) override
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
		CheckUciOption( const std::string& name, bool& value, const bool defVal):UciOption(name),_defaultValue(defVal), _value(value)
		{
			setValue( _defaultValue ? "true" : "false", false );
		}
		std::string print() const override{
			std::string s = "option name ";
			s += _name;
			s += " type check default ";
			s += ( _defaultValue ? "true" : "false" );
			return s;
		};	
		bool setValue( const std::string& s, bool verbose = true) override
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
		ButtonUciOption( const std::string& name, void (*callbackFunc)()):UciOption(name), _callbackFunc(callbackFunc){}
		std::string print() const  override{
			std::string s = "option name ";
			s += _name;
			s += " type button";
			return s;
		};
		bool setValue( const std::string&, bool verbose = true ) override {if(_callbackFunc){_callbackFunc();}(void)verbose; return true;}
	private:
		void (*_callbackFunc)();
	};
	
	std::list<std::unique_ptr<UciOption>> _optionList;
	
	std::string _getProgramNameAndVersion();
	void _printNameAndVersionMessage();
	
	Move _moveFromUci(const Position& pos,const  std::string& str);
	void _doPerft(const unsigned int n);
	
	
	bool _printUciInfo(std::istringstream& is, my_thread &thr);
	bool _stop(std::istringstream& is, my_thread &thr);
	bool _uciNewGame(std::istringstream& is, my_thread &thr);
	bool _eval(std::istringstream& is, my_thread &thr);
	bool _display(std::istringstream& is, my_thread &thr);
	bool _position(std::istringstream& is, my_thread &thr);
	
	bool _go(std::istringstream& is, my_thread &thr);
	bool _setoption(std::istringstream& is, my_thread &thr);
	bool _readyOk(std::istringstream& is, my_thread &thr);
	bool _benchmark(std::istringstream& is, my_thread &thr);
	bool _ponderHit(std::istringstream& is, my_thread &thr);
	bool _perft(std::istringstream& is, my_thread &thr);
	bool _divide(std::istringstream& is, my_thread &thr);
	bool _quit(std::istringstream& is, my_thread &thr);
	Position _pos;
};

const std::string UciManager::impl::_StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

UciManager::impl::impl(): _pos(Position::pawnHash::off)
{
	std::cout.rdbuf()->pubsetbuf( nullptr, 0 );
	std::cin.rdbuf()->pubsetbuf( nullptr, 0 );
	
	_optionList.emplace_back( new SpinUciOption("Hash",unusedSize, setTTSize, 1, 1, 262144));
	_optionList.emplace_back( new SpinUciOption("Threads", uciParameters::threads, nullptr, 1, 1, 256));
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
	_optionList.emplace_back( new CheckUciOption("reduceVerbosity", UciOutput::reduceVerbosity, false));
	_optionList.emplace_back( new CheckUciOption("UCI_Chess960", uciParameters::Chess960, false));
	
	_pos.setupFromFen(_StartFEN);
}

UciManager::impl::~impl()
{}

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
void UciManager::impl::_printNameAndVersionMessage()
{
	sync_cout << "id name " << _getProgramNameAndVersion() << sync_endl;
}

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
bool UciManager::impl::_printUciInfo(std::istringstream& , my_thread &)
{
	_printNameAndVersionMessage();
	sync_cout << "id author Marco Belli" << sync_endl;
	for( const auto& opt: _optionList ) sync_cout << opt->print()<<sync_endl;
	sync_cout << "uciok" << sync_endl;
	return false;
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
		if(str == UciOutput::displayUci(m, pos.isChess960()))
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
bool UciManager::impl::_position(std::istringstream& is, my_thread &)
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
		return false;
	}
	// todo parse fen!! invalid fen shall be rejected and Vajolet shall not crash
	_pos.setupFromFen(fen);

	Move m;
	// Parse move list (if any)
	while( is >> token && (m = _moveFromUci(_pos, token) ) )
	{
		_pos.doMove(m);
	}
	return false;
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

	long long int totalTime = std::max( st.getElapsedTime(), static_cast<int64_t>(1)) ;

	sync_cout << "Perft " << n << " leaf nodes: " << res << sync_endl;
	sync_cout << totalTime << "ms " << ((double)res) / (double)totalTime << " kN/s" << sync_endl;
}

bool UciManager::impl::_go(std::istringstream& is, my_thread &)
{
	SearchLimits limits;
	std::string token;
	bool searchMovesCommand = false;


	while (is >> token)
	{
		if (token == "searchmoves")			{searchMovesCommand = true;}
		else if (token == "wtime")			{ long long int x; is >> x; limits.setWTime(x);     searchMovesCommand = false;}
		else if (token == "btime")			{ long long int x; is >> x; limits.setBTime(x);     searchMovesCommand = false;}
		else if (token == "winc")				{ long long int x; is >> x; limits.setWInc(x);      searchMovesCommand = false;}
		else if (token == "binc")				{ long long int x; is >> x; limits.setBInc(x);      searchMovesCommand = false;}
		else if (token == "movestogo")	{ long long int x; is >> x; limits.setMovesToGo(x); searchMovesCommand = false;}
		else if (token == "depth")			{ int x;           is >> x; limits.setDepth(x);     searchMovesCommand = false;}
		else if (token == "nodes")			{ unsigned int x;  is >> x; limits.setNodeLimit(x); searchMovesCommand = false;}
		else if (token == "movetime")		{ long long int x; is >> x; limits.setMoveTime(x);  searchMovesCommand = false;}
		else if (token == "mate")				{ long long int x; is >> x; limits.setMate(x);      searchMovesCommand = false;}
		else if (token == "infinite")		{ limits.setInfiniteSearch();                       searchMovesCommand = false;}
		else if (token == "ponder")			{ limits.setPonder(true);                           searchMovesCommand = false;}
		else if (searchMovesCommand == true)
		{
			if( Move m = _moveFromUci(_pos, token); m != Move::NOMOVE )
			{
				limits.moveListInsert(m);
			}
		}
	}
	my_thread::getInstance().startThinking( _pos, limits );
	return false;
}

bool UciManager::impl::_setoption(std::istringstream& is, my_thread &th)
{
	if(th.isSearchRunning()) {
		sync_cout << "info string warning option not applied bacause search is in progress"<< sync_endl;
		return false;
	}
	std::string token, name, value;

	///////////////////////////////////////////////
	// PARSE INPUT
	///////////////////////////////////////////////
	is >> token; // Consume "name" token
	
	if( token != "name" )
	{
		sync_cout << "info string malformed command"<< sync_endl;
		return false;
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
		return false;
	}
	
	///////////////////////////////////////////////
	// EXECUTE ACTION
	///////////////////////////////////////////////
	
	auto &op = *it;
	if( !op->setValue(value) )
	{
		sync_cout << "info string malformed command"<< sync_endl;
		return false;
	}
	return false;
}

bool UciManager::impl::_eval(std::istringstream&, my_thread &) {
	Score s = _pos.eval<true>();
	sync_cout << "Eval:" <<  s / 10000.0 << sync_endl;
	sync_cout << "gamePhase:"  << _pos.getGamePhase( _pos.getActualState() )/65536.0 * 100 << "%" << sync_endl;
	return false;
}

bool UciManager::impl::_readyOk(std::istringstream& , my_thread &) {
	sync_cout << "readyok" << sync_endl;
	return false;
}

bool UciManager::impl::_divide(std::istringstream& is, my_thread &) {
	std::string token;
	if (is>>token)
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
	else {
		sync_cout << "malformed perft command" << sync_endl;
	}
	return false;
}

bool UciManager::impl::_perft(std::istringstream& is, my_thread &) {
	std::string token;
	if (is>>token)
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
	else {
		sync_cout << "malformed perft command" << sync_endl;
	}
	return false;
}

bool UciManager::impl::_stop(std::istringstream&, my_thread &thr) {
	thr.stopThinking();
	return false;
}

bool UciManager::impl::_display(std::istringstream&, my_thread &) {
	_pos.display();
	return false;
}

bool UciManager::impl::_uciNewGame(std::istringstream&, my_thread &) {
	return false;
}

bool UciManager::impl::_benchmark(std::istringstream&, my_thread &) {
	benchmark();
	return false;
}

bool UciManager::impl::_ponderHit(std::istringstream&, my_thread &thr) {
	thr.ponderHit();
	return false;
}

bool UciManager::impl::_quit(std::istringstream&, my_thread &thr) {
	thr.stopThinking();
	return true;
}

/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void UciManager::impl::uciLoop(std::istream& is)
{
	_printNameAndVersionMessage();
	
	my_thread &thr = my_thread::getInstance();
	std::string cmd;
	
	bool quit = false;
	
	std::map<std::string, bool (UciManager::impl::*)(std::istringstream&, my_thread &)> commands;
	commands["uci"] = &UciManager::impl::_printUciInfo;
	commands["stop"] = &UciManager::impl::_stop;
	commands["ucinewgame"] = &UciManager::impl::_uciNewGame;
	commands["d"] = &UciManager::impl::_display;
	commands["position"] = &UciManager::impl::_position;
	commands["setoption"] = &UciManager::impl::_setoption;
	commands["eval"] = &UciManager::impl::_eval;
	commands["isready"] = &UciManager::impl::_readyOk;
	commands["perft"] = &UciManager::impl::_perft;
	commands["divide"] = &UciManager::impl::_divide;
	commands["go"] = &UciManager::impl::_go;
	commands["bench"] = &UciManager::impl::_benchmark;
	commands["ponderhit"] = &UciManager::impl::_ponderHit;
	commands["quit"] = &UciManager::impl::_quit;
	
	do
	{
		if (!std::getline(is, cmd)) {// Block here waiting for input
			cmd = "quit";
		}
		
		std::string token;
		std::istringstream is(cmd);
		token.clear();
		is >> std::skipws >> token;
		
		auto it = commands.find(token);
		if (it != commands.end()) {
			quit = (*this.*(it->second))(is, thr);
		}
		else {
			sync_cout << "unknown command" << sync_endl;
			quit = false;
		}
	}while(!quit);
}



UciManager::UciManager(): pimpl{std::make_unique<impl>()}{}

UciManager::~UciManager() = default;

void UciManager::uciLoop(std::istream& is) { pimpl->uciLoop(is);}

