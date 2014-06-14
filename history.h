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



#ifndef HISTORY_H_
#define HISTORY_H_

#include <algorithm>
#include <cstring>
#include <array>
#include "vajolet.h"
#include "position.h"


class History{
public :

	void clear() { std::memset(table, 0, sizeof(table)); }


	inline void update(Position::bitboardIndex p, tSquare to, Score v){

		assert(p<Position::lastBitboard);
		assert(to<squareNumber);
		/*if(p >=Position::lastBitboard || to>=squareNumber){
			sync_cout<<"errore"<<sync_endl;
		}*/
		if (abs(table[p][to] + v) < Max){
			table[p][to] +=  v;
		}
		/*else{
			sync_cout<<"saturation"<<sync_endl;
			sync_cout<<table[p][to]<<sync_endl;
			while(1);
		}*/
	}
	inline Score getValue(Position::bitboardIndex p, tSquare to) const {
		assert(p<Position::lastBitboard);
		assert(to<squareNumber);
		return table[p][to];
	}

	inline void printBestMoves()const {
#define arrayLenght 100
		typedef struct s_elem{
			int to;
			int p;
			int max;
			bool operator<(const s_elem& m) const { return max > m.max; } // Ascending sort
			bool operator==(const s_elem& m) const { return max == m.max; }
		}elem ;
		std::array<elem,arrayLenght> bestMoves;

		for(int i=0;i<arrayLenght;i++){
			bestMoves[i].max=-2*Max;
		}

		for(int p=0; p<Position::lastBitboard;p++){
			for( int to =0;to<squareNumber;to++){
				if(table[p][to]>=bestMoves[arrayLenght-1].max && table[p][to]!=0){
					bestMoves[arrayLenght-1].max=table[p][to];
					bestMoves[arrayLenght-1].p=p;
					bestMoves[arrayLenght-1].to=to;

					std::stable_sort(bestMoves.begin(), bestMoves.end());
				}

			}
		}

		Position pp;
		int MAX= bestMoves[0].max;
		for(int i=0;i<arrayLenght;i++){
			if(bestMoves[i].max>std::max(0.2*MAX,0.0 )){
				sync_cout<<i<<") mossa minacciata "<<pp.PIECE_NAMES_FEN[bestMoves[i].p] <<" in "<<char('a'+FILES[bestMoves[i].to])<<char('1'+RANKS[bestMoves[i].to])<<" (value: "<< bestMoves[i].max<<")"<<sync_endl;
			}
		}
	}

	History(){}
private:


	static const Score Max = Score(500000);
	Score table[Position::lastBitboard][squareNumber];
};



#endif /* HISTORY_H_ */
