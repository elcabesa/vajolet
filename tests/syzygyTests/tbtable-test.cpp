#include "gtest/gtest.h"

#include "tSquare.h"
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


TEST(tbtable, map) {
	TBFile::setPaths("data");
	TBTableWDL tbt("KNNvKB");
	ASSERT_TRUE(tbt.mapFile());
	
	auto& pd = tbt.getPairsData(0, FILEA);
	ASSERT_EQ(pd.getPiece(0), whiteKing);
	ASSERT_EQ(pd.getPiece(1), blackKing);
	ASSERT_EQ(pd.getPiece(2), blackBishops);
	ASSERT_EQ(pd.getPiece(3), whiteKnights);
	ASSERT_EQ(pd.getPiece(4), whiteKnights);
	
	ASSERT_EQ(pd.getGroupLen(0), 3);
	ASSERT_EQ(pd.getGroupLen(1), 2);
	ASSERT_EQ(pd.getGroupLen(2), 0);
	ASSERT_EQ(pd.getGroupLen(3), 0);
	ASSERT_EQ(pd.getGroupLen(4), 0);
	ASSERT_EQ(pd.getGroupLen(5), 0);
	ASSERT_EQ(pd.getGroupLen(6), 0);
	ASSERT_EQ(pd.getGroupLen(7), 0);
	
	ASSERT_EQ(pd.getGroupIdx(0), 1);
	ASSERT_EQ(pd.getGroupIdx(1), 31332);
	ASSERT_EQ(pd.getGroupIdx(2), 57337560);
	ASSERT_EQ(pd.getGroupIdx(3), 0);
	ASSERT_EQ(pd.getGroupIdx(4), 0);
	ASSERT_EQ(pd.getGroupIdx(5), 0);
	ASSERT_EQ(pd.getGroupIdx(6), 0);
	ASSERT_EQ(pd.getGroupIdx(7), 0);
	
	ASSERT_EQ(pd.getFlags(), 0);
	ASSERT_EQ(pd.getSizeofBlock(), 32);
	ASSERT_EQ(pd.getSpan(), 1048576);
	ASSERT_EQ(pd.getSparseIndexSize(), 55);
	ASSERT_EQ(pd.getBlocksNum(), 1016);
	ASSERT_EQ(pd.getBlockLengthSize(), 1016);
	ASSERT_EQ(pd.getMaxSymLen(), 14);
	ASSERT_EQ(pd.getMinSymLen(), 1);
	//ASSERT_EQ((uint64_t)pd.getLowestSym(), 0x180016);
	
	ASSERT_EQ(pd.getBase64(0), 9223372036854775808ull);
	ASSERT_EQ(pd.getBase64(1), 9223372036854775808ull);
	ASSERT_EQ(pd.getBase64(2), 9223372036854775808ull);
	ASSERT_EQ(pd.getBase64(3), 9223372036854775808ull);
	ASSERT_EQ(pd.getBase64(4), 9223372036854775808ull);
	ASSERT_EQ(pd.getBase64(5), 8358680908399640576ull);
	ASSERT_EQ(pd.getBase64(6), 5188146770730811392ull);
	ASSERT_EQ(pd.getBase64(7), 3314649325744685056ull);
	ASSERT_EQ(pd.getBase64(8), 1549238271815450624ull);
	ASSERT_EQ(pd.getBase64(9), 108086391056891904ull);
	ASSERT_EQ(pd.getBase64(10), 63050394783186944ull);
	ASSERT_EQ(pd.getBase64(11), 18014398509481984ull);
	ASSERT_EQ(pd.getBase64(12), 2251799813685248ull);
	ASSERT_EQ(pd.getBase64(13), 0ull);
	
	ASSERT_EQ(pd.getSymLen(0), 0);
	ASSERT_EQ(pd.getSymLen(1), 14);
	ASSERT_EQ(pd.getSymLen(2), 11);
	ASSERT_EQ(pd.getSymLen(3), 8);
	ASSERT_EQ(pd.getSymLen(4), 72);
	ASSERT_EQ(pd.getSymLen(5), 17);
	ASSERT_EQ(pd.getSymLen(6), 13);
	ASSERT_EQ(pd.getSymLen(9), 8);
	ASSERT_EQ(pd.getSymLen(10), 95);
	ASSERT_EQ(pd.getSymLen(11), 8);
	ASSERT_EQ(pd.getSymLen(12), 97);
	ASSERT_EQ(pd.getSymLen(13), 26);
	ASSERT_EQ(pd.getSymLen(14), 15);
	ASSERT_EQ(pd.getSymLen(15), 10);
	ASSERT_EQ(pd.getSymLen(16), 17);
	ASSERT_EQ(pd.getSymLen(18), 17);
	ASSERT_EQ(pd.getSymLen(19), 17);
	ASSERT_EQ(pd.getSymLen(20), 41);
	ASSERT_EQ(pd.getSymLen(21), 17);
	ASSERT_EQ(pd.getSymLen(22), 10);
	ASSERT_EQ(pd.getSymLen(27), 25);
	ASSERT_EQ(pd.getSymLen(28), 169);
	ASSERT_EQ(pd.getSymLen(30), 111);
	ASSERT_EQ(pd.getSymLen(31), 9);
	ASSERT_EQ(pd.getSymLen(32), 107);
	ASSERT_EQ(pd.getSymLen(33), 78);
	ASSERT_EQ(pd.getSymLen(34), 9);
	ASSERT_EQ(pd.getSymLen(35), 235);
	ASSERT_EQ(pd.getSymLen(36), 111);
	ASSERT_EQ(pd.getSymLen(38), 11);
	ASSERT_EQ(pd.getSymLen(39), 187);
	ASSERT_EQ(pd.getSymLen(40), 17);
	ASSERT_EQ(pd.getSymLen(41), 20);
	ASSERT_EQ(pd.getSymLen(42), 103);
	ASSERT_EQ(pd.getSymLen(43), 14);
	ASSERT_EQ(pd.getSymLen(44), 13);
	ASSERT_EQ(pd.getSymLen(45), 12);
	ASSERT_EQ(pd.getSymLen(46), 17);
	ASSERT_EQ(pd.getSymLen(47), 29);
	ASSERT_EQ(pd.getSymLen(48), 104);
	ASSERT_EQ(pd.getSymLen(49), 66);
	ASSERT_EQ(pd.getSymLen(50), 6);
	ASSERT_EQ(pd.getSymLen(51), 8);
	ASSERT_EQ(pd.getSymLen(52), 226);
	ASSERT_EQ(pd.getSymLen(53), 16);
	ASSERT_EQ(pd.getSymLen(56), 125);
	ASSERT_EQ(pd.getSymLen(57), 180);
	ASSERT_EQ(pd.getSymLen(58), 231);
	ASSERT_EQ(pd.getSymLen(59), 216);
	ASSERT_EQ(pd.getSymLen(60), 168);
	ASSERT_EQ(pd.getSymLen(61), 28);
	ASSERT_EQ(pd.getSymLen(62), 223);
	ASSERT_EQ(pd.getSymLen(63), 123);
	ASSERT_EQ(pd.getSymLen(64), 16);
	ASSERT_EQ(pd.getSymLen(65), 13);
	ASSERT_EQ(pd.getSymLen(67), 130);
	ASSERT_EQ(pd.getSymLen(68), 10);
	ASSERT_EQ(pd.getSymLen(69), 40);
	ASSERT_EQ(pd.getSymLen(70), 35);
	ASSERT_EQ(pd.getSymLen(71), 12);
	ASSERT_EQ(pd.getSymLen(72), 128);
	ASSERT_EQ(pd.getSymLen(73), 37);
	ASSERT_EQ(pd.getSymLen(74), 17);
	ASSERT_EQ(pd.getSymLen(75), 138);
	ASSERT_EQ(pd.getSymLen(76), 8);
	ASSERT_EQ(pd.getSymLen(77), 80);
	ASSERT_EQ(pd.getSymLen(78), 249);
	ASSERT_EQ(pd.getSymLen(80), 7);
	ASSERT_EQ(pd.getSymLen(81), 1);
	ASSERT_EQ(pd.getSymLen(82), 7);
	ASSERT_EQ(pd.getSymLen(84), 63);
	ASSERT_EQ(pd.getSymLen(86), 59);
	ASSERT_EQ(pd.getSymLen(87), 221);
	ASSERT_EQ(pd.getSymLen(88), 8);
	ASSERT_EQ(pd.getSymLen(89), 15);
	ASSERT_EQ(pd.getSymLen(90), 5);
	ASSERT_EQ(pd.getSymLen(91), 38);
	ASSERT_EQ(pd.getSymLen(92), 108);
	ASSERT_EQ(pd.getSymLen(94), 32);
	ASSERT_EQ(pd.getSymLen(95), 24);
	ASSERT_EQ(pd.getSymLen(97), 26);
	ASSERT_EQ(pd.getSymLen(98), 5);
	ASSERT_EQ(pd.getSymLen(99), 16);
	ASSERT_EQ(pd.getSymLen(100), 69);
	ASSERT_EQ(pd.getSymLen(101), 6);
	ASSERT_EQ(pd.getSymLen(102), 26);
	ASSERT_EQ(pd.getSymLen(103), 123);
	ASSERT_EQ(pd.getSymLen(104), 78);
	ASSERT_EQ(pd.getSymLen(105), 12);
	ASSERT_EQ(pd.getSymLen(106), 10);
	ASSERT_EQ(pd.getSymLen(107), 16);
	ASSERT_EQ(pd.getSymLen(108), 200);
	ASSERT_EQ(pd.getSymLen(109), 2);
	ASSERT_EQ(pd.getSymLen(110), 26);
	ASSERT_EQ(pd.getSymLen(112), 6);
	ASSERT_EQ(pd.getSymLen(113), 113);
	ASSERT_EQ(pd.getSymLen(115), 15);
	ASSERT_EQ(pd.getSymLen(117), 127);
	ASSERT_EQ(pd.getSymLen(118), 103);
	ASSERT_EQ(pd.getSymLen(125), 203);
	ASSERT_EQ(pd.getSymLen(126), 6);
	ASSERT_EQ(pd.getSymLen(127), 8);
	ASSERT_EQ(pd.getSymLen(128), 21);
	ASSERT_EQ(pd.getSymLen(129), 5);
	ASSERT_EQ(pd.getSymLen(130), 8);
	ASSERT_EQ(pd.getSymLen(132), 193);
	ASSERT_EQ(pd.getSymLen(133), 192);
	ASSERT_EQ(pd.getSymLen(137), 143);
	ASSERT_EQ(pd.getSymLen(139), 119);
	ASSERT_EQ(pd.getSymLen(141), 6);
	ASSERT_EQ(pd.getSymLen(142), 8);
	ASSERT_EQ(pd.getSymLen(145), 223);
	ASSERT_EQ(pd.getSymLen(146), 89);
	ASSERT_EQ(pd.getSymLen(147), 99);
	ASSERT_EQ(pd.getSymLen(149), 4);
	ASSERT_EQ(pd.getSymLen(151), 11);
	ASSERT_EQ(pd.getSymLen(154), 241);
	ASSERT_EQ(pd.getSymLen(155), 7);
	ASSERT_EQ(pd.getSymLen(158), 243);
	ASSERT_EQ(pd.getSymLen(159), 199);
	ASSERT_EQ(pd.getSymLen(161), 6);
	ASSERT_EQ(pd.getSymLen(183), 143);
	ASSERT_EQ(pd.getSymLen(202), 65);
	ASSERT_EQ(pd.getSymLen(204), 255);
	

	auto &pd2 = tbt.getPairsData(1, FILEA);
	ASSERT_EQ(pd2.getPiece(0), whiteKing);
	ASSERT_EQ(pd2.getPiece(1), blackKing);
	ASSERT_EQ(pd2.getPiece(2), blackBishops);
	ASSERT_EQ(pd2.getPiece(3), whiteKnights);
	ASSERT_EQ(pd2.getPiece(4), whiteKnights);
	
	ASSERT_EQ(pd2.getGroupLen(0), 3);
	ASSERT_EQ(pd2.getGroupLen(1), 2);
	ASSERT_EQ(pd2.getGroupLen(2), 0);
	ASSERT_EQ(pd2.getGroupLen(3), 0);
	ASSERT_EQ(pd2.getGroupLen(4), 0);
	ASSERT_EQ(pd2.getGroupLen(5), 0);
	ASSERT_EQ(pd2.getGroupLen(6), 0);
	ASSERT_EQ(pd2.getGroupLen(7), 0);
	
	ASSERT_EQ(pd2.getGroupIdx(0), 1);
	ASSERT_EQ(pd2.getGroupIdx(1), 31332);
	ASSERT_EQ(pd2.getGroupIdx(2), 57337560);
	ASSERT_EQ(pd2.getGroupIdx(3), 0);
	ASSERT_EQ(pd2.getGroupIdx(4), 0);
	ASSERT_EQ(pd2.getGroupIdx(5), 0);
	ASSERT_EQ(pd2.getGroupIdx(6), 0);
	ASSERT_EQ(pd2.getGroupIdx(7), 0);
	
	ASSERT_EQ(pd2.getFlags(), 0);
	ASSERT_EQ(pd2.getSizeofBlock(), 32);
	ASSERT_EQ(pd2.getSpan(), 1048576);
	ASSERT_EQ(pd2.getSparseIndexSize(), 55);
	ASSERT_EQ(pd2.getBlocksNum(), 908);
	ASSERT_EQ(pd2.getBlockLengthSize(), 908);
	ASSERT_EQ(pd2.getMaxSymLen(), 10);
	ASSERT_EQ(pd2.getMinSymLen(), 1);
	/*ASSERT_EQ((uint64_t)pd2.getLowestSym(), 0x1802b2);*/
	
	ASSERT_EQ(pd2.getBase64(0), 9223372036854775808ull);
	ASSERT_EQ(pd2.getBase64(1), 9223372036854775808ull);
	ASSERT_EQ(pd2.getBase64(2), 9223372036854775808ull);
	ASSERT_EQ(pd2.getBase64(3), 9223372036854775808ull);
	ASSERT_EQ(pd2.getBase64(4), 8646911284551352320ull);
	ASSERT_EQ(pd2.getBase64(5), 5188146770730811392ull);
	ASSERT_EQ(pd2.getBase64(6), 2594073385365405696ull);
	ASSERT_EQ(pd2.getBase64(7), 72057594037927936ull);
	ASSERT_EQ(pd2.getBase64(8), 36028797018963968ull);
	ASSERT_EQ(pd2.getBase64(9), 0ull);
	
	
	ASSERT_EQ(pd2.getSymLen(0), 63);
	ASSERT_EQ(pd2.getSymLen(1), 95);
	ASSERT_EQ(pd2.getSymLen(3), 159);
	ASSERT_EQ(pd2.getSymLen(4), 223);
	ASSERT_EQ(pd2.getSymLen(5), 11);
	ASSERT_EQ(pd2.getSymLen(6), 6);
	ASSERT_EQ(pd2.getSymLen(7), 0);
	ASSERT_EQ(pd2.getSymLen(8), 231);
	ASSERT_EQ(pd2.getSymLen(9), 8);
	ASSERT_EQ(pd2.getSymLen(10), 8);
	ASSERT_EQ(pd2.getSymLen(11), 215);
	ASSERT_EQ(pd2.getSymLen(12), 239);
	ASSERT_EQ(pd2.getSymLen(13), 247);
	ASSERT_EQ(pd2.getSymLen(15), 8);
	ASSERT_EQ(pd2.getSymLen(16), 4);
	ASSERT_EQ(pd2.getSymLen(17), 14);
	ASSERT_EQ(pd2.getSymLen(18), 143);
	ASSERT_EQ(pd2.getSymLen(19), 159);
	ASSERT_EQ(pd2.getSymLen(21), 7);
	ASSERT_EQ(pd2.getSymLen(22), 3);
	ASSERT_EQ(pd2.getSymLen(23), 111);
	ASSERT_EQ(pd2.getSymLen(24), 175);
	ASSERT_EQ(pd2.getSymLen(26), 199);
	ASSERT_EQ(pd2.getSymLen(27), 183);
	ASSERT_EQ(pd2.getSymLen(29), 55);
	ASSERT_EQ(pd2.getSymLen(30), 6);
	ASSERT_EQ(pd2.getSymLen(31), 8);
	ASSERT_EQ(pd2.getSymLen(32), 119);
	ASSERT_EQ(pd2.getSymLen(35), 79);
	ASSERT_EQ(pd2.getSymLen(36), 87);
	ASSERT_EQ(pd2.getSymLen(38), 2);
	ASSERT_EQ(pd2.getSymLen(40), 8);
	ASSERT_EQ(pd2.getSymLen(41), 2);
	ASSERT_EQ(pd2.getSymLen(42), 39);
	ASSERT_EQ(pd2.getSymLen(43), 6);
	ASSERT_EQ(pd2.getSymLen(54), 65);
	ASSERT_EQ(pd2.getSymLen(58), 7);
	ASSERT_EQ(pd2.getSymLen(61), 5);
	ASSERT_EQ(pd2.getSymLen(63), 108);
	ASSERT_EQ(pd2.getSymLen(65), 2);
	ASSERT_EQ(pd2.getSymLen(66), 3);
	ASSERT_EQ(pd2.getSymLen(67), 3);
	ASSERT_EQ(pd2.getSymLen(69), 255);
	

}

