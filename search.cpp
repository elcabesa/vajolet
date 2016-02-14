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
#include <atomic>
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "history.h"
#include "book.h"
#include "thread.h"
#include "command.h"

#ifdef DEBUG_EVAL_SIMMETRY
	Position ppp;
#endif


search defaultSearch;
std::vector<rootMove> search::rootMoves;
std::atomic<unsigned long long> search::visitedNodes;


Score search::futility[5] = {0,6000,20000,30000,40000};
Score search::futilityMargin[7] = {0,10000,20000,30000,40000,50000,60000};
unsigned int search::FutilityMoveCounts[11] = {5,10,17,26,37,50,66,85,105,130,151};
Score search::PVreduction[32*ONE_PLY][64];
Score search::nonPVreduction[32*ONE_PLY][64];
unsigned int search::threads = 1;
unsigned int search::multiPVLines = 1;
unsigned int search::limitStrength = 0;
unsigned int search::eloStrenght = 3000;
bool search::useOwnBook = true;
bool search::bestMoveBook = false;
bool search::showCurrentLine = false;

void search::reloadPv( unsigned int i )
{
	if( rootMoves[i].PV.size() > 0)
	{
		auto PV = rootMoves[i].PV;
		unsigned int n = 0;

		for(const Move& m : PV)
		{
			if (!pos.isMoveLegal(m))
			{
				break;
			}

			const ttEntry* const tte = TT.probe(pos.getKey());

			if (!tte || tte->getPackedMove() != m.packed)
			{
				// Don't overwrite correct entries
				TT.store(pos.getKey(), SCORE_NONE, typeExact, -1000, m.packed, pos.eval<false>());
			}

			pos.doMove(m);
			n++;
		}

		for(unsigned int i = 0; i< n; i++)
		{
			pos.undoMove();
		}
	}
}

