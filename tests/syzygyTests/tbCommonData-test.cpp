#include "gtest/gtest.h"
#include "syzygy/tbCommonData.h"

TEST(tbCommonData, isOnDiagonalA1H8) {
	EXPECT_TRUE(TBCommonData::isOnDiagonalA1H8(B2));
	EXPECT_TRUE(TBCommonData::isOnDiagonalA1H8(E5));
	EXPECT_FALSE(TBCommonData::isOnDiagonalA1H8(D7));
	EXPECT_FALSE(TBCommonData::isOnDiagonalA1H8(H3));
}

TEST(tbCommonData, isAboveDiagonalA1H8) {
	EXPECT_FALSE(TBCommonData::isAboveDiagonalA1H8(B2));
	EXPECT_FALSE(TBCommonData::isAboveDiagonalA1H8(E5));
	EXPECT_TRUE(TBCommonData::isAboveDiagonalA1H8(D7));
	EXPECT_FALSE(TBCommonData::isAboveDiagonalA1H8(H3));
}

TEST(tbCommonData, isBelowDiagonalA1H8) {
	EXPECT_FALSE(TBCommonData::isBelowDiagonalA1H8(B2));
	EXPECT_FALSE(TBCommonData::isBelowDiagonalA1H8(E5));
	EXPECT_FALSE(TBCommonData::isBelowDiagonalA1H8(D7));
	EXPECT_TRUE(TBCommonData::isBelowDiagonalA1H8(H3));
}

TEST(tbCommonData, getBinomial) {
	EXPECT_EQ(TBCommonData::getBinomial(0, A1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(0, B1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(1, B1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(0, C1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(1, C1), 2);
	EXPECT_EQ(TBCommonData::getBinomial(2, C1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(0, D1), 1);
	EXPECT_EQ(TBCommonData::getBinomial(1, D1), 3);
	EXPECT_EQ(TBCommonData::getBinomial(2, D1), 3);
	EXPECT_EQ(TBCommonData::getBinomial(3, D1), 1);
	
	EXPECT_EQ(TBCommonData::getBinomial(1, A1), 0);
	EXPECT_EQ(TBCommonData::getBinomial(3, A1), 0);
	EXPECT_EQ(TBCommonData::getBinomial(5, A1), 0);
	EXPECT_EQ(TBCommonData::getBinomial(2, B1), 0);
}

TEST(tbCommonData, getMapA1D1D4) {
	EXPECT_EQ(TBCommonData::getMapA1D1D4(A2), 0);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(D1), 2);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(D2), 4);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(B2), 7);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(D4), 9);
	
	EXPECT_EQ(TBCommonData::getMapA1D1D4(F3), 0);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(G7), 0);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(A8), 0);
	EXPECT_EQ(TBCommonData::getMapA1D1D4(D5), 0);
}

TEST(tbCommonData, getMapB1H1H7) {
	EXPECT_EQ(TBCommonData::getMapB1H1H7(E2), 9);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(E4), 18);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(G4), 20);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(G6), 25);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(H7), 27);
	
	EXPECT_EQ(TBCommonData::getMapB1H1H7(B2), 0);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(F6), 0);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(B5), 0);
	EXPECT_EQ(TBCommonData::getMapB1H1H7(E7), 0);
}

TEST(tbCommonData, getMapKK) {
	EXPECT_EQ(TBCommonData::getMapKK(B1, D1), 0);
	EXPECT_EQ(TBCommonData::getMapKK(B1, E1), 1);
	EXPECT_EQ(TBCommonData::getMapKK(B1, C3), 12);
	EXPECT_EQ(TBCommonData::getMapKK(C1, E1), 59);
	
	EXPECT_EQ(TBCommonData::getMapKK(B2, B2), 0);
	EXPECT_EQ(TBCommonData::getMapKK(F7, A2), 0);
	EXPECT_EQ(TBCommonData::getMapKK(B2, D8), 0);
}

TEST(tbCommonData, getMapPawns) {
	EXPECT_EQ(TBCommonData::getMapPawns(A2), 47);
	EXPECT_EQ(TBCommonData::getMapPawns(H2), 46);
	EXPECT_EQ(TBCommonData::getMapPawns(A3), 45);
	EXPECT_EQ(TBCommonData::getMapPawns(H3), 44);
	
	EXPECT_EQ(TBCommonData::getMapPawns(A1), 0);
	EXPECT_EQ(TBCommonData::getMapPawns(D1), 0);
	EXPECT_EQ(TBCommonData::getMapPawns(F8), 0);
}

TEST(tbCommonData, getLeadPawnIdx) {
	EXPECT_EQ(TBCommonData::getLeadPawnIdx(1, A2), 0);
	EXPECT_NE(TBCommonData::getLeadPawnIdx(1, A3), 0);
	
	EXPECT_EQ(TBCommonData::getLeadPawnIdx(0, F3), 0);
	EXPECT_EQ(TBCommonData::getLeadPawnIdx(1, D8), 0);
}

TEST(tbCommonData, getLeadPawnsSize) {	
	EXPECT_NE(TBCommonData::getLeadPawnsSize(1, FILEA), 0);
	EXPECT_NE(TBCommonData::getLeadPawnsSize(3, FILEC), 0);
	
	EXPECT_EQ(TBCommonData::getLeadPawnsSize(0, FILED), 0);
	EXPECT_EQ(TBCommonData::getLeadPawnsSize(0, FILEC), 0);
	
	
}