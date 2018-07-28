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
#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "vajolet.h"
#include <algorithm>
#include <stdlib.h>
#include <cstring>
#include <array>
#include <vector>





enum ttType
{
	typeExact,
	typeScoreLowerThanAlpha,
	typeScoreHigherThanBeta,
	typeVoid
};

class ttEntry
{
private:

	signed int key:32; 			/*! 32 bit for the upper part of the key*/
	signed int packedMove:16;	/*! 16 bit for the move*/
	signed int depth:16;		/*! 16 bit for depth*/
	signed int value:23;		/*! 23 bit for the value*/
	signed int generation:8;		/*! 8 bit for the generation id*/
	signed int staticValue:23;	/*! 23 bit for the static evalutation (eval())*/
	signed int type:3;			/*! 2 bit for the type of the entry*/
							/*  144 bits total =16 bytes*/
public:
	ttEntry(unsigned int _Key, Score _Value, unsigned char _Type, signed short int _Depth, unsigned short _Move, Score _StaticValue, unsigned char _gen): key(_Key),packedMove(_Move),depth(_Depth), value(_Value),generation(_gen),staticValue(_StaticValue),type(_Type){}
	ttEntry(){}

	void save(unsigned int Key, Score Value, unsigned char Type, signed short int Depth, unsigned short Move, Score StaticValue, unsigned char gen)
	{
		assert(Value < SCORE_INFINITE || Value == SCORE_NONE);
		assert(Value >- SCORE_INFINITE);
		assert(StaticValue < SCORE_INFINITE);
		assert(StaticValue > -SCORE_INFINITE);
		assert(Type <= typeScoreHigherThanBeta);
		key = Key;
		value = Value;
		staticValue = StaticValue;
		if(Move)
		{
			packedMove = Move;
		}
		depth = Depth;
		generation = gen;
		type = Type;
	}

	void setGeneration(unsigned char gen)
	{
		generation = gen;
	}

	inline unsigned int getKey() const{ return key; }
	Score getValue()const { return value; }
	Score getStaticValue()const { return staticValue; }
	unsigned short getPackedMove()const { return packedMove; }
	signed short int getDepth()const { return depth; }
	unsigned char getType()const { return type; }
	unsigned char getGeneration()const { return generation; }
	
	
	bool isTypeGoodForBetaCutoff() const
	{
		return (getType() ==  typeScoreHigherThanBeta) || (getType() == typeExact);
	}
	bool isTypeGoodForAlphaCutoff() const
	{
		return (getType() ==  typeScoreLowerThanAlpha || getType() == typeExact);
	}
	
};

typedef	std::array< ttEntry, 4> ttCluster;



class transpositionTable
{
private:
	std::vector<ttCluster> table;
	unsigned long int elements;
	unsigned char generation;

	transpositionTable()
	{
		table.clear();
		table.shrink_to_fit();

		generation = 0;
		elements = 1;
	}
	static transpositionTable instance; // Guaranteed to be destroyed.
	// Instantiated on first use.
	
public:

	void clear();
	static transpositionTable& getInstance()
	{
		

		return instance;
	}
	transpositionTable(transpositionTable const&) = delete;
	void operator=(transpositionTable const&) = delete;
	
	void newSearch() { generation++; }
	unsigned long int setSize(unsigned long int mbSize);

	inline ttCluster& findCluster(U64 key)
	{
		return table[ static_cast<size_t>(((unsigned int)key) % elements) ];
	}

	inline void refresh(ttEntry& tte)
	{
		tte.setGeneration(generation);
	}

	ttEntry* probe(const U64 key);

	void store(const U64 key, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue);
	
	

	unsigned int getFullness() const
	{
		unsigned int cnt = 0u;
		unsigned int end = std::min( 250lu, elements );

		for (auto t = table.begin(); t != table.begin()+end; t++)
		{
			cnt+= std::count_if (t->begin(), t->end(), [=](ttEntry d){return d.getGeneration() == this->generation;});
		}
		return (unsigned int)(cnt*250lu/(end));
	}


	// value_to_tt() adjusts a mate score from "plies to mate from the root" to
	// "plies to mate from the current position". Non-mate scores are unchanged.
	// The function is called before storing a value to the transposition table.

	static Score scoreToTT(Score v, int ply)
	{
		assert(v<=SCORE_MATE);
		assert(v>=SCORE_MATED);
		assert(ply>=0);
		assert(v+ply<=SCORE_MATE);
		assert(v-ply>=SCORE_MATED);
		assert(v != SCORE_NONE);
		return  v >= SCORE_MATE_IN_MAX_PLY  ? v + ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v - ply : v;
	}


	// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
	// from the transposition table (where refers to the plies to mate/be mated
	// from current position) to "plies to mate/be mated from the root".

	static Score scoreFromTT(Score v, int ply)
	{

		assert(ply>=0);
		assert(v-ply<SCORE_MATE || v == SCORE_NONE);
		assert(v+ply>SCORE_MATED);
		assert(v>=SCORE_MATED);
		assert(v<=SCORE_MATE || v == SCORE_NONE);
		return  v == SCORE_NONE ? SCORE_NONE
				: v >= SCORE_MATE_IN_MAX_PLY  ? v - ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v + ply : v;
	}
};

class PerftTranspositionTable
{
public:
	PerftTranspositionTable(){}
	
	void store(const U64 key, signed short int depth, unsigned long long v);
	bool retrieve(const U64 key, unsigned int depth, unsigned long long& res);
};




#endif /* TRANSPOSITION_H_ */
