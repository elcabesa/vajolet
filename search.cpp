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

#include <random>
#include <ctime>

#include <vector>
#include <list>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "history.h"
#include "book.h"
#include "thread.h"
#include "command.h"
#include "syzygy/tbprobe.h"
#include "bitops.h"



#ifdef DEBUG_EVAL_SIMMETRY
	Position ppp;
#endif

Search defaultSearch;
std::vector<Move> Search::rootMoves;
int Search::globalReduction =0;


Score Search::futility[8] = {0,6000,12000,18000,24000,30000,36000,42000};
Score Search::futilityMargin[7] = {0,10000,20000,30000,40000,50000,60000};
unsigned int Search::FutilityMoveCounts[16] = {2,3,4,7,11,15,20,26,32,39,46,55,64,73,83,94};
Score Search::PVreduction[LmrLimit*ONE_PLY][64];
Score Search::nonPVreduction[LmrLimit*ONE_PLY][64];
unsigned int Search::threads = 1;
unsigned int Search::multiPVLines = 1;
bool Search::useOwnBook = true;
bool Search::bestMoveBook = false;
bool Search::showCurrentLine = false;
std::string Search::SyzygyPath ="<empty>";
unsigned int Search::SyzygyProbeDepth = 1;
bool Search::Syzygy50MoveRule= true;

static std::vector<Search> helperSearch;

unsigned long long Search::getVisitedNodes() const
{
	unsigned long long n = visitedNodes;
	for (auto& hs : helperSearch)
		n += hs.visitedNodes;
	return n;
}

unsigned long long Search::getTbHits() const
{
	unsigned long long n = tbHits;
	for (auto& hs : helperSearch)
		n += hs.tbHits;
	return n;
}

void Search::cleanMemoryBeforeStartingNewSearch(void)
{
	history.clear();
	captureHistory.clear();
	counterMoves.clear();
	cleanData();
	visitedNodes = 0;
	tbHits = 0;
}

