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

#include "bitBoardIndex.h"
#include "bitops.h"
#include "hashKey.h"
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
public:
	void insert(
		const HashKey& key,
		const simdScore res,
		const bitMap weakPawns,
		const bitMap passedPawns,
		const bitMap* const attackedSquares,
		const bitMap* const weakSquares,
		const bitMap* const holes) {

		pawnEntry& x = _probe(key);

		x.key = key;
		x.res[0] = res[0];
		x.res[1] = res[1];

		x.weakPawns = weakPawns;
		x.passedPawns = passedPawns;

		x.pawnAttacks[0] = attackedSquares[whitePawns];
		x.pawnAttacks[1] = attackedSquares[blackPawns];
		
		x.weakSquares[0] = weakSquares[white];
		x.weakSquares[1] = weakSquares[black];
		x.holes[0] = holes[white];
		x.holes[1] = holes[black];
	}
	
	bool getValues(
		const HashKey& pawnKey,
		simdScore& res,
		bitMap& weakPawns,
		bitMap& passedPawns,
		bitMap *const attackedSquares,
		bitMap * const weakSquares,
		bitMap * const holes) const {
	
	const pawnEntry& probePawn = _probe(pawnKey);
	
	if (probePawn.key == pawnKey) {
		weakPawns = probePawn.weakPawns;
		passedPawns = probePawn.passedPawns;
		attackedSquares[whitePawns] = probePawn.pawnAttacks[0];
		attackedSquares[blackPawns] = probePawn.pawnAttacks[1];
		weakSquares[white] = probePawn.weakSquares[0];
		weakSquares[black] = probePawn.weakSquares[1];
		holes[white] = probePawn.holes[0];
		holes[black] = probePawn.holes[1];
		res = {probePawn.res[0], probePawn.res[1], 0, 0};
		return true;
	} else {
		return false;
	}
}
private:
	static const int _size = 8192;
	unsigned int _getIndex( const HashKey& key ) const { return ( (unsigned int)key.getKey() ) % _size; }
	const pawnEntry& _probe(const HashKey& key) const { return _pawnTable[_getIndex(key)]; }
	pawnEntry& _probe(const HashKey& key) { return _pawnTable[_getIndex(key)]; }
	std::array<pawnEntry,_size> _pawnTable;
};

#endif /* TABLES_H_ */
