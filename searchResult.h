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

#ifndef SEARCH_RESULT_H_
#define SEARCH_RESULT_H_

#include "pvLine.h"
#include "score.h"

class SearchResult
{
public:
	Score alpha;
	Score beta;
	unsigned int depth;
	PVline PV;
	Score Res;
	SearchResult( Score Alpha, Score Beta, unsigned int Depth, PVline pv, Score res ): alpha(Alpha), beta(Beta), depth(Depth), PV(pv), Res(res){}
};

#endif