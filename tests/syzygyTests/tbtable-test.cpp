#include "gtest/gtest.h"
#include "syzygy/tbtable.h"


TEST(tbtable, constructor1) {
	TBTable tbt("KBNvKB.rtbw");
	
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
	TBTable tbt("KBBvKBB.rtbw");
	
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
	TBTable tbt("KBPPvKPPP.rtbw");
	
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
	TBTable tbt("KBvKPPP.rtbw");
	
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
	TBTable tbt("KBPPvKB.rtbz");
	
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
	TBTable t("KBPPvKB.rtbz");
	TBTable tbt(t);

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

TEST(tbtable, map) {
	TBTable tbt("KNNvKB.rtbw");
}

TEST(tbtable, map2) {
	TBTable tbt("KNNvKB.rtbz");
}

TEST(tbtable, getType) {
	ASSERT_EQ(TBTable("KBPPvKB.rtbz").getType(), DTZ);
	ASSERT_EQ(TBTable("KBPvK.rtbz").getType(), DTZ);
	ASSERT_EQ(TBTable("KBPPvKB.rtbw").getType(), WDL);
}

TEST(tbtable, getEndGame) {
	ASSERT_EQ(TBTable("KBPPvKB.rtbz").getEndGame(), "KBPPvKB");
	ASSERT_EQ(TBTable("KBPvK.rtbz").getEndGame(), "KBPvK");
	ASSERT_EQ(TBTable("KBPPvKB.rtbw").getEndGame(), "KBPPvKB");
}
