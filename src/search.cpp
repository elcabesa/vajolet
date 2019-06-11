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


#include <cmath>
#include <mutex>
#include <thread>


#include "book.h"
#include "game.h"
#include "vajo_io.h"
#include "movepicker.h"
#include "multiPVmanager.h"
#include "position.h"
#include "pvLineFollower.h"
#include "rootMove.h"
#include "search.h"
#include "searchData.h"
#include "searchLogger.h"
#include "searchTimer.h"
#include "timeManagement.h"
#include "thread.h"
#include "uciParameters.h"
#include "searchResult.h"
#include "syzygy/syzygy.h"
#include "transposition.h"
#include "vajolet.h"

#ifdef DEBUG_EVAL_SIMMETRY
void testSimmetry(const Position& pos)
{
	static Position ppp;

	ppp.setupFromFen(pos.getSymmetricFen(), pos.isChess960());

	Score staticEval = pos.eval<false>();
	Score test = ppp.eval<false>();

	if(test != staticEval)
	{
		sync_cout << "eval symmetry problem " << test << ":" << staticEval << sync_endl;
		pos.display();
		ppp.display();
		while(1);
	}
}
#endif



class Search::impl
{
public:

	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------
	static void initSearchParameters(void);
	static std::vector<impl> helperSearch;

	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	impl( SearchTimer& st, SearchLimits& sl, std::unique_ptr<UciOutput> UOI = UciOutput::create( ) ):_UOI(std::move(UOI)), _sl(sl), _st(st){}

	impl( const impl& other ) :_UOI(UciOutput::create()), _sl(other._sl), _st(other._st), _rootMovesToBeSearched(other._rootMovesToBeSearched){}
	impl& operator=(const impl& other)
	{
		// todo fare una copia fatta bene
		_sl = other._sl;
		_st = other._st;
		_UOI = UciOutput::create();
		_rootMovesToBeSearched = other._rootMovesToBeSearched;
		return * this;
	}
	SearchResult go(int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, PVline pvToBeFollowed = PVline() );

	void stopSearch(){ _stop = true;}
	void resetStopCondition(){ _stop = false;}

	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;
	void showLine(){ _showLine= true;}
	SearchResult manageNewSearch();
	Position& getPosition();

private:
	std::unique_ptr<logWriter> _lw;
	//--------------------------------------------------------
	// private enum definition
	//--------------------------------------------------------
	enum class nodeType
	{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	};

	//--------------------------------------------------------
	// private static members
	//--------------------------------------------------------
	static const int ONE_PLY = 16;
	static const int ONE_PLY_SHIFT = 4;
	static const unsigned int LmrLimit = 32;
	static Score futilityMargin[7];
	static unsigned int FutilityMoveCounts[2][16];
	static Score PVreduction[2][LmrLimit*ONE_PLY][64];
	static Score nonPVreduction[2][LmrLimit*ONE_PLY][64];

	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------
	std::unique_ptr<UciOutput> _UOI;

	bool _validIteration = false;
	Score _expectedValue = 0;
	eNextMove _initialTurn;
	bool _showLine = false;
	
	PVlineFollower _pvLineFollower;


	SearchData _sd;
	unsigned long long _visitedNodes;
	unsigned long long _tbHits;
	unsigned int _maxPlyReached;

	MultiPVManager _multiPVmanager;
	Position _pos;

	SearchLimits& _sl; // todo limits belong to threads
	SearchTimer& _st;
	std::vector<Move> _rootMovesToBeSearched;
	std::vector<Move> _rootMovesAlreadySearched;


	volatile bool _stop = false;

	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------
	void cleanMemoryBeforeStartingNewSearch();
	void generateRootMovesList(std::vector<Move>& rm, const std::list<Move>& ml);
	void filterRootMovesByTablebase(std::vector<Move>& rm);
	SearchResult manageQsearch();



	signed int razorMargin(unsigned int depth,bool cut) const { return 20000+depth*(1248/ONE_PLY)+cut*20000; }

	template<nodeType type>Score qsearch(unsigned int ply,int depth,Score alpha,Score beta, PVline& pvLine);
	template<nodeType type>Score alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,PVline& pvLine);

	rootMove aspirationWindow(const int depth, Score alpha, Score beta, const bool masterThread);
	void excludeRootMoves( std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, bool masterThread);
	void idLoop(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE, bool masterThread = false );

	void setUOI( std::unique_ptr<UciOutput> UOI );
	static Score futility(int depth, bool improving );
	Score getDrawValue() const;

	void _updateCounterMove( const Move& m );
	void _updateNodeStatistics(const unsigned int ply);
	//void _printRootMoveList() const;

	bool _manageDraw(const bool PVnode, PVline& pvLine);
	void _showCurrenLine( const unsigned int ply, const int depth );
	bool _MateDistancePruning( const unsigned int ply, Score& alpha, Score& beta) const;
	void _appendTTmoveIfLegal(  const Move& ttm, PVline& pvLine ) const;
	bool _canUseTTeValue( const bool PVnode, const Score beta, const Score ttValue, const ttEntry * const tte, short int depth ) const;
	const HashKey _getSearchKey( const bool excludedMove = false ) const;

	using tableBaseRes = struct{ ttType TTtype; Score value;};
	tableBaseRes _checkTablebase( const unsigned int ply, const int depth );

	static std::mutex _mutex;

	Game _game;


	Move _getPonderMoveFromHash( const Move bestMove );
	Move _getPonderMoveFromBook( const Move bookMove );
	void _waitStopPondering() const;


};

std::vector<Search::impl> Search::impl::helperSearch;


const int Search::impl::ONE_PLY;
const int Search::impl::ONE_PLY_SHIFT;

std::mutex  Search::impl::_mutex;

class voteSystem
{
public:
	voteSystem( const std::vector<rootMove>& res):_results(res){}

	void print( const std::map<unsigned short, int>& votes, const Score minScore, const rootMove& bm ) const
	{
		std::cout<<"----------VOTE SYSTEM-------------"<<std::endl;
		std::cout<<"minScore: "<<minScore<<std::endl;
		std::cout<<"----------------------------------"<<std::endl;
		for (auto &res : votes)
		{
			Move m(res.first);
			// todo shall it know wheher the move is chess960?
			std::cout<<"Move: "<<UciManager::displayUci(m, false)<<" votes: "<<res.second;
			if( bm == m ) std::cout<<" *****";
			std::cout<<std::endl;
		}
		std::cout<<"----------------------------------"<<std::endl;

		for (auto &res : _results)
		{
			if( res == bm)
			{
				std::cout<<"bestMove: "<<UciManager::displayUci(res.firstMove, false)<<" *****"<<std::endl;
			}
			else
			{
				std::cout<<"bestMove: "<<UciManager::displayUci(res.firstMove, false)<<std::endl;
			}
			std::cout<<"score: "<<res.score<<std::endl;
			std::cout<<"depth: "<<res.depth<<std::endl;
			std::cout<<"votes: "<<votes.at( res.firstMove.getPacked() )<<std::endl;
		}
	}

