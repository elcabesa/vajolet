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

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include "vajolet.h"
#include "io.h"
#include "position.h"
#include "search.h"


class Statistics{
public :
	static Statistics& instance(){
	  static Statistics instance;
	  return instance;
	}

	unsigned long long testedNodeTypeCut;
	unsigned long long testedNodeTypeAll;
	unsigned long long testedNodeTypePv;
	unsigned long long resultNodeTypeCut;
	unsigned long long resultNodeTypeAll;
	unsigned long long resultNodeTypePv;

	void gatherNodeTypeStat(search::nodeType expectedNodeType,search::nodeType resultNodeType);
	void printNodeTypeStat();
	void initNodeTypeStat();
private:
	Statistics(){}
};



#endif /* STATISTICS_H_ */
