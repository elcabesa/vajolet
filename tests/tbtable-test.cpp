#include "gtest/gtest.h"
#include "syzygy/tbtable.h"


TEST(tbtable, constructor1) {
	TBTable tbt("KBNvKB");
	
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
	TBTable tbt("KBBvKBB");
	
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
	TBTable tbt("KBPPvKPPP");
	
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
	TBTable tbt("KBvKPPP");
	
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
	TBTable tbt("KBPPvKB");
	
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
	TBTable t("KBPPvKB");
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