	const rootMove& getBestMove( bool verbose = false) const
	{

		std::map<unsigned short, int> votes;

		//////////////////////////////////////////
		// calc the min value
		//////////////////////////////////////////
		Score minScore = _results[0].score;
		for (auto &res : _results)
		{
			minScore = std::min( minScore, res.score );
			votes[ res.firstMove.getPacked() ] = 0;
		}

		//////////////////////////////////////////
		// calc votes
		//////////////////////////////////////////
		for (auto &res : _results)
		{
			votes[ res.firstMove.getPacked() ] += (int)( res.score - minScore ) + 40 * res.depth;
		}

		//////////////////////////////////////////
		// find the maximum
		//////////////////////////////////////////
		const rootMove* bestMove = &_results[0];
		int bestResult = votes[ _results[0].firstMove.getPacked() ];

		for ( auto &res : _results )
		{
			if( votes[ res.firstMove.getPacked() ] > bestResult )
			{
				bestResult = votes[ res.firstMove.getPacked() ];
				bestMove = &res;
			}
		}

		if( verbose ) print( votes, minScore, *bestMove);

		return *bestMove;
	}

private:
	const std::vector<rootMove>& _results;
};

Score Search::impl::futility(int depth, bool improving )
{
	return (6000/ONE_PLY) * depth - 2000 * improving;
}
Score Search::impl::futilityMargin[7] = {0};
unsigned int Search::impl::FutilityMoveCounts[2][16]= {{0},{0}};
Score Search::impl::PVreduction[2][LmrLimit*ONE_PLY][64];
Score Search::impl::nonPVreduction[2][LmrLimit*ONE_PLY][64];


unsigned long long Search::impl::getVisitedNodes() const
{
	unsigned long long n = _visitedNodes;
	for (auto& hs : helperSearch)
		n += hs._visitedNodes;
	return n;
}

unsigned long long Search::impl::getTbHits() const
{
	unsigned long long n = _tbHits;
	for (auto& hs : helperSearch)
		n += hs._tbHits;
	return n;
}

void Search::impl::cleanMemoryBeforeStartingNewSearch(void)
{
	_sd.cleanData();
	_visitedNodes = 0;
	_tbHits = 0;
	_multiPVmanager.clean();
	_rootMovesAlreadySearched.clear();
}

void Search::impl::filterRootMovesByTablebase(std::vector<Move>& rm)
{
	if(rm.size() > 0) {
		bool found = false;
		
		std::vector<extMove> rm2;
		for (auto m: rm) {
			extMove em(m);
			rm2.push_back(em);
		}

		unsigned int piecesCnt = bitCnt (_pos.getBitmap(whitePieces) | _pos.getBitmap(blackPieces));
		Syzygy& szg = Syzygy::getInstance();
		if (piecesCnt <= szg.getMaxCardinality() && _pos.getCastleRights() == noCastle) {
			
			found = szg.rootProbe(_pos, rm2) || szg.rootProbeWdl(_pos, rm2);
			
			if (found) {
				std::sort(rm2.begin(), rm2.end());
				std::reverse(rm2.begin(), rm2.end());
				Score Max = rm2[0].getScore();
				
				for (auto m: rm2) {
					if (m.getScore() < Max) {
						auto it = std::find(rm.begin(), rm.end(), m);
						if (it != rm.end()) {
							rm.erase(it);
						}
					}
				}
			}	
		}
	}
}

void Search::impl::generateRootMovesList( std::vector<Move>& rm, const std::list<Move>& ml)
{
	rm.clear();
	
	if( ml.size() == 0 )	// all the legal moves
	{
		Move m;
		MovePicker mp( _pos );
		while( ( m = mp.getNextMove() ) )
		{
			rm.emplace_back( m );
		}
	}
	else
	{
		//only selected moves
		for_each( ml.begin(), ml.end(), [&]( const Move &m){rm.emplace_back(m);} );
	}
}

SearchResult Search::impl::manageQsearch(void)
{
	PVline pvLine;
	Score res =qsearch<Search::impl::nodeType::PV_NODE>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, pvLine);
	
	_UOI->printScore( res/100 );
	
	return SearchResult( -SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, res );
}
/*
void Search::impl::_printRootMoveList() const
{
	unsigned int i = 0;
	sync_cout;
	std::cout<<"move list"<<std::endl;
	for( auto m: _rootMovesToBeSearched)
	{
		++i;
		std::cout<<i<<": "<<UciManager::displayUci(m)<<std::endl;
		
	}
	std::cout<<sync_endl;
}*/


rootMove Search::impl::aspirationWindow( const int depth, Score alpha, Score beta, const bool masterThread)
{
	timeManagement &tm = my_thread::getInstance().getTimeMan();

	rootMove bestMove(Move::NOMOVE);
	Score delta = 800;
	//----------------------------------
	// prepare alpha & beta
	//----------------------------------
	if (depth >= 5)
	{
		delta = 800;
		alpha = (Score) std::max((signed long long int)_expectedValue - delta,(signed long long int) SCORE_MATED);
		beta  = (Score) std::min((signed long long int)_expectedValue + delta,(signed long long int) SCORE_MATE);
	}

	int globalReduction = 0;
	unsigned int iteration = 0;

	//----------------------------------
	// aspiration window
	//----------------------------------
	do
	{	++iteration;
		_maxPlyReached = 0;
		_validIteration = false;
		_pvLineFollower.restart();
		PVline newPV;
		newPV.clear();

		_lw = std::unique_ptr<logWriter>(new logWriter(_pos.getFen(), depth, iteration));
		Score res = alphaBeta<Search::impl::nodeType::ROOT_NODE>(0, (depth-globalReduction) * ONE_PLY, alpha, beta, newPV);

		if(_validIteration || !_stop)
		{
			long long int elapsedTime = _st.getElapsedTime();

			if (res <= alpha)
			{
				if( uciParameters::multiPVLines == 1 )
				{
					_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, getVisitedNodes(), _pos.isChess960(), UciOutput::PVbound::upperbound);
				}

				alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

				globalReduction = 0;
				if( masterThread )
				{
					tm.notifyFailLow();
				}
			}
			else if (res >= beta)
			{
				if( uciParameters::multiPVLines == 1 )
				{
					_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, getVisitedNodes(), _pos.isChess960(), UciOutput::PVbound::lowerbound);
				}

				beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
				if(depth > 1)
				{
					globalReduction = 1;
				}
				if( masterThread )
				{
					tm.notifyFailOver();
				}
				_pvLineFollower.setPVline(newPV);

				bestMove = rootMove( newPV.getMove(0), newPV, res, _maxPlyReached, depth, getVisitedNodes(), elapsedTime );
			}
			else
			{
				bestMove = rootMove( newPV.getMove(0), newPV, res, _maxPlyReached, depth, getVisitedNodes(), elapsedTime );
				return bestMove;
				break;
			}
			
			delta += delta / 2;
		}
	}
	while(!_stop);

	return bestMove;

}

void Search::impl::excludeRootMoves( std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, bool masterThread )
{
	if( masterThread )
	{
		std::lock_guard<std::mutex> lock(_mutex);

		// get temporary results from all the threads
		std::map<unsigned short, unsigned int> tempBestMoves;
		for( auto& m: temporaryResults)
		{
			tempBestMoves[m.firstMove.getPacked()]++;
		}

		Move mostSearchedMove = Move(tempBestMoves.begin()->first);
		unsigned int max = tempBestMoves.begin()->second;
		for( const auto& m: tempBestMoves )
		{
			if( m.second > max)
			{
				max = m.second;
				mostSearchedMove = Move(m.first);
			}
		}

		unsigned int threshold = uciParameters::threads * 0.75;
		// and make some of search alternative moves
		for( unsigned int i = 1; i < uciParameters::threads; ++i)
		{
			if( i >= threshold)
			{
				toBeExcludedMove[i] = mostSearchedMove;
			}
			else
			{
				toBeExcludedMove[i] = Move::NOMOVE;
			}
		}
	}
	else
	{
		std::lock_guard<std::mutex> lock(_mutex);
		// filter out some root move to search alternatives
		_sd.setExcludedMove(0, toBeExcludedMove[index]);
	}
}

