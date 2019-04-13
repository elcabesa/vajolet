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
#include "data.h"

TEST(dataTest, bitSet){
	
	EXPECT_EQ(1, bitSet(A1));
	EXPECT_EQ(256, bitSet(A2));
	EXPECT_EQ(2, bitSet(B1));
	EXPECT_EQ(0, bitSet(squareNone));
	EXPECT_EQ(0, bitSet((tSquare)64));
	unsigned long long res = 1;
	for(tSquare s = A1; s<=squareNumber; ++s,res *=2 )
	{
		EXPECT_EQ(res, bitSet(s));
	}
}
