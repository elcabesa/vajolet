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
#include <chrono>
#include <vector>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "statistics.h"
#include "history.h"
#include "book.h"
#include "thread.h"

inline signed int razorMargin(unsigned int depth){
	return 20000+depth*78;
}

void search::printAllPV(Position & p, unsigned int count){


	for (unsigned int i=0; i<count; i++){
		Score res=rootMoves[i].previousScore;
		printPV(res,rootMoves[i].depth,rootMoves[i].selDepth,-SCORE_INFINITE,SCORE_INFINITE,p,rootMoves[i].time,i,rootMoves[i].PV,rootMoves[i].nodes);
	}
}

void search::printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, Position & p, unsigned long time,unsigned int count,std::vector<Move>& PV,unsigned long long nodes){
	sync_cout<<"info multipv "<< count+1<< " depth "<<(depth)<<" seldepth "<<seldepth <<" score ";
	if(abs(res) >SCORE_MATE_IN_MAX_PLY){
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else{
		std::cout<< "cp "<<(int)((float)res/100.0);
	}
	std::cout<<(res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");


	std::cout<<" nodes "<<nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout<<" nps "<<(unsigned int)((double)nodes*1000/(time))<<" time "<<(time);
#endif
	std::cout<<" pv ";
	for (auto it= PV.begin(); it != PV.end(); ++it){
		std::cout<<p.displayUci(*it)<<" ";
	}
	std::cout<<sync_endl;
}


Score search::futility[5]={0,6000,20000,30000,40000};
Score search::futilityMargin[7]={0,10000,20000,30000,40000,50000,60000};
Score search::FutilityMoveCounts[11]={5,10,17,26,37,50,66,85,105,130,151};
Score search::PVreduction[32*ONE_PLY][64];
Score search::nonPVreduction[32*ONE_PLY][64];
unsigned int search::multiPVLines=1;
bool search::useOwnBook=true;
bool search::bestMoveBook=false;

void search::startThinking(Position & p,searchLimits & l){
	signals.stop=false;
	TT.newSearch();
	History::instance().clear();



	limits=l;
	rootMoves.clear();
	//--------------------------------
	//	generate the list of root moves to be searched
	//--------------------------------
	if(limits.searchMoves.size()==0){
		Move m;
		m=0;
		Movegen mg(p,m);
		while ((m=mg.getNextMove()).packed){
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=m;
			rootMoves.push_back(rm);
		}
	}
	else{
		for(std::list<Move>::iterator it = limits.searchMoves.begin(); it != limits.searchMoves.end(); ++it){
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=*it;
			rootMoves.push_back(rm);
		}

	}

	unsigned int PVSize = std::min(search::multiPVLines, (unsigned int)rootMoves.size());
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Move m,oldBestMove;
	m=0;
	oldBestMove=0;
	Movegen mg(p,m);

	Move lastLegalMove;
	unsigned int legalMoves=0;
	while((m=mg.getNextMove()).packed){
		legalMoves++;
		lastLegalMove=m;
	}
	if(legalMoves==0){
		while((limits.infinite && !signals.stop) || limits.ponder){}
		sync_cout<<"bestmove 0000"<<sync_endl;
		return;
	}else if(legalMoves==1){
		while((limits.infinite && !signals.stop) || limits.ponder){}
		sync_cout<<"bestmove "<<p.displayUci(lastLegalMove)<<sync_endl;
		return;
	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------

	PolyglotBook pol;
	//std::cout<<"polyglot testing"<<std::endl;
	if(useOwnBook && !limits.infinite ){
		Move bookM=pol.probe(p,"book.bin",bestMoveBook);
		if(bookM.packed){
			while((limits.infinite && !signals.stop) || limits.ponder){}
			sync_cout<<"bestmove "<<p.displayUci(bookM)<<sync_endl;
			return;
		}
	}



	unsigned int selDepthBase=p.getActualState().ply;
	visitedNodes=0;
	startTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
	std::vector<Move> newPV;
	unsigned int depth=1;

	Score alpha=-SCORE_INFINITE,beta =SCORE_INFINITE;
	Score delta=1600;
	Score res=0;
	selDepth=selDepthBase;

	do{

		for (rootMove& rm : rootMoves){
			rm.previousScore = rm.score;
			rm.score=-SCORE_INFINITE;
		}

		for (PVIdx = 0; PVIdx < PVSize; PVIdx++)
		{

			//sync_cout<<"PVIdx="<<PVIdx<<sync_endl;


			if (depth >= 5)
			{
				delta = 800;
				alpha = std::max((signed long long int)(rootMoves[PVIdx].previousScore) - delta,(signed long long int)-SCORE_INFINITE);
				beta  = std::min((signed long long int)(rootMoves[PVIdx].previousScore) + delta,(signed long long int) SCORE_INFINITE);
			}
			//sync_cout<<"alpha="<<alpha<<sync_endl;
			//sync_cout<<"beta="<<beta<<sync_endl;

			// reload the last PV in the transposition table
			for(unsigned int i =0; i<=PVIdx; i++){
				int n=0;
				if(/*nodeType==typeExact && */rootMoves[i].PV.size()>0){
					for (auto it= rootMoves[i].PV.begin(); it != rootMoves[i].PV.end() && Movegen::isMoveLegal(p,*it); ++it){
							/*if(!Movegen::isMoveLegal(p,*it)){
								p.display();
								sync_cout<<"move:"<<p.displayUci(*it)<<sync_endl;
								sync_cout<<"packed:"<<(*it).packed<<sync_endl;
								break;
							}*/

							TT.store(p.getActualState().key, transpositionTable::scoreToTT((n%2)?-rootMoves[i].previousScore:rootMoves[i].previousScore, n),typeExact,depth-n*ONE_PLY, (*it).packed, p.eval(pawnHashTable));


							//sync_cout<<"insert in TT "<<p.displayUci(*it)<<sync_endl;
							p.doMove(*it);
							n++;

					}
					for (n--;n>=0;n--){
						//sync_cout<<"undo move "<<p.displayUci(rootMoves[i].PV[n])<<sync_endl;
						p.undoMove(rootMoves[i].PV[n]);
					}
				}
			}


			do{

				//sync_cout<<"SEARCH"<<sync_endl;
				selDepth=selDepthBase;
				newPV.clear();
				p.cleanStateInfo();
				res=alphaBeta<search::nodeType::ROOT_NODE>(0,p,depth*ONE_PLY,alpha,beta,newPV);

				/*sync_cout<<"FINISHED SEARCH"<<sync_endl;
				sync_cout<<"res="<<res<<sync_endl;
				sync_cout<<p.displayUci(newPV[0])<<sync_endl;
				sync_cout<<"PVsize "<<newPV.size()<<sync_endl;*/

				if(depth!=1 && signals.stop){
					//sync_cout<<"iterative deepening Stop"<<sync_endl;
					break;
				}
				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if(newPV.size()!=0 && res > alpha /*&& res < beta*/){
					std::vector<rootMove>::iterator it=std::find(rootMoves.begin(),rootMoves.end(),newPV[0]);
					if(it->firstMove==newPV[0]){
						it->PV=newPV;
						it->score=res;
						it->previousScore=res;
						it->selDepth=selDepth-selDepthBase;
						it->depth=depth;
						it->nodes=visitedNodes;
						it->time= now-startTime;
					}
					std::stable_sort(rootMoves.begin() + PVIdx, rootMoves.end());
					//sync_cout<<"stableSort OK "<<sync_endl;

				}

				if (res <= alpha)
				{
					//sync_cout<<"res<=alpha "<<sync_endl;

					//my_thread::timeMan.idLoopRequestToExtend=true;
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV,visitedNodes);
					alpha = std::max((signed long long int)(res) - delta,(signed long long int)-SCORE_INFINITE);

					TT.store(p.getActualState().key, transpositionTable::scoreToTT(rootMoves[PVIdx].previousScore, 0),typeExact,depth*ONE_PLY, (rootMoves[PVIdx].PV[0]).packed, p.eval(pawnHashTable));
					//sync_cout<<"new alpha "<<alpha<<sync_endl;
				}
				else if (res >= beta){
					if(oldBestMove.packed && oldBestMove!=newPV[0]){
						my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
						//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
					}
					//sync_cout<<"res>=beta "<<sync_endl;
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV,visitedNodes);
					beta = std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
					//sync_cout<<"new beta "<<beta<<sync_endl;
				}else{
					break;
				}
				delta += delta / 2;


			}while(1);
			//sync_cout<<"aspiration window ok "<<sync_endl;
			if(!signals.stop){

				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + PVIdx + 1);
				//sync_cout<<"stable sort ok "<<sync_endl;
/*				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (PVIdx + 1 == PVSize
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
					|| now - startTime > 3000
#endif
				)*/{
					//sync_cout<<"print pv "<<sync_endl;
					printAllPV(p, PVSize);

				}
			}
		}
		//-----------------------
		//	single good move at root
		//-----------------------
		if (depth >= 12
			&& !signals.stop
			&&  PVSize == 1
			&&  res > SCORE_MATED_IN_MAX_PLY)
		{

			Score rBeta = res - 20000;
			p.getActualState().excludedMove=newPV[0];
			p.getActualState().skipNullMove=true;
			std::vector<Move> locChildPV;
			Score temp = alphaBeta<search::nodeType::ALL_NODE>(0,p,(depth-3)*ONE_PLY,rBeta-1,rBeta,locChildPV);
			p.getActualState().skipNullMove=false;
			p.getActualState().excludedMove.packed=0;

			if(temp < rBeta){
				my_thread::timeMan.singularRootMoveCount++;
			}
		}

		if(oldBestMove.packed && oldBestMove!=newPV[0]){
			my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
			//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
		}
		oldBestMove=newPV[0];

		my_thread::timeMan.depth=depth;
		unsigned long time=my_thread::timeMan.minSearchTime;
		my_thread::timeMan.allocatedTime=std::max(time,(unsigned long int)(my_thread::timeMan.allocatedTime*0.87));
		//sync_cout<<"nuovo tempo allocato="<<my_thread::timeMan.allocatedTime<<sync_endl;

		my_thread::timeMan.idLoopIterationFinished=true;


		depth+=1;

	}while(depth<=(limits.depth? limits.depth:100) && !signals.stop);

	//sync_cout<<"print final bestMove "<<sync_endl;

	while(limits.ponder){

	}
	sync_cout<<"bestmove "<<p.displayUci(rootMoves[0].PV[0]);
	if(rootMoves[0].PV.size()>1)
	{
		std::cout<<" ponder "<<p.displayUci(rootMoves[0].PV[1]);
	}
	std::cout<<sync_endl;
#ifdef PRINT_STATISTICS
	Statistics::instance().printNodeTypeStat();
#endif



}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);
	assert(PV.size()==0);
	visitedNodes++;
	const bool PVnode=(type==search::nodeType::PV_NODE || type==search::nodeType::ROOT_NODE);
	const bool inCheck = pos.getActualState().checkers;
	Move threatMove;
	threatMove=0;


	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;

	if(type !=search::nodeType::ROOT_NODE){
		if(pos.isDraw() || signals.stop){
			//if(signals.stop){sync_cout<<"alpha beta initial Stop"<<sync_endl;}
			return 0;
		}

		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta){
#ifdef PRINT_STATISTICS
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
			return alpha;
		}

	}

	Move excludedMove=pos.getActualState().excludedMove;

	U64 posKey=excludedMove.packed?pos.getExclusionKey() :pos.getKey();
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte!=nullptr ? tte->getPackedMove() : 0;
	Score ttValue = tte!=nullptr ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (type!=search::nodeType::ROOT_NODE &&
			tte!=nullptr
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    //&& (	PVnode ?  tte->getType() == typeExact
		    // TODO vedere se nei PV node in cui ho un beta cutoff o un alpha cutoff ritornare il valore del TT
		    && (	PVnode ?  false
		            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		                              : (tte->getType() ==  typeScoreLowerThanAlpha || tte->getType() == typeExact)))
	{

		TT.refresh(tte);

		if (ttValue >= beta &&
			ttMove.packed &&
			!pos.isCaptureMoveOrPromotion(ttMove) &&
			!inCheck)
		{
			if(pos.getActualState().killers[0] != ttMove)
			{
				pos.getActualState().killers[1] = pos.getActualState().killers[0];
				pos.getActualState().killers[0] = ttMove;
			}
		}


		if(ttMove.packed && Movegen::isMoveLegal(pos,ttMove)){
			PV.clear();
			PV.push_back(ttMove);
		}
#ifdef PRINT_STATISTICS
		if(ttValue>=beta){
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
		}
		else if(ttValue<=alpha){
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
		}
		else{
			Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
		}
#endif

		return ttValue;
	}

	Score staticEval;
	Score eval;
	if(inCheck){
		staticEval=pos.eval(pawnHashTable);
		eval=staticEval;
	}
	else{
		if(tte!=nullptr)
		{
			staticEval=tte->getStaticValue();
			assert(staticEval<SCORE_INFINITE);
			assert(staticEval>-SCORE_INFINITE);
			eval=staticEval;
			if (ttValue != SCORE_NONE){
				if (
						((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > eval) )
						|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < eval) )
					)
				{
					eval = ttValue;
				}
			}

		}
		else
		{
			staticEval=pos.eval(pawnHashTable);
			eval=staticEval;
		}

	}