startThinkResult search::startThinking(unsigned int depth, Score alpha, Score beta)
{
	//------------------------------------
	//init the new search
	//------------------------------------
	Score res = 0;
	resetStartTime();
	//bool firstRun = true;


	TT.newSearch();
	history.clear();
	counterMoves.clear();
	cleanData();
	visitedNodes = 0;

	std::vector<search> helperSearch(threads-1);

	rootMoves.clear();


	//--------------------------------
	//	generate the list of root moves to be searched
	//--------------------------------
	if(limits.searchMoves.size() == 0)	// all the legal moves
	{
		Move m(Movegen::NOMOVE);
		Movegen mg(pos);
		while ((m = mg.getNextMove())!= Movegen::NOMOVE)
		{
			rootMove rm;
			rm.init(m);
			rootMoves.push_back(rm);
		}
	}
	else
	{	//only selected moves
		for_each(limits.searchMoves.begin(), limits.searchMoves.end(),
			[&](Move &m)
			{
			rootMove rm;
			rm.init(m);
			rootMoves.push_back(rm);
			}
		);
	}



	//-----------------------------
	// manage multi PV moves && limit strenght
	//-----------------------------
	unsigned int linesToBeSearched = search::multiPVLines;
	if(limitStrength)
	{
		int lines = (int)((-8.0/2000.0)*( eloStrenght - 1000 ) + 10.0 );
		unsigned int s = std::max(lines, 4);
		linesToBeSearched = std::max(linesToBeSearched, s);
	}
	linesToBeSearched = std::min(linesToBeSearched, (unsigned int)rootMoves.size());

	//----------------------------------
	// we can start the real search
	//----------------------------------



	std::list<Move> newPV;
	//unsigned int depth = 1;

	//Score alpha = -SCORE_INFINITE, beta = SCORE_INFINITE;
	Score delta = 800;
	Move oldBestMove(Movegen::NOMOVE);

	do
	{
		//----------------------------
		// iterative loop
		//----------------------------

		for (indexPV = 0; indexPV < linesToBeSearched; indexPV++)
		{

			//----------------------------------
			// prepare alpha & beta
			//----------------------------------
			if (/*!firstRun && */depth >= 5)
			{
				delta = 800;
				alpha = (Score) std::max((signed long long int)(rootMoves[indexPV].score) - delta,(signed long long int)-SCORE_INFINITE);
				beta  = (Score) std::min((signed long long int)(rootMoves[indexPV].score) + delta,(signed long long int) SCORE_INFINITE);
			}

			for (unsigned int x = indexPV; x < rootMoves.size() ; x++)
			{
				rootMoves[x].score = -SCORE_INFINITE;
			}

			//----------------------------------
			// reload the last PV in the transposition table
			//----------------------------------
			for(unsigned int i = 0; i<=indexPV; i++)
			{
				reloadPv(i);
			}
			//firstRun = false;


			unsigned int reduction = 0;

			do
			{
				//----------------------------
				// search at depth d with aspiration window
				//----------------------------

				maxPlyReached = 0;

				//----------------------------
				// multithread : lazy smp threads
				//----------------------------

				std::vector<std::list<Move>> pvl2(threads-1);
				std::vector<std::thread> helperThread;

				// launch helper threads
				for(unsigned int i = 0; i < (threads - 1); i++)
				{
					helperSearch[i].stop = false;
					helperSearch[i].pos = pos;
					helperThread.push_back( std::thread(alphaBeta<search::nodeType::HELPER_ROOT_NODE>, &helperSearch[i], 0, (depth-reduction+((i+1)%2))*ONE_PLY, alpha, beta, std::ref(pvl2[i])));
				}

				// main thread
				res = alphaBeta<search::nodeType::ROOT_NODE>(0, (depth-reduction) * ONE_PLY, alpha, beta, newPV);

				// stop helper threads
				for(unsigned int i = 0; i< (threads - 1); i++)
				{
					helperSearch[i].stop = true;
				}
				for(auto &t : helperThread)
				{
					t.join();
				}

				// don't stop befor having finished at least one iteration
				if(depth != 1 && stop)
				{
					break;
				}


				long long int elapsedTime  = getElapsedTime();

				assert(newPV.size()==0 || res >alpha);
				if( newPV.size() !=0 && res > alpha)
				{
					auto it = std::find(rootMoves.begin()+indexPV, rootMoves.end(), newPV.front() );

					assert( it->firstMove == newPV.front());

					//if(it->firstMove == newPV.front())
					//{
					it->PV = newPV;
					it->score = res;
					it->maxPlyReached = maxPlyReached;
					it->depth = depth;
					it->nodes = visitedNodes;
					it->time = elapsedTime;

					std::iter_swap( it, rootMoves.begin()+indexPV);

					//}

				}






				if (res <= alpha)
				{
					newPV.clear();
					newPV.push_back( rootMoves[indexPV].PV.front() );

					printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, visitedNodes);

					alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

					reduction = 0;

				}
				else if (res >= beta)
				{
					/*if(oldBestMove.packed && oldBestMove != newPV.front())
					{
						my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
					}*/

					printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, visitedNodes);

					beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
					if(depth > 1)
					{
						reduction = 1;
					}
				}
				else
				{
					break;
				}


				delta += delta / 2;



			}while(1);

			if(!stop)
			{

				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + indexPV + 1);
				printPVs( indexPV + 1 );
			}
		}


		//-----------------------
		//	single good move at root
		//-----------------------
		/*if (depth >= 12
			&& !stop
			&&  linesToBeSearched == 1
			&&  res > SCORE_MATED_IN_MAX_PLY)
		{

			Score rBeta = res - 20000;
			sd[0].excludeMove = newPV.front();
			sd[0].skipNullMove = true;
			std::list<Move> locChildPV;
			Score temp = alphaBeta<search::nodeType::ALL_NODE>(0, (depth-3) * ONE_PLY, rBeta - 1, rBeta, locChildPV);
			sd[0].skipNullMove = false;
			sd[0].excludeMove = Movegen::NOMOVE;

			if(temp < rBeta)
			{
				my_thread::timeMan.singularRootMoveCount++;
			}
		}*/

		//------------------------------------------------
		// check wheter or not the new best move has changed
		//------------------------------------------------
		/*if(oldBestMove.packed && oldBestMove != newPV.front()) // in the case increase the allocated time
		{
			my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
		}*/
		oldBestMove = newPV.front();


		/*my_thread::timeMan.depth = depth;
		unsigned long time = my_thread::timeMan.minSearchTime;
		my_thread::timeMan.allocatedTime = std::max(time, (unsigned long int)(my_thread::timeMan.allocatedTime * 0.90) ); // otherwise decrease the allocated time*/
		my_thread::timeMan.idLoopIterationFinished = true;




		depth += 1;

	}
	while( depth <= (limits.depth ? limits.depth : 100) && !stop);

	unsigned int bestMoveLine = 0;
	if( limitStrength )
	{
		double lambda = (eloStrenght - 1000.0) * (0.8/2000) + 0.2;
		std::mt19937_64 rnd;
		std::exponential_distribution<> uint_dist(lambda);

		long long int now = getTime();
		rnd.seed(now);
		double dres = uint_dist(rnd);

		bestMoveLine = std::min((unsigned int)dres, linesToBeSearched-1);
	}


	startThinkResult ret;
	ret.PV = rootMoves[bestMoveLine].PV;
	ret.depth = depth-1;
	ret.alpha = alpha;
	ret.beta = beta;


	return ret;

}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, std::list<Move>& pvLine)
{

	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);


	Position::state& st = pos.getActualState();
	visitedNodes++;

	const bool PVnode = ( type == search::nodeType::PV_NODE || type == search::nodeType::ROOT_NODE  || type == search::nodeType::HELPER_ROOT_NODE);
	const bool inCheck = pos.isInCheck();
	Move threatMove(Movegen::NOMOVE);


	//--------------------------------------
	// show current line if needed
	//--------------------------------------
	if( showLine && depth <= ONE_PLY)
	{
		showLine = false;
		showCurrLine(pos,ply);
	}

	//--------------------------------------
	// choose node type
	//--------------------------------------

	const search::nodeType childNodesType =
			type == search::nodeType::ALL_NODE ?
					search::nodeType::CUT_NODE :
					type == search::nodeType::CUT_NODE ? search::nodeType::ALL_NODE : search::nodeType::PV_NODE;

	if(type != search::nodeType::ROOT_NODE  && type !=search::nodeType::HELPER_ROOT_NODE)
	{
		if(pos.isDraw(PVnode) || stop)
		{
			if(PVnode)
			{
				pvLine.clear();
			}
			return std::max( (int)0, (int)(-5000 + pos.getPly()*7) );
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
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;

	ttMove = (tte != nullptr) ? tte->getPackedMove() : 0;
	Score ttValue = tte != nullptr ? transpositionTable::scoreFromTT(tte->getValue(), ply) : SCORE_NONE;

	if (	type != search::nodeType::ROOT_NODE
			&& type != search::nodeType::HELPER_ROOT_NODE
			&& tte != nullptr
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    && (	PVnode ?  false
		            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		                              : (tte->getType() ==  typeScoreLowerThanAlpha || tte->getType() == typeExact)))
	{
		TT.refresh(tte);

		//save killers
		if (ttValue >= beta
			&& ttMove.packed
			&& !pos.isCaptureMoveOrPromotion(ttMove)
			&& !inCheck)
		{
			saveKillers(ply ,ttMove);
		}

		if(PVnode)
		{
			if(ttMove.packed && pos.isMoveLegal(ttMove))
			{
				pvLine.clear();
				pvLine.push_back(ttMove);
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}


	//---------------------------------
	// calc the eval & static eval
	//---------------------------------

	Score staticEval;
	Score eval;
	if(inCheck || tte == nullptr)
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
					((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > eval) )
					|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < eval) )
				)
			{
				eval = ttValue;
			}
		}

	}

	//-----------------------------
	// reduction && pruning
	//-----------------------------
	if(!PVnode && !inCheck && abs(beta) < SCORE_MATE_IN_MAX_PLY)
	{

		//------------------------
		// razoring
		//------------------------
		// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
		//------------------------
		if (depth < 4 * ONE_PLY
			&&  eval + razorMargin(depth) <= alpha
			&&  alpha >= -SCORE_INFINITE+razorMargin(depth)
			//&&  abs(alpha) < SCORE_MATE_IN_MAX_PLY // implicito nell riga precedente
			&&  ((!ttMove.packed ) || type == ALL_NODE)
			&& !((pos.getNextTurn() && (pos.getBitmap(Position::blackPawns) & RANKMASK[A2])) || (!pos.getNextTurn() && (pos.getBitmap(Position::whitePawns) & RANKMASK[A7]) ) )
		)
		{
			Score ralpha = alpha - razorMargin(depth);
			assert(ralpha>=-SCORE_INFINITE);

			std::list<Move> childPV;
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
			&& depth < 4 * ONE_PLY
			&& eval > -SCORE_INFINITE + futility[ depth>>ONE_PLY_SHIFT ]
			&& eval - futility[depth>>ONE_PLY_SHIFT] >= beta
			&& abs(eval) < SCORE_KNOWN_WIN
			&& ((pos.getNextTurn() && st.nonPawnMaterial[2] >= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getNextTurn() && st.nonPawnMaterial[0] >= Position::pieceValue[Position::whiteKnights][0])))
		{
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

			U64 nullKey = pos.getKey();
			Score nullVal;
			std::list<Move> childPV;
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
				//return nullVal; TODO da testare se da vantaggi o no, semplifica il codice

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
			else
			{
				const ttEntry * const tteNull = TT.probe(nullKey);
				threatMove = tteNull != nullptr ? tteNull->getPackedMove() : 0;
			}

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
		){
			Score s;
			Score rBeta = beta + 8000;
			int rDepth = depth -ONE_PLY- 3*ONE_PLY;

			Movegen mg(pos, *this, ply, rDepth, ttMove);
			mg.setupProbCutSearch(pos.getCapturedPiece());

			Move m;
			std::list<Move> childPV;
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

		std::list<Move> childPV;
		const search::nodeType iidType = type;
		assert(d >= ONE_PLY);
		alphaBeta<iidType>(ply, d, alpha, beta, childPV);

		sd[ply].skipNullMove = skipBackup;

		tte = TT.probe(posKey);
		ttMove = tte != nullptr ? tte->getPackedMove() : 0;
	}




	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Movegen::NOMOVE);

	Move m;
	Movegen mg(pos, *this, ply, depth, ttMove);
	unsigned int moveNumber = 0;
	unsigned int quietMoveCount = 0;
	Move quietMoveList[64];

	bool singularExtensionNode =
		type != search::nodeType::ROOT_NODE
		&& type != search::nodeType::HELPER_ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed != 0
		&& !excludedMove.packed // Recursive singular search is not allowed
		&& tte != nullptr
		&& (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		&&  tte->getDepth() >= depth - 3 * ONE_PLY;

	while (bestScore <beta  && ( m = mg.getNextMove() ) != Movegen::NOMOVE)
	{

		assert(m.packed);
		if(m == excludedMove)
		{
			continue;
		}

		// search only the moves in the search list
		if((type == search::nodeType::ROOT_NODE || type == search::nodeType::HELPER_ROOT_NODE) && !std::count(rootMoves.begin() + indexPV, rootMoves.end(), m))
		{
			continue;
		}
		moveNumber++;


		bool captureOrPromotion = pos.isCaptureMoveOrPromotion(m);

		if(!captureOrPromotion && quietMoveCount < 64)
		{
			quietMoveList[quietMoveCount++] = m;
		}

		bool moveGivesCheck = pos.moveGivesCheck(m);
		bool isDangerous = moveGivesCheck || pos.isCastleMove(m) || pos.isPassedPawnMove(m);

		int ext = 0;
		if(PVnode && isDangerous)
		{
			ext = ONE_PLY;
		}
		else if( moveGivesCheck && pos.seeSign(m) >= 0)
		{
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if( singularExtensionNode
			&& !ext
			&&  m == ttMove
			&&  abs(ttValue) < SCORE_KNOWN_WIN
//			&& abs(beta) < SCORE_MATE_IN_MAX_PLY
		)
		{

			std::list<Move> childPv;

			Score rBeta = ttValue - int(depth*20);

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
		if( !PVnode
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore > SCORE_MATED_IN_MAX_PLY
		){
			assert(moveNumber > 1);

			if(newDepth < 11*ONE_PLY
				&& moveNumber >= FutilityMoveCounts[newDepth >> ONE_PLY_SHIFT]
				&& (!threatMove.packed)
				)
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
				printCurrMoveNumber(moveNumber, m, visitedNodes, elapsed);
			}
		}


		pos.doMove(m);
		Score val;
		std::list<Move> childPV;

		if(PVnode)
		{
			if(moveNumber==1)
			{
				if(newDepth < ONE_PLY)
				{
					val = -qsearch<search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
				}
				else
				{
					val = -alphaBeta<search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
				}
			}
			else
			{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch = true;
				if( depth >= 3*ONE_PLY
					&& !captureOrPromotion
					&& !isDangerous
					&& m != ttMove
					&& !mg.isKillerMove(m)
				)
				{
					assert(moveNumber!=0);

					int reduction = PVreduction[ std::min(depth, 32*ONE_PLY-1) ][ std::min(moveNumber, (unsigned int)63) ];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction != 0)
					{
						val = -alphaBeta<search::nodeType::CUT_NODE>(ply+1, d, -alpha-1, -alpha, childPV);
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
						val = -qsearch<search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}

					if( val > alpha && val < beta )
					{
						if( newDepth < ONE_PLY )
						{
							val = -qsearch<search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
						}
						else
						{
							val = -alphaBeta<search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
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
				&& !captureOrPromotion
				&& !isDangerous
				&& m != ttMove
				&& !mg.isKillerMove(m)
			)
			{
				int reduction = nonPVreduction[std::min(depth, 32*ONE_PLY-1)][std::min(moveNumber, (unsigned int)63)];
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
						val = -qsearch<search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
					else
					{
						val = -alphaBeta<search::nodeType::CUT_NODE>(ply+1, newDepth, -alpha-1, -alpha, childPV);
					}
				}
			}
		}

		pos.undoMove();

		if(val > bestScore)
		{
			bestScore = val;

			if(val > alpha)
			{
				bestMove = m;
				if(PVnode)
				{
					alpha = val;
				}
				if(type == search::nodeType::ROOT_NODE || type ==search::nodeType::HELPER_ROOT_NODE || (PVnode && !stop))
				{
					if(PVnode)
					{
						pvLine.clear();
						pvLine.push_back(bestMove);
						pvLine.splice(pvLine.end(),childPV);
					}
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
			bestScore = std::max( (int)0, (int)(-5000 + pos.getPly()*7) );
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
		TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove.packed, staticEval);
	}

	// save killer move & update history
	if (bestScore >= beta
		&& !pos.isCaptureMoveOrPromotion(bestMove)
		&& !inCheck)
	{
		saveKillers(ply,bestMove);

		// update history
		Score bonus = Score(depth * depth)/(ONE_PLY*ONE_PLY);
		history.update(pos.getPieceAt((tSquare)bestMove.bit.from), (tSquare) bestMove.bit.to, bonus);
		if(quietMoveCount > 1)
		{
			for (unsigned int i = 0; i < quietMoveCount - 1; i++)
			{
				Move m = quietMoveList[i];
				history.update(pos.getPieceAt((tSquare)m.bit.from), (tSquare) m.bit.to, -bonus);
			}
		}

		Move previousMove = pos.getActualState().currentMove;
		if(previousMove.packed)
		{
			counterMoves.update(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, bestMove);
		}

	}
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(unsigned int ply, int depth, Score alpha, Score beta, std::list<Move>& pvLine)
{

	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);

	const bool PVnode = (type == search::nodeType::PV_NODE);
	assert(PVnode || alpha+1==beta);

	bool inCheck = pos.isInCheck();

	maxPlyReached = std::max(ply, maxPlyReached);
	visitedNodes++;


	if(pos.isDraw(PVnode) || stop)
	{
		if(PVnode)
		{
			pvLine.clear();
		}
		return std::max((int)0,(int)(-5000 + pos.getPly()*7));
	}
/*	//---------------------------------------
	//	MATE DISTANCE PRUNING
	//---------------------------------------

	alpha = std::max(matedIn(ply), alpha);
	beta = std::min(mateIn(ply+1), beta);

	if(ply>20)
	{sync_cout<<ply<<"    "<< alpha<<":"<< beta<<sync_endl;}
	if (alpha >= beta)
	{
		return alpha;
	}*/

	//----------------------------
	//	next node type
	//----------------------------
	const search::nodeType childNodesType =
		type == search::nodeType::ALL_NODE?
			search::nodeType::CUT_NODE:
			type == search::nodeType::CUT_NODE ? search::nodeType::ALL_NODE:
				search::nodeType::PV_NODE;


	ttEntry* const tte = TT.probe(pos.getKey());
	Move ttMove;
	ttMove = tte ? tte->getPackedMove() : Movegen::NOMOVE;

	Movegen mg(pos, *this, ply, depth, ttMove);
	int TTdepth = mg.setupQuiescentSearch(inCheck, depth);
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (tte
		&& tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false/*tte->getType() == typeExact*/
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha || tte->getType() == typeExact)))
	{
		TT.refresh(tte);

		if(PVnode)
		{
			if(ttMove.packed && pos.isMoveLegal(ttMove))
			{
				pvLine.clear();
				pvLine.push_back(ttMove);
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}

	ttType TTtype = typeScoreLowerThanAlpha;


	Score staticEval = tte ? tte->getStaticValue() : pos.eval<false>();
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
		futilityBase = bestScore + 5000;



		/*if (ttValue != SCORE_NONE)
		{
			if (
					((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > staticEval) )
					|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < staticEval) )
				)
			{
				bestScore = ttValue;
			}
		}*/
	}
	else
	{
		bestScore = -SCORE_INFINITE;
		futilityBase = -SCORE_INFINITE;

	}


	if(bestScore > alpha)
	{
		assert(!inCheck);

		// TODO testare se la riga TTtype=typeExact; ha senso
		if(PVnode)
		{
			pvLine.clear();
		}

		if( bestScore >= beta)
		{
			if( !pos.isCaptureMoveOrPromotion(ttMove) )
			{
				saveKillers(ply,ttMove);
			}
			if(!stop)
			{
				TT.store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, ttMove.packed, staticEval);
			}
			return bestScore;
		}
		alpha = bestScore;
		TTtype = typeExact;
		


	}


	//----------------------------
	//	try the captures
	//----------------------------
	Move m;
	Move bestMove = ttMove;

	std::list<Move> childPV;

	Position::state &st = pos.getActualState();
	while (bestScore < beta  &&  (m = mg.getNextMove()) != Movegen::NOMOVE)
	{
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert(m.packed);


		// allow only queen promotion at deeper search
		if( !inCheck && (TTdepth <- 1*ONE_PLY) && (m.bit.flags == Move::fpromotion) && (m.bit.promotion != Move::promQueen))
		{
			continue;
		}

		// at very deep search allow only recapture
		if(depth < -7 * ONE_PLY && !inCheck)
		{
			if(st.currentMove.bit.to != m.bit.to)
			{
				continue;
			}
		}

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		if(	!PVnode
			&& !inCheck
			&& m != ttMove
			&& m.bit.flags != Move::fpromotion
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
						+ (m.bit.flags == Move::fenpassant ? Position::pieceValue[Position::whitePawns][1] : 0);

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

		pos.doMove(m);
		Score val = -qsearch<childNodesType>(ply+1, depth-ONE_PLY, -beta, -alpha, childPV);


		pos.undoMove();


		if( val > bestScore)
		{
			bestScore = val;
			if( val > alpha )
			{
				bestMove = m;
				TTtype = typeExact;
				alpha = val;

				if(PVnode && !stop)
				{
					pvLine.clear();
					pvLine.push_back(bestMove);
					pvLine.splice( pvLine.end(), childPV );

				}
				if( bestScore >= beta)
				{
					if( !pos.isCaptureMoveOrPromotion(bestMove) && !inCheck )
					{
						saveKillers(ply,bestMove);
					}
					if(!stop)
					{
						TT.store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, bestMove.packed, staticEval);
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
			pvLine.clear();
		}
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);




	if( !stop )
	{
		TT.store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), TTtype, (short int)TTdepth, bestMove.packed, staticEval);
	}
	return bestScore;

}


