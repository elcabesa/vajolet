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

void transpositionTable::setSize(unsigned long int mbSize)
{
	long long unsigned int size = (long unsigned int)( ((unsigned long long int)mbSize << 20) / sizeof(ttCluster));
	elements = size;
	//if(table)
	//{
		free(table);
	//}
	table = (ttCluster*)calloc((size_t)elements,sizeof(ttCluster));
	if (table == nullptr)
	{
		std::cerr << "Failed to allocate " << mbSize<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void transpositionTable::clear()
{
	std::memset(table, 0, (size_t) elements * sizeof(ttCluster));
}

ttEntry* transpositionTable::probe(const U64 key) const
{

	ttCluster& ttc = findCluster(key);
	unsigned int keyH = (unsigned int)(key >> 32);

	
	for(auto& d: ttc.data)
	{
		if ( d.getKey() == keyH )
			return &d;
	}

	return nullptr;
}

void transpositionTable::store(const U64 key, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue)
{

	bool cc1,cc2,cc3,cc4;
	ttEntry *candidate;
	unsigned int key32 = (unsigned int)(key >> 32); // Use the high 32 bits as key inside the cluster

	ttCluster& ttc = findCluster(key);
	candidate = &ttc.data[0];

	assert(replace!=nullptr);

	for(auto& d: ttc.data)
	{
		if(!d.getKey() || d.getKey() == key32) // Empty or overwrite old
		{
			candidate = &d;
			
			break;
		}


		cc1 = candidate->getGeneration() == generation;
		cc2 = d.getGeneration() == generation;
		cc3 = d.getType() == typeExact;
		cc4 = d.getDepth() < candidate->getDepth();


		if( (cc1 && cc4) || (!(cc2 || cc3) && (cc4 || cc1)) )
		{
			candidate = &d;
		}

	}
	assert(candidate != nullptr);
	//if(!move){move = candidate->getPackedMove();}
	candidate->save(key32, value, type, depth, move, statValue, generation);




}
