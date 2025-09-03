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
#ifndef FENSAVER_H_
#define FENSAVER_H_

#include <iostream>
#include <fstream>
#include <set>

#include "position.h"
#include "searchLimits.h"
#include "searchTimer.h"
#include "search.h"
#include "transposition.h"


class FenSaver {
	
public:
	FenSaver(unsigned int decimation, unsigned int n);
	void save(Position& pos);

private:	

	struct QsearchRes{
		std::string fen;
		std::set<unsigned int> features;
		bool invertRes;
		Score res;
	};

	const unsigned int _decimation;
	unsigned int _counter = 0;
	std::ofstream _stream;
	SearchLimits _sl;
	SearchTimer _st;
	transpositionTable _tt;
	Search _src;
	Position _pos;
	uint_fast64_t _saved = 0;
	unsigned int _logDecimationCnt = 0;
    
	void writeFen(QsearchRes& res);
	void writeRes(Score res);
    
    std::set<unsigned int> featuresIndex;
	unsigned int _n;
	double _totalError = 0.0;
	unsigned long long _totalCnt = 0;
	unsigned long long _highDiffCnt = 0;

	QsearchRes _getQuiescentPosFeatures(const Position& p);
	Score _getSearchRes(const Position& p);
};

#endif /* SELFPLAY_H_ */
