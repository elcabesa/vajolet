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


#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include "score.h"
#include "vajolet.h"

class HashKey;
class Move;

//
//https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028912#21028912
//
// Allocator adaptor that interposes construct() calls to
// convert value initialization into default initialization.
template <typename T, typename A=std::allocator<T>>
class default_init_allocator : public A {
  typedef std::allocator_traits<A> a_t;
public:
  template <typename U> struct rebind {
    using other =
      default_init_allocator<
        U, typename a_t::template rebind_alloc<U>
      >;
  };

  using A::A;

  template <typename U>
  void construct(U* ptr)
    noexcept(std::is_nothrow_default_constructible<U>::value) {
    ::new(static_cast<void*>(ptr)) U;
  }
  template <typename U, typename...Args>
  void construct(U* ptr, Args&&... args) {
    a_t::construct(static_cast<A&>(*this),
                   ptr, std::forward<Args>(args)...);
  }
};

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
	inline void save(unsigned int Key, Score Value, unsigned char Type, signed short int Depth, unsigned short Move, Score StaticValue, unsigned char gen);
	inline void setGeneration(unsigned char gen);
	
	friend class transpositionTable;
public:
	explicit ttEntry(unsigned int _Key, Score _Value, unsigned char _Type, signed short int _Depth, unsigned short _Move, Score _StaticValue, unsigned char _gen): key(_Key), packedMove(_Move), depth(_Depth), value(_Value), generation(_gen), staticValue(_StaticValue), type(_Type){}
	explicit ttEntry(){}

	inline unsigned int getKey() const{ return key; }
	Score getValue()const { return value; }
	Score getStaticValue()const { return staticValue; }
	unsigned short getPackedMove()const { return packedMove; }
	signed short int getDepth()const { return depth; }
	ttType getType()const { return static_cast<ttType>(type); }
	unsigned char getGeneration()const { return generation; }

	bool isTypeGoodForBetaCutoff() const
	{
		return (getType() ==  ttType::typeScoreHigherThanBeta) || (getType() == ttType::typeExact);
	}
	bool isTypeGoodForAlphaCutoff() const
	{
		return (getType() ==  ttType::typeScoreLowerThanAlpha || getType() == ttType::typeExact);
	}
	
};

using ttCluster = std::array<ttEntry, 4>;



class transpositionTable
{
private:
	std::vector<ttCluster, default_init_allocator<ttCluster>> _table;
	uint64_t _elements;
	unsigned char _generation;

	transpositionTable(transpositionTable const&) = delete;
	//void operator=(transpositionTable const&) = delete;
	ttCluster& findCluster(uint64_t key);
	
public:

	explicit transpositionTable()
	{
		_table.clear();
		_table.shrink_to_fit();

		_generation = 0;
		_elements = 1;
	}

	void clear();
	/*static transpositionTable& getInstance()
	{
		static transpositionTable instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}*/
	
	void newSearch();
	uint64_t setSize(unsigned long int mbSize);
	void refresh(ttEntry& tte);
	ttEntry* probe(const HashKey& k);
	void store(const HashKey& k, Score value, unsigned char type, signed short int depth, const Move& move, Score statValue);
	unsigned int getFullness() const;
	
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
private:
	transpositionTable& _tt;
public:
	explicit PerftTranspositionTable(transpositionTable& tt): _tt(tt){}
	
	void store(const HashKey& key, signed short int depth, unsigned long long v);
	bool retrieve(const HashKey& key, unsigned int depth, unsigned long long& res);
};

#endif /* TRANSPOSITION_H_ */
