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
#include "./../hashKey.h"

TEST(dhashKeyTest, keysNotNull) {
	
	HashKey k(0);
	
	for (tSquare sq = A1; sq < squareNumber; ++sq) {
		for (bitboardIndex piece = empty; piece < lastBitboard; ++piece) {
			k.updatePiece(sq, piece);
			EXPECT_NE(k.getKey(), 0);
			k.updatePiece(sq, piece);
			EXPECT_EQ(k.getKey(), 0);
		}
	}
	
	k.changeSide();
	EXPECT_NE(k.getKey(), 0);
	k.changeSide();
	EXPECT_EQ(k.getKey(), 0);
	
	for (tSquare sq = A1; sq < squareNumber; ++sq) {
		k.changeEp(sq);
		EXPECT_NE(k.getKey(), 0);
		k.changeEp(sq);
		EXPECT_EQ(k.getKey(), 0);
	}
	
	k.setCastlingRight(0);
	EXPECT_EQ(k.getKey(), 0);
	k.setCastlingRight(0);
	EXPECT_EQ(k.getKey(), 0);
	
	for (unsigned int x = 1; x < 16; ++x) {
		k.setCastlingRight(x);
		EXPECT_NE(k.getKey(), 0);
		k.setCastlingRight(x);
		EXPECT_EQ(k.getKey(), 0);
	}
	
	EXPECT_NE(k.getExclusionKey().getKey(), 0);

}

TEST(dhashKeyTest, testEqual) {
	
	HashKey k1(175473);
	HashKey k2(175473);
	HashKey k3(420588);
	
	ASSERT_EQ(k1, k2);
	ASSERT_NE(k1, k3);
	
}
