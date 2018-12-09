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
#include <map>


#include "bitops.h"
#include "command.h"
#include "history.h"
#include "io.h"
#include "movepicker.h"
#include "position.h"
#include "search.h"
#include "transposition.h"
#include "thread.h"
#include "uciParameters.h"
#include "syzygy/tbprobe.h"
#include "vajolet.h"

#ifdef DEBUG_EVAL_SIMMETRY
	
	
	void testSimmetry(Position& pos)
	{
		static Position ppp;
		
		ppp.setupFromFen(pos.getSymmetricFen());
		
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

const int Search::ONE_PLY;
const int Search::ONE_PLY_SHIFT;

std::mutex  Search::_mutex;

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
			std::cout<<"Move: "<<displayUci(m)<<" votes: "<<res.second;
			if( bm == m ) std::cout<<" *****";
			std::cout<<std::endl;
		}
		std::cout<<"----------------------------------"<<std::endl;

		for (auto &res : _results)
		{
			if( res == bm)
			{
				std::cout<<"bestMove: "<<displayUci(res.firstMove)<<" *****"<<std::endl;
			}
			else
			{
				std::cout<<"bestMove: "<<displayUci(res.firstMove)<<std::endl;
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

Score Search::futility(int depth, bool improving )
{
	return 375 * depth - 2000 * improving;
}
Score Search::futilityMargin[7] = {0};
unsigned int Search::FutilityMoveCounts[2][16]= {{0},{0}};
Score Search::PVreduction[2][LmrLimit*ONE_PLY][64];
Score Search::nonPVreduction[2][LmrLimit*ONE_PLY][64];


static std::vector<Search> helperSearch;

unsigned long long Search::getVisitedNodes() const
{
	unsigned long long n = _visitedNodes;
	for (auto& hs : helperSearch)
		n += hs._visitedNodes;
	return n;
}

unsigned long long Search::getTbHits() const
{
	unsigned long long n = _tbHits;
	for (auto& hs : helperSearch)
		n += hs._tbHits;
	return n;
}

void Search::cleanMemoryBeforeStartingNewSearch(void)
{
	_sd.history.clear();
	_sd.captureHistory.clear();
	_sd.counterMoves.clear();
	_sd.cleanData();
	_visitedNodes = 0;
	_tbHits = 0;
	_multiPVresult.clear();
	_rootMovesAlreadySearched.clear();
}
inline void Search::enableFollowPv()
{
	_followPV = true;
}
inline void Search::disableFollowPv()
{
	_followPV = false;
}
inline void Search::manageLineToBefollowed(unsigned int ply, Move& ttMove)
{
	if (_followPV)
	{
		unsigned int lastElementIndex = _pvLineToFollow.size() - 1;
		// if line is already finished, _stop following PV
		if( ply > lastElementIndex )
		{
			disableFollowPv();
		}
		else
		{
			// overwrite the ttMove
			PVline::iterator it = _pvLineToFollow.begin();
			std::advance(it, ply);
			ttMove = *it;
			// if this is the last move of the PVline, _stop following it
			if( ply == lastElementIndex )
			{
				disableFollowPv();
			}
		}
	}
}

void Search::filterRootMovesByTablebase( std::vector<Move>& rm )
{
	unsigned results[TB_MAX_MOVES];

	unsigned int piecesCnt = bitCnt (pos.getBitmap(whitePieces) | pos.getBitmap(blackPieces));

	if ( piecesCnt <= TB_LARGEST )
	{
		unsigned result = tb_probe_root(pos.getBitmap(whitePieces),
			pos.getBitmap(blackPieces),
			pos.getBitmap(blackKing) | pos.getBitmap(whiteKing),
			pos.getBitmap(blackQueens) | pos.getBitmap(whiteQueens),
			pos.getBitmap(blackRooks) | pos.getBitmap(whiteRooks),
			pos.getBitmap(blackBishops) | pos.getBitmap(whiteBishops),
			pos.getBitmap(blackKnights) | pos.getBitmap(whiteKnights),
			pos.getBitmap(blackPawns) | pos.getBitmap(whitePawns),
			pos.getActualStateConst().getIrreversibleMoveCount(),
			pos.getActualStateConst().getCastleRights(),
			pos.hasEpSquare() ? pos.getEpSquare(): 0,
			pos.isWhiteTurn(),
			results);

		if (result != TB_RESULT_FAILED)
		{

			const unsigned wdl = TB_GET_WDL(result);
			assert(wdl<5);

			unsigned r;
			for (int i = 0; (r = results[i]) != TB_RESULT_FAILED; i++)
			{
				const unsigned moveWdl = TB_GET_WDL(r);

				unsigned ep = TB_GET_EP(r);
				Move m( (tSquare)TB_GET_FROM(r), (tSquare)TB_GET_TO(r) );
				if (ep)
				{
					m.setFlag( Move::fenpassant );
				}
				switch (TB_GET_PROMOTES(r))
				{
				case TB_PROMOTES_QUEEN:
					m.setFlag( Move::fpromotion );
					m.setPromotion( Move::promQueen );
					break;
				case TB_PROMOTES_ROOK:
					m.setFlag( Move::fpromotion );
					m.setPromotion( Move::promRook );
					break;
				case TB_PROMOTES_BISHOP:
					m.setFlag( Move::fpromotion );
					m.setPromotion( Move::promBishop );
					break;
				case TB_PROMOTES_KNIGHT:
					m.setFlag( Move::fpromotion );
					m.setPromotion( Move::promKnight );
					break;
				default:
					break;
				}



				auto position = std::find(rm.begin(), rm.end(), m);
				if (position != rm.end()) // == myVector.end() means the element was not found
				{
					if (moveWdl >= wdl)
					{
						// preserve move
					}
					else
					{
						rm.erase(position);
					}

				}

			}
		}
	}
}

void Search::generateRootMovesList( std::vector<Move>& rm, std::list<Move>& ml)
{
	rm.clear();
	
	if( ml.size() == 0 )	// all the legal moves
	{
		Move m;
		MovePicker mp( pos );
		while( ( m = mp.getNextMove() ) )
		{
			rm.emplace_back( m );
		}
	}
	else
	{
		//only selected moves
		for_each( ml.begin(), ml.end(), [&](Move &m){rm.emplace_back(m);} );
	}
}

startThinkResult Search::manageQsearch(void)
{
	PVline pvLine;
	Score res =qsearch<Search::nodeType::PV_NODE>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, pvLine);
	
	_UOI->printScore( res/100 );
	
	return startThinkResult( -SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, res );
}
/*
void Search::_printRootMoveList() const
{
	unsigned int i = 0;
	sync_cout;
	std::cout<<"move list"<<std::endl;
	for( auto m: _rootMovesToBeSearched)
	{
		++i;
		std::cout<<i<<": "<<displayUci(m)<<std::endl;
		
	}
	std::cout<<sync_endl;
}*/


rootMove Search::aspirationWindow( const int depth, Score alpha, Score beta, const bool masterThread)
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

	//----------------------------------
	// aspiration window
	//----------------------------------
	do
	{
		_maxPlyReached = 0;
		_validIteration = false;
		enableFollowPv();
		PVline newPV;
		newPV.clear();

		Score res = alphaBeta<Search::nodeType::ROOT_NODE>(0, (depth-globalReduction) * ONE_PLY, alpha, beta, newPV);

		if(_validIteration || !_stop)
		{
			long long int elapsedTime = _st.getElapsedTime();

			if (res <= alpha)
			{

				_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, getVisitedNodes(), UciOutput::upperbound);

				alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

				globalReduction = 0;
				if( masterThread )
				{
					tm.notifyFailLow();
				}

			}
			else if (res >= beta)
			{

				_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, getVisitedNodes(), UciOutput::lowerbound);

				beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
				if(depth > 1)
				{
					globalReduction = 1;
				}
				if( masterThread )
				{
					tm.notifyFailOver();
				}
				_pvLineToFollow = newPV;

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



	}while(!_stop);

	return bestMove;

}

