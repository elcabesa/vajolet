#include <fstream>
#include <vector>

#include "gtest/gtest.h"
#include "perft.h"
#include "position.h"
#include "transposition.h"

TEST(PerftTest, frcTest) {
	
	std::ifstream infile("fischer.txt");
	ASSERT_FALSE(infile.fail());
	std::string line;
	
	Position pos;
	
	unsigned int n = 0;
	while (std::getline(infile, line))
	{
		std::size_t found = line.find_first_of(",");
		std::string fen = line.substr(0, found);
		pos.setupFromFen( fen ); 

		while (found != std::string::npos )
		{	
			std::size_t start = found+1;
			found=line.find_first_of(",",found + 1 );
			
			unsigned long long ull = std::stoull (line.substr(start, found-start));
			transpositionTable tt;
			PerftTranspositionTable ptt(tt);
			unsigned long long int res = Perft(pos, ptt).perft(6);
			std::cout<<++n<<" perft(6) = "<<res<<std::endl;
			ASSERT_EQ(ull, res);
		}
	}
}

TEST(PerftTest, perft) {
	
	std::ifstream infile("perft.txt");
	ASSERT_FALSE(infile.fail());
	std::string line;
	
	Position pos;
	
	unsigned int n = 0;
	while (std::getline(infile, line))
	{
		++n;
		std::size_t found = line.find_first_of(",");
		std::string fen = line.substr(0, found);
		pos.setupFromFen( fen ); 

		unsigned int i = 0;
		while (found != std::string::npos )
		{	
			std::size_t start = found+1;
			found=line.find_first_of(",",found + 1 );
			
			unsigned long long ull = std::stoull (line.substr(start, found-start));
			transpositionTable tt;
			PerftTranspositionTable ptt(tt);
			unsigned long long int res = Perft(pos, ptt).perft(++i);
			std::cout<<n<<" perft("<<i<<") = "<<res<<std::endl;
			ASSERT_EQ(ull, res);
		}
	}
}

