#include "gtest/gtest.h"
#include "syzygy/tbfile.h"

#ifndef _WIN32
	constexpr char SepChar = ':';
#else
	constexpr char SepChar = ';';
#endif

TEST(tbfile, exist) {
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_FALSE(TBFile::exist("open.txt"));
	
	TBFile::setPaths("data");
	
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_TRUE(TBFile::exist("open.txt"));
	
}

TEST(tbfile, setPaths) {
		
	TBFile::setPaths("data");
	
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_TRUE(TBFile::exist("open.txt"));
	
	TBFile::setPaths("");
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_FALSE(TBFile::exist("open.txt"));
	
	std::string s = "data";
	s += SepChar;
	s += "data2";
	TBFile::setPaths(s);
	
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_TRUE(TBFile::exist("open.txt"));
	
	s = "data2";
	s += SepChar;
	s += "data";
	TBFile::setPaths(s);
	
	EXPECT_FALSE(TBFile::exist("ciao.txt"));
	EXPECT_TRUE(TBFile::exist("open.txt"));

}

TEST(tbfile, constructor) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));
	TBFile f("open.txt");
	ASSERT_TRUE(f.isValid());
	EXPECT_EQ(f[0],'c');
	EXPECT_EQ(f[1],'i');
	EXPECT_EQ(f[2],'a');
	EXPECT_EQ(f[3],'o');
	EXPECT_EQ(f[4],'M');
	EXPECT_EQ(f[5],'o');
	EXPECT_EQ(f[6],'n');
	EXPECT_EQ(f[7],'d');
	EXPECT_EQ(f[8],'o');
	
	TBFile f2("nono.txt");
	ASSERT_FALSE(f2.isValid());
	
}

TEST(tbfile, functional) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));
	ASSERT_TRUE(TBFile::exist("open2.txt"));
	
	TBFile f("open.txt");
	TBFile f2("open2.txt");
	
	ASSERT_TRUE(f.isValid());
	ASSERT_TRUE(f2.isValid());
	
	EXPECT_EQ(f[0],'c');
	EXPECT_EQ(f2[0],'p');
	EXPECT_EQ(f[1],'i');
	EXPECT_EQ(f2[1],'i');
	EXPECT_EQ(f[2],'a');
	EXPECT_EQ(f2[2],'p');
	EXPECT_EQ(f[3],'o');
	EXPECT_EQ(f2[3],'p');
	EXPECT_EQ(f[4],'M');
	EXPECT_EQ(f2[4],'o');
	EXPECT_EQ(f[5],'o');
	EXPECT_EQ(f[6],'n');
	EXPECT_EQ(f[7],'d');
	EXPECT_EQ(f[8],'o');
	
}