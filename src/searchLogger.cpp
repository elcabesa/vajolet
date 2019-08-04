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

logWriter::logWriter(std::string fen, const unsigned int depth, unsigned int iteration) {
	std::string fileName("log_");
	std::replace(fen.begin(), fen.end(), ' ', '_');
	std::replace(fen.begin(), fen.end(), '/', '-');
	fileName += fen + "_" + std::to_string(depth) + "_" + std::to_string(iteration) + ".log";
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

void logWriter::writeFloat(const float x) {
	_log << x;
}

void logWriter::writeMove(const Move& m) {
	_log << UciManager::displayUci(m, false);
}

unsigned int logNode::_ply = 0;

logNode::logNode(logWriter& lw, unsigned int ply, int depth, Score alpha, Score beta, std::string type): _lw(lw) {
	_indentate(_ply);
	_lw.writeString(type);
	_lw.writeChar('(');
	_lw.writeNumber(ply);
	_lw.writeChar(',');
	_lw.writeFloat(depth/16.0);
	_lw.writeChar(',');
	_lw.writeNumber(alpha);
	_lw.writeChar(',');
	_lw.writeNumber(beta);
	_lw.writeChar(')');
	_lw.writeChar('{');
	++_ply;
}

logNode::~logNode() {
	--_ply;
	_indentate(_ply);
	_lw.writeChar('}');
}

void logNode::startSection(const std::string& s) {
	_indentate(_ply);
	_lw.writeString("section ");
	_lw.writeString(s);
	_lw.writeChar('{');
	++_ply;
	
}
void logNode::endSection() {
	--_ply;
	_indentate(_ply);
	_lw.writeChar('}');
}

void logNode::_indentate(unsigned int ply) {
	_lw.writeChar('\n');
	for(unsigned int i = 0; i < ply; ++i) {_lw.writeChar('\t');}
}

void logNode::test(const std::string& s) {
	_indentate(_ply);
	_lw.writeString("test");
	_lw.writeString(s);
}

void logNode::doMove(const Move& m) {
	_indentate(_ply);
	_lw.writeString("do move ");
	_lw.writeMove(m);
	_lw.writeChar('{');
	++_ply;
}

void logNode::undoMove() {
	--_ply;
	_indentate(_ply);
	_lw.writeChar('}');
	
}

void logNode::skipMove(const Move& m, const std::string& s) {
	_indentate(_ply);
	_lw.writeMove(m);
	_lw.writeString(" skipped due to ");
	_lw.writeString(s);
}

void logNode::raisedAlpha() {
	_indentate(_ply);
	_lw.writeString("raised alpha");
}

void logNode::isImproving() {
	_indentate(_ply);
	_lw.writeString("is improving");
}

void logNode::raisedbestScore() {
	_indentate(_ply);
	_lw.writeString("raised bestScore");
}

void logNode::logReturnValue(Score val) {
	_indentate(_ply);
	_lw.writeString("return: ");
	_lw.writeNumber(val);
}

void logNode::logTTprobe(const ttEntry& tte) {
	_indentate(_ply);
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
	_indentate(_ply);
	_lw.writeString("Static Eval: ");
	_lw.writeNumber(eval);
}

void logNode::refineEval(Score eval) {
	_indentate(_ply);
	_lw.writeString("refined Eval: ");
	_lw.writeNumber(eval);
}

void logNode::calcBestScore(Score eval) {
	_indentate(_ply);
	_lw.writeString("BestScore: ");
	_lw.writeNumber(eval);
}

void logNode::ExtendedDepth() {
	_indentate(_ply);
	_lw.writeString("extended depth");
}

void logNode::doLmrSearch() {
	_indentate(_ply);
	_lw.writeString("do lmr search");
}

void logNode::doFullDepthSearchSearch() {
	_indentate(_ply);
	_lw.writeString("do full depth search");
}

void logNode::doFullWidthSearchSearch() {
	_indentate(_ply);
	_lw.writeString("do full width search");
}