void Search::impl::idLoop(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, int depth, Score alpha, Score beta, bool masterThread)
{
	//_printRootMoveList();
	rootMove& bestMove = temporaryResults[index];
	
	my_thread &thr = my_thread::getInstance();
	// manage multi PV moves
	_multiPVmanager.setLinesToBeSearched( std::min( uciParameters::multiPVLines, (unsigned int)_rootMovesToBeSearched.size()) );

	// ramdomly initialize the bestmove
	bestMove = rootMove(_rootMovesToBeSearched[0]);
	
	do
	{
		_UOI->setDepth(depth);
		_UOI->printDepth();

		//----------------------------
		// exclude root moves in multithread search
		//----------------------------
		excludeRootMoves(temporaryResults, index, toBeExcludedMove, masterThread);

		//----------------------------
		// iterative loop
		//----------------------------
		_rootMovesAlreadySearched.clear();
		
		//----------------------------------
		// multi PV loop
		//----------------------------------
		for ( _multiPVmanager.startNewIteration(); _multiPVmanager.thereArePvToBeSearched(); _multiPVmanager.goToNextPV() )
		{
			_UOI->setPVlineIndex(_multiPVmanager.getPVNumber());
			//----------------------------------
			// reload PV
			//----------------------------------
			if( rootMove rm(Move::NOMOVE); _multiPVmanager.getNextRootMove(rm) )
			{
				_expectedValue = rm.score;
				_pvLineFollower.setPVline(rm.PV);
			}
			else
			{
				_expectedValue = -SCORE_INFINITE;
				_pvLineFollower.clear();
			}

			//----------------------------------
			// aspiration window
			//----------------------------------
			rootMove res = aspirationWindow( depth, alpha, beta, masterThread);
			if( res.firstMove != Move::NOMOVE )
			{
				bestMove = res;
				_multiPVmanager.insertMove(bestMove);
				_rootMovesAlreadySearched.push_back(bestMove.firstMove);
			}

			// at depth 1 only print the PV at the end of search
			if(!_stop && depth == 1)
			{
				_UOI->printPV(res.score, _maxPlyReached, _st.getElapsedTime(), res.PV, getVisitedNodes(), _pos.isChess960(), UciOutput::PVbound::upperbound);
			}
			if(!_stop && uciParameters::multiPVLines > 1)
			{
				auto mpRes = _multiPVmanager.get();
				bestMove = mpRes[0];
				_UOI->printPVs(mpRes, _pos.isChess960());
			}
		}

		if( masterThread )
		{
			thr.getTimeMan().notifyIterationHasBeenFinished();
		}
	}
	while( ++depth <= (_sl.isDepthLimitedSearch() ? _sl.getDepth() : 100) && !_stop);

}

SearchResult Search::impl::go(int depth, Score alpha, Score beta, PVline pvToBeFollowed)
{
	//------------------------------------
	//init the new search
	//------------------------------------
	
	//clean transposition table
	transpositionTable::getInstance().newSearch();
	
	_pvLineFollower.setPVline(pvToBeFollowed);
	
	//--------------------------------
	// generate the list of root moves to be searched
	//--------------------------------
	generateRootMovesList(_rootMovesToBeSearched, _sl.getMoveList());
	
	//--------------------------------
	//	tablebase probing, filtering rootmoves to be searched
	//--------------------------------
	if(!_sl.isSearchMovesMode() && uciParameters::multiPVLines==1)
	{
		filterRootMovesByTablebase(_rootMovesToBeSearched);
	}
	

	// setup main thread
	cleanMemoryBeforeStartingNewSearch();

	// setup other threads
	helperSearch.clear();
	helperSearch.resize( uciParameters::threads - 1, *this );

	for (auto& hs : helperSearch)
	{
		// mute helper thread
		hs.setUOI(UciOutput::create( UciOutput::type::mute ) );
		
		// setup helper thread
		hs.cleanMemoryBeforeStartingNewSearch();
	}
	
	_initialTurn = _pos.getNextTurn();
	//----------------------------------
	// we can start the real search
	//----------------------------------
	
	// manage depth 0 search ( return qsearch )
	if(_sl.getDepth() == 0)
	{
		return manageQsearch();
	}

	//----------------------------
	// multithread : lazy smp threads
	//----------------------------

	
	std::vector<std::thread> helperThread;
	Move m(0);
	rootMove rm(m);
	std::vector<rootMove> helperResults( uciParameters::threads, rm);
	std::vector<Move> toBeExcludedMove( uciParameters::threads, Move::NOMOVE);

	// launch helper threads
	for( unsigned int i = 1; i < ( uciParameters::threads); ++i)
	{

		helperResults[i].firstMove = m;
		helperSearch[i-1].resetStopCondition();
		helperSearch[i-1]._pos = _pos;
		helperSearch[i-1]._pvLineFollower.setPVline(pvToBeFollowed);
		helperThread.emplace_back( std::thread(&Search::impl::idLoop, &helperSearch[i-1], std::ref(helperResults), i, std::ref(toBeExcludedMove), depth, alpha, beta, false));
	}

	//----------------------------------
	// iterative deepening loop
	//----------------------------------
	helperResults[0].firstMove = m;
	idLoop(helperResults, 0, toBeExcludedMove, depth, alpha, beta, true);
	
	// _stop helper threads
	for(unsigned int i = 0; i< ( uciParameters::threads - 1); i++)
	{
		helperSearch[i].stopSearch();
	}
	for(auto &t : helperThread)
	{
		t.join();
	}
	
	//----------------------------------
	// gather results
	//----------------------------------
	const rootMove& bestMove = voteSystem(helperResults).getBestMove();
	
	return SearchResult( alpha, beta, bestMove.depth, bestMove.PV, bestMove.score);

}

inline void Search::impl::_updateCounterMove(const Move& m)
{
	if( const Move& previousMove = _pos.getActualState().getCurrentMove() )
	{
		_sd.getCounterMove().update( _pos.getPieceAt( previousMove.getTo() ), previousMove.getTo(), m );
	}
}

bool Search::impl::_manageDraw(const bool PVnode, PVline& pvLine)
{
	if(_pos.isDraw(PVnode) || _stop)
	{
		if(PVnode)
		{
			pvLine.clear();
		}
		return true;
	}
	return false;
}

void Search::impl::_showCurrenLine( const unsigned int ply, const int depth )
{
	if( _showLine && depth <= ONE_PLY)
	{
		_showLine = false;
		_UOI->showCurrLine(_pos, ply);
	}
}

inline bool Search::impl::_MateDistancePruning( const unsigned int ply, Score& alpha, Score& beta) const
{
	alpha = std::max(matedIn(ply), alpha);
	beta = std::min(mateIn(ply+1), beta);
	if (alpha >= beta)
	{
		return true;
	}
	return false;
}
void Search::impl::_appendTTmoveIfLegal( const Move& ttm, PVline& pvLine ) const
{
	if( _pos.isMoveLegal(ttm) )
	{
		pvLine.set( ttm );
	}
	else
	{
		pvLine.clear();
	}
}

