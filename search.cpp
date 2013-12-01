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
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "statistics.h"




void search::startThinking(Position & p){
	signals.stop=false;
	TT.newSearch();
	//Statistics::instance().initNodeTypeStat();
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Move m;
	m.packed=0;
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
	std::vector<Move> newPV, PV;
	int depth=ONE_PLY;

	Score alpha, beta;
	Score delta;
	Score res=0;

	do{
		if(depth<=2*ONE_PLY){
			alpha= -SCORE_INFINITE;
			beta = SCORE_INFINITE;
		}
		else{
			delta= 1600;
			alpha= res - delta;
			beta= res + delta;
		}

		do{
			newPV.clear();
			selDepth=selDepthBase;


			res=alphaBeta<search::nodeType::ROOT_NODE>(0,p,depth,alpha,beta,newPV);

			if(depth==ONE_PLY || !signals.stop){

				// print info
				unsigned long endTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				sync_cout<<"info depth "<<depth/ONE_PLY<<" seldepth "<< selDepth-selDepthBase<<" score ";
				if(abs(res) >SCORE_MATE_IN_MAX_PLY){
					std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
				}
				else{
					std::cout<< "cp "<<(int)((float)res/100.0);
				}

				std::cout<<(res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");
				ttType nodeType=res>=beta? typeScoreHigherThanBeta: res<=alpha? typeScoreLowerThanAlpha:typeExact;


				std::cout<<" nodes "<<visitedNodes<<" nps "<<(unsigned int)((double)visitedNodes*1000/(endTime-startTime))<<" time "<<(endTime-startTime);
				std::cout<<" pv ";
				int i=0;
				for (auto it= newPV.begin(); it != newPV.end(); ++it){
					std::cout<<p.displayUci(*it)<<" ";

					if(nodeType==typeExact){
						TT.store(p.getActualState().key, transpositionTable::scoreToTT((i%2)?-res:res, i),nodeType,depth-i*ONE_PLY, (*it).packed, p.eval());
					}
					p.doMove(*it);
					i++;
				}
				std::cout<<sync_endl;


				for (int i=newPV.size()-1;i>=0;i--){
					p.undoMove(newPV[i]);
				}
				PV=newPV;


			}

			if(res>=beta || res<=alpha){
				alpha= -SCORE_INFINITE;
				beta= +SCORE_INFINITE;

			}
			else{
				break;
			}

		}while(1);

		depth+=ONE_PLY;
	}while(depth<200*ONE_PLY && !signals.stop);
	sync_cout<<"bestmove "<<p.displayUci(PV[0]);
	if(PV.size()>1)
	{
		std::cout<<" ponder "<<p.displayUci(PV[1]);
	}
	std::cout<<sync_endl;

	//Statistics::instance().printNodeTypeStat();

	//sync_cout<<"bestmove "<<p.displayUci(mg.getGeneratedMove(uint_dist(rnd)%mg.getGeneratedMoveNumber()))<<sync_endl;


}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	visitedNodes++;
	const bool PVnode=(type==search::nodeType::PV_NODE || type==search::nodeType::ROOT_NODE);


	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;


	if(type !=search::nodeType::ROOT_NODE){
		if(pos.isDraw() || signals.stop){
			return 0;
		}

		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta)
			return alpha;

	}

	if(depth<ONE_PLY){
		return qsearch<type>(ply,pos,depth,alpha,beta,PV);
	}


	U64 posKey=pos.getActualState().key;
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove.packed=tte ? tte->getPackedMove() : 0;
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (type!=search::nodeType::ROOT_NODE &&
			tte
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    && (	PVnode ?  tte->getType() == typeExact
		            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta)
		                              : (tte->getType() ==  typeScoreLowerThanAlpha)))
		{

			TT.refresh(tte);
			PV.clear();
			PV.push_back(ttMove);
			return ttValue;
		}




	if(!pos.getActualState().checkers){

		// null move pruning
		if(
			type !=search::nodeType::PV_NODE &&
			type !=search::nodeType::ROOT_NODE &&
			pos.getActualState().pliesFromNull>1 &&
			((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]> 30000) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]> 30000))
		){
			pos.doNullMove();
			std::vector<Move> childPV;
			Score val = -alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos, depth - 3*ONE_PLY, -beta, -beta+1, childPV);
			pos.undoNullMove();
			if (val >= beta){
				//Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
				return val;
			}
		}

	}



	Score bestScore=-SCORE_INFINITE;
	bool searchFirstMove=true;

	Move bestMove;
	bestMove.packed=0;
	Move m;
	Movegen mg(pos,ttMove);
	unsigned int moveNumber=0;

	while (bestScore <beta  && (m=mg.getNextMove()).packed) {
		pos.doMove(m);

		Score val;
		std::vector<Move> childPV;
		if(type==search::nodeType::PV_NODE ||
			type==search::nodeType::ROOT_NODE ){
			if(searchFirstMove){
				searchFirstMove=false;
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"FIRST alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
				}
#endif
				val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,depth-ONE_PLY,-beta,-alpha,childPV);
#ifdef PRINT_PV_CHANGES
				sync_cout<<"FIRST ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
			}
			else{
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
				}
