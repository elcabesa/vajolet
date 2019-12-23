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

#include <cmath>
#include <iostream>

#include "hashKey.h"
#include "move.h"
#include "transposition.h"
#include "vajolet.h"


inline void ttEntry::save(unsigned int Key, Score Value, unsigned char Type, signed short int Depth, unsigned short Move, Score StaticValue, unsigned char gen)
{
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

inline void ttEntry::setGeneration(unsigned char gen)
{
	generation = gen;
}


uint64_t transpositionTable::setSize(unsigned long int mbSize)
{

	uint64_t size = (uint64_t)( (((uint64_t)mbSize) << 20) / sizeof(ttCluster));
	std::cout<<"mbSize: "<<mbSize<<std::endl;
	std::cout<<"sizeof(ttCluster): "<<sizeof(ttCluster)<<std::endl;
	
	_elements = size;
	std::cout<<"_elements: "<<_elements<<std::endl;
	std::cout<<"elements bits:"<<std::log(_elements) / std::log(2)<<std::endl;

	_table.clear();
	_table.shrink_to_fit();
	try
	{
		_table.resize(_elements);
	}
	catch(...)
	{
		std::cerr << "Failed to allocate " << mbSize<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}
	return _elements * 4;
}

void transpositionTable::newSearch() {_generation++;}

static ttEntry null(0,SCORE_NONE, typeVoid, -100, 0, 0, 0);
ttEntry* transpositionTable::probe( const HashKey& k )
{

	const auto key = k.getKey();

	ttCluster& ttc = findCluster(key);
	unsigned int keyH = (uint32_t)(key >> 32);

	auto it = std::find_if (ttc.begin(), ttc.end(), [keyH](ttEntry p){return p.getKey()==keyH;});
	if( it != ttc.end())
	{
		return it;
	}

	return &null;
}


void transpositionTable::store(const HashKey& k, Score value, unsigned char type, signed short int depth, const Move& move, Score statValue)
{
	if( move != Move::NOMOVE || type != typeExact)
	{
		assert(value < SCORE_INFINITE || value == SCORE_NONE);
		assert(value >- SCORE_INFINITE);
		assert(statValue < SCORE_INFINITE);
		assert(statValue > -SCORE_INFINITE);
		assert(type <= typeScoreHigherThanBeta);
	}

	const auto key = k.getKey();
	ttEntry *candidate;
	unsigned int keyH = (uint32_t)(key >> 32); // Use the high 32 bits as key inside the cluster

	ttCluster& ttc = findCluster(key);

	auto it = std::find_if (ttc.begin(), ttc.end(), [keyH](ttEntry p){return (!p.getKey()) || (p.getKey()==keyH);});
	if( it != ttc.end())
	{
		candidate = it;
	}
	else
	{
		candidate = &ttc[0];
		for(auto& d: ttc)
		{
			bool cc1,cc2,cc3,cc4;

			cc1 = candidate->getGeneration() == _generation;
			cc2 = d.getGeneration() == _generation;
			cc3 = d.getType() == typeExact;
			cc4 = d.getDepth() < candidate->getDepth();


			if( (cc1 && cc4) || (!(cc2 || cc3) && (cc4 || cc1)) )
			{
				candidate = &d;
			}

		}
	}
	assert(candidate != nullptr);
	candidate->save(keyH, value, type, depth, move.getPacked(), statValue, _generation);

}
void transpositionTable::clear()
{
	ttCluster ttc;
	ttc.fill(ttEntry(0,0,0,0,0,0,0));
	std::fill(_table.begin(), _table.end(), ttc);
}

inline ttCluster& transpositionTable::findCluster(uint64_t key)
{
	return _table[(uint32_t(key) * uint64_t(_elements)) >> 32];
}

void transpositionTable::refresh(ttEntry& tte)
{
	tte.setGeneration(_generation);
}

unsigned int transpositionTable::getFullness() const
{
	unsigned int cnt = 0u;
	unsigned int end = std::min( 250llu,uint64_t(_elements) );

	for (auto t = _table.begin(); t != _table.begin()+end; t++)
	{
		cnt+= std::count_if (t->begin(), t->end(), [=](ttEntry d){return d.getGeneration() == this->_generation;});
	}
	return (unsigned int)(cnt*250llu/(end));
}


void PerftTranspositionTable::store(const HashKey& key, signed short int depth, unsigned long long v)
{
	transpositionTable::getInstance().store(key, Score(v&0x7FFFFF), typeExact, depth, Move::NOMOVE, (v>>23)&0x7FFFFF);
}

bool PerftTranspositionTable::retrieve(const HashKey& key, unsigned int depth, unsigned long long& res)
{
	ttEntry* tte = transpositionTable::getInstance().probe( key );
	
	if( tte->getKey() == (key.getKey()>>32) && (unsigned int)tte->getDepth() == depth )
	{
		res = (unsigned long long)(((unsigned int)tte->getValue())&0x7FFFFF) + (((unsigned long long)((unsigned int)tte->getStaticValue())&0x7FFFFF)<<23);

		return true;
	}
	return false;
	
}
