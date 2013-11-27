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
#ifndef SEARCH_H_
#define SEARCH_H_

#include "vajolet.h"
#include "position.h"
#include "move.h"
#include <vector>
#include <list>

class searcLimits{
public:
	bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,depth,nodes,mate,moveTime;

	std::list<Move> searchMoves;

};



inline Score mateIn(int ply) {
  return SCORE_MATE - ply;
}

inline Score matedIn(int ply) {
  return -SCORE_MATE + ply;
}


class search{

public:
	struct sSignal{
		bool stop=false;
	}signals;
	typedef enum eNodeType{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	} nodeType;
	void startThinking(Position & p);
private:
	template<nodeType type>Score alphaBeta(unsigned int ply,Position & p,int depth,Score alpha,Score beta,std::vector<Move> & PV);
	template<nodeType type>Score qsearch(unsigned int ply,Position & p,int depth,Score alpha,Score beta,std::vector<Move> & PV);
	unsigned long long visitedNodes;
	unsigned int selDepth;
	bool stop;
};

#endif /* SEARCH_H_ */
