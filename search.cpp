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
	for(int depth=ONE_PLY;depth<4*ONE_PLY;depth+=ONE_PLY){

		PV.clear();
		Score res=alphaBeta<search::nodeType::ROOT_NODE>(p,depth,-SCORE_INFINITE,SCORE_INFINITE,PV);
		unsigned long endTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		sync_cout<<"info depth "<<depth/ONE_PLY<<" score "<<(float)res/10000.0<<" nodes "<<visitedNodes<<" nps "<<(unsigned int)((double)visitedNodes*1000/(endTime-startTime))<<sync_endl;
		sync_cout<<"info pv ";
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


	if(depth<ONE_PLY){
		return pos.eval();
	}

	Score bestScore=-SCORE_INFINITE;

	Movegen mg;
	mg.generateMoves<Movegen::allMg>(pos);
	unsigned int moveNumber=0;
	while (moveNumber <mg.getGeneratedMoveNumber()) {
		pos.doMove(mg.getGeneratedMove(moveNumber));
		std::vector<Move> childPV;
		Score val=-alphaBeta<search::nodeType::PV_NODE>(pos,depth-ONE_PLY,-beta,-alpha,childPV);
		pos.undoMove(mg.getGeneratedMove(moveNumber));

		if(val>beta){
			return val;
		}
		//if(type==ROOT_NODE){
		//	sync_cout<<pos.displayUci(mg.getGeneratedMove(moveNumber))<<" "<<val/10000.0<<sync_endl;
		//}

		if(val>bestScore){
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

	// draw
	if(!moveNumber&& !pos.getActualState().checkers){
		bestScore=0;
	}

	return bestScore;

}


