#include "gtest/gtest.h"
#include "syzygy/tbtable.h"
#include "syzygy/tbtableWDL.h"
#include "syzygy/tbtableDTZ.h"


TEST(tbtable, constructor1) {
	TBTableWDL tbt("KBNvKB");
	
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
	TBTableWDL tbt("KBBvKBB");
	
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
	TBTableWDL tbt("KBPPvKPPP");
	
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
	TBTableWDL tbt("KBvKPPP");
	
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
	TBTableWDL tbt("KBPPvKB");
	
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
	TBTableWDL t("KBPPvKB");
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
	TBTableWDL tbt("KNNvKB");
}

TEST(tbtable, map2) {
	TBTableWDL t("KNNvKB");
	TBTableDTZ tbt(t);
}
*/
TEST(tbtable, getType) {
	
	TBTableWDL t("KBPPvKB");
	TBTableDTZ t2(t);
	
	ASSERT_EQ(t2.getType(), DTZ);
	ASSERT_EQ(t.getType(), WDL);
}


TEST(tbtable, getEndGame) {
	ASSERT_EQ(TBTableWDL("KBPPvKB").getEndGame(), "KBPPvKB");
	ASSERT_EQ(TBTableWDL("KBPvK").getEndGame(), "KBPvK");
	ASSERT_EQ(TBTableWDL("KBPPvKB").getEndGame(), "KBPPvKB");
}
