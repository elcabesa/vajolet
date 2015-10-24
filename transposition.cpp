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

void transpositionTable::setSize(long long unsigned int mbSize){
	long long unsigned int size =  (mbSize << 20) / sizeof(ttCluster);
	sync_cout<<size<<sync_endl;
	elements=size;
	if(table){
		free(table);
	}
	table =(ttCluster*)calloc(elements,sizeof(ttCluster));
	if (table==nullptr)
	{
		std::cerr << "Failed to allocate " << mbSize<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void transpositionTable::clear(){
	std::memset(table, 0, elements * sizeof(ttCluster));
}

ttEntry* transpositionTable::probe(const U64 key) const {

	ttCluster * ttc=findCluster(key);
	assert(ttc!=nullptr);
	unsigned int keyH = (unsigned int)(key >> 32);


	for (unsigned i = 0; i < 4; i++){
		if (ttc->data[i].getKey() == keyH)
			return &ttc->data[i];
	}

	return nullptr;
}

void transpositionTable::store(const U64 key, Score v, unsigned char b, signed short int d, unsigned short m, Score statV) {

	int c1, c2, c3;
	ttEntry *tte, *replace;
	unsigned int key32 = (unsigned int)(key >> 32); // Use the high 32 bits as key inside the cluster

	ttCluster * ttc=findCluster(key);
	assert(ttc!=nullptr);
	tte = replace = (*ttc).data;

	assert(tte!=nullptr);
	assert(replace!=nullptr);

	for (unsigned i = 0; i < 4; i++, tte++)
	{
		if(!tte->getKey() || tte->getKey() == key32) // Empty or overwrite old
		{
			if (!m){
				m = tte->getPackedMove(); // Preserve any existing ttMove
			}

			replace = tte;
			break;
		}

		// Implement replace strategy
		c1 = (replace->getGeneration() == generation ?  2 : 0);
		c2 = (tte->getGeneration() == generation || tte->getType() == typeExact ? -2 : 0);
		c3 = (tte->getDepth() < replace->getDepth() ?  1 : 0);

		if (c1 + c2 + c3 > 0){
			replace = tte;
		}
	}
	assert(replace!=nullptr);
	if(replace->getSearchId()!=generation){
		usedElements++;
	}
	replace->save(key32, v, b, d, m, statV);
	replace->setGeneration(generation);



}
