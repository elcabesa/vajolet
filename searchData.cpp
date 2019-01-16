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
	Move * const tempKillers =  story[ply].killers;

	tempKillers[1] = 0;
	tempKillers[0] = 0;
}
void SearchData::cleanData(void)
{
	unsigned int t = 0;
	for( auto&x: story)
	{
		
		x.excludeMove = Move::NOMOVE;
		x.skipNullMove = false;
		x.killers[0] = Move::NOMOVE;
		x.killers[1] = Move::NOMOVE;
		x.staticEval = 0;
		x.inCheck = false;
		++t;
	}
}

void SearchData::saveKillers(unsigned int ply, const Move& m)
{
	Move * const tempKillers = story[ply].killers;
	if(tempKillers[0] != m)
	{
		tempKillers[1] = tempKillers[0];
		tempKillers[0] = m;
	}

}