#endif
				val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);

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
					val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,depth-ONE_PLY,-beta,-alpha,childPV);
#ifdef PRINT_PV_CHANGES
					sync_cout<<"OTHER ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
				}

			}
		}
		else{
			val=-alphaBeta<childNodesType>(ply+1,pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);
		}



		pos.undoMove(m);


		if(type==ROOT_NODE && depth>10*ONE_PLY && !signals.stop){
			sync_cout<<"info currmove "<<pos.displayUci(m)<<" nodes "<<visitedNodes<< sync_endl;
		}


		if(val>bestScore){
			bestScore=val;

			if(val>alpha){
				bestMove.packed=m.packed;
				alpha =val;
				if(type ==search::nodeType::ROOT_NODE|| !signals.stop){

					PV.clear();
					PV.push_back(bestMove);
					std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
				}
			}
		}

		moveNumber++;
		searchFirstMove=false;
	}


	// draw
	if(!moveNumber){
		if(!pos.getActualState().checkers){
			bestScore=0;
		}
		else{
			bestScore=matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;


	//Statistics::instance().gatherNodeTypeStat(type,bestScore >= beta?CUT_NODE:PVnode && bestMove.packed? PV_NODE:ALL_NODE );
	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					PVnode && bestMove.packed ? typeExact : typeScoreLowerThanAlpha,
							depth, bestMove.packed, pos.eval());
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
	ttMove.packed=tte ? tte->getPackedMove() : 0;
	Movegen mg(pos,ttMove);
	int TTdepth=mg.setupQuiescentSearch(pos.getActualState().checkers,depth);
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (tte
		&& tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  tte->getType() == typeExact
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha)))
	{
		PV.clear();
		PV.push_back(ttMove);
		return ttValue;
	}

	ttType TTtype=typeScoreLowerThanAlpha;

	//----------------------------
	//	stand pat score
	//----------------------------
	Score staticEval=pos.eval();
	Score bestScore;
	if(inCheck){
		bestScore=-SCORE_INFINITE;
	}
	else{
		bestScore=staticEval;
	}


	if(bestScore>alpha){

		alpha=bestScore;
		// TODO testare se la riga TTtype=typeExact; ha senso
		TTtype=typeExact;
	}
	Score futilityBase=bestScore+20000;

	//----------------------------
	//	try the captures
	//----------------------------
	unsigned int moveNumber=0;
	Move m;
	Move bestMove;
	bestMove.packed=0;
	while (bestScore <beta  &&  (m=mg.getNextMove()).packed) {
		moveNumber++;

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		// TODO aggiungere mossa non è passed pawn push
		if(!PVnode &&
			!inCheck &&
			!pos.moveGivesCheck(m) &&
			m.packed != ttMove.packed &&
			m.flags != Move::fpromotion
		){
			Score futilityValue=futilityBase
                    + Position::pieceValue[pos.squares[m.to]%Position::emptyBitmap][1]
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
		if(type != search::nodeType::PV_NODE &&
				!inCheck &&
				m.flags != Move::fpromotion &&
				m.packed != ttMove.packed &&
				pos.seeSign(m)<0){
			continue;
		}

		pos.doMove(m);
		Score val;
		std::vector<Move> childPV;
		val=-qsearch<childNodesType>(ply+1,pos,depth-ONE_PLY,-beta,-alpha,childPV);
		pos.undoMove(m);

		if(val>bestScore){
			bestMove.packed=m.packed;
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

	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta ? typeScoreHigherThanBeta : TTtype,
			TTdepth, bestMove.packed, staticEval);
	return bestScore;

}


