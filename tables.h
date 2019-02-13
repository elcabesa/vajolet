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


#ifndef TABLES_H_
#define TABLES_H_


#include <array>
#include <cstdint>

#include "bitops.h"
#include "score.h"

class pawnEntry
{
public:
	HashKey key;
	bitMap weakPawns;
	bitMap passedPawns;
	bitMap pawnAttacks[2];
	bitMap weakSquares[2];
	bitMap holes[2];
	Score res[2];
};

class pawnTable
{
static const int size = 8192;
public:
	void insert(const HashKey& key,simdScore res,bitMap weak, bitMap passed,bitMap whiteAttack, bitMap blackAttack, bitMap weakSquareWhite,bitMap weakSquareBlack, bitMap whiteHoles, bitMap blackHoles){

		pawnEntry& x= probe(key);

		x.key = key;
		x.res[0]=res[0];
		x.res[1]=res[1];

		x.weakPawns=weak;
		x.passedPawns=passed;

		x.pawnAttacks[0]=whiteAttack;
		x.pawnAttacks[1]=blackAttack;
		x.weakSquares[0]=weakSquareWhite;
		x.weakSquares[1]=weakSquareBlack;
		x.holes[0]=whiteHoles ;
		x.holes[1]=blackHoles;
	}

	pawnEntry& probe(const HashKey& key)
	{
		return _pawnTable[ _getIndex( key ) ];
	}
private:
	unsigned int _getIndex( const HashKey& key ) const { return ( (unsigned int)key.getKey() ) % size; }
	std::array<pawnEntry,size> _pawnTable;
};

#endif /* TABLES_H_ */
