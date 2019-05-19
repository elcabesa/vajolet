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

#ifndef TBTABLEDTZ_H
#define TBTABLEDTZ_H

#include<string>

#include "tbtable.h"

class TBTableWDL;

class TBTableDTZ: public TBTable {
	int _mapScore(const tFile f, int value, const int wdl) const;
	bool _checkDtzStm(unsigned int stm, tFile f) const;
public:
	explicit TBTableDTZ(const TBTableWDL& other);
	TBType getType() const;	
};

#endif