void Search::filterRootMovesByTablebase( std::vector<Move>& rm )
{
	unsigned results[TB_MAX_MOVES];

	unsigned int piecesCnt = bitCnt (pos.getBitmap(Position::whitePieces) | pos.getBitmap(Position::blackPieces));

	if ( piecesCnt <= TB_LARGEST )
	{
		unsigned result = tb_probe_root(pos.getBitmap(Position::whitePieces),
			pos.getBitmap(Position::blackPieces),
			pos.getBitmap(Position::blackKing) | pos.getBitmap(Position::whiteKing),
			pos.getBitmap(Position::blackQueens) | pos.getBitmap(Position::whiteQueens),
			pos.getBitmap(Position::blackRooks) | pos.getBitmap(Position::whiteRooks),
			pos.getBitmap(Position::blackBishops) | pos.getBitmap(Position::whiteBishops),
			pos.getBitmap(Position::blackKnights) | pos.getBitmap(Position::whiteKnights),
			pos.getBitmap(Position::blackPawns) | pos.getBitmap(Position::whitePawns),
			pos.getActualState().fiftyMoveCnt,
			pos.getActualState().castleRights,
			pos.getActualState().epSquare == squareNone? 0 : pos.getActualState().epSquare ,
			pos.getActualState().nextMove== Position::whiteTurn,
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
				Move m( Movegen::NOMOVE );
				m.bit.from = TB_GET_FROM(r);
				m.bit.to = TB_GET_TO(r);
				if (ep)
				{
					m.bit.flags = Move::fenpassant;
				}
				switch (TB_GET_PROMOTES(r))
				{
				case TB_PROMOTES_QUEEN:
					m.bit.flags = Move::fpromotion;
					m.bit.promotion = Move::promQueen;
					break;
				case TB_PROMOTES_ROOK:
					m.bit.flags = Move::fpromotion;
					m.bit.promotion = Move::promRook;
					break;
				case TB_PROMOTES_BISHOP:
					m.bit.flags = Move::fpromotion;
					m.bit.promotion = Move::promBishop;
					break;
				case TB_PROMOTES_KNIGHT:
					m.bit.flags = Move::fpromotion;
					m.bit.promotion = Move::promKnight;
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
		Move m(Movegen::NOMOVE);
		for(  Movegen mg(pos); ( m = mg.getNextMove() ) != Movegen::NOMOVE; )
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
	PVline PV;
	Score res =qsearch<Search::nodeType::PV_NODE>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, PV);
	
	_UOI->printScore( res/100 );
	
	startThinkResult ret;
	
	ret.PV = PV;
	ret.depth = 0;
	ret.alpha = -SCORE_INFINITE;
	ret.beta = SCORE_INFINITE;
	
	return ret;
}

startThinkResult Search::startThinking(int depth, Score alpha, Score beta, PVline pv)
{
	//------------------------------------
	//init the new search
	//------------------------------------
	
	
	
	//clean transposition table
	transpositionTable::getInstance().newSearch();
	
	// setup main thread
	cleanMemoryBeforeStartingNewSearch();

	// setup other threads
	helperSearch.clear();
	helperSearch.resize(threads-1);

	for (auto& hs : helperSearch)
	{
		// mute helper thread
		hs.setUOI(UciOutput::create( UciOutput::mute ) );
		
		// setup helper thread
		hs.cleanMemoryBeforeStartingNewSearch();
	}

	//--------------------------------
	// generate the list of root moves to be searched
	//--------------------------------
	generateRootMovesList( rootMoves, limits.searchMoves );
	
	
	// manage multi PV moves
	unsigned int linesToBeSearched = std::min(Search::multiPVLines, (unsigned int)rootMoves.size());

	//--------------------------------
	//	tablebase probing, filtering rootmoves to be searched
	//--------------------------------
	if(limits.searchMoves.size() == 0 && Search::multiPVLines==1)
	{
		filterRootMovesByTablebase( rootMoves );
	}
	
	
	//----------------------------------
	// we can start the real search
	//----------------------------------
	
	// manage depth 0 search ( return qsearch )
	if(limits.depth == 0)
	{
		return manageQsearch();
	}

	
	//----------------------------------
	// iterative deepening loop
	//----------------------------------
	Score delta = 800;
	
	// ramdomly initialize the bestmove
	rootMove bestMove = rootMoves[0];
	
	do
	{
		_UOI->printDepth(depth);
		//----------------------------
		// iterative loop
		//----------------------------

		std::vector<rootMove> previousIterationResults = rootMovesSearched;
		rootMovesSearched.clear();
		
		for (indexPV = 0; indexPV < linesToBeSearched; indexPV++)
		{

			//----------------------------------
			// prepare alpha & beta
			//----------------------------------
			if (depth >= 5)
			{
				delta = 800;
				alpha = (Score) std::max((signed long long int)(previousIterationResults[indexPV].score) - delta,(signed long long int) SCORE_MATED);
				beta  = (Score) std::min((signed long long int)(previousIterationResults[indexPV].score) + delta,(signed long long int) SCORE_MATE);
			}

			Search::globalReduction = 0;

			if( previousIterationResults.size()> indexPV )
			{
				ExpectedValue = previousIterationResults[indexPV].score;
				PV = previousIterationResults[indexPV].PV;
				followPV = true;
			}
			else
			{
				ExpectedValue = 0;
				PV.clear();
				followPV = false;
			}
			
			
			do
			{

				//----------------------------
				// search at depth d with aspiration window
				//----------------------------

				maxPlyReached = 0;
				validIteration = false;
				followPV = true;

				
				
				

				//----------------------------
				// multithread : lazy smp threads
				//----------------------------

				std::vector<PVline> pvl2(threads-1);
				std::vector<std::thread> helperThread;

				// launch helper threads
				for(unsigned int i = 0; i < (threads - 1); i++)
				{
					helperSearch[i].stop = false;
					helperSearch[i].pos = pos;
					helperSearch[i].PV = PV;
					helperSearch[i].followPV = true;
 					helperThread.emplace_back( std::thread(&Search::alphaBeta<Search::nodeType::ROOT_NODE>, &helperSearch[i], 0, (depth-globalReduction+((i+1)%2))*ONE_PLY, alpha, beta, std::ref(pvl2[i])));
				}

				PVline newPV;
				newPV.clear();
				
				// main thread
				Score res = alphaBeta<Search::nodeType::ROOT_NODE>(0, (depth-globalReduction) * ONE_PLY, alpha, beta, newPV);

				// stop helper threads
				for(unsigned int i = 0; i< (threads - 1); i++)
				{
					helperSearch[i].stop = true;
				}
				for(auto &t : helperThread)
				{
					t.join();
				}

				if(validIteration ||!stop)
				{
					long long int elapsedTime  = getElapsedTime();

					if (res <= alpha)
					{

						_UOI->printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, getVisitedNodes());

						alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

						globalReduction = 0;
						my_thread::timeMan.idLoopAlpha = true;
						my_thread::	timeMan.idLoopBeta = false;
						
						//PV = newPV;
						followPV = true;

					}
					else if (res >= beta)
					{

						_UOI->printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, getVisitedNodes());

						beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
						if(depth > 1)
						{
							globalReduction = 1;
						}
						my_thread::timeMan.idLoopAlpha = false;
						my_thread::timeMan.idLoopBeta = true;
						
						PV = newPV;
						followPV = true;
												
						bestMove = rootMove( newPV.front(), newPV, res, maxPlyReached, depth, getVisitedNodes(), elapsedTime );
					}
					else
					{
						bestMove = rootMove( newPV.front(), newPV, res, maxPlyReached, depth, getVisitedNodes(), elapsedTime );
						
						rootMovesSearched.push_back(bestMove);

						break;
					}


					delta += delta / 2;
				}



			}while(!stop);

			if((!stop || validIteration) && (linesToBeSearched>1 || depth == 1) )
			{

				// Sort the PV lines searched so far and update the GUI				
				std::stable_sort(rootMovesSearched.begin(), rootMovesSearched.end());
				bestMove = rootMovesSearched[0];
				
				_UOI->printPVs( rootMovesSearched );
			}
		}



		my_thread::timeMan.idLoopIterationFinished = true;
		my_thread::timeMan.idLoopAlpha = false;
		my_thread::	timeMan.idLoopBeta = false;

	}
	while( ++depth <= (limits.depth != -1 ? limits.depth : 100) && !stop);


	startThinkResult ret;

	ret.PV = bestMove.PV;
	ret.depth = depth-1;
	ret.alpha = alpha;
	ret.beta = beta;
	ret.Res = bestMove.score;

	return ret;

}

