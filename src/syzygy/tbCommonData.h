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

#ifndef TBCOMMON_DATA_H
#define TBCOMMON_DATA_H

#include "tSquare.h"

class TBCommonData {
private:
	static unsigned int _MapB1H1H7[squareNumber];
	static unsigned int _MapA1D1D4[squareNumber];
	static unsigned int _MapKK[10][squareNumber]; // [MapA1D1D4][SQUARE_NB]
	static unsigned int _Binomial[6][squareNumber]; // [k][n] k elements from a set of n elements
	static tSquare _MapPawns[squareNumber];
	static unsigned int _LeadPawnIdx[6][squareNumber]; // [leadPawnsCnt][SQUARE_NB]
	static unsigned int _LeadPawnsSize[6][4];       // [leadPawnsCnt][FILE_A..FILE_D]
	
	static void _initMapB1H1H7();
	static void _initMapA1D1D4();
	static void _initMapKK();
	static void _initBinomial();
	static void _initPawnsData();
	
	static int _offsetA1H8(const tSquare sq);
	
public:
	static void init();
	static bool isOnDiagonalA1H8(const tSquare sq);
	static bool isAboveDiagonalA1H8(const tSquare sq);
	static bool isBelowDiagonalA1H8(const tSquare sq);
	static unsigned int getMapB1H1H7(const tSquare sq);
	static unsigned int getMapA1D1D4(const tSquare sq);
	static unsigned int getMapKK(const tSquare sq1, const tSquare sq2);
	static unsigned int getBinomial(const unsigned int idx, const tSquare sq);
	static tSquare getMapPawns(const tSquare sq);
	static unsigned int getLeadPawnIdx(const unsigned int idx, const tSquare sq);
	static unsigned int getLeadPawnsSize(const unsigned int idx, const tFile file);
	
};

#endif