inline bool Search::impl::_canUseTTeValue( const bool PVnode, const Score beta, const Score ttValue, const ttEntry * const tte, short int depth ) const
{
	return
		( tte->getDepth() >= depth )
		&& ( ttValue != SCORE_NONE )// Only in case of TT access race
		&& (
			PVnode ?
				false :
			ttValue >= beta ?
				tte->isTypeGoodForBetaCutoff():
				tte->isTypeGoodForAlphaCutoff()
		);
}

inline const HashKey Search::impl::_getSearchKey( const bool excludedMove ) const
{
	return excludedMove ? _pos.getExclusionKey() : _pos.getKey();
}

inline Search::impl::tableBaseRes Search::impl::_checkTablebase(const unsigned int ply, const int depth)
{
	tableBaseRes res{typeScoreLowerThanAlpha, SCORE_NONE};
	Syzygy& szg = Syzygy::getInstance();
	
	if (szg.getMaxCardinality()) {
		
		res.TTtype = typeScoreLowerThanAlpha;
		unsigned int piecesCnt = bitCnt (_pos.getBitmap(whitePieces) | _pos.getBitmap(blackPieces));

		if (piecesCnt <= szg.getMaxCardinality()
			&& (piecesCnt < szg.getMaxCardinality() || depth >= (int)( uciParameters::SyzygyProbeDepth * ONE_PLY ) )
			&& _pos.getActualState().getIrreversibleMoveCount() == 0
			&& _pos.getCastleRights() == noCastle ) {
			
			ProbeState err;			
			WDLScore wdl = szg.probeWdl(_pos, err);
			
			if (err != ProbeState::FAIL) {
				++_tbHits;

				if (uciParameters::Syzygy50MoveRule) {
					switch(wdl)
					{
					case WDLScore::WDLLoss:
						res.value = SCORE_MATED + 100 + ply;
						res.TTtype = typeScoreLowerThanAlpha;
						break;
					case WDLScore::WDLBlessedLoss:
						res.TTtype = typeExact;
						res.value = -100;
						break;
					case WDLScore::WDLDraw:
						res.TTtype = typeExact;
						res.value = 0;
						break;
					case WDLScore::WDLCursedWin:
						res.TTtype = typeExact;
						res.value = 100;
						break;
					case WDLScore::WDLWin:
						res.TTtype = typeScoreHigherThanBeta;
						res.value = SCORE_MATE -100 -ply;
						break;
					default:
						res.value = 0;
					}

				}
				else
				{
					switch(wdl)
					{
					case WDLScore::WDLLoss:
					case WDLScore::WDLBlessedLoss:
						res.TTtype = typeScoreLowerThanAlpha;
						res.value = SCORE_MATED + 100 + ply;
						break;
					case WDLScore::WDLDraw:
						res.TTtype = typeExact;
						res.value = 0;
						break;
					case WDLScore::WDLCursedWin:
					case WDLScore::WDLWin:
						res.TTtype = typeScoreHigherThanBeta;
						res.value = SCORE_MATE - 100 - ply;
						break;
					default:
						res.value = 0;
					}
				}
			}
		}
	}
	return res;

}

