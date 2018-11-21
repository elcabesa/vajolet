#include <iostream>
#include <fstream>
#include <string>

#include "gtest/gtest.h"

#include "./../position.h"
#include "./../syzygy/tbprobe.h"

TEST(Syzygy, test)
{
	Position pos;
	
	std::string line;
	std::ifstream myfile ("testsyzygy.csv");
	
	ASSERT_TRUE(myfile.is_open());
	
	tb_init("D:/vajolet/syzygy");
	ASSERT_TRUE(TB_LARGEST > 0);
	
	
	unsigned long int num = 0;
	unsigned long int testedNum = 0;
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
		
		
		unsigned result1 = tb_probe_wdl(pos.getBitmap(whitePieces),
			pos.getBitmap(blackPieces),
			pos.getBitmap(blackKing) | pos.getBitmap(whiteKing),
			pos.getBitmap(blackQueens) | pos.getBitmap(whiteQueens),
			pos.getBitmap(blackRooks) | pos.getBitmap(whiteRooks),
			pos.getBitmap(blackBishops) | pos.getBitmap(whiteBishops),
			pos.getBitmap(blackKnights) | pos.getBitmap(whiteKnights),
			pos.getBitmap(blackPawns) | pos.getBitmap(whitePawns),
			0,//pos.getActualState().fiftyMoveCnt,
			pos.getActualState().getCastleRights(),
			pos.getActualState().epSquare == squareNone? 0 : pos.getActualState().epSquare ,
			pos.getActualState().nextMove== Position::whiteTurn);

		//EXPECT_NE(result1, TB_RESULT_FAILED);
		int wdl_res = TB_GET_WDL(result1)-2;
		
		
		unsigned results[TB_MAX_MOVES];
		unsigned result2 = tb_probe_root(pos.getBitmap(whitePieces),
			pos.getBitmap(blackPieces),
			pos.getBitmap(blackKing) | pos.getBitmap(whiteKing),
			pos.getBitmap(blackQueens) | pos.getBitmap(whiteQueens),
			pos.getBitmap(blackRooks) | pos.getBitmap(whiteRooks),
			pos.getBitmap(blackBishops) | pos.getBitmap(whiteBishops),
			pos.getBitmap(blackKnights) | pos.getBitmap(whiteKnights),
			pos.getBitmap(blackPawns) | pos.getBitmap(whitePawns),
			pos.getActualState().fiftyMoveCnt,
			pos.getActualState().getCastleRights(),
			pos.getActualState().epSquare == squareNone? 0 : pos.getActualState().epSquare ,
			pos.getActualState().nextMove== Position::whiteTurn,
			results);
			
		//EXPECT_NE(result2, TB_RESULT_FAILED);
		int dtz_res = TB_GET_DTZ(result2);
		
		if( result1 != TB_RESULT_FAILED && result2 != TB_RESULT_FAILED )
		{
			
			++testedNum;
			if( wdl_res != std::stoi(wdl) || ( (result2 != TB_RESULT_CHECKMATE) && dtz_res!= std::abs(std::stoi(dtz))))
			{
				std::cout <<"-------------------"<< std::endl;
				std::cout <<"FEN: "<< fen << std::endl;
				std::cout <<"DTZ: "<< dtz << std::endl;
				std::cout <<"WDL: "<< wdl << std::endl;
				std::cout <<"PROBE DTZ:"<< dtz_res << std::endl;
				std::cout <<"PROBE WDL:"<< wdl_res << std::endl;
				std::cout <<"checkmate:"<< (TB_RESULT_CHECKMATE == result2 )<<std::endl;
				//exit(0);
			}
			EXPECT_EQ(wdl_res, std::stoi(wdl));
			EXPECT_EQ(dtz_res, (result2 != TB_RESULT_CHECKMATE)? std::abs(std::stoi(dtz)): 0);
		}
		
		
		//std::cout<<pos.getFen()<<std::endl;
		
		if( num %10000 == 0 )
		{
			std::cout<<testedNum<<"/"<<num<<" ("<< (testedNum*100.0/num) <<"%)"<<std::endl;
		}
		
	}
	std::cout<<num<<std::endl;
	myfile.close();
	


}


