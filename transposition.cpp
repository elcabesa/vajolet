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

#include "transposition.h"
#include "io.h"


transpositionTable TT;

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
ttEntry* transpositionTable::probe(const U64 key)
{


	ttCluster& ttc = findCluster(key);
	unsigned int keyH = (unsigned int)(key >> 32);

	auto it = std::find_if (ttc.begin(), ttc.end(), [keyH](ttEntry p){return p.getKey()==keyH;});
	if( it != ttc.end())
	{
		return it;
	}

	return &null;
}


void transpositionTable::store(const U64 key, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue)
{


	ttEntry *candidate;
	unsigned int keyH = (unsigned int)(key >> 32); // Use the high 32 bits as key inside the cluster

	ttCluster& ttc = findCluster(key);
	candidate = &ttc[0];

	assert(candidate!=nullptr);

	auto it = std::find_if (ttc.begin(), ttc.end(), [keyH](ttEntry p){return (!p.getKey()) || (p.getKey()==keyH);});
	if( it != ttc.end())
	{
		candidate = it;
	}
	else
	{
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
