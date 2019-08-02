/*
	This file is part of Vajolet.
	
	Copyright (c) 2013 Ronald de Man
	Copyright (c) 2015 basil00
	Copyright (C) 2016-2019 Marco Costalba, Lucas Braesch
	Modifications Copyright (c) 2016-2019 by Jon Dart
	Modifications Copyright (c) 2019 by Marco Belli

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
#include "tbtableDTZ.h"
#include "tbtableWDL.h"


TBTableDTZ:: TBTableDTZ(const TBTableWDL& other): TBTable(reinterpret_cast<const TBTable&>(other), "rtbz", 1) {
}

TBType TBTableDTZ::getType() const{
	return TBType::DTZ;
}

WDLScore TBTableDTZ::_mapScore(const tFile f, int value, const WDLScore wdl) const {

	constexpr int WDLMap[] = { 1, 3, 0, 2, 0 };

	auto flags = getPairsData(0, f).getFlags();
	const auto& idx = getPairsData(0, f).getMapIdx();
	
	if (flags & PairsData::MappedFlag) {
		if (flags & PairsData::WideFlag) {
			value = ((uint16_t *)_map)[idx[WDLMap[transformWdlToOffset(wdl)]] + value];
		} else {
			value = _map[idx[WDLMap[transformWdlToOffset(wdl)]] + value];
		}
	}

	// DTZ tables store distance to zero in number of moves or plies. We
	// want to return plies, so we have convert to plies when needed.
	if (   (wdl == WDLScore::WDLWin  && !(flags & PairsData::WinPliesFlag))
		|| (wdl == WDLScore::WDLLoss && !(flags & PairsData::LossPliesFlag))
		||  wdl == WDLScore::WDLCursedWin
		||  wdl == WDLScore::WDLBlessedLoss) {
		value *= 2;
	}

	return static_cast<WDLScore>(value + 1);
}



bool TBTableDTZ::_checkDtzStm(unsigned int stm, tFile f) const {

	auto flags = getPairsData(stm, f).getFlags();
	return	(flags & PairsData::STMFlag) == stm
			|| ((getKey() == getKey2()) && !hasPawns());
}

