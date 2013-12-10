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


void search::printAllPV(unsigned int depth,unsigned int seldepth,Score alpha, Score beta, Position & p, unsigned long time, unsigned int count){


	for (unsigned int i=0; i<=count; i++){
		Score res=rootMoves[i].score;
		printPV(res,depth,seldepth,alpha,beta,p,time,i,rootMoves[i].PV);
	}
}

void search::printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, Position & p, unsigned long time,unsigned int count,std::vector<Move>& PV){
	sync_cout<<"info multipv "<< count+1<< " depth "<<(depth)<<" seldepth "<<seldepth <<" score ";
	if(abs(res) >SCORE_MATE_IN_MAX_PLY){
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else{
		std::cout<< "cp "<<(int)((float)res/100.0);
	}
	std::cout<<(res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");


	std::cout<<" nodes "<<visitedNodes<<" nps "<<(unsigned int)((double)visitedNodes*1000/(time))<<" time "<<(time);
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

void search::startThinking(Position & p,searchLimits & l){
	signals.stop=false;
	TT.newSearch();
	History::instance().clear();
	//p.cleanKillers();

	//Statistics::instance().initNodeTypeStat();

	limits=l;
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

	unsigned int PVSize = std::min(search::multiPVLines, rootMoves.size());
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Move m;
	m=0;
	Movegen mg(p,m);

	Move lastLegalMove;
	unsigned int legalMoves=0;
	while((m=mg.getNextMove()).packed){
		legalMoves++;
		lastLegalMove=m;
	}
	if(legalMoves==0){
		sync_cout<<"bestmove 0000"<<sync_endl;
		return;
	}else if(legalMoves==1){
		sync_cout<<"bestmove "<<p.displayUci(lastLegalMove)<<sync_endl;
		return;
	}



	unsigned int selDepthBase=p.getActualState().ply;
	visitedNodes=0;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
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
				delta = 1600;
				alpha = std::max((signed long long int)(rootMoves[PVIdx].previousScore) - delta,(signed long long int)-SCORE_INFINITE);
				beta  = std::min((signed long long int)(rootMoves[PVIdx].previousScore) + delta,(signed long long int) SCORE_INFINITE);
			}
			//sync_cout<<"alpha="<<alpha<<sync_endl;
			//sync_cout<<"beta="<<beta<<sync_endl;

			do{

				// reload the last PV in the transposition table
				for(unsigned int i =0; i<=PVIdx; i++){
					int n=0;
					//sync_cout<<"insert"<<sync_endl;
					// insert PV in the TT
					//sync_cout<<rootMoves[i].score<<sync_endl;
					ttType nodeType=rootMoves[i].score>=beta? typeScoreHigherThanBeta: rootMoves[i].score<=alpha? typeScoreLowerThanAlpha:typeExact;
					if(nodeType==typeExact && rootMoves[i].PV.size()>0){
						for (auto it= rootMoves[i].PV.begin(); it != rootMoves[i].PV.end(); ++it){
							if(Movegen::isMoveLegal(p,*it)){

								TT.store(p.getActualState().key, transpositionTable::scoreToTT((n%2)?-rootMoves[i].score:rootMoves[i].score, n),nodeType,depth-n*ONE_PLY, (*it).packed, p.eval());

								//sync_cout<<"insert in TT "<<p.displayUci(*it)<<sync_endl;
								p.doMove(*it);
								n++;
							}

						}
						for (n--;n>=0;n--){
							//sync_cout<<"undo move "<<p.displayUci(rootMoves[i].PV[n])<<sync_endl;
							p.undoMove(rootMoves[i].PV[n]);
						}
					}
				}

				newPV.clear();
				res=alphaBeta<search::nodeType::ROOT_NODE>(0,p,depth*ONE_PLY,alpha,beta,newPV);

				//sync_cout<<"res="<<res<<sync_endl;
				//sync_cout<<p.displayUci(newPV[0])<<sync_endl;
				//sync_cout<<"PVsize "<<newPV.size()<<sync_endl;

				if(depth!=1 && signals.stop){
					break;
				}
				if(newPV.size()!=0){
					std::vector<rootMove>::iterator it=std::find(rootMoves.begin(),rootMoves.end(),newPV[0]);
					if(it->firstMove==newPV[0]){
						it->PV=newPV;
						it->score=res;
					}
					std::stable_sort(rootMoves.begin() + PVIdx, rootMoves.end());
					//sync_cout<<"stableSort OK "<<sync_endl;

				}

				if (res <= alpha)
				{
					//sync_cout<<"res<=alpha "<<sync_endl;
					unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV);
					alpha = std::max((signed long long int)(res) - delta,(signed long long int)-SCORE_INFINITE);
				}
				else if (res >= beta){
					//sync_cout<<"res>=alpha "<<sync_endl;
					unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV);
					beta = std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
				}else{
					break;
				}
				delta += delta / 2;


			}while(1);

			if(!signals.stop){
				//sync_cout<<"aspiration window ok "<<sync_endl;
				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + PVIdx + 1);
				//sync_cout<<"stable sort ok "<<sync_endl;
				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (PVIdx + 1 == PVSize || now - startTime > 3000){
					//sync_cout<<"print pv "<<sync_endl;
					printAllPV(depth,selDepth-selDepthBase,-SCORE_INFINITE, SCORE_INFINITE,p, now-startTime, PVIdx);
				}
			}
		}
		depth+=1;
	}while(depth<=(limits.depth? limits.depth:100) && !signals.stop);

	//sync_cout<<"print final bestMove "<<sync_endl;
	signals.stop=true;
	sync_cout<<"bestmove "<<p.displayUci(rootMoves[0].PV[0]);
	if(rootMoves[0].PV.size()>1)
	{
		std::cout<<" ponder "<<p.displayUci(rootMoves[0].PV[1]);
	}
	std::cout<<sync_endl;
	visitedNodes=0;

	//Statistics::instance().printNodeTypeStat();



}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

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


	assert(depth>=ONE_PLY);
	assert(alpha<beta);
	if(type !=search::nodeType::ROOT_NODE){
		if(pos.isDraw() || signals.stop){
			return 0;
		}

		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta){
			//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
			return alpha;
		}

	}

	Move & excludedMove=pos.getActualState().excludedMove;

	U64 posKey=excludedMove.packed?pos.getExclusionKey() :pos.getKey();
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte ? tte->getPackedMove() : 0;
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (type!=search::nodeType::ROOT_NODE &&
			tte
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    //&& (	PVnode ?  tte->getType() == typeExact
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
		return ttValue;
	}

	Score staticEval;
	Score eval;
	if(inCheck){
		staticEval=pos.eval();
		eval=staticEval;
	}
	else{
		if(tte)
		{
			staticEval=tte->getStaticValue();
			eval=staticEval;
			if (ttValue != SCORE_NONE){
				if (((tte->getType() == typeScoreHigherThanBeta) && ttValue > eval)
					|| ((tte->getType() == typeScoreLowerThanAlpha) && ttValue < eval)){
					eval = ttValue;
				}
			}

		}
		else
		{
			staticEval=pos.eval();
			eval=staticEval;
		}

	}

	//------------------------
	// razoring
	//------------------------
	// TODO trovare un valore buono da mettere al posto di +30000 ( sara' una funzione di depth??)
	if (!PVnode
		&& !inCheck
		&&  depth < 4 * ONE_PLY
		&&  eval + 30000 < beta
		&&  !ttMove.packed
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&& !((pos.getActualState().nextMove && (pos.bitBoard[Position::blackPawns] & RANKMASK[A2])) || (!pos.getActualState().nextMove && (pos.bitBoard[Position::whitePawns] & RANKMASK[A7]) ) )
	)
	{
		Score rbeta = beta - 30000;
		std::vector<Move> childPV;
		Score v = qsearch<childNodesType>(ply,pos,0, rbeta-1, rbeta, childPV);
		if (v < rbeta){
			//Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
			return v;
		}
	}

	//---------------------------
	//	 STATIC NULL MOVE PRUNING
	//---------------------------
	if (!PVnode
		&& !inCheck
		&& !pos.getActualState().skipNullMove
		&&  depth < 4 * ONE_PLY
		&&  eval - futility[depth>>ONE_PLY_SHIFT] >= beta
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		//&&  abs(eval) < VALUE_KNOWN_WIN
		&&  ((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]> 30000) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]> 30000)))
	{
		//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
		return eval - futility[depth>>ONE_PLY_SHIFT];
	}


	//---------------------------
	//	 NULL MOVE PRUNING
	//---------------------------

	if(!PVnode
		&& !inCheck
		&& depth>=ONE_PLY
		&& eval>=beta
		&& !pos.getActualState().skipNullMove
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&&((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]> 30000) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]> 30000))
	){
		// Null move dynamic reduction based on depth
		int red = 3 * ONE_PLY + depth / 4;

		// Null move dynamic reduction based on value
		if (eval - 10000 > beta){
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
				//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
				return nullVal;
			}

			childPV.clear();
			// Do verification search at high depths
			pos.getActualState().skipNullMove=true;
			Score val = alphaBeta<childNodesType>(ply,pos, depth - red, -beta, -beta+1, childPV);
			pos.getActualState().skipNullMove=false;
			if (val >= beta){
				//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
				return nullVal;
			}

		}
		else
		{
			ttEntry * tte=TT.probe(nullKey);
			threatMove=tte? tte->getPackedMove(): 0;
		}

	}

	//------------------------
	//	PROB CUT
	//------------------------

	if(!PVnode &&
		!inCheck &&
		depth>=5*ONE_PLY &&
		!pos.getActualState().skipNullMove &&
		abs(beta)<SCORE_MATE_IN_MAX_PLY
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
			s=-alphaBeta<childNodesType>(ply+1,pos,rDepth,-rBeta,-rBeta+1,childPV);
			pos.undoMove(m);
			if(s>=rBeta){
				//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
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
		alphaBeta<iidType>(ply,pos, d, alpha, beta, childPV);

		pos.getActualState().skipNullMove=skipBackup;

		tte = TT.probe(posKey);
		ttMove=tte ? tte->getPackedMove():0;
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
		&& tte
		&& (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		&&  tte->getDepth()>= depth - 3 * ONE_PLY;

	while (bestScore <beta  && (m=mg.getNextMove()).packed) {

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

		// todo aggiungere passed pawn push alle mosse dangeous
		bool moveGivesCheck=pos.moveGivesCheck(m);
		bool isDangerous=moveGivesCheck || pos.isCastleMove(m);

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
						val=-alphaBeta<childNodesType>(ply+1,pos,d,-alpha-1,-alpha,childPV);
						if(val<=alpha){
							doFullDepthSearch=false;
						}
					}
				}


				if(doFullDepthSearch){

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
					val=-alphaBeta<childNodesType>(ply+1,pos,d,-alpha-1,-alpha,childPV);
					if(val<=alpha){
						doFullDepthSearch=false;
					}
				}
			}


			if(doFullDepthSearch){
				childPV.clear();
				if(newDepth<ONE_PLY){
					val=-qsearch<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
				}else{
					val=-alphaBeta<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
				}
			}
		}



		pos.undoMove(m);

		if(type==ROOT_NODE && depth>10*ONE_PLY && !signals.stop){
			sync_cout<<"info currmovenumber "<<moveNumber<<" currmove "<<pos.displayUci(m)<<" nodes "<<visitedNodes<< sync_endl;
		}


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
					PVnode && bestMove.packed ? typeExact : typeScoreLowerThanAlpha,
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
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	const bool PVnode=(type==search::nodeType::PV_NODE);
	bool inCheck=pos.getActualState().checkers;


	if(pos.isDraw() || signals.stop){
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
	    && (	PVnode ?  tte->getType() == typeExact
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta|| tte->getType() == typeExact)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha|| tte->getType() == typeExact)))
	{

		if(ttMove.packed && Movegen::isMoveLegal(pos,ttMove)){
			PV.clear();
			PV.push_back(ttMove);
		}
		return ttValue;
	}

	ttType TTtype=typeScoreLowerThanAlpha;

	//----------------------------
	//	stand pat score
	//----------------------------
	Score staticEval;
	Score bestScore;
	if(inCheck){
		staticEval=pos.eval();
		bestScore=-SCORE_INFINITE;
	}
	else{
		if(tte)
		{
			staticEval=tte->getStaticValue();
		}
		else
		{
			staticEval=pos.eval();
		}
		bestScore=staticEval;

	}


	if(bestScore>alpha){

		alpha=bestScore;
		// TODO testare se la riga TTtype=typeExact; ha senso
		TTtype=typeExact;
	}
	// todo trovare un valore buono per il futility
	Score futilityBase=bestScore+20000;

	//----------------------------
	//	try the captures
	//----------------------------
	unsigned int moveNumber=0;
	Move m;
	Move bestMove;
	bestMove=0;
	while (bestScore <beta  &&  (m=mg.getNextMove()).packed) {
		moveNumber++;

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		// TODO aggiungere mossa non è passed pawn push
		if(!PVnode &&
			!inCheck &&
			!pos.moveGivesCheck(m) &&
			m != ttMove &&
			m.flags != Move::fpromotion
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
			bestMove=m;
			bestScore=val;
			if(val>alpha){
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
	return bestScore;

}