template<Search::impl::nodeType type> Score Search::impl::alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{
	logNode ln(*_lw, ply, depth, alpha, beta);
	//--------------------------------------
	// node asserts
	//--------------------------------------
	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);

	//--------------------------------------
	// initialize node constants
	//--------------------------------------
	const bool PVnode = ( type == nodeType::PV_NODE || type == nodeType::ROOT_NODE );
	const bool inCheck = _pos.isInCheck();
	_sd.setInCheck(ply, inCheck);

	_updateNodeStatistics(ply);
	_sd.clearKillers(ply+1);

	//--------------------------------------
	// show current line if needed
	//--------------------------------------
	_showCurrenLine( ply, depth );

	//--------------------------------------
	// choose node type
	//--------------------------------------
	const nodeType childNodesType =
		type == nodeType::ALL_NODE ?
			nodeType::CUT_NODE :
		type == nodeType::CUT_NODE ?
			nodeType::ALL_NODE :
			nodeType::PV_NODE;
	(void)childNodesType;	// to suppress warning in root node and PV nodes


	if(type != nodeType::ROOT_NODE )
	{
		//---------------------------------------
		//	Manage Draw
		//---------------------------------------
		ln.testIsDraw();
		if( _manageDraw( PVnode, pvLine) ) {
			ln.logReturnValue(getDrawValue());
			return getDrawValue();
		}
		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		ln.testMateDistancePruning();
		if( _MateDistancePruning( ply, alpha, beta) ) {
			ln.logReturnValue(alpha);
			return alpha;
		}
	}

	const Move& excludedMove = _sd.getExcludedMove(ply);
	const HashKey& posKey = _getSearchKey( (bool)excludedMove );

	//--------------------------------------
	// test the transposition table
	//--------------------------------------
	ttEntry* tte = transpositionTable::getInstance().probe( posKey );
	Move ttMove( tte->getPackedMove() );
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(), ply);

	ln.testCanUseTT();
	if (	type != nodeType::ROOT_NODE
			&& _canUseTTeValue( PVnode, beta, ttValue, tte, depth )
		)
	{
		transpositionTable::getInstance().refresh(*tte);
		
		if constexpr (PVnode)
		{
			_appendTTmoveIfLegal( ttMove, pvLine);
		}

		//save killers
		if (ttValue >= beta
			&& ttMove
			&& !_pos.isCaptureMoveOrPromotion(ttMove)
			&& !inCheck)
		{
			_sd.saveKillers(ply, ttMove);
			_updateCounterMove( ttMove );
		}
		ln.logReturnValue(ttValue);
		return ttValue;
	}
	
	//--------------------------------------
	// overwrite ttMove with move from move from PVlineToBeFollowed
	//--------------------------------------
	if constexpr ( PVnode )
	{
		if (_pvLineFollower.getNextMove(ply, ttMove))
		{
			assert(_pos.isMoveLegal(ttMove));
		}
		
	}

	//--------------------------------------
	// table base prove
	//--------------------------------------
	if constexpr (!PVnode)
	{
		ln.testCheckTablebase();
		auto res = _checkTablebase(ply, depth);
		if( res.value != SCORE_NONE )
		{
			if(	res.TTtype == typeExact || (res.TTtype == typeScoreHigherThanBeta  && res.value >=beta) || (res.TTtype == typeScoreLowerThanAlpha && res.value <=alpha)	)
			{
				transpositionTable::getInstance().store(posKey,
					transpositionTable::scoreToTT(res.value, ply),
					res.TTtype,
					std::min( 100 * ONE_PLY , depth + 6 * ONE_PLY),
					ttMove,
					_pos.eval<false>());
				ln.logReturnValue(res.value);
				return res.value;
			}
		}
	}


	//---------------------------------
	// calc the eval & static eval
	//---------------------------------

	Score staticEval;
	Score eval;
	if(inCheck || tte->getType() == typeVoid)
	{
		staticEval = _pos.eval<false>();
		eval = staticEval;
		ln.calcStaticEval(staticEval);

#ifdef DEBUG_EVAL_SIMMETRY
		testSimmetry(_pos);
#endif
	}
	else
	{
		staticEval = tte->getStaticValue();
		eval = staticEval;
		assert(staticEval < SCORE_INFINITE);
		assert(staticEval > -SCORE_INFINITE);
		ln.calcStaticEval(staticEval);

		if (ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > eval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < eval) )
				)
			{
				ln.refineEval(ttValue);
				eval = ttValue;
			}
		}
	}
	
	_sd.setStaticEval(ply, staticEval);
	
	bool improving = false;
	if( ply <2 || inCheck || ( ply >=2 && _sd.getInCheck(ply - 2) ) || ( ply >=2 && _sd.getStaticEval(ply) >= _sd.getStaticEval(ply-2) ) )
	{
		improving = true;
	}

	//-----------------------------
	// reduction && pruning
	//-----------------------------
	if constexpr ( !PVnode )
	{
		if(!inCheck )
		{

			//------------------------
			// razoring
			//------------------------
			// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
			//------------------------
			if (!_sd.skipNullMove(ply)
				&&  depth < 4 * ONE_PLY
				&&  eval + razorMargin(depth, type == nodeType::CUT_NODE) <= alpha
				&&  alpha >= -SCORE_INFINITE+razorMargin(depth, type == nodeType::CUT_NODE)
				&&  ( !ttMove || type == nodeType::ALL_NODE)
			)
			{
				PVline childPV;
				Score v = qsearch<nodeType::CUT_NODE>(ply,0 , alpha, alpha+1, childPV);
				if (v <= alpha)
				{
					return v;
				}
			}

			//---------------------------
			//	 STATIC NULL MOVE PRUNING
			//---------------------------
			//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
			//---------------------------
			if (!_sd.skipNullMove(ply)
				&& depth < 8 * ONE_PLY
				&& eval - futility( depth, improving ) >= beta
				&& eval < SCORE_KNOWN_WIN
				&& _pos.hasActivePlayerNonPawnMaterial()
			)
			{
				assert((depth>>ONE_PLY_SHIFT)<8);
				return eval;
			}


			//---------------------------
			//	 NULL MOVE PRUNING
			//---------------------------
			// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
			// this search let us know about threat move by the opponent.
			//---------------------------

			if( depth >= ONE_PLY
				&& eval >= beta
				&& !_sd.skipNullMove(ply)
				&& _pos.hasActivePlayerNonPawnMaterial()
			){
				int newPly = ply + 1;
				// Null move dynamic reduction based on depth
				int red = 3 * ONE_PLY + depth / 4;

				// Null move dynamic reduction based on value
				if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta)
				{
					red += ONE_PLY;
				}

				_pos.doNullMove();
				_sd.setSkipNullMove(newPly, true);

				//uint64_t nullKey = _pos.getKey();
				Score nullVal;
				PVline childPV;
				if( depth - red < ONE_PLY )
				{
					nullVal = -qsearch<childNodesType>(newPly, 0, -beta, -beta + 1, childPV);
				}
				else
				{
					nullVal = -alphaBeta<childNodesType>(newPly, depth - red, -beta, -beta + 1, childPV);
				}

				_pos.undoNullMove();
				_sd.setSkipNullMove(newPly, false);

				if (nullVal >= beta)
				{
					// Do not return unproven mate scores
					if (nullVal >= SCORE_MATE_IN_MAX_PLY)
					{
						nullVal = beta;
					}

					if (depth < 12 * ONE_PLY)
					{
						return nullVal;
					}

					// Do verification search at high depths
					_sd.setSkipNullMove(ply, true);
					assert(depth - red >= ONE_PLY);
					Score val;
					val = alphaBeta<childNodesType>(ply, depth - red, beta - 1, beta, childPV);
					_sd.setSkipNullMove(ply, false);
					if (val >= beta)
					{
						return nullVal;
					}

				}

			}

			//------------------------
			//	PROB CUT
			//------------------------
			//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
			//------------------------

			if( depth >= 5 * ONE_PLY
				&&  !_sd.skipNullMove(ply)
				// && abs(beta)<SCORE_KNOWN_WIN
				// && eval> beta-40000
				&& abs(beta) < SCORE_MATE_IN_MAX_PLY
			){
				Score s;
				Score rBeta = std::min(beta + 8000, SCORE_INFINITE);
				int rDepth = depth - ONE_PLY - 3 * ONE_PLY;

				MovePicker mp(_pos, _sd, ply, ttMove);
				mp.setupProbCutSearch( _pos.getCapturedPiece() );

				Move m;
				PVline childPV;
				unsigned int pbCount = 0u;
				while( ( m = mp.getNextMove() ) && ( pbCount < 3 ) )
				{
					if( m == excludedMove )
					{
						continue;
					}

					++pbCount;
					_pos.doMove(m);

					assert(rDepth>=ONE_PLY);
					s = -alphaBeta<childNodesType>(ply + 1, rDepth, -rBeta, -rBeta + 1, childPV);

					_pos.undoMove();

					if(s >= rBeta)
					{
						return s;
					}

				}
			}
		}

	}


	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? 5 * ONE_PLY : 8 * ONE_PLY)
		&& !ttMove
		&& (PVnode || staticEval + 10000 >= beta))
	{
		int d = depth - 2 * ONE_PLY - (PVnode ? 0 : depth / 4);

		bool skipBackup = _sd.skipNullMove(ply);
		_sd.setSkipNullMove(ply, true);

		PVline childPV;
		const nodeType iidType = type;
		assert(d >= ONE_PLY);
		alphaBeta<iidType>(ply, d, alpha, beta, childPV);

		_sd.setSkipNullMove(ply, skipBackup);

		tte = transpositionTable::getInstance().probe(posKey);
		ttMove = tte->getPackedMove();
	}




	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Move::NOMOVE);

	Move m;
	MovePicker mp(_pos, _sd, ply, ttMove);
	unsigned int moveNumber = 0;
	unsigned int quietMoveCount = 0;
	Move quietMoveList[64];
	unsigned int captureMoveCount = 0;
	Move captureMoveList[32];

	bool singularExtensionNode =
		type != nodeType::ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove
		&& !excludedMove // Recursive singular Search is not allowed
		&& tte != nullptr
		&& tte->isTypeGoodForBetaCutoff()
		&& tte->getDepth() >= depth - 3 * ONE_PLY;

	while (bestScore <beta  && ( m = mp.getNextMove() ) )
	{

		assert( m );
		if(m == excludedMove)
		{
			continue;
		}

		// Search only the moves in the Search list
		if( type == nodeType::ROOT_NODE && ( std::count(_rootMovesAlreadySearched.begin(), _rootMovesAlreadySearched.end(), m ) || !std::count(_rootMovesToBeSearched.begin(), _rootMovesToBeSearched.end(), m) ) )
		{
			continue;
		}
		++moveNumber;


		bool captureOrPromotion = _pos.isCaptureMoveOrPromotion(m);


		bool moveGivesCheck = _pos.moveGivesCheck(m);
		bool isDangerous = moveGivesCheck || m.isCastleMove() || _pos.isPassedPawnMove(m);
		bool FutilityMoveCountFlag = (depth < 16 * ONE_PLY ) && (moveNumber >= FutilityMoveCounts[improving][depth >> ONE_PLY_SHIFT]);

		int ext = 0;
		if(PVnode && isDangerous )
		{
			ext = ONE_PLY;
		}
		else if( moveGivesCheck && _pos.seeSign(m) >= 0 && !FutilityMoveCountFlag)
		{
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if( singularExtensionNode
			&& !ext
			&& m == ttMove
			//&&  abs(ttValue) < SCORE_KNOWN_WIN
			&& abs(beta) < SCORE_MATE_IN_MAX_PLY
		)
		{

			PVline childPv;

			Score rBeta = ttValue - int(depth * (320 / ONE_PLY));
			rBeta = std::max( rBeta, -SCORE_MATE + 1 );

			_sd.setExcludedMove(ply, m);
			Score temp = alphaBeta<nodeType::ALL_NODE>(ply, depth/2, rBeta - 1, rBeta, childPv);
			_sd.setExcludedMove(ply, Move::NOMOVE);

			if(temp < rBeta)
			{
				ext = ONE_PLY;
		    }
		}

		int newDepth = depth - ONE_PLY + ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if( type != nodeType::ROOT_NODE
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore > SCORE_MATED_IN_MAX_PLY
		){
			assert(moveNumber > 1);

			if(FutilityMoveCountFlag)
			{
				assert((newDepth>>ONE_PLY_SHIFT)<11);
				continue;
			}

			if(newDepth < 7 * ONE_PLY)
			{
				Score localEval = eval + futilityMargin[newDepth >> ONE_PLY_SHIFT];
				if(localEval <= alpha)
				{
					if constexpr ( !PVnode )
					{
						bestScore = std::max(bestScore, localEval);
					}
					assert((newDepth>>ONE_PLY_SHIFT)<7);
					continue;
				}
			}

			if(newDepth < 4 * ONE_PLY && _pos.seeSign(m) < 0)
			{
				continue;
			}
		}

		if(type == nodeType::ROOT_NODE)
		{
			long long int elapsed = _st.getElapsedTime();
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!_stop
				)
			{
				_UOI->printCurrMoveNumber(moveNumber, m, getVisitedNodes(), elapsed, _pos.isChess960());
			}
		}


		_pos.doMove(m);
		Score val;
		PVline childPV;

		if constexpr (PVnode)
		{
			if(moveNumber==1)
			{
				if(newDepth < ONE_PLY)
				{
					val = -qsearch<nodeType::PV_NODE>(ply + 1, newDepth, -beta, -alpha, childPV);
				}
				else
				{
					val = -alphaBeta<nodeType::PV_NODE>(ply + 1, newDepth, -beta, -alpha, childPV);
				}
			}
			else
			{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch = true;
				if( depth >= 3 * ONE_PLY
					&& (!captureOrPromotion || FutilityMoveCountFlag )
					&& !isDangerous
					&& m != ttMove
					&& !mp.isKillerMove(m)
				)
				{
					assert(moveNumber != 0);

					int reduction = PVreduction[improving][ std::min(depth, int((LmrLimit * ONE_PLY) - 1)) ][ std::min(moveNumber, (unsigned int)63) ];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction != 0)
					{
						val = -alphaBeta<nodeType::CUT_NODE>(ply + 1, d, -alpha - 1, -alpha, childPV);
						if(val <= alpha)
						{
							doFullDepthSearch = false;
						}
					}
				}


				if(doFullDepthSearch)
				{

					if(newDepth < ONE_PLY)
					{
						val = -qsearch<nodeType::CUT_NODE>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<nodeType::CUT_NODE>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}

					if( val > alpha && val < beta )
					{
						if( newDepth < ONE_PLY )
						{
							val = -qsearch<nodeType::PV_NODE>(ply + 1, newDepth, -beta, -alpha, childPV);
						}
						else
						{
							val = -alphaBeta<nodeType::PV_NODE>(ply + 1, newDepth, -beta, -alpha, childPV);
						}
					}
				}
			}
		}
		else
		{

			//------------------------------
			//	LMR
			//------------------------------
			bool doFullDepthSearch = true;
			if( depth >= 3 * ONE_PLY
				&& (!captureOrPromotion || FutilityMoveCountFlag )
				&& !isDangerous
				&& m != ttMove
				&& !mp.isKillerMove(m)
			)
			{
				int reduction = nonPVreduction[improving][std::min(depth, int((LmrLimit * ONE_PLY) - 1))][std::min(moveNumber, (unsigned int)63)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction != 0)
				{
					val = -alphaBeta<childNodesType>(ply + 1, d, -alpha - 1, -alpha, childPV);
					if(val <= alpha)
					{
						doFullDepthSearch = false;
					}
				}
			}

			if(doFullDepthSearch)
			{

				if(moveNumber<5)
				{
					if(newDepth < ONE_PLY)
					{
						val = -qsearch<childNodesType>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<childNodesType>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
				}
				else
				{
					if(newDepth < ONE_PLY)
					{
						val = -qsearch<nodeType::CUT_NODE>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<nodeType::CUT_NODE>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
				}
			}
		}

		_pos.undoMove();

		if(!_stop && val > bestScore)
		{
			bestScore = val;

			if(bestScore > alpha)
			{
				bestMove = m;
				if constexpr (PVnode)
				{
					alpha = bestScore;
					pvLine.appendNewPvLine( bestMove, childPV);
					if(type == nodeType::ROOT_NODE && uciParameters::multiPVLines == 1 )
					{
						if(val < beta && depth > 1 * ONE_PLY)
						{
							_UOI->printPV(val, _maxPlyReached, _st.getElapsedTime(), pvLine, getVisitedNodes(), _pos.isChess960());
						}
						if(val > _expectedValue - 800)
						{
							_validIteration = true;
						}
					}
				}
			}
		}
		
		if( m != bestMove )
		{
			if(!captureOrPromotion)
			{
				if(quietMoveCount < 64)
				{
					quietMoveList[quietMoveCount++] = m;
				}
			}
			else
			{
				if(captureMoveCount < 32)
				{
					captureMoveList[captureMoveCount++] = m;
				}
			}
		}
	}


	// draw

	if(!moveNumber)
	{
		if( excludedMove )
		{
			return alpha;
		}
		else if(!inCheck)
		{
			bestScore = std::min( (int)0, (int)(-5000 + _pos.getPly()*250) );
		}
		else
		{
			bestScore = matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;

	if(!_stop)
	{
		transpositionTable::getInstance().store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove ) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove, staticEval);
	}

	// save killer move & update history
	if (bestScore >= beta && !inCheck)
	{
		if (!_pos.isCaptureMoveOrPromotion(bestMove))
		{
			_sd.saveKillers(ply, bestMove);

			// update history
			int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
			Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY * ONE_PLY);

			auto& history = _sd.getHistory();
			history.update( _pos.isWhiteTurn() ? white: black, bestMove, bonus);

			for (unsigned int i = 0; i < quietMoveCount; ++i)
			{
				Move m = quietMoveList[i];
				history.update( _pos.isWhiteTurn() ? white: black, m, -bonus);
			}
			
			_updateCounterMove( bestMove );
		}
		else
		{
			//if( _pos.isCaptureMove( bestMove ) )
			{
				// update capture history
				int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
				Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY * ONE_PLY);

				auto & capt= _sd.getCaptureHistory();
				capt.update( _pos.getPieceAt(bestMove.getFrom()), bestMove, _pos.getPieceAt(bestMove.getTo()), bonus);

				for (unsigned int i = 0; i < captureMoveCount; i++)
				{
					Move m = captureMoveList[i];
					//if( _pos.isCaptureMove( m ) )
					{
						capt.update( _pos.getPieceAt(m.getFrom()), m, _pos.getPieceAt(m.getTo()), -bonus);
					}
				}
			}
		}
	}
	return bestScore;

}

