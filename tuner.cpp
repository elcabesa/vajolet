/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#include <algorithm>

#include "vajolet.h"
#include "command.h"
#include "io.h"
#include "bitops.h"
#include "data.h"
#include "hashKeys.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "search.h"
#include "eval.h"
#include "parameters.h"
#include "syzygy/tbprobe.h"
#include "eval.h"

#include <fstream>

std::ifstream infile("oracle.epd");
struct results
{
	results(const std::string& _FEN, const double _res):FEN(_FEN),res(_res){}
	std::string FEN;
	double res;
};
std::vector<results> positions;


/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	sync_cout<<"Vajolet tuner"<<sync_endl;
}

int readFile() {
	std::string line;
	if (infile.is_open()) {
		sync_cout<<"start parsing file"<<sync_endl;
		while (getline(infile, line)) {
			double res = 0;
			std::size_t delimiter = line.find(" c9 ");
			std::string FEN = line.substr(0, delimiter);
			std::string RESULT = line.substr(delimiter + 4);

			res = std::stod(RESULT);

			positions.emplace_back(results(FEN, res));

		}
		infile.close();
		sync_cout<<"finished parsing file"<<sync_endl;
		sync_cout<<positions.size()<<" parsed positions"<<sync_endl;
	}
	else
	{
		std::cout << "Unable to open file" << std::endl;
		return -1;
	}
	return 0;
}



long double calcSigleError2(Position &p, long double res)
{
	
	long double eval = p.eval<false>();
	
	long double sateval = std::min( std::max(eval, -200000.0L ), 200000.0L );
	long double satres = std::min( std::max(res, -200000.0L ), 200000.0L );
	

	/*if( std::abs(sateval - satres) > 200000 )
	{
		std::cout<<p.getFen()<<" "<<sateval <<" "<<satres<<" " << sateval - satres<<std::endl;
	}*/
	
	long double error = sateval - satres ;
	return std::pow(error, 2);
	
}



long double calcError2(void)
{
	Position p;
	initMobilityBonus();
	long double totalError = 0.0;

	for(const auto& v : positions)
	{
		p.setupFromFen(v.FEN);
		totalError += calcSigleError2(p, v.res);

	}
	totalError /= positions.size();
	return totalError;
}






struct parameter
{
	std::string name;
	long double value[4];
	unsigned int count;
	simdScore* pointer;
	long double partialDerivate[4];
	long double totalGradient[4];
	long double totalError[4];
	bool requireUpdate;
	parameter(std::string _name, simdScore* _pointer,unsigned int _count, bool _r= false):name(_name),count(_count),pointer(_pointer),requireUpdate(_r)
	{
		for( int i = 0; i <4 ; ++i)
		{
		value[i] = (*pointer)[i] ;
		partialDerivate[i] = 0;
		totalGradient[i] = 1e-8;
		totalError[i] = 0;
		}
	}
};

std::vector<parameter> parameters;

void updateParameter(parameter& par,unsigned int i, const long double delta)
{
	(*(par.pointer))[i] = int (par.value[i] + delta);
	/*if(par.requireUpdate)
	{
		Position::initPstValues();
		initMobilityBonus();
	}*/

}





