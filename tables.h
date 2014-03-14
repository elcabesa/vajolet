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

#include <vector>


template<class Entry, int Size>
struct HashTable
{
public:
	HashTable() : e(Size, Entry()) {}
	Entry* operator[](U64 k) {return &e[(unsigned int)k % (Size)]; }

private:
  std::vector<Entry> e;
};


class pawnEntry
{
public:
	U64 key;
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
	void insert(U64 key,simdScore res,bitMap weak, bitMap passed,bitMap whiteAttack, bitMap blackAttack, bitMap weakSquareWhite,bitMap weakSquareBlack, bitMap whiteHoles, bitMap blackHoles){
		pawnEntry* x=pawnTable[key];
		x->key=key;
		x->res[0]=res[0];
		x->res[1]=res[1];

		x->weakPawns=weak;
		x->passedPawns=passed;

		x->pawnAttacks[0]=whiteAttack;
		x->pawnAttacks[1]=blackAttack;
		x->weakSquares[0]=weakSquareWhite;
		x->weakSquares[1]=weakSquareBlack;
		x->holes[0]=whiteHoles ;
		x->holes[1]=blackHoles;
	}

	pawnEntry* probe(U64 key){
		pawnEntry* x=pawnTable[key];
		if(x->key==key){
			return x;
		}
		return nullptr;

	}
private:
	HashTable<pawnEntry, 65536> pawnTable;
};


class evalEntry
{
public:
	U64 key;
	Score res;
};

class evalTable
{
public:
	void insert(U64 key,Score res){
		evalEntry* x=evalTable[key];
		x->key=key;
		x->res=res;
	}

	evalEntry* probe(U64 key){
		evalEntry* x=evalTable[key];
		if(x->key==key){
			return x;
		}
		return nullptr;

	}
private:
	HashTable<evalEntry, 524288> evalTable;
};

#endif /* TABLES_H_ */
