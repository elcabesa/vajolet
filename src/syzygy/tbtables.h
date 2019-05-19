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

#ifndef TBTABLES_H
#define TBTABLES_H

#include <deque>
#include <unordered_map>
#include <utility>

#include "bitBoardIndex.h"
#include "hashKey.h"
#include "tbtableDTZ.h"
#include "tbtableWDL.h"

class Position;

// class TBTables creates and keeps ownership of the TBTable objects, one for
// each TB file found. It supports a fast, hash based, table lookup. Populated
// at init time, accessed at probe time.
class TBTables {
	
	size_t MaxCardinality;
	
	using Entry = std::pair<TBTableWDL*, TBTableDTZ*>;
	
	std::unordered_map<HashKey, Entry, HashKey> _hashTable;
	std::deque<TBTableWDL> _wdlTable;
    std::deque<TBTableDTZ> _dtzTable;
	
	void _add(const std::vector<bitboardIndex>& pieces);

public:
	TBTables();
	void clear();
	void init();
	size_t size() const;
	TBTableWDL& getWDL(const HashKey& k) const;
	TBTableDTZ& getDTZ(const HashKey& k) const;
	bool exists(const HashKey& k) const;
	size_t getMaxCardinality() const;
	int probeWDL(const Position& pos, ProbeState& result, WDLScore wdl = WDLDraw) const;
	int probeDTZ(const Position& pos, ProbeState& result, WDLScore wdl = WDLDraw) const;
};

#endif