/*!	\brief	main function
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
int main()
{
	//----------------------------------
	//	init global data
	//----------------------------------
	std::cout.rdbuf()->pubsetbuf( nullptr, 0 );
	std::cin.rdbuf()->pubsetbuf( nullptr, 0 );
	initData();
	HashKeys::init();
	Position::initScoreValues();
	Position::initCastleRightsMask();
	Movegen::initMovegenConstant();

	Search::initLMRreduction();
	TT.setSize(1);
	Position::initMaterialKeys();
	initMobilityBonus();
	tb_init(Search::SyzygyPath.c_str());
	
	enablePawnHash =false;

	//----------------------------------
	//	main loop
	//----------------------------------
	printStartInfo();


	readFile();

	parameters.push_back(parameter("initialPieceValue[2]",&initialPieceValue[2],2,true));
	parameters.push_back(parameter("initialPieceValue[3]",&initialPieceValue[3],2,true));
	parameters.push_back(parameter("initialPieceValue[4]",&initialPieceValue[4],2,true));
	parameters.push_back(parameter("initialPieceValue[5]",&initialPieceValue[5],2,true));
	parameters.push_back(parameter("initialPieceValue[6]",&initialPieceValue[6],1,true));

	parameters.push_back(parameter("PawnD3",&PawnD3,2,true));
	parameters.push_back(parameter("PawnD4",&PawnD4,2,true));
	parameters.push_back(parameter("PawnD5",&PawnD5,2,true));
	parameters.push_back(parameter("PawnE3",&PawnE3,2,true));
	parameters.push_back(parameter("PawnE4",&PawnE4,2,true));
	parameters.push_back(parameter("PawnE5",&PawnE5,2,true));
	parameters.push_back(parameter("PawnCentering",&PawnCentering,2,true));
	parameters.push_back(parameter("PawnRankBonus",&PawnRankBonus,2,true));
	parameters.push_back(parameter("KnightPST",&KnightPST,2,true));
	parameters.push_back(parameter("BishopPST",&BishopPST,2,true));
	parameters.push_back(parameter("RookPST",&RookPST,2,true));
	parameters.push_back(parameter("QueenPST",&QueenPST,2,true));
	parameters.push_back(parameter("KingPST",&KingPST,2,true));

	parameters.push_back(parameter("BishopBackRankOpening",&BishopBackRankOpening,2,true));
	parameters.push_back(parameter("KnightBackRankOpening",&KnightBackRankOpening,2,true));
	parameters.push_back(parameter("RookBackRankOpening",&RookBackRankOpening,2,true));
	parameters.push_back(parameter("QueenBackRankOpening",&QueenBackRankOpening,2,true));
	parameters.push_back(parameter("BishopOnBigDiagonals",&BishopOnBigDiagonals,2,true));


	parameters.push_back(parameter("queenMobilityPars",&queenMobilityPars,4,true));
	parameters.push_back(parameter("rookMobilityPars",&rookMobilityPars,4,true));
	parameters.push_back(parameter("bishopMobilityPars",&bishopMobilityPars,4,true));
	parameters.push_back(parameter("knightMobilityPars",&knightMobilityPars,4,true));
	parameters.push_back(parameter("isolatedPawnPenalty",&isolatedPawnPenalty,2));
	parameters.push_back(parameter("isolatedPawnPenaltyOpp",&isolatedPawnPenaltyOpp,2));
	parameters.push_back(parameter("doubledPawnPenalty",&doubledPawnPenalty,2));
	parameters.push_back(parameter("backwardPawnPenalty",&backwardPawnPenalty,2));
	parameters.push_back(parameter("chainedPawnBonus",&chainedPawnBonus,2));
	parameters.push_back(parameter("passedPawnFileAHPenalty",&passedPawnFileAHPenalty,2));
	parameters.push_back(parameter("passedPawnSupportedBonus",&passedPawnSupportedBonus,2));
	parameters.push_back(parameter("candidateBonus",&candidateBonus,2));
	parameters.push_back(parameter("passedPawnBonus",&passedPawnBonus,2));
	parameters.push_back(parameter("passedPawnUnsafeSquares",&passedPawnUnsafeSquares,2));
	parameters.push_back(parameter("passedPawnBlockedSquares",&passedPawnBlockedSquares,2));
	parameters.push_back(parameter("passedPawnDefendedSquares",&passedPawnDefendedSquares,2));
	parameters.push_back(parameter("passedPawnDefendedBlockingSquare",&passedPawnDefendedBlockingSquare,2));
	parameters.push_back(parameter("unstoppablePassed",&unstoppablePassed,2));
	parameters.push_back(parameter("rookBehindPassedPawn",&rookBehindPassedPawn,2));
	parameters.push_back(parameter("EnemyRookBehindPassedPawn",&EnemyRookBehindPassedPawn,2));
	parameters.push_back(parameter("holesPenalty",&holesPenalty,2));
	parameters.push_back(parameter("pawnCenterControl",&pawnCenterControl,2));
	parameters.push_back(parameter("pawnBigCenterControl",&pawnBigCenterControl,2));
	parameters.push_back(parameter("pieceCoordination",&pieceCoordination,2));
	parameters.push_back(parameter("piecesCenterControl",&piecesCenterControl,2));
	parameters.push_back(parameter("piecesBigCenterControl",&piecesBigCenterControl,2));
	parameters.push_back(parameter("rookOn7Bonus",&rookOn7Bonus,2));
	parameters.push_back(parameter("rookOnPawns",&rookOnPawns,2));
	parameters.push_back(parameter("queenOn7Bonus",&queenOn7Bonus,2));
	parameters.push_back(parameter("queenOnPawns",&queenOnPawns,2));
	parameters.push_back(parameter("rookOnOpen",&rookOnOpen,2));
	parameters.push_back(parameter("rookOnSemi",&rookOnSemi,2));
	parameters.push_back(parameter("rookTrapped",&rookTrapped,2));
	parameters.push_back(parameter("rookTrappedKingWithoutCastling",&rookTrappedKingWithoutCastling,2));
	parameters.push_back(parameter("knightOnOutpost",&knightOnOutpost,2));
	parameters.push_back(parameter("knightOnOutpostSupported",&knightOnOutpostSupported,2));
	parameters.push_back(parameter("knightOnHole",&knightOnHole,2));
	parameters.push_back(parameter("KnightAttackingWeakPawn",&KnightAttackingWeakPawn,2));
	parameters.push_back(parameter("bishopOnOutpost",&bishopOnOutpost,2));
	parameters.push_back(parameter("bishopOnOutpostSupported",&bishopOnOutpostSupported,2));
	parameters.push_back(parameter("bishopOnHole",&bishopOnHole,2));
	parameters.push_back(parameter("badBishop",&badBishop,2));
	parameters.push_back(parameter("tempo",&tempo,2));
	parameters.push_back(parameter("bishopPair",&bishopPair,2));
	parameters.push_back(parameter("ownKingNearPassedPawn",&ownKingNearPassedPawn,2));
	parameters.push_back(parameter("enemyKingNearPassedPawn",&enemyKingNearPassedPawn,2));
	parameters.push_back(parameter("spaceBonus",&spaceBonus,2));
	parameters.push_back(parameter("undefendedMinorPenalty",&undefendedMinorPenalty,2));

	parameters.push_back(parameter("attackedByPawnPenalty[2]",&attackedByPawnPenalty[2],2));
	parameters.push_back(parameter("attackedByPawnPenalty[3]",&attackedByPawnPenalty[3],2));
	parameters.push_back(parameter("attackedByPawnPenalty[4]",&attackedByPawnPenalty[4],2));
	parameters.push_back(parameter("attackedByPawnPenalty[5]",&attackedByPawnPenalty[5],2));

	parameters.push_back(parameter("weakPiecePenalty[2][2]",&weakPiecePenalty[2][2],2));
	parameters.push_back(parameter("weakPiecePenalty[2][3]",&weakPiecePenalty[2][3],2));
	parameters.push_back(parameter("weakPiecePenalty[2][4]",&weakPiecePenalty[2][4],2));
	parameters.push_back(parameter("weakPiecePenalty[2][5]",&weakPiecePenalty[2][5],2));
	parameters.push_back(parameter("weakPiecePenalty[2][6]",&weakPiecePenalty[2][6],2));

	parameters.push_back(parameter("weakPiecePenalty[3][2]",&weakPiecePenalty[3][2],2));
	parameters.push_back(parameter("weakPiecePenalty[3][3]",&weakPiecePenalty[3][3],2));
	parameters.push_back(parameter("weakPiecePenalty[3][4]",&weakPiecePenalty[3][4],2));
	parameters.push_back(parameter("weakPiecePenalty[3][5]",&weakPiecePenalty[3][5],2));
	parameters.push_back(parameter("weakPiecePenalty[3][6]",&weakPiecePenalty[3][6],2));

	parameters.push_back(parameter("weakPiecePenalty[4][2]",&weakPiecePenalty[4][2],2));
	parameters.push_back(parameter("weakPiecePenalty[4][3]",&weakPiecePenalty[4][3],2));
	parameters.push_back(parameter("weakPiecePenalty[4][4]",&weakPiecePenalty[4][4],2));
	parameters.push_back(parameter("weakPiecePenalty[4][5]",&weakPiecePenalty[4][5],2));
	parameters.push_back(parameter("weakPiecePenalty[4][6]",&weakPiecePenalty[4][6],2));

	parameters.push_back(parameter("weakPiecePenalty[5][2]",&weakPiecePenalty[5][2],2));
	parameters.push_back(parameter("weakPiecePenalty[5][3]",&weakPiecePenalty[5][3],2));
	parameters.push_back(parameter("weakPiecePenalty[5][4]",&weakPiecePenalty[5][4],2));
	parameters.push_back(parameter("weakPiecePenalty[5][5]",&weakPiecePenalty[5][5],2));
	parameters.push_back(parameter("weakPiecePenalty[5][6]",&weakPiecePenalty[5][6],2));

	parameters.push_back(parameter("weakPiecePenalty[6][2]",&weakPiecePenalty[6][2],2));
	parameters.push_back(parameter("weakPiecePenalty[6][3]",&weakPiecePenalty[6][3],2));
	parameters.push_back(parameter("weakPiecePenalty[6][4]",&weakPiecePenalty[6][4],2));
	parameters.push_back(parameter("weakPiecePenalty[6][5]",&weakPiecePenalty[6][5],2));
	parameters.push_back(parameter("weakPiecePenalty[6][6]",&weakPiecePenalty[6][6],2));

	parameters.push_back(parameter("weakPawnAttackedByKing",&weakPawnAttackedByKing,2));

	parameters.push_back( parameter( "KingAttackWeights", &KingAttackWeights, 4 ) );
	parameters.push_back( parameter( "kingShieldBonus", &kingShieldBonus, 1 ) );
	parameters.push_back( parameter( "kingFarShieldBonus", &kingFarShieldBonus, 1 ) );
	parameters.push_back( parameter( "kingStormBonus", &kingStormBonus, 3 ) );
	parameters.push_back( parameter( "kingSafetyBonus", &kingSafetyBonus, 2 ) );
	parameters.push_back( parameter( "kingSafetyPars1", &kingSafetyPars1, 4 ) );
	parameters.push_back( parameter( "kingSafetyPars2", &kingSafetyPars2, 4 ) );




	unsigned int totParameters = 0;
	for(auto& p : parameters)
	{
		for( unsigned int i = 0; i < p.count; ++i)
		{
			++totParameters;
		}
	}

	std::cout<<"start to optimizing "<<totParameters<<" parameters"<<std::endl;

	
	/*for(auto& p : parameters)
	{
		std::cout<<p.name<<" "<<p.value<<std::endl;
	}*/

	std::vector<parameter> bestParameters;

	std::random_device rd;
	std::mt19937 g(rd());
	
	std::uniform_int_distribution<> dis(-10, 10);
	
	unsigned long iteration = 0;
	unsigned long bestIteration = 0;

	bool stop = false;
	
	long double error = calcError2();// big number
	long double minValue = error;
	std::cout<<"iteration #"<<iteration<<std::endl;
	std::cout<<"startError "<<error<<std::endl;

	while(!stop)
	{
		++iteration;
		std::cout<<"--------------------------------------------------------------------"<<std::endl;
		std::cout<<"iteration #"<<iteration<<std::endl;
		std::cout<<"shuffle"<<std::endl;
		std::shuffle(positions.begin(), positions.end(), g);

		for(auto& p : parameters)
		{
			for( unsigned int i = 0; i < 4; ++i)
			{
				updateParameter(p,i, dis(g));

			}
		}
		Position::initPstValues();
		initMobilityBonus();
		


		error = calcError2();

		std::cout<<"newError "<<error<<std::endl;
		if( error < minValue)
		{
			for(auto& p : parameters)
			{
				for( unsigned int i = 0; i < 4; ++i)
				{
					(p.value[i]) = (*(p.pointer))[i];
				}
			}
			
			std::cout<<"###### NEW BEST ITERATION ####"<<std::endl;

			minValue = error;
			bestIteration = iteration;
			std::cout<<"bestIteration "<<bestIteration<<" minError "<<minValue<<std::endl;
			bestParameters.clear();
			std::copy(parameters.begin(), parameters.end(),std::back_inserter(bestParameters));
			std::cout<<"BEST PARAMETERS"<<std::endl;
			for(auto& p : bestParameters)
			{
				std::cout<<"simdScore "<<p.name<<" =  {" ;
				
				for( unsigned int i = 0; i < 4; ++i)
				{
					//std::cout<<p.name<<"["<<i<<"] = "<<(int)p.value[i]<<" ("<<p.value[i]<<")"<<std::endl;
					std::cout<<(int)p.value[i];
					if(i<3)
					{
						std::cout<<",";
					}
				}
				std::cout<<"};"<<std::endl;
			}

		}
	}


	return 0;
}