#ifdef PRINT_STATISTICS
	Statistics::instance().testedAll=false;
	Statistics::instance().testedCut=false;
#endif
	//------------------------
	// razoring
	//------------------------
	// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
	//------------------------
	if (/*!PVnode
		&& */!inCheck
		&&  depth < 4 * ONE_PLY
		&&  eval + razorMargin(depth) <= alpha
		&&  alpha >= -SCORE_INFINITE+razorMargin(depth)
		//&&  abs(alpha) < SCORE_MATE_IN_MAX_PLY // implicito nell riga precedente
		&&  ((/*type==CUT_NODE &&*/!ttMove.packed ) || type==ALL_NODE)
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&& !((pos.getActualState().nextMove && (pos.bitBoard[Position::blackPawns] & RANKMASK[A2])) || (!pos.getActualState().nextMove && (pos.bitBoard[Position::whitePawns] & RANKMASK[A7]) ) )
	)
	{
		Score ralpha = alpha - razorMargin(depth);
		assert(ralpha>=-SCORE_INFINITE);
		std::vector<Move> childPV;
		Score v = qsearch<childNodesType>(ply,pos,0, ralpha, ralpha+1, childPV);
		if (v <= ralpha)
		{
#ifdef PRINT_STATISTICS
			if(type==ALL_NODE){
				Statistics::instance().testedAll=true;
				Statistics::instance().testedAllPruning++;
			}
			else{
				Statistics::instance().testedCut=true;
				Statistics::instance().testedCutPruning++;
			}
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
#endif
			return v;
		}
	}

	//---------------------------
	//	 STATIC NULL MOVE PRUNING
	//---------------------------
	//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
	//---------------------------
	if (!PVnode
		&& !inCheck
		&& !pos.getActualState().skipNullMove
		&&  depth < 4 * ONE_PLY
		&& eval >-SCORE_INFINITE + futility[depth>>ONE_PLY_SHIFT]
		&&  eval - futility[depth>>ONE_PLY_SHIFT] >= beta
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		//&&  abs(eval) < SCORE_KNOWN_WIN
		&&  ((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0])))
	{
		assert((eval -futility[depth>>ONE_PLY_SHIFT] >-SCORE_INFINITE));
#ifdef PRINT_STATISTICS
		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
		return eval - futility[depth>>ONE_PLY_SHIFT];
	}


	//---------------------------
	//	 NULL MOVE PRUNING
	//---------------------------
	// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
	// this search let us know about threat move by the opponent.
	//---------------------------

	if(!PVnode
		&& !inCheck
		&& depth>=ONE_PLY
		&& eval>=beta
		&& !pos.getActualState().skipNullMove
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&&((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0]))
	){
		// Null move dynamic reduction based on depth
		int red = 3 * ONE_PLY + depth / 4;

		// Null move dynamic reduction based on value
		if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta){
			red += ONE_PLY;
		}
		pos.doNullMove();
		U64 nullKey= pos.getActualState().key;


		std::vector<Move> childPV;
		Score nullVal;
		if(depth-red<ONE_PLY ){
			nullVal = -qsearch<childNodesType>(ply+1,pos,0,-beta,-beta+1,childPV);
		}else
		{
			nullVal = -alphaBeta<childNodesType>(ply+1,pos, depth - red, -beta, -beta+1, childPV);
		}
		pos.undoNullMove();



		if (nullVal >= beta)
		{
			// Do not return unproven mate scores
			if (nullVal >= SCORE_MATE_IN_MAX_PLY){
				nullVal = beta;
			}

			if (depth < 12 * ONE_PLY){
#ifdef PRINT_STATISTICS
				Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
				return nullVal;
			}

			childPV.clear();
			// Do verification search at high depths
			pos.getActualState().skipNullMove=true;
			assert(depth - red>=ONE_PLY);
			Score val = alphaBeta<childNodesType>(ply,pos, depth - red, -beta, -beta+1, childPV);
			pos.getActualState().skipNullMove=false;
			if (val >= beta){
#ifdef PRINT_STATISTICS
				Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
				return nullVal;
			}

		}
		else
		{
			ttEntry * tte=TT.probe(nullKey);
			threatMove=tte!=nullptr? tte->getPackedMove(): 0;
		}

	}

	//------------------------
	//	PROB CUT
	//------------------------
	//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
	//------------------------

	if(!PVnode &&
		!inCheck &&
		depth>=5*ONE_PLY &&
		!pos.getActualState().skipNullMove &&
		abs(beta)<SCORE_KNOWN_WIN
		//&& abs(beta)<SCORE_MATE_IN_MAX_PLY
	){
		Score s;
		Score rBeta=beta+8000;
		int rDepth=depth -ONE_PLY- 3*ONE_PLY;
		Movegen mg(pos,ttMove);
		mg.setupProbCutSearch(pos.getActualState().capturedPiece);

		Move m;
		while((m=mg.getNextMove()).packed){
			pos.doMove(m);
			std::vector<Move> childPV;
			assert(rDepth>=ONE_PLY);
			s=-alphaBeta<childNodesType>(ply+1,pos,rDepth,-rBeta,-rBeta+1,childPV);
			pos.undoMove(m);
			if(s>=rBeta){
#ifdef PRINT_STATISTICS
				Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
				return s;
			}

		}


	}

	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? 5 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed == 0
		&& (PVnode || staticEval+10000>= beta))
	{
		int d = depth - 2 * ONE_PLY - (PVnode ? 0 : depth / 4);

		bool skipBackup=pos.getActualState().skipNullMove;
		pos.getActualState().skipNullMove=true;

		std::vector<Move> childPV;
		const search::nodeType iidType=PVnode? (search::nodeType::PV_NODE) : type;
		assert(d>=ONE_PLY);
		alphaBeta<iidType>(ply,pos, d, alpha, beta, childPV);

		pos.getActualState().skipNullMove=skipBackup;

		tte = TT.probe(posKey);
		ttMove= tte!=nullptr ? tte->getPackedMove():0;
	}




	Score bestScore=-SCORE_INFINITE;

	Move bestMove;
	bestMove=0;
	Move m;
	Movegen mg(pos,ttMove);
	unsigned int moveNumber=0;
	unsigned int quietMoveCount =0;
	Move quietMoveList[64];

	bool singularExtensionNode=
		type!=search::nodeType::ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed != 0
		&& !excludedMove.packed // Recursive singular search is not allowed
		&& tte!=nullptr
		&& (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		&&  tte->getDepth()>= depth - 3 * ONE_PLY;

	while (bestScore <beta  && (m=mg.getNextMove()).packed) {

		assert(m.packed);
		if(m== excludedMove){
			continue;
		}


		// search only the moves in the search list
		if(type==search::nodeType::ROOT_NODE && !std::count(rootMoves.begin()+PVIdx,rootMoves.end(),m)){
			continue;
		}

		moveNumber++;


		bool captureOrPromotion =pos.isCaptureMoveOrPromotion(m);
		if(!captureOrPromotion && quietMoveCount < 64){
			quietMoveList[quietMoveCount++]=m;
		}

		bool moveGivesCheck=pos.moveGivesCheck(m);
		bool isDangerous=moveGivesCheck || pos.isCastleMove(m) || pos.isPassedPawnMove(m);

		int ext=0;
		if(PVnode && isDangerous){
			ext = ONE_PLY;
		}else if(moveGivesCheck && pos.seeSign(m) >= 0){
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if(singularExtensionNode
			&& !ext
			&&  m == ttMove
			&&  abs(ttValue) < SCORE_KNOWN_WIN
		)
		{

			Score rBeta = ttValue - int(depth*10);
			pos.getActualState().excludedMove=m;
			bool backup=pos.getActualState().skipNullMove;
			pos.getActualState().skipNullMove=true;
			std::vector<Move> locChildPV;
			Score temp = alphaBeta<childNodesType>(ply,pos,depth/2,rBeta-1,rBeta,locChildPV);
			pos.getActualState().skipNullMove=backup;
			pos.getActualState().excludedMove.packed=0;

			if(temp < rBeta){
				ext = ONE_PLY;
		    }
		}

		int newDepth= depth-ONE_PLY+ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if(!PVnode
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore>SCORE_MATED_IN_MAX_PLY
		){
			assert(moveNumber>1);

			if(depth < 11*ONE_PLY
				&& moveNumber >= FutilityMoveCounts[depth>>ONE_PLY_SHIFT]
				&& (!threatMove.packed)
				)
			{
				continue;
			}


			if(depth<7*ONE_PLY){
				Score localEval= eval + futilityMargin[newDepth>>ONE_PLY_SHIFT];
				if(localEval<beta){
					bestScore = std::max(bestScore, localEval);
					continue;
				}
			}

			if(newDepth < 4 * ONE_PLY
				&& pos.seeSign(m) < 0)
			{
				continue;
			}



		}

		if(type==ROOT_NODE){
			unsigned long elapsed=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count()-startTime;
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000
#endif
				&& !signals.stop
				){
				sync_cout<<"info currmovenumber "<<moveNumber<<" currmove "<<pos.displayUci(m)<<" nodes "<<visitedNodes<<" time "<<elapsed << sync_endl;
			}
		}

		pos.doMove(m);

		Score val;
		std::vector<Move> childPV;
		if(PVnode){
			if(moveNumber==1){
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"FIRST alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
				}
#endif

				if(newDepth<ONE_PLY){
					val=-qsearch<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
				}else{
					val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
				}
#ifdef PRINT_PV_CHANGES
				sync_cout<<"FIRST ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
			}
			else{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch=true;
				if(depth>3*ONE_PLY
					&& !captureOrPromotion
					&& !isDangerous
					&&  m != ttMove
					&&  m != pos.getActualState().killers[0]
					&&  m != pos.getActualState().killers[1]
				)
				{
					int reduction = PVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction!=0){
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"LMR alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<" red "<<(float)(reduction)/ONE_PLY<<sync_endl;
					}
#endif
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,d,-alpha-1,-alpha,childPV);
						if(val<=alpha){
							doFullDepthSearch=false;
						}
					}
				}


				if(doFullDepthSearch){

					childPV.clear();
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
					}
