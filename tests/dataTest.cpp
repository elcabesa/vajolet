#include <vector>
#include "gtest/gtest.h"

#include "./../data.h"





TEST(dataTest, bitSet){
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
