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
#include "./../history.h"

TEST(historyTest, clear) {
	
	History h;
	h.clear();
	
	for (Color c = white; c <= black; ++c) {
		for (tSquare from = A1; from < squareNumber; ++from) {
			for (tSquare to = A1; to < squareNumber; ++to) {
				ASSERT_EQ(h.getValue(c, Move(from, to)), 0);
			}
		}
	} 	
}

TEST(historyTest, update) {
	
	History h;
	h.clear();
	
	Move m1(A3,C6);
	h.update(white, m1, 300);
	
	ASSERT_NE(h.getValue(white, m1), 0); 	
}

TEST(historyTest, updateColorIndipendency) {
	
	History h;
	h.clear();
	
	Move m1(F4,D7);
	h.update(white, m1, -20);
	h.update(black, m1, 200);
	
	ASSERT_NE(h.getValue(white, m1), h.getValue(black, m1)); 	
}

TEST(historyTest, updateMoveIndipendency) {
	
	History h;
	h.clear();
	
	Move m1(F4,D7);
	Move m2(C8,D5);
	h.update(black, m1, -20);
	auto r1 = h.getValue(black, m1);
	h.update(black, m2, 200);
	auto r2 = h.getValue(black, m2);
	
	ASSERT_NE(r1, r2); 	
}

TEST(historyTest, updateMoveEvolution) {
	
	History h;
	h.clear();
	
	Move m(F4,D7);


	auto r = h.getValue(black, m);
	for (int n = 0; n < 200; ++n) {
		h.update(black, m, 20);
		auto r1 = h.getValue(black, m);
		ASSERT_GE(r1, r); 	
		r = r1;
	}
	ASSERT_EQ(r, 16000); 	
	
	for (int n = 0; n < 200; ++n) {
		h.update(black, m, -20);
		auto r1 = h.getValue(black, m);
		ASSERT_LE(r1, r); 	
		r = r1;
	}
	ASSERT_EQ(r, -16000); 	

}