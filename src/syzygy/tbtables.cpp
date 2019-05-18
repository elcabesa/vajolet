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

#include <string>
#include "command.h"
#include "tbfile.h"
#include "tbtables.h"

TBTables::TBTables(): MaxCardinality(0){}

void TBTables::clear() {
	_hashTable.clear();
	_wdlTable.clear();
	_dtzTable.clear();
	MaxCardinality = 0;
}
size_t TBTables::size() const {
	return _wdlTable.size();
}

// If the corresponding file exists two new objects TBTable<WDL> and TBTable<DTZ>
// are created and added to the lists and hash table. Called at init time.
void TBTables::add(const std::vector<bitboardIndex>& pieces) {
	std::string code;

    for (auto p : pieces){
		code += UciManager::getPieceName(p);
	}
	code.insert(code.find('K', 1), "v");
	
	if(!TBFile::exist(code + ".rtbw")) { // Only WDL file is checked
		return;
	}	
	MaxCardinality = std::max(pieces.size(), MaxCardinality);
	
	_wdlTable.emplace_back(code);
	_dtzTable.emplace_back(_wdlTable.back());
	
	// Insert into the hash keys for both colors: KRvK with KR white and black
	Entry e(&_wdlTable.back(), &_dtzTable.back());
	std::pair<HashKey,Entry> p1(_wdlTable.back().getKey(),e);
	std::pair<HashKey,Entry> p2(_wdlTable.back().getKey2(),e);
	_hashTable.insert(p1);
	_hashTable.insert(p2);
}

const TBTableWDL& TBTables::getWDL(const HashKey& k) const {
	
	return *(_hashTable.at(k).first);
}

const TBTableDTZ& TBTables::getDTZ(const HashKey& k) const {
	return *(_hashTable.at(k).second);
}

size_t TBTables::getMaxCardinality() const {
	return MaxCardinality;
}
