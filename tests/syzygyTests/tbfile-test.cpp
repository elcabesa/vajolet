#include "gtest/gtest.h"
#include "syzygy/tbfile.h"

#ifndef _WIN32
	constexpr char SepChar = ':';
#else
	constexpr char SepChar = ';';
#endif

TEST(tbfile, exist) {
	TBFile::setPaths("");
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
	
	ASSERT_EQ(f.size(), 9);
	
	
	TBFile f2("nono.txt");
	ASSERT_FALSE(f2.isValid());
	ASSERT_EQ(f2.size(), 0);
	
}

TEST(tbfile, functional) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));
	ASSERT_TRUE(TBFile::exist("open2.txt"));
	
	TBFile f("open.txt");
	const TBFile f2("open2.txt");
	
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

TEST(tbfile, twoMappedFile) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));
	ASSERT_TRUE(TBFile::exist("open2.txt"));
	
	TBFile f("open.txt");
	ASSERT_TRUE(f.isValid());
	
	{
		const TBFile f2("open.txt");
		ASSERT_TRUE(f2.isValid());
		
		EXPECT_EQ(f[0],'c');
		EXPECT_EQ(f[1],'i');
		EXPECT_EQ(f[2],'a');
		EXPECT_EQ(f[3],'o');
		
		EXPECT_EQ(f2[0],'c');
		EXPECT_EQ(f2[1],'i');
		EXPECT_EQ(f2[2],'a');
		EXPECT_EQ(f2[3],'o');
		EXPECT_EQ(f2[4],'M');
		EXPECT_EQ(f2[5],'o');
		EXPECT_EQ(f2[6],'n');
		EXPECT_EQ(f2[7],'d');
		EXPECT_EQ(f2[8],'o');
	}
	
	EXPECT_EQ(f[4],'M');
	EXPECT_EQ(f[5],'o');
	EXPECT_EQ(f[6],'n');
	EXPECT_EQ(f[7],'d');
	EXPECT_EQ(f[8],'o');
}

TEST(tbfile, emptyConstructor) {
	
	TBFile::setPaths("data");
	TBFile f;
	ASSERT_FALSE(f.isValid());

}

TEST(tbfile, moveAssignment) {
	
	TBFile::setPaths("data");
	TBFile f;
	ASSERT_FALSE(f.isValid());
	f = TBFile("open.txt");
	
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
	
	ASSERT_EQ(f.size(), 9);
	
	f = TBFile("open2.txt");
	
	EXPECT_EQ(f[0],'p');
	EXPECT_EQ(f[1],'i');
	EXPECT_EQ(f[2],'p');
	EXPECT_EQ(f[3],'p');
	EXPECT_EQ(f[4],'o');
	
	ASSERT_EQ(f.size(), 5);
	

}

TEST(tbfile, addressof) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));

	TBFile f("open.txt");
	const uint8_t * data = &f;
	
	ASSERT_TRUE(f.isValid());
	
	EXPECT_EQ(*data,'c');
	EXPECT_EQ(*(++data),'i');
	EXPECT_EQ(*(++data),'a');
	EXPECT_EQ(*(++data),'o');
	EXPECT_EQ(*(++data),'M');
	EXPECT_EQ(*(++data),'o');
	EXPECT_EQ(*(++data),'n');
	EXPECT_EQ(*(++data),'d');
	EXPECT_EQ(*(++data),'o');
}

TEST(tbfile, constAddressof) {
	
	TBFile::setPaths("data");
	ASSERT_TRUE(TBFile::exist("open.txt"));

	const TBFile f("open.txt");
	const uint8_t * data = &f;
	
	ASSERT_TRUE(f.isValid());
	
	EXPECT_EQ(*data,'c');
	EXPECT_EQ(*(++data),'i');
	EXPECT_EQ(*(++data),'a');
	EXPECT_EQ(*(++data),'o');
	EXPECT_EQ(*(++data),'M');
	EXPECT_EQ(*(++data),'o');
	EXPECT_EQ(*(++data),'n');
	EXPECT_EQ(*(++data),'d');
	EXPECT_EQ(*(++data),'o');
}