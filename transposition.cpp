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

#include <iostream>

#include "transposition.h"
#include "vajolet.h"


transpositionTable transpositionTable::instance; // Guaranteed to be destroyed.
unsigned long int transpositionTable::setSize(unsigned long int mbSize)
{

	long long unsigned int size = (long unsigned int)( ((unsigned long long int)mbSize << 20) / sizeof(ttCluster));
	elements = size;

	table.clear();
	table.shrink_to_fit();
	try
	{
		table.resize(elements);
	}
	catch(...)
	{
		std::cerr << "Failed to allocate " << mbSize<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}
	return elements * 4;
}


static ttEntry null(0,SCORE_NONE, typeVoid, -100, 0, 0, 0);
ttEntry* transpositionTable::probe( const HashKey& k )
{

	const auto key = k.getKey();

	ttCluster& ttc = findCluster(key);
	unsigned int keyH = (unsigned int)(key >> 32);

	auto it = std::find_if (ttc.begin(), ttc.end(), [keyH](ttEntry p){return p.getKey()==keyH;});
	if( it != ttc.end())
	{
		return it;
	}

	return &null;
}


void transpositionTable::store(const HashKey& k, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue)
{

	const auto key = k.getKey();
	ttEntry *candidate;
	unsigned int keyH = (unsigned int)(key >> 32); // Use the high 32 bits as key inside the cluster

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

			cc1 = candidate->getGeneration() == generation;
			cc2 = d.getGeneration() == generation;
			cc3 = d.getType() == typeExact;
			cc4 = d.getDepth() < candidate->getDepth();


			if( (cc1 && cc4) || (!(cc2 || cc3) && (cc4 || cc1)) )
			{
				candidate = &d;
			}

		}
	}
	assert(candidate != nullptr);
	candidate->save(keyH, value, type, depth, move, statValue, generation);

}
void transpositionTable::clear()
{
	ttCluster ttc;
	ttc.fill(ttEntry(0,0,0,0,0,0,0));
	std::fill(table.begin(), table.end(), ttc);
}

void PerftTranspositionTable::store(const HashKey& key, signed short int depth, unsigned long long v)
{
	transpositionTable::getInstance().store(key, Score(v&0x7FFFFF), typeExact, depth, 0, (v>>23)&0x7FFFFF);
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
