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
#ifndef ECASTLE_H_
#define ECASTLE_H_


using Color = enum eColor
{
	white = 0,
	black = 1
};

enum eCastle	// castleRights
{

	wCastleOO  = 1,
	wCastleOOO = 2,
	bCastleOO  = 4,
	bCastleOOO = 8,
	castleOO = wCastleOO,
	castleOOO = wCastleOOO
};

inline eCastle operator|(const eCastle c1, eCastle c2) { return eCastle(int(c1) | int(c2)); }

inline Color operator++(Color& d, int) { Color r = d; d = Color(int(d) + 1); return r; }
inline Color& operator++(Color& d) { d = Color(int(d) + 1); return d; }


#endif
