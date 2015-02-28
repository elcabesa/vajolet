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
#include "history.h"
#include "eval.h"
#include <vector>
#include <list>
#include <cmath>


typedef struct pvl{
	unsigned int lenght;
	Move list[MAX_PV_LENGTH];
}PVline;


class searchLimits{
public:
	volatile bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,depth,nodes,mate,moveTime;

	std::list<Move> searchMoves;
	searchLimits(){
		ponder=false;
		infinite =false;
		wtime=0;
		btime=0;
		winc=0;
		binc=0;
		movesToGo=0;
		depth=0;
		nodes=0;
		mate=0;
		moveTime=0;
	}

};

class rootMove{
public:
	Score score;
	Score previousScore;
	PVline PV;
	Move firstMove;
	unsigned int selDepth;
	unsigned int depth;
	unsigned long long nodes;
	unsigned long time;
	bool operator<(const rootMove& m) const { return score > m.score; } // Ascending sort
	bool operator==(const Move& m) const { return firstMove.packed == m.packed; }
};


inline Score mateIn(int ply) {
  return SCORE_MATE - ply;
}

inline Score matedIn(int ply) {
  return SCORE_MATED + ply;
}


class search{


	static Score futility[5];
	static Score futilityMargin[7];
	static unsigned int FutilityMoveCounts[11];
	static Score PVreduction[32*ONE_PLY][64];
	static Score nonPVreduction[32*ONE_PLY][64];
	unsigned long startTime;




	unsigned int indexPV;
	void printAllPV(Position & p,unsigned int count);
	void printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, Position & p, unsigned long time,unsigned int count,PVline * PV,unsigned long long nods);
public:
	searchLimits limits;
	History history;
	std::vector<rootMove> rootMoves;
	static unsigned int multiPVLines;
	static unsigned int limitStrength;
	static unsigned int eloStrenght;
	static bool useOwnBook;
	static bool bestMoveBook;
	static bool showCurrentLine;
	volatile bool showLine;
	static void initLMRreduction(void){
		for (int d = 1; d < 32*ONE_PLY; d++)
			for (int mc = 1; mc < 64; mc++)
			{
				double    PVRed = -1.5 + 0.33*log(double(d)) * log(double(mc));
				double nonPVRed = -1.2 + 0.4*log(double(d)) * log(double(mc));
				PVreduction[d][mc]=(Score)(PVRed >= 1.0 ? floor(PVRed * int(ONE_PLY)) : 0);
				nonPVreduction[d][mc]=(Score)(nonPVRed >= 1.0 ? floor(nonPVRed * int(ONE_PLY)) : 0);
			}
	};
	volatile struct sSignal{
		volatile bool stop=false;
	}signals;

	typedef enum eNodeType{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	} nodeType;
	Score startThinking(Position & p,searchLimits & limits);
	unsigned long long getVisitedNodes(){
		return visitedNodes;
	}
	template<nodeType type>Score qsearch(unsigned int ply,Position & p,int depth,Score alpha,Score beta,PVline * pvLine);
private:
	template<nodeType type>Score alphaBeta(unsigned int ply,Position & p,int depth,Score alpha,Score beta,PVline *  pvLine);

	unsigned long long visitedNodes;
	unsigned int selDepth;
	bool stop;
};

#endif /* SEARCH_H_ */
