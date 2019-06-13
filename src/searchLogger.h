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
#ifndef SEARCH_LOGGER_H_
#define SEARCH_LOGGER_H_

#include <fstream>
#include <iostream>

#include "score.h"

class ttEntry;
class Move;

class logWriter {
public:
	logWriter(std::string fen, const unsigned int depth, unsigned int iteration);
	~logWriter();
	void writeString(const std::string & st);
	void writeChar(const char c);
	void writeNumber(const long long x);
	void writeMove(const Move& m);
private:
	std::ofstream _log;
};

class logNode {
public:
	logNode(logWriter& lw, unsigned int ply, int depth, Score alpha, Score beta);
	~logNode();
	void testIsDraw();
	void testMateDistancePruning();
	void testCanUseTT();
	void testCheckTablebase();
	void testStandPat();
	void testMated();
	void testPruning();
	void testRazoring();
	void testStaticNullMovePruning();
	void testNullMovePruning();
	void testDoVerification();
	void raisedAlpha();
	void raisedbestScore();
	void isImproving();
	void testMove(const Move& m);
	void skipMove();
	void calcStaticEval(Score eval);
	void calcBestScore(Score eval);
	void refineEval(Score eval);
	void logTTprobe(const ttEntry& tte);
	void logReturnValue(Score val);
private:
	void _indentate(unsigned int ply);
	logWriter& _lw;
	unsigned int _ply;
};

#endif