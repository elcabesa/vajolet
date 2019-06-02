#include "gtest/gtest.h"
#include "syzygy/tbpairs.h"


TEST(PairsData, setPiece) {
	PairsData p;
	
	p.setPiece(0, 4);
	p.setPiece(1, 14);
	ASSERT_EQ(p.getPiece(0), whiteRooks);
	ASSERT_EQ(p.getPiece(1), blackKing);
}