TEST(tbtable, map2) {
	TBFile::setPaths("data");
	TBTableWDL t("KNNvKB");
	TBTableDTZ tbt(t);
	ASSERT_TRUE(tbt.mapFile());
	
	auto& pd = tbt.getPairsData(0, FILEA);
	ASSERT_EQ(pd.getPiece(0), whiteKing);
	ASSERT_EQ(pd.getPiece(1), blackKing);
	ASSERT_EQ(pd.getPiece(2), blackBishops);
	ASSERT_EQ(pd.getPiece(3), whiteKnights);
	ASSERT_EQ(pd.getPiece(4), whiteKnights);
	
	auto& pd2 = tbt.getPairsData(1, FILEA);
	ASSERT_EQ(pd2.getPiece(0), whiteKing);
	ASSERT_EQ(pd2.getPiece(1), blackKing);
	ASSERT_EQ(pd2.getPiece(2), blackBishops);
	ASSERT_EQ(pd2.getPiece(3), whiteKnights);
	ASSERT_EQ(pd2.getPiece(4), whiteKnights);
}

TEST(tbtable, map3) {
	TBFile::setPaths("data");
	TBTableWDL t("KRRvKR");
	TBTableDTZ tbt(t);
	ASSERT_FALSE(tbt.mapFile());
}

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