inline void Search::impl::_updateNodeStatistics(const unsigned int ply)
{
	_maxPlyReached = std::max(ply, _maxPlyReached);
	++_visitedNodes;
}


template<Search::impl::nodeType type> Score Search::impl::qsearch(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{
	logNode ln(*_lw, ply, depth, alpha, beta);
	//---------------------------------------
	//	node asserts
	//---------------------------------------
	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);
	//---------------------------------------
	//	initialize constants
	//---------------------------------------
	const bool PVnode = (type == nodeType::PV_NODE);
	assert( PVnode || alpha + 1 == beta );

	bool inCheck = _pos.isInCheck();
	_sd.setInCheck(ply, inCheck);

	_updateNodeStatistics(ply);
	
	ln.testIsDraw();
	if( _manageDraw( PVnode, pvLine) ) {
		ln.logReturnValue(getDrawValue());
		return getDrawValue();
	}
	//---------------------------------------
	//	MATE DISTANCE PRUNING
	//---------------------------------------
	//if( _MateDistancePruning( ply, alpha, beta) ) return alpha;

	//----------------------------
	//	next node type
	//----------------------------
	const nodeType childNodesType =
		type == nodeType::ALL_NODE ?
			nodeType::CUT_NODE :
		type == nodeType::CUT_NODE ?
			nodeType::ALL_NODE :
			nodeType::PV_NODE;


	const HashKey& posKey = _getSearchKey();
	ttEntry* const tte = transpositionTable::getInstance().probe( _pos.getKey() );
	ln.logTTprobe(*tte);
	Move ttMove( tte->getPackedMove() );
	if(!_pos.isMoveLegal(ttMove)) {
		ttMove = Move::NOMOVE;
	}
	
	// overwrite ttMove with move from move from PVlineToBeFollowed
	if constexpr ( PVnode )
	{
		if (_pvLineFollower.getNextMove(ply, ttMove))
		{
			assert(_pos.isMoveLegal(ttMove));
		}
	}
	

	MovePicker mp(_pos, _sd, ply, ttMove);
	
	short int TTdepth = mp.setupQuiescentSearch(inCheck, depth) * ONE_PLY;
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(), ply);

	ln.testCanUseTT();
	if( _canUseTTeValue( PVnode, beta, ttValue, tte, TTdepth ) )
	{
		transpositionTable::getInstance().refresh(*tte);
		if constexpr (PVnode)
		{
			_appendTTmoveIfLegal( ttMove, pvLine);
		}
		ln.logReturnValue(ttValue);
		return ttValue;
	}


	ttType TTtype = typeScoreLowerThanAlpha;


	Score staticEval = (tte->getType() != typeVoid) ? tte->getStaticValue() : _pos.eval<false>();
	ln.calcStaticEval(staticEval);