void Search::idLoop(std::vector<rootMove>& temporaryResults, unsigned int index,std::vector<Move>& toBeExcludedMove, int depth, Score alpha, Score beta, bool masterThread)
{
	//_printRootMoveList();
	rootMove& bestMove = temporaryResults[index];
	
	my_thread &thr = my_thread::getInstance();
	// manage multi PV moves
	unsigned int linesToBeSearched = std::min( uciParameters::multiPVLines, (unsigned int)_rootMovesToBeSearched.size());

	// ramdomly initialize the bestmove

	bestMove = _rootMovesToBeSearched[0];
	
	do
	{
		_UOI->setDepth(depth);
		_UOI->printDepth();

		if( masterThread )
		{
			std::lock_guard<std::mutex> lock(_mutex);
			sync_cout<<"------------------MASTER THREAD--------------------"<<std::endl;
			std::cout<<"depth: "<<depth<<std::endl;

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

				std::cout<<displayUci(Move(m.first))<<":"<<m.second<<std::endl;
			}

			std::cout<<"Most Searched Move: "<<displayUci(mostSearchedMove)<<std::endl;

			unsigned int threshold = uciParameters::threads * 0.75;
			// and make some of search alternative moves
			for( unsigned int i = 1; i < uciParameters::threads; ++i)
			{

				std::cout<<"threshold:" << threshold <<std::endl;
				if( i >= threshold)
				{
					toBeExcludedMove[i] = mostSearchedMove;
				}
				else
				{
					toBeExcludedMove[i] = Move::NOMOVE;
				}
				std::cout<<"set excluded move for thread "<<i<<" "<<displayUci(toBeExcludedMove[i])<<std::endl;
			}

			std::cout<<sync_endl;
		}
		else
		{
			std::lock_guard<std::mutex> lock(_mutex);
			sync_cout<<"------------------HELPER THREAD "<<index<<"--------------------"<<std::endl;
			std::cout<<"depth: "<<depth<<std::endl;
			// filter out some root move to search alternatives
			std::cout<<"excluding from search "<<displayUci(toBeExcludedMove[index])<<std::endl;
			_sd.story[0].excludeMove = toBeExcludedMove[index];
			std::cout<<sync_endl;
		}

		//----------------------------
		// iterative loop
		//----------------------------

		const auto previousIterationResults = _multiPVresult;

		_multiPVresult.clear();
		_rootMovesAlreadySearched.clear();
		
		//----------------------------------
		// multi PV loop
		//----------------------------------
		for ( unsigned int _multiPVcounter = 0; _multiPVcounter < linesToBeSearched; ++_multiPVcounter )
		{
			_UOI->setPVlineIndex(_multiPVcounter);
			//----------------------------------
			// reload PV
			//----------------------------------
			if( previousIterationResults.size() > _multiPVcounter )
			{
				_expectedValue = previousIterationResults[_multiPVcounter].score;
				_pvLineToFollow = previousIterationResults[_multiPVcounter].PV;
			}
			else
			{
				_expectedValue = -SCORE_INFINITE;
				_pvLineToFollow.clear();

			}

			//----------------------------------
			// aspiration window
			//----------------------------------
			rootMove res = aspirationWindow( depth, alpha, beta, masterThread);
			if( res.firstMove != Move::NOMOVE )
			{
				bestMove = res;
				_multiPVresult.push_back(bestMove);
				_rootMovesAlreadySearched.push_back(bestMove.firstMove);
			}

			if((!_stop || _validIteration) && (linesToBeSearched > 1 || depth == 1) )
			{

				// Sort the PV lines searched so far and update the GUI				
				std::stable_sort(_multiPVresult.begin(), _multiPVresult.end());
				bestMove = _multiPVresult[0];
				
				_UOI->printPVs( _multiPVresult );
			}
		}


		if( masterThread )
		{
			thr.getTimeMan().notifyIterationHasBeenFinished();
		}
	}
	while( ++depth <= (_sl.depth != -1 ? _sl.depth : 100) && !_stop);

}

