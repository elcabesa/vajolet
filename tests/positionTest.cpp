#include <vector>
#include "gtest/gtest.h"

#include "./../data.h"
#include "./../tSquare.h"
#include "./../move.h"
#include "./../position.h"

TEST(PositionTest, isMoveLegal){
	Position pos;
	pos.setupFromFen("3r4/5pk1/2R3p1/7p/1bKP1P1P/r1p3P1/1pB5/1R6 b - - 2 46"); 
	
	Move m(B2, A1, Move::fpromotion, Move::promQueen);
	EXPECT_FALSE(pos.isMoveLegal(m));
	
	// todo move in a setup from fen test
	EXPECT_EQ(squareNone, pos.getActualStateConst().getEpSquare());
	
}

// todo move in data
TEST(PositionTest, bitSet){
	unsigned int i = 64;
	EXPECT_EQ(1, bitSet(A1));
	EXPECT_EQ(256, bitSet(A2));
	EXPECT_EQ(2, bitSet(B1));
	EXPECT_EQ(0, bitSet(squareNone));
	EXPECT_EQ(0, bitSet((tSquare)i));
	unsigned long long res = 1;
	for(tSquare s = A1; s<=squareNumber; ++s,res *=2 )
	{
		EXPECT_EQ(res, bitSet(s));
	}
}
