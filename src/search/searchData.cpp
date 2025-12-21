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

#include "searchData.h"

const SearchData _defaultSearchData; // convert to const

void SearchData::clearKillers(unsigned int ply)
{
	Move * const tempKillers =  _story[ply].killers;

	tempKillers[1] = Move::NOMOVE;
	tempKillers[0] = Move::NOMOVE;
}
void SearchData::cleanData(void)
{
	_history.clear();
	_captureHistory.clear();
	_counterMoves.clear();
	for( auto&x: _story)
	{
		
		x.excludeMove = Move::NOMOVE;
		x.skipNullMove = false;
		x.killers[0] = Move::NOMOVE;
		x.killers[1] = Move::NOMOVE;
		x.staticEval = 0;
		x.inCheck = false;
	}
}

void SearchData::saveKillers(unsigned int ply, const Move& m)
{
	Move * const tempKillers = _story[ply].killers;
	if(tempKillers[0] != m)
	{
		tempKillers[1] = tempKillers[0];
		tempKillers[0] = m;
	}

}

void SearchData::setExcludedMove(unsigned int ply, const Move& m)
{
	assert(ply < STORY_LENGTH);
	_story[ply].excludeMove = m;
}

const Move& SearchData::getExcludedMove(unsigned int ply)
{
	assert(ply < STORY_LENGTH);
	return _story[ply].excludeMove;
}