template<Search::nodeType type> Score Search::alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{

	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);




	const Position::state& st = pos.getActualState();
	visitedNodes++;
	clearKillers(ply+1);

	const bool PVnode = ( type == Search::nodeType::PV_NODE || type == Search::nodeType::ROOT_NODE );
	const bool inCheck = pos.isInCheck();
	//Move threatMove(Movegen::NOMOVE);


	//--------------------------------------
	// show current line if needed
	//--------------------------------------
	if( showLine && depth <= ONE_PLY)
	{
		showLine = false;
		_UOI->showCurrLine(pos,ply);
	}

	//--------------------------------------
	// choose node type
	//--------------------------------------

	const Search::nodeType childNodesType =
			type == Search::nodeType::ALL_NODE ?
					Search::nodeType::CUT_NODE :
					type == Search::nodeType::CUT_NODE ? Search::nodeType::ALL_NODE : Search::nodeType::PV_NODE;

	if(type != Search::nodeType::ROOT_NODE )
	{
		if(pos.isDraw(PVnode) || stop)
		{
			if(PVnode)
			{
				pvLine.reset();
			}
			return std::min( (int)0, (int)(-5000 + pos.getPly()*250) );
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

	const Move& excludedMove = sd[ply].excludeMove;

	U64 posKey = excludedMove.packed ? pos.getExclusionKey() : pos.getKey();

	//--------------------------------------
	// test the transposition table
	//--------------------------------------
	ttEntry* tte = transpositionTable::getInstance().probe(posKey);
	Move ttMove = tte->getPackedMove();
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
			&& ttMove.packed
			&& !pos.isCaptureMoveOrPromotion(ttMove)
			&& !inCheck)
		{
			saveKillers(ply ,ttMove);
			
			Move previousMove = pos.getActualState().currentMove;
			if(previousMove.packed)
			{
				counterMoves.update(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, ttMove);
			}
		}
		

		if(PVnode)
		{
			if(pos.isMoveLegal(ttMove))
			{
				pvLine.appendNewMove( ttMove );
			}
			else
			{
				pvLine.reset();
			}
		}
		return ttValue;
	}

	if (PVnode && followPV)
	{
		if(ply >= PV.size())
		{
			followPV = false;
		}
		else
		{
			PVline::iterator it = PV.begin();
			std::advance(it, ply);
			ttMove = *it;
			if(ply >= PV.size() - 1)
			{
				followPV = false;
			}
		}
	}

	//Tablebase probe
	if (!PVnode && TB_LARGEST)
	{
		unsigned int piecesCnt = bitCnt (pos.getBitmap(Position::whitePieces) | pos.getBitmap(Position::blackPieces));

		if (    piecesCnt <= TB_LARGEST
			&& (piecesCnt <  TB_LARGEST || depth >= (int)(SyzygyProbeDepth*ONE_PLY))
			&&  pos.getActualState().fiftyMoveCnt == 0)
		{
			unsigned result = tb_probe_wdl(pos.getBitmap(Position::whitePieces),
				pos.getBitmap(Position::blackPieces),
				pos.getBitmap(Position::blackKing) | pos.getBitmap(Position::whiteKing),
				pos.getBitmap(Position::blackQueens) | pos.getBitmap(Position::whiteQueens),
				pos.getBitmap(Position::blackRooks) | pos.getBitmap(Position::whiteRooks),
				pos.getBitmap(Position::blackBishops) | pos.getBitmap(Position::whiteBishops),
				pos.getBitmap(Position::blackKnights) | pos.getBitmap(Position::whiteKnights),
				pos.getBitmap(Position::blackPawns) | pos.getBitmap(Position::whitePawns),
				pos.getActualState().fiftyMoveCnt,
				pos.getActualState().castleRights,
				pos.getActualState().epSquare == squareNone? 0 : pos.getActualState().epSquare ,
				pos.getActualState().nextMove== Position::whiteTurn);

			if(result != TB_RESULT_FAILED)
			{
				tbHits++;

				Score value;
				unsigned wdl = TB_GET_WDL(result);
				assert(wdl<5);
				if(Syzygy50MoveRule)
				{
					switch(wdl)
					{
					case 0:
						value = SCORE_MATED +100 +ply;
						break;
					case 1:
						value = -100;
						break;
					case 2:
						value = 0;
						break;
					case 3:
						value = 100;
						break;
					case 4:
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
						value = SCORE_MATED +100 +ply;
						break;
					case 2:
						value = 0;
						break;
					case 3:
					case 4:
						value = SCORE_MATE -100 -ply;
						break;
					default:
						value = 0;
					}
				}

				
				transpositionTable::getInstance().store(posKey,
						transpositionTable::scoreToTT(value, ply),
						typeExact,
						std::min(90, depth + 6 * ONE_PLY),
						ttMove.packed,
						pos.eval<false>());

				return value;
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
		ppp.setupFromFen(pos.getSymmetricFen());
		Score test=ppp.eval<false>();
		if(test!=eval){
			sync_cout<<1<<" "<<test<<" "<<eval<<sync_endl;
			pos.display();
			while(1);
		}
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

	//-----------------------------
	// reduction && pruning
	//-----------------------------
	if(!PVnode && !inCheck )
	{

		//------------------------
		// razoring
		//------------------------
		// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
		//------------------------
		if (!sd[ply].skipNullMove
			&&  depth < 4 * ONE_PLY
			&&  eval + razorMargin(depth,type==CUT_NODE) <= alpha
			&&  alpha >= -SCORE_INFINITE+razorMargin(depth,type==CUT_NODE)
			&&  ((!ttMove.packed ) || type == ALL_NODE)
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
		if (!sd[ply].skipNullMove
			&& depth < 8 * ONE_PLY
			//&& eval > -SCORE_INFINITE + futility[ depth>>ONE_PLY_SHIFT ]
			&& eval - futility[depth>>ONE_PLY_SHIFT] >= beta
			&& eval < SCORE_KNOWN_WIN
			&& ((pos.getNextTurn() && st.nonPawnMaterial[2] >= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getNextTurn() && st.nonPawnMaterial[0] >= Position::pieceValue[Position::whiteKnights][0])))
		{
			assert((depth>>ONE_PLY_SHIFT)<8);
			assert((eval -futility[depth>>ONE_PLY_SHIFT] >-SCORE_INFINITE));
			return eval - futility[depth>>ONE_PLY_SHIFT];
		}


		//---------------------------
		//	 NULL MOVE PRUNING
		//---------------------------
		// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
		// this search let us know about threat move by the opponent.
		//---------------------------

		if( depth >= ONE_PLY
			&& eval >= beta
			&& !sd[ply].skipNullMove
			&& ((pos.getNextTurn() && st.nonPawnMaterial[2] >= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getNextTurn() && st.nonPawnMaterial[0] >= Position::pieceValue[Position::whiteKnights][0]))
		){
			// Null move dynamic reduction based on depth
			int red = 3 * ONE_PLY + depth / 4;

			// Null move dynamic reduction based on value
			if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta)
			{
				red += ONE_PLY;
			}

			pos.doNullMove();
			sd[ply+1].skipNullMove = true;

			//U64 nullKey = pos.getKey();
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
			sd[ply+1].skipNullMove = false;

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
				sd[ply].skipNullMove = true;
				assert(depth - red >= ONE_PLY);
				Score val;
				/*if(depth-red < ONE_PLY)
				{
					val = qsearch<childNodesType>(ply, depth-red, beta-1, beta,childPV);
				}
				else
				{*/
					val = alphaBeta<childNodesType>(ply, depth - red, beta-1, beta, childPV);
				/*}*/
				sd[ply].skipNullMove = false;
				if (val >= beta)
				{
					return nullVal;
				}

			}
			/*else
			{
				const ttEntry * const tteNull = transpositionTable::getInstance().probe(nullKey);
				threatMove = tteNull != nullptr ? tteNull->getPackedMove() : Movegen::NOMOVE;
			}*/

		}

		//------------------------
		//	PROB CUT
		//------------------------
		//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
		//------------------------

		if( depth >= 5*ONE_PLY
			&&  !sd[ply].skipNullMove
			// && abs(beta)<SCORE_KNOWN_WIN
			// && eval> beta-40000
			&& abs(beta) < SCORE_MATE_IN_MAX_PLY
		){
			Score s;
			Score rBeta = std::min(beta + 8000, SCORE_INFINITE);
			int rDepth = depth -ONE_PLY- 3*ONE_PLY;

			Movegen mg(pos, *this, ply, ttMove);
			mg.setupProbCutSearch(pos.getCapturedPiece());

			Move m;
			PVline childPV;
			while((m = mg.getNextMove()) != Movegen::NOMOVE)
			{
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


	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? 5 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed == 0
		&& (PVnode || staticEval + 10000 >= beta))
	{
		int d = depth - 2 * ONE_PLY - (PVnode ? 0 : depth / 4);

		bool skipBackup = sd[ply].skipNullMove;
		sd[ply].skipNullMove = true;

		PVline childPV;
		const Search::nodeType iidType = type;
		assert(d >= ONE_PLY);
		alphaBeta<iidType>(ply, d, alpha, beta, childPV);

		sd[ply].skipNullMove = skipBackup;

		tte = transpositionTable::getInstance().probe(posKey);
		ttMove = tte->getPackedMove();
	}




	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Movegen::NOMOVE);

	Move m;
	Movegen mg(pos, *this, ply, ttMove);
	unsigned int moveNumber = 0;
	unsigned int quietMoveCount = 0;
	Move quietMoveList[64];
	unsigned int captureMoveCount = 0;
	Move captureMoveList[32];

	bool singularExtensionNode =
		type != Search::nodeType::ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed != 0
		&& !excludedMove.packed // Recursive singular Search is not allowed
		&& tte != nullptr
		&& tte->isTypeGoodForBetaCutoff()
		&&  tte->getDepth() >= depth - 3 * ONE_PLY;

	while (bestScore <beta  && ( m = mg.getNextMove() ) != Movegen::NOMOVE)
	{

		assert(m.packed);
		if(m == excludedMove)
		{
			continue;
		}

		// Search only the moves in the Search list
		if( type == Search::nodeType::ROOT_NODE && std::count(rootMovesSearched.begin(), rootMovesSearched.end(), m) && !std::count(rootMoves.begin(), rootMoves.end(), m) )
		{
			continue;
		}
		moveNumber++;


		bool captureOrPromotion = pos.isCaptureMoveOrPromotion(m);


		bool moveGivesCheck = pos.moveGivesCheck(m);
		bool isDangerous = moveGivesCheck || m.isCastleMove() || pos.isPassedPawnMove(m);
		bool FutilityMoveCountFlag = depth < 16*ONE_PLY && moveNumber >= FutilityMoveCounts[depth >> ONE_PLY_SHIFT];

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

			sd[ply].excludeMove = m;
			bool backup = sd[ply].skipNullMove;
			sd[ply].skipNullMove = true;
			Score temp = alphaBeta<ALL_NODE>(ply, depth/2, rBeta-1, rBeta, childPv);
			sd[ply].skipNullMove = backup;
			sd[ply].excludeMove = Movegen::NOMOVE;

			if(temp < rBeta)
			{
				ext = ONE_PLY;
		    }
		}

		int newDepth = depth-ONE_PLY+ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if( /*!PVnode*/type != Search::nodeType::ROOT_NODE
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
					bestScore = std::max(bestScore, localEval);
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
			long long int elapsed = getElapsedTime();
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!stop
				)
			{
				_UOI->printCurrMoveNumber(moveNumber, m, getVisitedNodes(), elapsed);
			}
		}


		pos.doMove(m);
		Score val;
		PVline childPV;

		if(PVnode)
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
					&& !mg.isKillerMove(m)
				)
				{
					assert(moveNumber!=0);

					int reduction = PVreduction[ std::min(depth, int(LmrLimit*ONE_PLY-1)) ][ std::min(moveNumber, (unsigned int)63) ];
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
				&& !mg.isKillerMove(m)
			)
			{
				int reduction = nonPVreduction[std::min(depth, int(LmrLimit*ONE_PLY-1))][std::min(moveNumber, (unsigned int)63)];
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

		if(!stop && val > bestScore)
		{
			bestScore = val;

			if(bestScore > alpha)
			{
				bestMove = m;
				if(PVnode)
				{
					alpha = bestScore;
					pvLine.appendNewPvLine( bestMove, childPV);
					if(type == Search::nodeType::ROOT_NODE && Search::multiPVLines==1)
					{
						if(val < beta && depth > 1*ONE_PLY)
						{
							_UOI->printPV(val, depth/ONE_PLY+globalReduction, maxPlyReached, -SCORE_INFINITE, SCORE_INFINITE, getElapsedTime(), indexPV, pvLine, getVisitedNodes());
						}
						if(val > ExpectedValue - 800)
						{
							validIteration = true;
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
		if( excludedMove.packed)
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

	if(!stop)
	{
		transpositionTable::getInstance().store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove.packed, staticEval);
	}

	// save killer move & update history
	if (bestScore >= beta
		&& !inCheck)
	{
		if (!pos.isCaptureMoveOrPromotion(bestMove))
		{
			saveKillers(ply,bestMove);

			// update history
			int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
			Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY*ONE_PLY);

			history.update(pos.getNextTurn() == Position::whiteTurn ? white: black, bestMove, bonus);

			for (unsigned int i = 0; i < quietMoveCount; i++)
			{
				Move m = quietMoveList[i];
				history.update(pos.getNextTurn() == Position::whiteTurn ? white: black, m, -bonus);
			}
			
			Move previousMove = pos.getActualState().currentMove;
			if(previousMove.packed)
			{
				counterMoves.update(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, bestMove);
			}
		}
		else
		{
			// update capture history
			int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
			Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY*ONE_PLY);

			captureHistory.update( pos.getPieceAt((tSquare)bestMove.bit.from), bestMove, pos.getPieceAt((tSquare)bestMove.bit.to), bonus);

			for (unsigned int i = 0; i < captureMoveCount; i++)
			{
				Move m = captureMoveList[i];
				captureHistory.update( pos.getPieceAt((tSquare)m.bit.from), m, pos.getPieceAt((tSquare)m.bit.to), -bonus);
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

	maxPlyReached = std::max(ply, maxPlyReached);
	visitedNodes++;



	if(pos.isDraw(PVnode) || stop)
	{

		if(PVnode)
		{
			pvLine.reset();
		}
		return std::min((int)0,(int)(-5000 + pos.getPly()*250));
	}
/*	//---------------------------------------
	//	MATE DISTANCE PRUNING
	//---------------------------------------

	alpha = std::max(matedIn(ply), alpha);
	beta = std::min(mateIn(ply+1), beta);
	*/

	//----------------------------
	//	next node type
	//----------------------------
	const Search::nodeType childNodesType =
		type == Search::nodeType::ALL_NODE?
			Search::nodeType::CUT_NODE:
			type == Search::nodeType::CUT_NODE ? Search::nodeType::ALL_NODE:
				Search::nodeType::PV_NODE;


	ttEntry* const tte = transpositionTable::getInstance().probe(pos.getKey());
	Move ttMove = tte->getPackedMove();

	Movegen mg(pos, *this, ply, ttMove);
	int TTdepth = mg.setupQuiescentSearch(inCheck, depth);
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(),ply);
	if (tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false
	            : ttValue >= beta ? tte->isTypeGoodForBetaCutoff()
	                              : tte->isTypeGoodForAlphaCutoff()))
	{
		transpositionTable::getInstance().refresh(*tte);

		if(PVnode)
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

	if (PVnode && followPV)
	{
		if(ply >= PV.size())
		{
			followPV = false;
		}
		else
		{
			PVline::iterator it = PV.begin();
			std::advance(it, ply);

			ttMove = *it;

			if(ply >= PV.size() - 1)
			{
				followPV = false;
			}
		}
	}

	ttType TTtype = typeScoreLowerThanAlpha;


	Score staticEval = tte->getType()!=typeVoid ? tte->getStaticValue() : pos.eval<false>();
#ifdef DEBUG_EVAL_SIMMETRY
	ppp.setupFromFen(pos.getSymmetricFen());
	Score test = ppp.eval<false>();
	if(test != staticEval)
	{
		sync_cout << 3 << " " << test << " " << staticEval << " " << pos.eval<false>() << sync_endl;
		pos.display();
		ppp.display();
		while(1);
	}
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
			if(PVnode)
			{
				pvLine.reset();
			}

			if( bestScore >= beta)
			{
				if( !pos.isCaptureMoveOrPromotion(ttMove) )
				{
					saveKillers(ply,ttMove);
				}
				if(!stop)
				{
					transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, ttMove.packed, staticEval);
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

	const Position::state& st = pos.getActualState();
	while (/*bestScore < beta  &&  */(m = mg.getNextMove()) != Movegen::NOMOVE)
	{
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert(m.packed);


		if(!inCheck)
		{
			// allow only queen promotion at deeper search
			if( (TTdepth <- 1*ONE_PLY) && ( m.isPromotionMove() ) && (m.bit.promotion != Move::promQueen))
			{
				continue;
			}

			// at very deep search allow only recapture
			if(depth < -7 * ONE_PLY && st.currentMove.bit.to != m.bit.to)
			{
				continue;
			}

			//----------------------------
			//	futility pruning (delta pruning)
			//----------------------------
			if(	!PVnode
				&& m != ttMove
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
							+ Position::pieceValue[pos.getPieceAt((tSquare)m.bit.to)][1]
							+ ( m.isEnPassantMove() ? Position::pieceValue[Position::whitePawns][1] : 0);

					if( m.bit.flags == Move::fpromotion )
					{
						futilityValue += Position::pieceValue[m.bit.promotion + Position::whiteQueens][1] - Position::pieceValue[Position::whitePawns][1];
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

				if(PVnode && !stop)
				{
					pvLine.appendNewPvLine( bestMove, childPV ); 

				}
				if( bestScore >= beta)
				{
					if( !pos.isCaptureMoveOrPromotion(bestMove) && !inCheck )
					{
						saveKillers(ply,bestMove);
					}
					if(!stop)
					{
						transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, bestMove.packed, staticEval);
					}
					return bestScore;
				}
			}
		}
	}

	if(bestScore == -SCORE_INFINITE)
	{
		assert(inCheck);
		if(PVnode)
		{
			pvLine.reset();
		}
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);




	if( !stop )
	{
		transpositionTable::getInstance().store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), TTtype, (short int)TTdepth, bestMove.packed, staticEval);
	}
	return bestScore;

}


