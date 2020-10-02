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
#ifndef NNUE_H_
#define NNUE_H_ 

#include <set>
#include <array>

#include "bitBoardIndex.h"
#include "linear.h"
#include "relu.h"
#include "score.h"
#include "model.h"
#include "tSquare.h"
#include "vajolet.h"

class Position;
class SparseInput;

class DifferentialList {
public:
    void clear();
    void serialize(SparseInput& s, unsigned int offset);
    void add(unsigned int f);
    void remove(unsigned int f);
private:
    // TODO minimize size
    // TODO save max element stored
    std::array<unsigned int, 50> _addList;
    std::array<unsigned int, 50> _removeList;
    unsigned int _addPos = 0;
    unsigned int _removePos = 0;

};

class FeatureList {
public:
    void clear();
    //void serialize(SparseInput& s, unsigned int offset);
    void add(unsigned int f);
    unsigned int get(unsigned int index);
    unsigned int size();
private:

    // TODO minimize size
    // TODO save max element stored
    std::array<unsigned int, 100> _list;
    unsigned int _pos = 0;
};

class NNUE {
public:
    NNUE();
    bool load(std::string path);
    void clear();
    Score eval(const Position& pos);
    static bool loaded();
    static std::set<unsigned int> createFeatures(const Position& pos);
    void createWhiteFeatures(const Position& pos, FeatureList& fl);
    void createBlackFeatures(const Position& pos, FeatureList& fl);
    static unsigned int calcWhiteFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int calcBlackFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int whiteFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int blackFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int turnOffset(bool myturn);
    void concatenateFeature(FeatureList f1, FeatureList f2, FeatureList& complete);
    static unsigned int mapWhitePiece(const bitboardIndex piece);
    static unsigned int mapBlackPiece(const bitboardIndex piece);

    bool whiteNoIncrementalEval;
    bool blackNoIncrementalEval;

    void removePiece(const Position& pos, bitboardIndex piece, tSquare sq);
    void addPiece(const Position& pos, bitboardIndex piece, tSquare sq);
private:
    Model _modelW;
    Model _modelB;
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    Model _m;
#endif
    static bool _loaded;
    static std::vector<double> bias00;
    static std::vector<double> bias01;
    static std::vector<double> bias1;
    static std::vector<double> bias2;
    static std::vector<double> bias3;

    static std::vector<double> weight00;
    static std::vector<double> weight01;
    static std::vector<double> weight1;
    static std::vector<double> weight2;
    static std::vector<double> weight3;

    linearActivation _linear;
	reluActivation _relu;

    DifferentialList _whiteW;
    DifferentialList _blackW;
    DifferentialList _whiteB;
    DifferentialList _blackB;

    FeatureList _completeWhiteFeatureList;
    FeatureList _completeBlackFeatureList;
    FeatureList _completeFeatureList;

    //unsigned int incrementalCount = 0;
    //unsigned int completeCount = 0;
};



#endif
