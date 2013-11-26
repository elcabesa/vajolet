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


std::mt19937_64 rnd;
std::uniform_int_distribution<uint64_t> uint_dist;



void search::startThinking(Position & p){
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Movegen mg(p);
	Move m;
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

	std::vector<Move> PV;
	for(int depth=ONE_PLY;depth<4*ONE_PLY;depth+=ONE_PLY){

		PV.clear();
		selDepth=0;
		Score res=alphaBeta<search::nodeType::ROOT_NODE>(p,depth,-SCORE_INFINITE,SCORE_INFINITE,PV);
		unsigned long endTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		sync_cout<<"info depth "<<depth/ONE_PLY<<" seldepth "<< selDepth-selDepthBase<<" score cp "<<(int)((float)res/100.0)<<" nodes "<<visitedNodes<<" nps "<<(unsigned int)((double)visitedNodes*1000/(endTime-startTime))<<" time "<<(endTime-startTime);
		std::cout<<" pv ";
		for (auto & m :PV){
			std::cout<<p.displayUci(m)<<" ";
		}
		std::cout<<sync_endl;
	}
	sync_cout<<"bestmove "<<p.displayUci(PV[0]);
	if(PV.size()>1)
	{
		std::cout<<" ponder "<<p.displayUci(PV[1]);
	}
	std::cout<<sync_endl;

	//sync_cout<<"bestmove "<<p.displayUci(mg.getGeneratedMove(uint_dist(rnd)%mg.getGeneratedMoveNumber()))<<sync_endl;


}

template<search::nodeType type> Score search::alphaBeta(Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	visitedNodes++;
#ifdef DEBUG1
	unsigned long long tempVisitedNodes=visitedNodes;
#endif

	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;



	if(depth<ONE_PLY){
		return qsearch<type>(pos,depth,alpha,beta,PV);
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
			Score val = -alphaBeta<childNodesType>(pos, depth - 3*ONE_PLY, -beta, -beta+1, childPV);
			pos.undoNullMove();
			if (val >= beta){return val;}
		}

	}



	Score bestScore=-SCORE_INFINITE;
	bool searchFirstMove=true;

	Movegen mg(pos);
	unsigned int moveNumber=0;
	Move m;
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
				val=-alphaBeta<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);

			}
			else{
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
				}
#endif
				val=-alphaBeta<search::nodeType::CUT_NODE>(pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);

				if(val>alpha && val < beta ){
#ifdef DEBUG1
					if(type==ROOT_NODE){
						sync_cout<<"info currmove "<<pos.displayUci(m)<<" val "<<val/10000.0<<" nodes "<<visitedNodes<<" deltaNodes "<<visitedNodes-tempVisitedNodes<< sync_endl;
						tempVisitedNodes=visitedNodes;
							}
#endif
					childPV.clear();
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"RESEARCH alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
					}
#endif
					val=-alphaBeta<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);

				}

			}
		}
		else{
			val=-alphaBeta<childNodesType>(pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);
		}



		pos.undoMove(m);

#ifdef DEBUG1
		if(type==ROOT_NODE){
			sync_cout<<"info currmove "<<pos.displayUci(m)<<" val "<<val/10000.0<<" nodes "<<visitedNodes<<" deltaNodes "<<visitedNodes-tempVisitedNodes<< sync_endl;
			tempVisitedNodes=visitedNodes;
		}
#endif

		if(val>=bestScore){
			bestScore=val;
			if(val>alpha){
				alpha =val;
				PV.clear();
				PV.push_back(m);
				std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
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
			bestScore=-SCORE_MATE;
		}
	}
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	// if in check do evasion
	if(pos.getActualState().checkers){
		return alphaBeta<type>(pos,ONE_PLY,alpha,beta,PV);
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

	//----------------------------
	//	stand pat score
	//----------------------------
	Score bestScore=pos.eval();
	if(bestScore>alpha){
		alpha=bestScore;
	}
	Score futilityBase=bestScore+20000;

	//----------------------------
	//	try the captures
	//----------------------------
	Movegen mg(pos);
	mg.setupQuiescentSearch();
	unsigned int moveNumber=0;
	Move m;
	while (bestScore <beta  &&  (m=mg.getNextMove()).packed) {

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		// TODO aggiungere mossa != ttmove
		// TODO testare se aggiungere o no !movegivesCheck() &&
		// TODO aggiungere mossa non è passed pawn push

		if(type != search::nodeType::PV_NODE &&
				m.flags != Move::fpromotion
		){
			Score futilityValue=futilityBase
                    + Position::pieceValue[pos.board[m.to]%Position::emptyBitmap][1]
                    + (m.flags == Move::fenpassant ? Position::pieceValue[Position::whitePawns][1] : 0);

			if (futilityValue < beta)
			{
				bestScore = std::max(bestScore, futilityValue);
				moveNumber++;
				continue;
			}

		}


		//----------------------------
		//	don't check moves with negative see
		//----------------------------

		// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
		// TODO aggiungere mossa != ttmove
		// TODO testare se aggiungere o no !movegivesCheck() &&
		if(type != search::nodeType::PV_NODE &&
				m.flags != Move::fpromotion &&
				/*m.packed != TTMOVE:packed  &&*/
				pos.seeSign(m)<0){
			moveNumber++;
			continue;
		}
		pos.doMove(m);
		Score val;
		std::vector<Move> childPV;
		val=-qsearch<childNodesType>(pos,depth-ONE_PLY,-beta,-alpha,childPV);
		pos.undoMove(m);

		if(val>=bestScore){
			bestScore=val;
			if(val>alpha){
				alpha =val;
				PV.clear();
				PV.push_back(m);
				std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
			}
		}
		moveNumber++;
	}
	return bestScore;

}


