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

#include "gtest/gtest.h"
#include "nnue.h"

TEST(NNUE, calcWhiteFeature)
{  
    std::set<unsigned int> featuresIndex;
    for(tSquare ksq = tSquare::A1; ksq != squareNumber; ++ksq) {
        for(tSquare pSquare = tSquare::A1; pSquare != squareNumber; ++pSquare) {
            for(unsigned int piece = 0; piece <10; ++piece) {
                featuresIndex.insert(NNUE::calcWhiteFeature(false, piece, pSquare, ksq));
                featuresIndex.insert(NNUE::calcWhiteFeature(true, piece, pSquare, ksq));
            }
        }
    }
    ASSERT_EQ(featuresIndex.size(), 40960*2);
}

TEST(NNUE, calcBlackFeature)
{  
    std::set<unsigned int> featuresIndex;
    for(tSquare ksq = tSquare::A1; ksq != squareNumber; ++ksq) {
        for(tSquare pSquare = tSquare::A1; pSquare != squareNumber; ++pSquare) {
            for(unsigned int piece = 0; piece <10; ++piece) {
                featuresIndex.insert(NNUE::calcBlackFeature(false, piece, pSquare, ksq));
                featuresIndex.insert(NNUE::calcBlackFeature(true, piece, pSquare, ksq));
            }
        }
    }
    ASSERT_EQ(featuresIndex.size(), 40960*2);
}

TEST(NNUE, EquivalentFeature) {
     ASSERT_EQ(
         NNUE::calcWhiteFeature(true, 1, tSquare::A7, tSquare::E4),
         NNUE::calcBlackFeature(false, 1, tSquare::A2, tSquare::E5)
    );
}



