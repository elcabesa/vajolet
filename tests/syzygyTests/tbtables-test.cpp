#include <stdexcept>
#include "gtest/gtest.h"

#include "position.h"
#include "syzygy/tbtables.h"

TEST(tbtables, constructor1) {
	TBFile::setPaths("data");
	TBTables t;
}

TEST(tbtables, add) {
	TBFile::setPaths("data");
	TBTables t;
	std::vector<bitboardIndex> pieces = {King, Knights, Knights, King, Bishops};
	t.add(pieces);
	ASSERT_EQ(t.getMaxCardinality(), 5);
	ASSERT_EQ(t.size(), 1);
}

TEST(tbtables, add2) {
	TBFile::setPaths("data");
	TBTables t;
	std::vector<bitboardIndex> pieces = {King, Knights, Knights, King, Bishops};
	t.add(pieces);
	std::vector<bitboardIndex> pieces2 = {King, Knights, King};
	t.add(pieces2);
	ASSERT_EQ(t.getMaxCardinality(), 5);
	ASSERT_EQ(t.size(), 1);
}

TEST(tbtables, clear) {
	TBFile::setPaths("data");
	TBTables t;
	std::vector<bitboardIndex> pieces = {King, Knights, Knights, King, Bishops};
	t.add(pieces);
	
	t.clear();
	
	ASSERT_EQ(t.getMaxCardinality(), 0);
	ASSERT_EQ(t.size(), 0);	
}

TEST(tbtables, getWDL) {
	Position p;
	p.setup("KNNKB", white);
	TBFile::setPaths("data");
	TBTables t;
	std::vector<bitboardIndex> pieces = {King, Knights, Knights, King, Bishops};
	t.add(pieces);
	ASSERT_EQ(t.getWDL(p.getMaterialKey()).getCompleteFileName(), "KNNvKB.rtbw");
}

TEST(tbtables, getDTZ) {
	Position p;
	p.setup("KNNKB", white);
	TBFile::setPaths("data");
	TBTables t;
	std::vector<bitboardIndex> pieces = {King, Knights, Knights, King, Bishops};
	t.add(pieces);
	ASSERT_EQ(t.getDTZ(p.getMaterialKey()).getCompleteFileName(), "KNNvKB.rtbz");
}

TEST(tbtablesDeath, getWDL) {
	Position p;
	p.setup("KNNKB", white);
	TBTables t;
	ASSERT_THROW(t.getWDL(p.getMaterialKey()), std::out_of_range);
}

TEST(tbtablesDeath, getDTZ) {
	Position p;
	p.setup("KNNKB", white);
	TBTables t;
	ASSERT_THROW(t.getDTZ(p.getMaterialKey()), std::out_of_range);
}
