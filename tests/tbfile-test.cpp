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