startThinkResult Search::startThinking(int depth, Score alpha, Score beta, PVline pvToBeFollowed)
{
	//------------------------------------
	//init the new search
	//------------------------------------
	
	//clean transposition table
	transpositionTable::getInstance().newSearch();
	
	_pvLineToFollow = pvToBeFollowed;
	
	//--------------------------------
	// generate the list of root moves to be searched
	//--------------------------------
	generateRootMovesList( _rootMovesToBeSearched, _sl.searchMoves );
	
	//--------------------------------
	//	tablebase probing, filtering rootmoves to be searched
	//--------------------------------
	if(_sl.searchMoves.size() == 0 && uciParameters::multiPVLines==1)
	{
		filterRootMovesByTablebase( _rootMovesToBeSearched );
	}
	

	// setup main thread
	cleanMemoryBeforeStartingNewSearch();

	// setup other threads
	helperSearch.clear();
	helperSearch.resize( uciParameters::threads - 1, *this );

	for (auto& hs : helperSearch)
	{
		// mute helper thread
		hs.setUOI(UciOutput::create( UciOutput::mute ) );
		
		// setup helper thread
		hs.cleanMemoryBeforeStartingNewSearch();
	}
	
	_initialTurn = pos.getNextTurn();
	//----------------------------------
	// we can start the real search
	//----------------------------------
	
	// manage depth 0 search ( return qsearch )
	if(_sl.depth == 0)
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
		helperSearch[i-1].pos = pos;
		helperSearch[i-1]._pvLineToFollow = _pvLineToFollow;
		helperThread.emplace_back( std::thread(&Search::idLoop, &helperSearch[i-1], std::ref(helperResults), i, std::ref(toBeExcludedMove),depth, alpha, beta, false));
	}

	//----------------------------------
	// iterative deepening loop
	//----------------------------------
	helperResults[0].firstMove = m;
	idLoop(helperResults, 0, toBeExcludedMove,depth, alpha, beta, true);
	
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
	
	return startThinkResult( alpha, beta, bestMove.depth, bestMove.PV, bestMove.score);

}

inline void Search::_updateCounterMove(const Move& m)
{
	if( const Move& previousMove = pos.getActualStateConst().getCurrentMove() )
	{
		_sd.counterMoves.update( pos.getPieceAt( previousMove.getTo() ), previousMove.getTo(), m );
	}
}

