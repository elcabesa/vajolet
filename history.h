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
			while(1);
		}*/
	}
	inline Score getValue(Position::bitboardIndex p, tSquare to) const {
		assert(p<Position::lastBitboard);
		assert(to<squareNumber);
		return table[p][to];
	}

	History(){}
private:


	static const Score Max = Score(500000);
	Score table[Position::lastBitboard][squareNumber];
};



#endif /* HISTORY_H_ */
