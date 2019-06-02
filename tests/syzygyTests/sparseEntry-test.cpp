#include "gtest/gtest.h"
#include "syzygy/sparseEntry.h"


TEST(SparseEntry, test) {
	uint8_t data[6] = {0x12, 0x34, 0x45, 0x35, 0x8F, 0x72};
	SparseEntry *se = reinterpret_cast<SparseEntry*>(data);
	ASSERT_EQ(se->getBlock(), 0x35453412);
	ASSERT_EQ(se->getOffset(), 0x728F);
}