#ifdef DEBUG_EVAL_SIMMETRY
	testSimmetry(_pos);
#endif

	//----------------------------
	//	stand pat score
	//----------------------------
	Score bestScore;
	Score futilityBase;
	if(!inCheck)
	{
		bestScore = staticEval;
		// todo trovare un valore buono per il futility

		if( /*!PVnode && */ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > staticEval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < staticEval) )
			)
			{
				bestScore = ttValue;
			}
		}
		ln.calcBestScore(bestScore);

		if(bestScore > alpha)
		{
			assert(!inCheck);

			// TODO testare se la riga TTtype=typeExact; ha senso
			if constexpr (PVnode)
			{
				pvLine.clear();
			}

			ln.testStandPat();
			if( bestScore >= beta)
			{
				if( !_pos.isCaptureMoveOrPromotion(ttMove) )
				{
					_sd.saveKillers(ply, ttMove);
				}
				if(!_stop)
				{
					transpositionTable::getInstance().store(posKey, transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, ttMove, staticEval);
				}
				ln.logReturnValue(bestScore);
				return bestScore;
			}
			ln.raisedAlpha();
			alpha = bestScore;
			TTtype = typeExact;

		}
		
		futilityBase = bestScore + 5050;


	}
	else
	{
		bestScore = -SCORE_INFINITE;
		futilityBase = -SCORE_INFINITE;
	}


	//----------------------------
	//	try the captures
	//----------------------------
	Move m;
	Move bestMove(Move::NOMOVE);

	PVline childPV;

	while ( ( m = mp.getNextMove() ) )
	{
		ln.testMove(m);
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert( m );


		if(!inCheck)
		{
			// allow only queen promotion at deeper search
			if( (TTdepth < -1 * ONE_PLY) && ( m.isPromotionMove() ) && (m.getPromotionType() != Move::promQueen))
			{
				ln.skipMove();
				continue;
			}

			// at very deep search allow only recapture
			if(depth < -7 * ONE_PLY && _pos.getActualState().getCurrentMove().getTo() != m.getTo())
			{
				ln.skipMove();
				continue;
			}

			//----------------------------
			//	futility pruning (delta pruning)
			//----------------------------
			//if constexpr ( !PVnode )
			//{
				//if(
					//m != ttMove
					//&& m.bit.flags != Move::fpromotion
					//&& !_pos.moveGivesCheck(m)
					//)
				//{
					bool moveGiveCheck = _pos.moveGivesCheck(m);
					if(
						!moveGiveCheck
						&& !_pos.isPassedPawnMove(m)
						&& abs(staticEval)<SCORE_KNOWN_WIN
					)
					{
						Score futilityValue = futilityBase
								+ Position::pieceValue[_pos.getPieceAt(m.getTo())][1]
								+ ( m.isEnPassantMove() ? Position::pieceValue[whitePawns][1] : 0);

						if( m.isPromotionMove() )
						{
							futilityValue += Position::pieceValue[m.getPromotionType() + whiteQueens][1] - Position::pieceValue[whitePawns][1];
						}

						if (futilityValue <= alpha)
						{
							bestScore = std::max(bestScore, futilityValue);
							ln.skipMove();
							continue;
						}
						
						if (futilityBase <= alpha && _pos.seeSign(m) <= 0)
						{
							bestScore = std::max(bestScore, futilityBase);
							ln.skipMove();
							continue;
						}

					}


					//----------------------------
					//	don't check moves with negative see
					//----------------------------

					// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
					// TODO testare se aggiungere o no !movegivesCheck() &&
					if(
							//!moveGiveCheck &&
							_pos.seeSign(m) < 0)
					{
						ln.skipMove();
						continue;
					}
				//}
			//}

		}
		
		_pos.doMove(m);
		Score val = -qsearch<childNodesType>(ply+1, depth - ONE_PLY, -beta, -alpha, childPV);
		_pos.undoMove();

		if(val > bestScore)
		{
			ln.raisedbestScore();
			bestScore = val;
			if( bestScore > alpha )
			{
				ln.raisedAlpha();
				bestMove = m;
				TTtype = typeExact;
				alpha = bestScore;

				if(PVnode && !_stop)
				{
					pvLine.appendNewPvLine( bestMove, childPV ); 

				}
				if( bestScore >= beta)
				{
					if( !_pos.isCaptureMoveOrPromotion(bestMove) && !inCheck )
					{
						_sd.saveKillers(ply, bestMove);
					}
					if(!_stop)
					{
						transpositionTable::getInstance().store(posKey, transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, bestMove, staticEval);
					}
					ln.logReturnValue(bestScore);
					return bestScore;
				}
			}
		}
	}

	
	if(bestScore == -SCORE_INFINITE)
	{
		ln.testMated();
		assert(inCheck);
		if constexpr (PVnode)
		{
			pvLine.clear();
		}
		ln.logReturnValue(matedIn(ply));
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);

	if( !_stop )
	{
		transpositionTable::getInstance().store(posKey, transpositionTable::scoreToTT(bestScore, ply), TTtype, (short int)TTdepth, bestMove, staticEval);
	}
	ln.logReturnValue(bestScore);
	return bestScore;

}

