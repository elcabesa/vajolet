#include "gtest/gtest.h"
#include "syzygy/tbtable.h"
#include "syzygy/tbtableWDL.h"
#include "syzygy/tbtableDTZ.h"


TEST(tbtable, constructor1) {
	TBTableWDL tbt("KBNvKB.rtbw");
	
	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_NE(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 5);
	ASSERT_EQ(tbt.getPawnCount(0), 0 );
	ASSERT_EQ(tbt.getPawnCount(1), 0 );
	ASSERT_FALSE(tbt.hasPawns());
	ASSERT_TRUE(tbt.hasUniquePieces());
}

TEST(tbtable, constructor2) {
	TBTableWDL tbt("KBBvKBB.rtbw");
	
	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_EQ(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 6);
	ASSERT_EQ(tbt.getPawnCount(0), 0 );
	ASSERT_EQ(tbt.getPawnCount(1), 0 );
	ASSERT_FALSE(tbt.hasPawns());
	ASSERT_FALSE(tbt.hasUniquePieces());
}

TEST(tbtable, constructor3) {
	TBTableWDL tbt("KBPPvKPPP.rtbw");
	
	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_NE(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 8);
	ASSERT_EQ(tbt.getPawnCount(0), 2 );
	ASSERT_EQ(tbt.getPawnCount(1), 3 );
	ASSERT_TRUE(tbt.hasPawns());
	ASSERT_TRUE(tbt.hasUniquePieces());
}

TEST(tbtable, constructor4) {
	TBTableWDL tbt("KBvKPPP.rtbw");
	
	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_NE(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 6);
	ASSERT_EQ(tbt.getPawnCount(0), 3 );
	ASSERT_EQ(tbt.getPawnCount(1), 0 );
	ASSERT_TRUE(tbt.hasPawns());
	ASSERT_TRUE(tbt.hasUniquePieces());
}

TEST(tbtable, constructor5) {
	TBTableWDL tbt("KBPPvKB.rtbw");
	
	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_NE(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 6);
	ASSERT_EQ(tbt.getPawnCount(0), 2 );
	ASSERT_EQ(tbt.getPawnCount(1), 0 );
	ASSERT_TRUE(tbt.hasPawns());
	ASSERT_TRUE(tbt.hasUniquePieces());
}

TEST(tbtable, copyConstructor) {
	TBTableWDL t("KBPPvKB.rtbw");
	TBTableDTZ tbt(t);

	ASSERT_NE(tbt.getKey(), HashKey(0));
	ASSERT_NE(tbt.getKey2(), HashKey(0));
	ASSERT_NE(tbt.getKey(), tbt.getKey2());
	ASSERT_EQ(tbt.getKey(), tbt.getKey());
	ASSERT_EQ(tbt.getKey2(), tbt.getKey2());
	ASSERT_EQ(tbt.getPieceCount(), 6);
	ASSERT_EQ(tbt.getPawnCount(0), 2 );
	ASSERT_EQ(tbt.getPawnCount(1), 0 );
	ASSERT_TRUE(tbt.hasPawns());
	ASSERT_TRUE(tbt.hasUniquePieces());
}

/*
TEST(tbtable, map) {
	TBTableWDL tbt("KNNvKB.rtbw");
}

TEST(tbtable, map2) {
	TBTableWDL t("KNNvKB.rtbz");
	TBTableDTZ tbt(t);
}
*/
TEST(tbtable, getType) {
	
	TBTableWDL t("KBPPvKB.rtbw");
	TBTableDTZ t2(t);
	
	ASSERT_EQ(t2.getType(), DTZ);
	ASSERT_EQ(t.getType(), WDL);
}


TEST(tbtable, getEndGame) {
	ASSERT_EQ(TBTableWDL("KBPPvKB.rtbw").getEndGame(), "KBPPvKB");
	ASSERT_EQ(TBTableWDL("KBPvK.rtb").getEndGame(), "KBPvK");
	ASSERT_EQ(TBTableWDL("KBPPvKB.rtbw").getEndGame(), "KBPPvKB");
}