template<Search::nodeType type> Score Search::alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{

	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);

	++_visitedNodes;
	_maxPlyReached = std::max(ply, _maxPlyReached);
	_sd.clearKillers(ply+1);

	const bool PVnode = ( type == Search::nodeType::PV_NODE || type == Search::nodeType::ROOT_NODE );
	const bool inCheck = pos.isInCheck();
	bool improving = false;
	_sd.story[ply].inCheck = inCheck;
	//Move threatMove(Move::NOMOVE);


	//--------------------------------------
	// show current line if needed
	//--------------------------------------
	if( _showLine && depth <= ONE_PLY)
	{
		_showLine = false;
		_UOI->showCurrLine(pos,ply);
	}

	//--------------------------------------
	// choose node type
	//--------------------------------------

	const Search::nodeType childNodesType =
			type == Search::nodeType::ALL_NODE ?
					Search::nodeType::CUT_NODE :
					type == Search::nodeType::CUT_NODE ? Search::nodeType::ALL_NODE : Search::nodeType::PV_NODE;
					
	(void)childNodesType;	// to suppress warning in root node and PV nodes

	if(type != Search::nodeType::ROOT_NODE )
	{
		if(pos.isDraw(PVnode) || _stop)
		{
			if constexpr (PVnode)
			{
				pvLine.clear();
			}
			return getDrawValue();
		}

		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta)
		{
			return alpha;
		}

	}

	const Move& excludedMove = _sd.story[ply].excludeMove;

	const HashKey& posKey = excludedMove ? pos.getExclusionKey() : pos.getKey();

	//--------------------------------------
	// test the transposition table
	//--------------------------------------
	ttEntry* tte = transpositionTable::getInstance().probe( posKey );
	Move ttMove( tte->getPackedMove() );
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(), ply);

	if (	type != Search::nodeType::ROOT_NODE
			&& tte->getDepth() >= (depth +1 - ONE_PLY)
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    && (	PVnode ?  false
		            : ttValue >= beta ? tte->isTypeGoodForBetaCutoff()
		                              : tte->isTypeGoodForAlphaCutoff()))
	{
		transpositionTable::getInstance().refresh(*tte);

		//save killers
		if (ttValue >= beta
			&& ttMove
			&& !pos.isCaptureMoveOrPromotion(ttMove)
			&& !inCheck)
		{
			_sd.saveKillers(ply, ttMove);
			_updateCounterMove( ttMove );
		}
		

		if constexpr (PVnode)
		{
			if(pos.isMoveLegal(ttMove))
			{
				pvLine.appendNewMove( ttMove );
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}
	
	// overwrite ttMove with move from move from PVlineToBeFollowed
	if constexpr ( PVnode )
	{
		manageLineToBefollowed(ply, ttMove);
	}

	//Tablebase probe
	if constexpr (!PVnode)
	{
		if( TB_LARGEST )
		{
			ttType TTtype = typeScoreLowerThanAlpha;
			unsigned int piecesCnt = bitCnt (pos.getBitmap(whitePieces) | pos.getBitmap(blackPieces));

			if (    piecesCnt <= TB_LARGEST
				&& (piecesCnt <  TB_LARGEST || depth >= (int)( uciParameters::SyzygyProbeDepth * ONE_PLY ) )
				&&  pos.getActualStateConst().getIrreversibleMoveCount() == 0)
			{
				unsigned result = tb_probe_wdl(pos.getBitmap(whitePieces),
					pos.getBitmap(blackPieces),
					pos.getBitmap(blackKing) | pos.getBitmap(whiteKing),
					pos.getBitmap(blackQueens) | pos.getBitmap(whiteQueens),
					pos.getBitmap(blackRooks) | pos.getBitmap(whiteRooks),
					pos.getBitmap(blackBishops) | pos.getBitmap(whiteBishops),
					pos.getBitmap(blackKnights) | pos.getBitmap(whiteKnights),
					pos.getBitmap(blackPawns) | pos.getBitmap(whitePawns),
					pos.getActualStateConst().getIrreversibleMoveCount(),
					pos.getActualStateConst().getCastleRights(),
					pos.hasEpSquare() ? pos.getEpSquare(): 0,
					pos.isWhiteTurn() );

				if(result != TB_RESULT_FAILED)
				{
					++_tbHits;

					Score value;
					unsigned wdl = TB_GET_WDL(result);
					assert(wdl<5);
					if( uciParameters::Syzygy50MoveRule )
					{
						switch(wdl)
						{
						case 0:
							value = SCORE_MATED +100 +ply;
							TTtype = typeScoreLowerThanAlpha;
							break;
						case 1:
							TTtype = typeExact;
							value = -100;
							break;
						case 2:
							TTtype = typeExact;
							value = 0;
							break;
						case 3:
							TTtype = typeExact;
							value = 100;
							break;
						case 4:
							TTtype = typeScoreHigherThanBeta;
							value = SCORE_MATE -100 -ply;
							break;
						default:
							value = 0;
						}

					}
					else
					{
						switch(wdl)
						{
						case 0:
						case 1:
							TTtype = typeScoreLowerThanAlpha;
							value = SCORE_MATED +100 +ply;
							break;
						case 2:
							TTtype = typeExact;
							value = 0;
							break;
						case 3:
						case 4:
							TTtype = typeScoreHigherThanBeta;
							value = SCORE_MATE -100 -ply;
							break;
						default:
							value = 0;
						}
					}


					if(	TTtype == typeExact || (TTtype == typeScoreHigherThanBeta  && value >=beta) || (TTtype == typeScoreLowerThanAlpha && value <=alpha)	)
					{
						transpositionTable::getInstance().store(posKey,
							transpositionTable::scoreToTT(value, ply),
							TTtype,
							std::min(90, depth + 6 * ONE_PLY),
							ttMove.getPacked(),
							pos.eval<false>());

						return value;
					}
				}
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
		staticEval = pos.eval<false>();
		eval = staticEval;

#ifdef DEBUG_EVAL_SIMMETRY
		testSimmetry(pos);
#endif
	}
	else
	{
		staticEval = tte->getStaticValue();
		assert(staticEval < SCORE_INFINITE);
		assert(staticEval > -SCORE_INFINITE);

		eval = staticEval;


		if (ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > eval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < eval) )
				)
			{
				eval = ttValue;
			}
		}
	}
	
	_sd.story[ply].staticEval = staticEval;
	
	if( ply <2 || inCheck || ( ply >=2 && _sd.story[ply-2].inCheck ) || ( ply >=2 && _sd.story[ply].staticEval >= _sd.story[ply-2].staticEval ) )
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
			if (!_sd.story[ply].skipNullMove
				&&  depth < 4 * ONE_PLY
				&&  eval + razorMargin(depth,type==CUT_NODE) <= alpha
				&&  alpha >= -SCORE_INFINITE+razorMargin(depth,type==CUT_NODE)
				&&  ( !ttMove || type == ALL_NODE)
			)
			{
				Score ralpha = alpha - razorMargin(depth,type==CUT_NODE);
				assert(ralpha>=-SCORE_INFINITE);

				PVline childPV;
				Score v = qsearch<CUT_NODE>(ply,0, ralpha, ralpha+1, childPV);
				if (v <= ralpha)
				{
					return v;
				}
			}

			//---------------------------
			//	 STATIC NULL MOVE PRUNING
			//---------------------------
			//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
			//---------------------------

			const state& st = pos.getActualStateConst();

			if (!_sd.story[ply].skipNullMove
				&& depth < 8 * ONE_PLY
				&& eval - futility( depth, improving ) >= beta
				&& eval < SCORE_KNOWN_WIN
				&& ( ( pos.isBlackTurn() && st.getNonPawnValue()[2] >= Position::pieceValue[whiteKnights][0]) || ( pos.isWhiteTurn() && st.getNonPawnValue()[0] >= Position::pieceValue[whiteKnights][0])))
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
				&& !_sd.story[ply].skipNullMove
				&& ( ( pos.isBlackTurn() && st.getNonPawnValue()[2] >= Position::pieceValue[whiteKnights][0]) || ( pos.isWhiteTurn() && st.getNonPawnValue()[0] >= Position::pieceValue[whiteKnights][0]))
			){
				// Null move dynamic reduction based on depth
				int red = 3 * ONE_PLY + depth / 4;

				// Null move dynamic reduction based on value
				if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta)
				{
					red += ONE_PLY;
				}

				pos.doNullMove();
				_sd.story[ply+1].skipNullMove = true;

				//uint64_t nullKey = pos.getKey();
				Score nullVal;
				PVline childPV;
				if( depth-red < ONE_PLY )
				{
					nullVal = -qsearch<childNodesType>(ply+1, 0, -beta, -beta+1, childPV);
				}
				else
				{
					nullVal = -alphaBeta<childNodesType>(ply+1, depth - red, -beta, -beta+1, childPV);
				}

				pos.undoNullMove();
				_sd.story[ply+1].skipNullMove = false;

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
					_sd.story[ply].skipNullMove = true;
					assert(depth - red >= ONE_PLY);
					Score val;
						val = alphaBeta<childNodesType>(ply, depth - red, beta-1, beta, childPV);
					_sd.story[ply].skipNullMove = false;
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

			if( depth >= 5*ONE_PLY
				&&  !_sd.story[ply].skipNullMove
				// && abs(beta)<SCORE_KNOWN_WIN
				// && eval> beta-40000
				&& abs(beta) < SCORE_MATE_IN_MAX_PLY
			){
				Score s;
				Score rBeta = std::min(beta + 8000, SCORE_INFINITE);
				int rDepth = depth -ONE_PLY- 3*ONE_PLY;

				MovePicker mp(pos, _sd, ply, ttMove);
				mp.setupProbCutSearch( pos.getCapturedPiece() );

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
					pos.doMove(m);

					assert(rDepth>=ONE_PLY);
					s = -alphaBeta<childNodesType>(ply+1, rDepth, -rBeta ,-rBeta+1, childPV);

					pos.undoMove();

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

		bool skipBackup = _sd.story[ply].skipNullMove;
		_sd.story[ply].skipNullMove = true;

		PVline childPV;
		const Search::nodeType iidType = type;
		assert(d >= ONE_PLY);
		alphaBeta<iidType>(ply, d, alpha, beta, childPV);

		_sd.story[ply].skipNullMove = skipBackup;

		tte = transpositionTable::getInstance().probe(posKey);
		ttMove = tte->getPackedMove();
	}




	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Move::NOMOVE);

	Move m;
	MovePicker mp(pos, _sd, ply, ttMove);
	unsigned int moveNumber = 0;
	unsigned int quietMoveCount = 0;
	Move quietMoveList[64];
	unsigned int captureMoveCount = 0;
	Move captureMoveList[32];

	bool singularExtensionNode =
		type != Search::nodeType::ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove
		&& !excludedMove // Recursive singular Search is not allowed
		&& tte != nullptr
		&& tte->isTypeGoodForBetaCutoff()
		&&  tte->getDepth() >= depth - 3 * ONE_PLY;

	while (bestScore <beta  && ( m = mp.getNextMove() ) )
	{

		assert( m );
		if(m == excludedMove)
		{
			continue;
		}

		// Search only the moves in the Search list
		if( type == Search::nodeType::ROOT_NODE && ( std::count(_rootMovesAlreadySearched.begin(), _rootMovesAlreadySearched.end(), m ) || !std::count(_rootMovesToBeSearched.begin(), _rootMovesToBeSearched.end(), m) ) )
		{
			continue;
		}
		++moveNumber;


		bool captureOrPromotion = pos.isCaptureMoveOrPromotion(m);


		bool moveGivesCheck = pos.moveGivesCheck(m);
		bool isDangerous = moveGivesCheck || m.isCastleMove() || pos.isPassedPawnMove(m);
		bool FutilityMoveCountFlag = depth < 16*ONE_PLY && moveNumber >= FutilityMoveCounts[improving][depth >> ONE_PLY_SHIFT];

		int ext = 0;
		if(PVnode && isDangerous )
		{
			ext = ONE_PLY;
		}
		else if( moveGivesCheck && pos.seeSign(m) >= 0 && !FutilityMoveCountFlag)
		{
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if( singularExtensionNode
			&& !ext
			&&  m == ttMove
			//&&  abs(ttValue) < SCORE_KNOWN_WIN
			&& abs(beta) < SCORE_MATE_IN_MAX_PLY
		)
		{

			PVline childPv;

			Score rBeta = ttValue - int(depth*20);
			rBeta = std::max( rBeta, -SCORE_MATE + 1 );

			_sd.story[ply].excludeMove = m;
			Score temp = alphaBeta<ALL_NODE>(ply, depth/2, rBeta-1, rBeta, childPv);
			_sd.story[ply].excludeMove = Move::NOMOVE;

			if(temp < rBeta)
			{
				ext = ONE_PLY;
		    }
		}

		int newDepth = depth-ONE_PLY+ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if( type != Search::nodeType::ROOT_NODE
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


			if(newDepth < 7*ONE_PLY)
			{
				Score localEval = eval + futilityMargin[newDepth >> ONE_PLY_SHIFT];
				if(localEval<beta)
				{
					if constexpr ( !PVnode )
					{
						bestScore = std::max(bestScore, localEval);
					}
					assert((newDepth>>ONE_PLY_SHIFT)<7);
					continue;
				}
			}

			if(newDepth < 4 * ONE_PLY && pos.seeSign(m) < 0)
			{
				continue;
			}



		}

		if(type == ROOT_NODE)
		{
			long long int elapsed = _st.getElapsedTime();
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!_stop
				)
			{
				_UOI->printCurrMoveNumber(moveNumber, m, getVisitedNodes(), elapsed);
			}
		}


		pos.doMove(m);
		Score val;
		PVline childPV;

		if constexpr (PVnode)
		{
			if(moveNumber==1)
			{
				if(newDepth < ONE_PLY)
				{
					val = -qsearch<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
				}
				else
				{
					val = -alphaBeta<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
				}
			}
			else
			{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch = true;
				if( depth >= 3*ONE_PLY
					&& (!captureOrPromotion || FutilityMoveCountFlag )
					&& !isDangerous
					&& m != ttMove
					&& !mp.isKillerMove(m)
				)
				{
					assert(moveNumber!=0);

					int reduction = PVreduction[improving][ std::min(depth, int(LmrLimit*ONE_PLY-1)) ][ std::min(moveNumber, (unsigned int)63) ];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction != 0)
					{
						val = -alphaBeta<Search::nodeType::CUT_NODE>(ply+1, d, -alpha-1, -alpha, childPV);
						if(val<=alpha)
						{
							doFullDepthSearch = false;
						}
					}
				}


				if(doFullDepthSearch)
				{

					if(newDepth<ONE_PLY)
					{
						val = -qsearch<Search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<Search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}

					if( val > alpha && val < beta )
					{
						if( newDepth < ONE_PLY )
						{
							val = -qsearch<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
						}
						else
						{
							val = -alphaBeta<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
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
			if( depth >= 3*ONE_PLY
				&& (!captureOrPromotion || FutilityMoveCountFlag )
				&& !isDangerous
				&& m != ttMove
				&& !mp.isKillerMove(m)
			)
			{
				int reduction = nonPVreduction[improving][std::min(depth, int(LmrLimit*ONE_PLY-1))][std::min(moveNumber, (unsigned int)63)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction != 0)
				{
					val = -alphaBeta<childNodesType>(ply+1, d, -alpha-1, -alpha, childPV);
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
						val = -qsearch<childNodesType>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<childNodesType>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
				}
				else
				{
					if(newDepth < ONE_PLY)
					{
						val = -qsearch<Search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<Search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
				}
			}
		}

		pos.undoMove();

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
					if(type == Search::nodeType::ROOT_NODE && uciParameters::multiPVLines == 1 )
					{
						if(val < beta && depth > 1*ONE_PLY)
						{
							_UOI->printPV(val, _maxPlyReached, _st.getElapsedTime(), pvLine, getVisitedNodes());
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
			bestScore = std::min( (int)0, (int)(-5000 + pos.getPly()*250) );
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
							(short int)depth, bestMove.getPacked(), staticEval);
	}

	// save killer move & update history
	if (bestScore >= beta
		&& !inCheck)
	{
		if (!pos.isCaptureMoveOrPromotion(bestMove))
		{
			_sd.saveKillers(ply, bestMove);

			// update history
			int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
			Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY*ONE_PLY);

			_sd.history.update( pos.isWhiteTurn() ? white: black, bestMove, bonus);

			for (unsigned int i = 0; i < quietMoveCount; i++)
			{
				Move m = quietMoveList[i];
				_sd.history.update( pos.isWhiteTurn() ? white: black, m, -bonus);
			}
			
			_updateCounterMove( bestMove );
		}
		else
		{
			//if( pos.isCaptureMove( bestMove ) )
			{
				// update capture history
				int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
				Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY*ONE_PLY);

				_sd.captureHistory.update( pos.getPieceAt(bestMove.getFrom()), bestMove, pos.getPieceAt(bestMove.getTo()), bonus);

				for (unsigned int i = 0; i < captureMoveCount; i++)
				{
					Move m = captureMoveList[i];
					//if( pos.isCaptureMove( m ) )
					{
						_sd.captureHistory.update( pos.getPieceAt(m.getFrom()), m, pos.getPieceAt(m.getTo()), -bonus);
					}
				}
			}
		}
		
		

	}
	return bestScore;

}


template<Search::nodeType type> Score Search::qsearch(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{

	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);

	const bool PVnode = (type == Search::nodeType::PV_NODE);
	assert(PVnode || alpha+1==beta);

	bool inCheck = pos.isInCheck();
	_sd.story[ply].inCheck = inCheck;

	_maxPlyReached = std::max(ply, _maxPlyReached);
	++_visitedNodes;



	if(pos.isDraw(PVnode) || _stop)
	{

		if constexpr (PVnode)
		{
			pvLine.clear();
		}
		return getDrawValue();
	}
	//---------------------------------------
	//	MATE DISTANCE PRUNING
	//---------------------------------------
	//
	//alpha = std::max(matedIn(ply), alpha);
	//beta = std::min(mateIn(ply+1), beta);


	//----------------------------
	//	next node type
	//----------------------------
	const Search::nodeType childNodesType =
		type == Search::nodeType::ALL_NODE?
			Search::nodeType::CUT_NODE:
			type == Search::nodeType::CUT_NODE ? Search::nodeType::ALL_NODE:
				Search::nodeType::PV_NODE;


	ttEntry* const tte = transpositionTable::getInstance().probe(pos.getKey());
	Move ttMove( tte->getPackedMove() );

	MovePicker mp(pos, _sd, ply, ttMove);
	int TTdepth = mp.setupQuiescentSearch(inCheck, depth) * ONE_PLY;
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(),ply);
	if (tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false
	            : ttValue >= beta ? tte->isTypeGoodForBetaCutoff()
	                              : tte->isTypeGoodForAlphaCutoff()))
	{
		transpositionTable::getInstance().refresh(*tte);

		if constexpr (PVnode)
		{
			if(pos.isMoveLegal(ttMove))
			{
				pvLine.appendNewMove(ttMove);
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}

	// overwrite ttMove with move from move from PVlineToBeFollowed
	if constexpr ( PVnode )
	{
		manageLineToBefollowed(ply, ttMove);
	}

	ttType TTtype = typeScoreLowerThanAlpha;


	Score staticEval = tte->getType()!=typeVoid ? tte->getStaticValue() : pos.eval<false>();
#ifdef DEBUG_EVAL_SIMMETRY
	testSimmetry(pos);
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




		if (ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > staticEval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < staticEval) )
			)
			{
				bestScore = ttValue;
			}
		}

		if(bestScore > alpha)
		{
			assert(!inCheck);

			// TODO testare se la riga TTtype=typeExact; ha senso
			if constexpr (PVnode)
			{
				pvLine.clear();
			}

			if( bestScore >= beta)
			{
				if( !pos.isCaptureMoveOrPromotion(ttMove) )
				{
					_sd.saveKillers(ply, ttMove);
				}
				if(!_stop)
				{
					transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, ttMove.getPacked(), staticEval);
				}
				return bestScore;
			}
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
	Move bestMove = ttMove;

	PVline childPV;

	while ( ( m = mp.getNextMove() ) )
	{
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert( m );


		if(!inCheck)
		{
			// allow only queen promotion at deeper search
			if( (TTdepth <- 1*ONE_PLY) && ( m.isPromotionMove() ) && (m.getPromotionType() != Move::promQueen))
			{
				continue;
			}

			// at very deep search allow only recapture
			if(depth < -7 * ONE_PLY && pos.getActualStateConst().getCurrentMove().getTo() != m.getTo())
			{
				continue;
			}

			//----------------------------
			//	futility pruning (delta pruning)
			//----------------------------
			if constexpr ( !PVnode )
			{
				if(
					m != ttMove
					//&& m.bit.flags != Move::fpromotion
					//&& !pos.moveGivesCheck(m)
					)
				{
					bool moveGiveCheck = pos.moveGivesCheck(m);
					if(
							!moveGiveCheck
							&& !pos.isPassedPawnMove(m)
							&& abs(staticEval)<SCORE_KNOWN_WIN
					)
					{
						Score futilityValue = futilityBase
								+ Position::pieceValue[pos.getPieceAt(m.getTo())][1]
								+ ( m.isEnPassantMove() ? Position::pieceValue[whitePawns][1] : 0);

						if( m.isPromotionMove() )
						{
							futilityValue += Position::pieceValue[m.getPromotionType() + whiteQueens][1] - Position::pieceValue[whitePawns][1];
						}


						if (futilityValue < beta)
						{
							bestScore = std::max(bestScore, futilityValue);
							continue;
						}
						if (futilityBase < beta && pos.seeSign(m) <= 0)
						{
							bestScore = std::max(bestScore, futilityBase);
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
							pos.seeSign(m) < 0)
					{
						continue;
					}
				}
			}

		}
		pos.doMove(m);
		Score val = -qsearch<childNodesType>(ply+1, depth-ONE_PLY, -beta, -alpha, childPV);


		pos.undoMove();


		if( val > bestScore)
		{
			bestScore = val;
			if( bestScore > alpha )
			{
				bestMove = m;
				TTtype = typeExact;
				alpha = bestScore;

				if(PVnode && !_stop)
				{
					pvLine.appendNewPvLine( bestMove, childPV ); 

				}
				if( bestScore >= beta)
				{
					if( !pos.isCaptureMoveOrPromotion(bestMove) && !inCheck )
					{
						_sd.saveKillers(ply, bestMove);
					}
					if(!_stop)
					{
						transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, bestMove.getPacked(), staticEval);
					}
					return bestScore;
				}
			}
		}
	}

	if(bestScore == -SCORE_INFINITE)
	{
		assert(inCheck);
		if constexpr (PVnode)
		{
			pvLine.clear();
		}
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);




	if( !_stop )
	{
		transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), TTtype, (short int)TTdepth, bestMove.getPacked(), staticEval);
	}
	return bestScore;

}

void Search::setUOI( std::unique_ptr<UciOutput> UOI )
{
	// manage output syncronization
	sync_cout;
	_UOI = std::move(UOI);
	std::cout<<sync_noNewLineEndl;
}

inline Score Search::getDrawValue() const
{
	int contemptSign = ( pos.getNextTurn() == _initialTurn) ? 1 : -1;
	return contemptSign * std::min( (int)0, (int)(-5000 + pos.getPly()*250) );
}

void Search::initSearchParameters(void)
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
			double    PVRed = -1.5 + 0.33 * log(double(d)) * log(double(mc));
			double nonPVRed = -1.2 + 0.37 * log(double(d)) * log(double(mc));

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

inline void SearchData::clearKillers(unsigned int ply)
{
	Move * const tempKillers =  story[ply].killers;

	tempKillers[1] = 0;
	tempKillers[0] = 0;
}
inline void SearchData::cleanData(void)
{
	std::memset(story, 0, sizeof(story));
}

inline void SearchData::saveKillers(unsigned int ply, Move& m)
{
	Move * const tempKillers = story[ply].killers;
	if(tempKillers[0] != m)
	{
		tempKillers[1] = tempKillers[0];
		tempKillers[0] = m;
	}

}
