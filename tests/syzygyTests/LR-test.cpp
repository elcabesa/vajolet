#include "gtest/gtest.h"
#include "syzygy/LR.h"


TEST(LR, test) {
	uint8_t data[3] = {0x12, 0x34, 0x45};
	LR *lr = reinterpret_cast<LR*>(data);
	ASSERT_EQ(lr->getLeft(), 0x0412);
	ASSERT_EQ(lr->getRight(), 0x0453);
}


TEST(LR, test2) {
	uint8_t data[6] = {0x12, 0x34, 0x45, 0x34, 0x45, 0x12};
	LR *lr = reinterpret_cast<LR*>(data);
	ASSERT_EQ(lr[0].getLeft(), 0x0412);
	ASSERT_EQ(lr[0].getRight(), 0x0453);
	ASSERT_EQ(lr[1].getLeft(), 0x0534);
	ASSERT_EQ(lr[1].getRight(), 0x0124);
}
