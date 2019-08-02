#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "gtest/gtest.h"

#include "position.h"
#include "syzygy/syzygy.h"

void initializeTest() {
	auto& szg =Syzygy::getInstance();
	szg.setPath("C:/Users/elcab/Downloads/syzygy");
	ASSERT_EQ(szg.getMaxCardinality(), 5);
}
void syzygyTest()
{
	Position pos;
	
	std::string line;
	std::ifstream myfile ("testsyzygy.csv");
	
	ASSERT_TRUE(myfile.is_open());

	auto start = std::chrono::system_clock::now();
	
	auto& szg =Syzygy::getInstance();
	unsigned long int num = 0;
	unsigned long int testedNum = 0;
	ProbeState result;
	while ( std::getline (myfile,line) )
	{
		++num;
		//std::cout << line << std::endl;
		
		size_t delimiter = line.find_first_of(',');
		ASSERT_NE(std::string::npos, delimiter);
		std::string fen = line.substr(0, delimiter);
		
		line = line.substr(delimiter+1);
		delimiter = line.find_first_of(',');
		ASSERT_NE(std::string::npos, delimiter);
		std::string dtz = line.substr(0, delimiter);
		
		line = line.substr(delimiter+1);
		delimiter = line.find_first_of(',');
		ASSERT_NE(std::string::npos, delimiter);
		std::string wdl = line.substr(0, delimiter);
		
		pos.setupFromFen(fen); 
		
		WDLScore score = szg.probeWdl(pos, result);

		ASSERT_NE(result, ProbeState::FAIL);
		ASSERT_EQ(static_cast<int>(score), std::stoi(wdl));
		if (static_cast<int>(score) == std::stoi(wdl)) {++testedNum;}
		
		int dtz2 = szg.probeDtz(pos, result);

		ASSERT_NE(result, ProbeState::FAIL);
		ASSERT_EQ(dtz2, std::stoi(dtz));
		
		if( num %10000 == 0 )
		{
			auto now = std::chrono::system_clock::now();
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
			std::cout<<testedNum<<"/"<<num<<" ("<< (testedNum*100.0/num) <<"%) nps: "<<1000.0*testedNum/milliseconds.count()<<std::endl;
		}
		
	}
	std::cout<<num<<std::endl;
	myfile.close();
}


TEST(Syzygy, test)
{
	initializeTest();
	syzygyTest();
}


TEST(Syzygy, multiThreadTest)
{
	initializeTest();
	std::thread t(&syzygyTest);
	syzygyTest();
	t.join();
}