#endif
					if(newDepth<ONE_PLY){
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}

					if(val>alpha && val < beta ){
#ifdef DEBUG1
						if(type==ROOT_NODE){
							sync_cout<<"info currmove "<<pos.displayUci(m)<<" val "<<val/10000.0<<" nodes "<<visitedNodes<< sync_endl;
						}
#endif
						childPV.clear();
#ifdef DEBUG1
						if(type==search::nodeType::ROOT_NODE){
							sync_cout<<"RESEARCH alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
						}
#endif
						if(newDepth<ONE_PLY){
							val=-qsearch<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
						}
						else{
							val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
						}
#ifdef PRINT_PV_CHANGES
						sync_cout<<"OTHER ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
					}
				}

			}
		}
		else{

			//------------------------------
			//	LMR
			//------------------------------
			bool doFullDepthSearch=true;
			if(depth>3*ONE_PLY
				&& !captureOrPromotion
				&& !isDangerous
				&&  m != ttMove
				&&  m != pos.getActualState().killers[0]
				&&  m != pos.getActualState().killers[1]
			)
			{
				int reduction = nonPVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction!=0){
					childPV.clear();
					val=-alphaBeta<childNodesType>(ply+1,pos,d,-alpha-1,-alpha,childPV);
					if(val<=alpha){
						doFullDepthSearch=false;
					}
				}
			}




			if(doFullDepthSearch){
				childPV.clear();

				if(moveNumber<5){
					if(newDepth<ONE_PLY){
						val=-qsearch<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						val=-alphaBeta<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}
				}
				else{
					if(newDepth<ONE_PLY){
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}

				}

			}
		}



		pos.undoMove(m);




		if(val>bestScore){
			bestScore=val;

			if(val>alpha){
				bestMove=m;
				alpha =val;
				if(type ==search::nodeType::ROOT_NODE|| !signals.stop){

					PV.clear();
					PV.push_back(bestMove);
					std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
				}
			}
		}
	}


	// draw

	if(!moveNumber){
		if( excludedMove.packed){
#ifdef PRINT_STATISTICS
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
#endif
			return alpha;
		}else if(!inCheck){
			bestScore=0;
		}
		else{
			bestScore=matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;


	if(!signals.stop){
	//Statistics::instance().gatherNodeTypeStat(type,bestScore >= beta?CUT_NODE:PVnode && bestMove.packed? PV_NODE:ALL_NODE );
	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							depth, bestMove.packed, staticEval);
	}


	// save killer move & update history
	if (bestScore >= beta &&
			// TODO provare a fare solamente pos.isCaptureMove
		!pos.isCaptureMoveOrPromotion(bestMove) &&
		!inCheck)
	{
		if(pos.getActualState().killers[0] != bestMove)
		{
			pos.getActualState().killers[1] = pos.getActualState().killers[0];
			pos.getActualState().killers[0] = bestMove;
		}


		// update history
		// todo controllare se fare +=depth^2 e -=(depth^2)/(numero di mosse quiet) per avere media nulla
		// todo controllare se usare depth o qualche depth scalata
		Score bonus = Score(depth * depth);
		History::instance().update(pos.squares[bestMove.from],(tSquare) bestMove.to, bonus);
		if(quietMoveCount>1){
			for (int i = 0; i < quietMoveCount - 1; i++){
				Move m;
				m= quietMoveList[i];
				History::instance().update(pos.squares[m.from],(tSquare) m.to, -bonus);
			}
		}
	}
#ifdef PRINT_STATISTICS
	if(bestScore>beta){
		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
	}else if(PVnode && bestMove.packed){
		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
	}else{
		if(Statistics::instance().testedAll==true){
			Statistics::instance().correctAllPruning++;
		}
		if(Statistics::instance().testedCut==true){
			Statistics::instance().correctCutPruning++;
		}
		Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
	}
#endif
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(PV.size()==0);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);
	const bool PVnode=(type==search::nodeType::PV_NODE);
	bool inCheck=pos.getActualState().checkers;


	if(pos.isDraw() || signals.stop){
		//if(signals.stop){sync_cout<<"qsearch stop"<<sync_endl;}
#ifdef PRINT_STATISTICS
		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
#endif
		return 0;
	}

	if(pos.getActualState().ply>selDepth){
		selDepth=pos.getActualState().ply;
	}

	//----------------------------
	//	next node type
	//----------------------------
	const search::nodeType childNodesType=
		type==search::nodeType::ALL_NODE?
			search::nodeType::CUT_NODE:
			type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
				search::nodeType::PV_NODE;


	visitedNodes++;

	U64 posKey=pos.getActualState().key;
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte ? tte->getPackedMove() : 0;
	Movegen mg(pos,ttMove);
	int TTdepth=mg.setupQuiescentSearch(pos.getActualState().checkers,depth);
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (tte
		&& tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false/*tte->getType() == typeExact*/
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta|| tte->getType() == typeExact)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha|| tte->getType() == typeExact)))
	{
		PV.clear();
		if(ttMove.packed && Movegen::isMoveLegal(pos,ttMove)){
			PV.push_back(ttMove);
		}
#ifdef PRINT_STATISTICS
		if(ttValue>=beta){
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
		}
		else if(ttValue<=alpha){
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
		}
		else{
			Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
		}
#endif
		return ttValue;
	}

	ttType TTtype=typeScoreLowerThanAlpha;

	//----------------------------
	//	stand pat score
	//----------------------------
	Score staticEval;
	Score bestScore;
	if(inCheck){
		staticEval=pos.eval(pawnHashTable);
		bestScore=-SCORE_INFINITE;
	}
	else{
		if(tte)
		{
			staticEval=tte->getStaticValue();
		}
		else
		{
			staticEval=pos.eval(pawnHashTable);
		}
		bestScore=staticEval;

	}


	if(bestScore>alpha){

		alpha=bestScore;
		// TODO testare se la riga TTtype=typeExact; ha senso
		TTtype=typeExact;
	}
	// todo trovare un valore buono per il futility
	Score futilityBase=bestScore+5000;

	//----------------------------
	//	try the captures
	//----------------------------
	unsigned int moveNumber=0;
	Move m;
	Move bestMove;
	bestMove=0;
	while (bestScore <beta  &&  (m=mg.getNextMove()).packed) {
		assert(alpha<beta);
		assert(beta<=SCORE_INFINITE);
		assert(alpha>=-SCORE_INFINITE);
		moveNumber++;

		assert(m.packed);

		if(depth<-7*ONE_PLY && !inCheck){
			if(pos.getActualState().currentMove.to!= m.to){
				continue;
			}
		}

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		if(!PVnode &&
			!inCheck &&
			!pos.moveGivesCheck(m) &&
			m != ttMove &&
			m.flags != Move::fpromotion &&
			!pos.isPassedPawnMove(m)
		){
			Score futilityValue=futilityBase
                    + Position::pieceValue[pos.squares[m.to]%Position::separationBitmap][1]
                    + (m.flags == Move::fenpassant ? Position::pieceValue[Position::whitePawns][1] : 0);

			if (futilityValue < beta)
			{
				bestScore = std::max(bestScore, futilityValue);
				continue;
			}

		}


		//----------------------------
		//	don't check moves with negative see
		//----------------------------

		// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
		// TODO testare se aggiungere o no !movegivesCheck() &&
		if(PVnode &&
				!inCheck &&
				m.flags != Move::fpromotion &&
				m != ttMove &&
				pos.seeSign(m)<0){
			continue;
		}

		pos.doMove(m);
		Score val;
		std::vector<Move> childPV;
		val=-qsearch<childNodesType>(ply+1,pos,depth-ONE_PLY,-beta,-alpha,childPV);

		pos.undoMove(m);


		if(val>bestScore){
			bestScore=val;
			if(val>alpha){
				bestMove=m;
				TTtype=typeExact;
				alpha =val;
				if(!signals.stop){
					PV.clear();
					PV.push_back(m);
					std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
				}
			}
		}

	}

	// draw
	if(!moveNumber && inCheck){
		bestScore=matedIn(ply);
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;

	if(!signals.stop){
	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta ? typeScoreHigherThanBeta : TTtype,
			TTdepth, bestMove.packed, staticEval);
	}
#ifdef PRINT_STATISTICS
	if(bestScore>beta){
		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
	}else if(TTtype==typeExact){
		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
	}else{
		Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
	}
#endif

	return bestScore;

}