void Search::impl::setUOI( std::unique_ptr<UciOutput> UOI )
{
	// manage output syncronization
	sync_cout;
	_UOI = std::move(UOI);
	std::cout<<sync_noNewLineEndl;
}

inline Score Search::impl::getDrawValue() const
{
	int contemptSign = ( _pos.getNextTurn() == _initialTurn) ? 1 : -1;
	return contemptSign * std::min( (int)0, (int)(-5000 + _pos.getPly()*250) );
}

void Search::impl::initSearchParameters(void)
{
	/***************************************************
	 * LRM
	 ***************************************************/

	for (int mc = 1; mc < 64; ++mc)
	{
		PVreduction[0][0][mc] = 0;
		nonPVreduction[1][0][mc] = 0;
	}
	for (unsigned int d = 1; d < LmrLimit*ONE_PLY; ++d)
	{
		for (int mc = 1; mc < 64; ++mc)
		{
			double dd = (16.0 * d) / ONE_PLY;
			
			double    PVRed = -1.5 + 0.33 * log(double(dd)) * log(double(mc));
			double nonPVRed = -1.2 + 0.37 * log(double(dd)) * log(double(mc));

			PVreduction[1][d][mc] = (Score)(PVRed >= 1.0 ? floor(PVRed * int(ONE_PLY)) : 0);
			nonPVreduction[1][d][mc] = (Score)(nonPVRed >= 1.0 ? floor(nonPVRed * int(ONE_PLY)) : 0);

			PVreduction[0][d][mc] = PVreduction[1][d][mc];
			nonPVreduction[0][d][mc] = nonPVreduction[1][d][mc];

			if(    PVreduction[0][d][mc] > int(ONE_PLY) ){    PVreduction[0][d][mc] += int(ONE_PLY); }
			if( nonPVreduction[0][d][mc] > int(ONE_PLY) ){ nonPVreduction[0][d][mc] += int(ONE_PLY); }
		}
	}
	/***************************************************
	 * FUTILITY
	 ***************************************************/
	for (unsigned int d = 0; d < 7; ++d)
	{
		futilityMargin[d] = d*10000;
	}

	/***************************************************
	 * FUTILITY MOVE COUNT
	 ***************************************************/
	for (unsigned int d = 0; d < 16; ++d)
	{
		FutilityMoveCounts[0][d] = int(2.52 + 0.704 * std::pow( d, 1.8));
		FutilityMoveCounts[1][d] = int(4.5 + 0.704 * std::pow( d, 2.0));
	}
}

Move Search::impl::_getPonderMoveFromHash(const Move bestMove )
{
	Move ponderMove(0);
	_pos.doMove( bestMove );
	
	const ttEntry* const tte = transpositionTable::getInstance().probe(_pos.getKey());
	
	Move m( tte->getPackedMove() );
	if( _pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_pos.undoMove();
	
	return ponderMove;
}

Move Search::impl::_getPonderMoveFromBook(const Move bookMove )
{
	Move ponderMove(0);
	_pos.doMove( bookMove );
	PolyglotBook pol;
	Move m = pol.probe( _pos, uciParameters::bestMoveBook);
	
	if( _pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_pos.undoMove();
	
	return ponderMove;
}

void Search::impl::_waitStopPondering() const
{
	while(_sl.isPondering()){}
}

Position& Search::impl::getPosition()
{
	return _pos;
}

SearchResult Search::impl::manageNewSearch()
{
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/

	if( _game.isNewGame(_pos))
	{
		_game.CreateNewGame(_pos.isChess960());

	}
	_game.insertNewMoves(_pos);

	unsigned int legalMoves = _pos.getNumberOfLegalMoves();

	if(legalMoves == 0)
	{
		_UOI->printPV( Move::NOMOVE, _pos.isChess960());
		
		_waitStopPondering();

		_UOI->printBestMove( Move::NOMOVE, Move::NOMOVE, _pos.isChess960() );

		PVline pvLine;
		return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );
	}
	
	if( legalMoves == 1 && !_sl.isInfiniteSearch() )
	{
		Move bestMove = MovePicker( _pos ).getNextMove();
		
		_UOI->printPV(bestMove, _pos.isChess960());
		
		_waitStopPondering();
		
		Move ponderMove = _getPonderMoveFromHash( bestMove );
		
		_UOI->printBestMove( bestMove, ponderMove, _pos.isChess960() );

		PVline pvLine;
		pvLine.set(bestMove, ponderMove);
		return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );

	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if( uciParameters::useOwnBook && !_sl.isInfiniteSearch() )
	{
		PolyglotBook pol;
		Move bookM = pol.probe( _pos, uciParameters::bestMoveBook);
		if( bookM )
		{
			_UOI->printPV(bookM, _pos.isChess960());
			
			_waitStopPondering();
			
			Move ponderMove = _getPonderMoveFromBook( bookM );
			
			_UOI->printBestMove(bookM, ponderMove, _pos.isChess960());
			
			PVline pvLine;
			pvLine.set(bookM, ponderMove);
			return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );
		}
	}
	
	//if( game.isPonderRight() )
	//{
	//	Game::GamePosition gp = game.getNewSearchParameters();
	//
	//	PVline newPV;
	//	std::copy( gp.PV.begin(), gp.PV.end(), std::back_inserter( newPV ) );
	//	
	//	newPV.resize(gp.depth/2 + 1);
	//	newPV.pop_front();
	//	newPV.pop_front();
	//	res = src.go( gp.depth/2 + 1, gp.alpha, gp.beta, newPV );
	//}
	//else
	
	SearchResult res = go();
	
	PVline PV = res.PV;

	_waitStopPondering();

	//-----------------------------
	// print out the choosen line
	//-----------------------------

	_UOI->printGeneralInfo( transpositionTable::getInstance().getFullness(), getTbHits(), getVisitedNodes(), _st.getElapsedTime());
	
	Move bestMove = PV.getMove(0);
	Move ponderMove = PV.getMove(1);
	if( !ponderMove )
	{
		ponderMove = _getPonderMoveFromHash( bestMove );
	}
	
	_UOI->printBestMove( bestMove, ponderMove, _pos.isChess960() );

	_game.savePV(PV, res.depth, res.alpha, res.beta);
	
	return res;

}



void Search::initSearchParameters(){ Search::impl::initSearchParameters(); }
Search::Search( SearchTimer& st, SearchLimits& sl, std::unique_ptr<UciOutput> UOI):pimpl{std::make_unique<impl>(st, sl, std::move(UOI))}{}
Search::~Search() = default;
void Search::stopSearch(){ pimpl->stopSearch(); }

void Search::resetStopCondition(){ pimpl->resetStopCondition(); }
unsigned long long Search::getVisitedNodes() const{ return pimpl->getVisitedNodes(); }
unsigned long long Search::getTbHits() const{ return pimpl->getTbHits(); }
void Search::showLine(){ pimpl->showLine(); }
SearchResult Search::manageNewSearch(){ return pimpl->manageNewSearch(); }
Position& Search::getPosition(){ return pimpl->getPosition(); }
