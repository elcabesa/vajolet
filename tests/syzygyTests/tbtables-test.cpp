#include <stdexcept>
#include "gtest/gtest.h"

#include "position.h"
#include "syzygy/tbtables.h"

TEST(tbtables, constructor1) {
	TBFile::setPaths("data");
	TBTables t;
}

TEST(tbtables, init) {
	TBFile::setPaths("data");
	TBTables t;
	t.init();
	ASSERT_EQ(t.getMaxCardinality(), 5);
	ASSERT_EQ(t.size(), 1);
}

TEST(tbtables, clear) {
	TBFile::setPaths("data");
	TBTables t;
	t.init();
	t.clear();
	
	ASSERT_EQ(t.getMaxCardinality(), 0);
	ASSERT_EQ(t.size(), 0);	
}

TEST(tbtables, getWDL) {
	Position p;
	p.setup("KNNKB", white);
	TBFile::setPaths("data");
	TBTables t;
	t.init();
	ASSERT_EQ(t.getWDL(p.getMaterialKey()).getCompleteFileName(), "KNNvKB.rtbw");
}

TEST(tbtables, getDTZ) {
	Position p;
	p.setup("KNNKB", white);
	TBFile::setPaths("data");
	TBTables t;
	t.init();
	ASSERT_EQ(t.getDTZ(p.getMaterialKey()).getCompleteFileName(), "KNNvKB.rtbz");
}

TEST(tbtablesDeath, getWDL) {
	Position p;
	p.setup("KNKB", white);
	TBTables t;
	t.init();
	ASSERT_THROW(t.getWDL(p.getMaterialKey()), std::out_of_range);
}

TEST(tbtablesDeath, getDTZ) {
	Position p;
	p.setup("KNKB", white);
	TBTables t;
	t.init();
	ASSERT_THROW(t.getDTZ(p.getMaterialKey()), std::out_of_range);
}
