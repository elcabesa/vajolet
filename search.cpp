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
	Movegen mg;
	mg.generateMoves<Movegen::allMg>(p);
	if(mg.getGeneratedMoveNumber()==0){
		sync_cout<<"bestmove 0000"<<sync_endl;
		return;
	}
	if(mg.getGeneratedMoveNumber()==1){
		sync_cout<<"bestmove "<<p.displayUci(mg.getGeneratedMove(0))<<sync_endl;
		return;
	}


	visitedNodes=0;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();

	std::vector<Move> PV;
	for(int depth=ONE_PLY;depth<10*ONE_PLY;depth+=ONE_PLY){

		PV.clear();
		Score res=alphaBeta<search::nodeType::ROOT_NODE>(p,depth,-SCORE_INFINITE,SCORE_INFINITE,PV);
		unsigned long endTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		sync_cout<<"info depth "<<depth/ONE_PLY<<" score cp "<<(int)((float)res/100.0)<<" nodes "<<visitedNodes<<" nps "<<(unsigned int)((double)visitedNodes*1000/(endTime-startTime))<<" time "<<(endTime-startTime);
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
	unsigned long long tempVisitedNodes=visitedNodes;

	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;



	if(depth<ONE_PLY){
		return qsearch<type>(pos,depth,alpha,beta,PV);
	}



	Score bestScore=-SCORE_INFINITE;
	bool searchFirstMove=true;

	Movegen mg;
	mg.generateMoves<Movegen::allMg>(pos);
	unsigned int moveNumber=0;

	while (bestScore <beta  && moveNumber <mg.getGeneratedMoveNumber()) {
		pos.doMove(mg.getGeneratedMove(moveNumber));

		Score val;
		std::vector<Move> childPV;
		if(type==search::nodeType::PV_NODE ||
			type==search::nodeType::ROOT_NODE ){
			if(searchFirstMove){
				searchFirstMove=false;
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"FIRST alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
				}
				val=-alphaBeta<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);

			}
			else{
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
				}
				val=-alphaBeta<search::nodeType::CUT_NODE>(pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);

				if(val>alpha && val < beta ){
					if(type==ROOT_NODE){
						sync_cout<<"info currmove "<<pos.displayUci(mg.getGeneratedMove(moveNumber))<<" val "<<val/10000.0<<" nodes "<<visitedNodes<<" deltaNodes "<<visitedNodes-tempVisitedNodes<< sync_endl;
						tempVisitedNodes=visitedNodes;
							}
					childPV.clear();
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"RESEARCH alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
					}
					val=-alphaBeta<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);

				}

			}
		}
		else{
			val=-alphaBeta<childNodesType>(pos,depth-ONE_PLY,-alpha-1,-alpha,childPV);
		}



		pos.undoMove(mg.getGeneratedMove(moveNumber));


		if(type==ROOT_NODE){
			sync_cout<<"info currmove "<<pos.displayUci(mg.getGeneratedMove(moveNumber))<<" val "<<val/10000.0<<" nodes "<<visitedNodes<<" deltaNodes "<<visitedNodes-tempVisitedNodes<< sync_endl;
			tempVisitedNodes=visitedNodes;
		}

		if(val>=bestScore){
			bestScore=val;
			if(val>alpha){
				alpha =val;
				PV.clear();
				PV.push_back(mg.getGeneratedMove(moveNumber));
				std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
			}
		}

		moveNumber++;
		searchFirstMove=false;
	}

	if(bestScore>beta){
		return bestScore;
	}

	// draw
	if(!moveNumber&& !pos.getActualState().checkers){
		bestScore=0;
	}

	return bestScore;

}


template<search::nodeType type> Score search::qsearch(Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	// if in check do evasion
	if(pos.getActualState().checkers){
		return alphaBeta<type>(pos,ONE_PLY,alpha,beta,PV);
	}


	visitedNodes++;

	Score bestScore=pos.eval();		//stand pat score

	// try the moves
	Movegen mg;
	mg.generateMoves<Movegen::captureMg>(pos);

	unsigned int moveNumber=0;
	while (bestScore <beta  && moveNumber <mg.getGeneratedMoveNumber()) {
		pos.doMove(mg.getGeneratedMove(moveNumber));

		Score val;
		std::vector<Move> childPV;
		val=-qsearch<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);
		pos.undoMove(mg.getGeneratedMove(moveNumber));

		if(val>=bestScore){
			bestScore=val;
			if(val>alpha){
				alpha =val;
				PV.clear();
				PV.push_back(mg.getGeneratedMove(moveNumber));
				std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
			}
		}
		moveNumber++;
	}

	if(bestScore>beta){
		return bestScore;
	}


	return bestScore;

}


