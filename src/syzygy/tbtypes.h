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

#ifndef TBTYPES_H
#define TBTYPES_H

#include <cstdint>

constexpr int TBPIECES = 7; // Max number of supported pieces

enum WDLScore {
	WDLLoss        = -2, // Loss
	WDLBlessedLoss = -1, // Loss, but draw under 50-move rule
	WDLDraw        =  0, // Draw
	WDLCursedWin   =  1, // Win, but draw under 50-move rule
	WDLWin         =  2, // Win

	WDLScoreNone  = -1000
};

inline WDLScore operator-(WDLScore d) { return WDLScore(-int(d)); }

enum TBType {WDL, DTZ}; // Used as template parameter

using Sym = uint16_t;

#endif