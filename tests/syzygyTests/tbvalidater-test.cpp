#include "gtest/gtest.h"

#include "syzygy/tbfile.h"
#include "syzygy/tbvalidater.h"

TEST(tbvalidater, rtbw) {
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("KNNvKB.rtbw"));
	TBFile tbf("KNNvKB.rtbw");
	ASSERT_TRUE(tbf.isValid());
	ASSERT_TRUE(TBValidater::validate(tbf, WDL, "KNNvKB.rtbw"));
}

TEST(tbvalidater, rtbz) {
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("KNNvKB.rtbz"));
	TBFile tbf("KNNvKB.rtbz");
	ASSERT_TRUE(tbf.isValid());
	ASSERT_TRUE(TBValidater::validate(tbf, DTZ, "KNNvKB.rtbz"));
}

TEST(tbvalidaterDeathTest, invalidFile) {
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));
	TBFile tbf("open.txt");
	ASSERT_TRUE(tbf.isValid());
	EXPECT_EXIT(TBValidater::validate(tbf, DTZ, "open.txt"), ::testing::ExitedWithCode(EXIT_FAILURE), "Corrupt tablebase file open.txt");
}

TEST(tbvalidaterDeathTest, wrongMagics1) {
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("KNNvKB.rtbw"));
	TBFile tbf("KNNvKB.rtbw");
	ASSERT_TRUE(tbf.isValid());
	EXPECT_EXIT(TBValidater::validate(tbf, DTZ, "KNNvKB.rtbw"), ::testing::ExitedWithCode(EXIT_FAILURE), "Corrupted table in file KNNvKB.rtbw");
}

TEST(tbvalidaterDeathTest, wrongMagics2) {
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("KNNvKB.rtbz"));
	TBFile tbf("KNNvKB.rtbz");
	ASSERT_TRUE(tbf.isValid());
	EXPECT_EXIT(TBValidater::validate(tbf, WDL, "KNNvKB.rtbz"), ::testing::ExitedWithCode(EXIT_FAILURE), "Corrupted table in file KNNvKB.rtbz");
}

