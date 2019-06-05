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
#include <algorithm>

#include "command.h"
#include "move.h"
#include "searchLogger.h"
#include "transposition.h"

logWriter::logWriter(std::string fen, const unsigned int depth) {
	std::string fileName("log_");
	std::replace(fen.begin(), fen.end(), ' ', '_');
	std::replace(fen.begin(), fen.end(), '/', '-');
	fileName += fen + "_" + std::to_string(depth) + ".log";
	_log.open(fileName, std::ofstream::trunc);
}

logWriter::~logWriter() {
	if (_log.is_open()) {
		_log.close();
	}
}

void logWriter::writeString(const std::string& st) {
	_log << st;
}

void logWriter::writeChar(const char c) {
	_log << c;
}

void logWriter::writeNumber(const long long x) {
	_log << x;
}

void logWriter::writeMove(const Move& m) {
	_log << UciManager::displayUci(m, false);
}


logNode::logNode(logWriter& lw, unsigned int ply, int depth, Score alpha, Score beta): _lw(lw), _ply(ply) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i < _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeChar('{');
	_lw.writeNumber(ply);
	_lw.writeChar(',');
	_lw.writeNumber(depth);
	_lw.writeChar(',');
	_lw.writeNumber(alpha);
	_lw.writeChar(',');
	_lw.writeNumber(beta);
	_lw.writeChar(';');
}

logNode::~logNode() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i < _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeChar('}');
}


void logNode::testIsDraw() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("testIsDraw");
}

void logNode::testCanUseTT() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("testCanUseTT");
}

void logNode::testStandPat() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("testStandPat");
}

void logNode::testMated() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("is mated?");
}

void logNode::testMove(Move& m) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("testing move ");
	_lw.writeMove(m);
}

void logNode::skipMove() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("...skipped");
}

void logNode::raisedAlpha() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("raised alpha");
}

void logNode::raisedbestScore() {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("raised bestScore");
}

void logNode::logReturnValue(Score val) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("return: ");
	_lw.writeNumber(val);
}

void logNode::logTTprobe(const ttEntry& tte) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("TTprobe v: ");
	_lw.writeNumber(tte.getValue());
	_lw.writeString(" sv: ");
	_lw.writeNumber(tte.getStaticValue());
	_lw.writeString(" move: ");
	_lw.writeMove(Move(tte.getPackedMove()));
	_lw.writeString(" depth: ");
	_lw.writeNumber(tte.getDepth());
	_lw.writeString(" type: ");
	_lw.writeNumber(tte.getType());
	
}

void logNode::calcStaticEval(Score eval) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("Static Eval: ");
	_lw.writeNumber(eval);
}

void logNode::calcBestScore(Score eval) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i <= _ply; ++i) {_lw.writeChar('\t');}
	_lw.writeString("BestScore: ");
	_lw.writeNumber(eval);
}