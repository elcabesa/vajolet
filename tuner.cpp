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
#include <random>
#include "command.h"
#include "io.h"
#include "bitops.h"
#include "data.h"
#include "hashKeys.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "search.h"
#include "score.h"
#include "eval.h"
#include "parameters.h"
#include "uciParameters.h"
#include "syzygy/tbprobe.h"
#include "eval.h"

#include <fstream>

const int arraySize = 200;
//const int arrayScaling = 300;
unsigned long long int array[arraySize]= {0};

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
	
	if( std::abs( sateval ) <= 199900.0L && std::abs( satres ) <= 199900.0L)
	{
		/*unsigned int index = std::abs(sateval - satres)/ arrayScaling;
		if( index >= arraySize )
		{
			index = arraySize -1;
		}
		array[index]++;
		

		if( 
		std::abs(sateval - satres) >15000
		//&& std::abs(sateval - satres) < 14100  
		//&& bitCnt(p.getOccupationBitmap()) >3 
		)
		{
			std::cout<<p.getFen()<<" "<<sateval <<" "<<satres<<" " << sateval - satres<<std::endl;
		}*/
	}
	
	long double error = sateval - satres ;
	return std::pow(error, 2);
	
}



long double calcError2(void)
{
	Position p;
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
	if( (*(par.pointer))[i] == 0 )
	{
		(*(par.pointer))[i] = 1;
	}
	/*if(par.requireUpdate)
	{
		Position::initPstValues();
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
	transpositionTable::getInstance().setSize(1);
	Position::initMaterialKeys();
	tb_init(uciParameters::SyzygyPath.c_str());
	
	enablePawnHash =false;

	//----------------------------------
	//	main loop
	//----------------------------------
	printStartInfo();


	readFile();

/*
++	parameters.push_back(parameter("initialPieceValue[2]",&initialPieceValue[2],2,true));
++	parameters.push_back(parameter("initialPieceValue[3]",&initialPieceValue[3],2,true));
++	parameters.push_back(parameter("initialPieceValue[4]",&initialPieceValue[4],2,true));
++	parameters.push_back(parameter("initialPieceValue[5]",&initialPieceValue[5],2,true));
++	parameters.push_back(parameter("initialPieceValue[6]",&initialPieceValue[6],1,true));

++	parameters.push_back(parameter("PawnD3",&PawnD3,2,true));
++	parameters.push_back(parameter("PawnD4",&PawnD4,2,true));
++	parameters.push_back(parameter("PawnD5",&PawnD5,2,true));
++	parameters.push_back(parameter("PawnE3",&PawnE3,2,true));
++	parameters.push_back(parameter("PawnE4",&PawnE4,2,true));
++	parameters.push_back(parameter("PawnE5",&PawnE5,2,true));
++	parameters.push_back(parameter("PawnCentering",&PawnCentering,2,true));
++	parameters.push_back(parameter("PawnRankBonus",&PawnRankBonus,2,true));
++	parameters.push_back(parameter("KnightPST",&KnightPST,2,true));
++	parameters.push_back(parameter("BishopPST",&BishopPST,2,true));
++	parameters.push_back(parameter("RookPST",&RookPST,2,true));
++	parameters.push_back(parameter("QueenPST",&QueenPST,2,true));
++	parameters.push_back(parameter("KingPST",&KingPST,2,true));

++	parameters.push_back(parameter("BishopBackRankOpening",&BishopBackRankOpening,2,true));
++	parameters.push_back(parameter("KnightBackRankOpening",&KnightBackRankOpening,2,true));
++	parameters.push_back(parameter("RookBackRankOpening",&RookBackRankOpening,2,true));
++	parameters.push_back(parameter("QueenBackRankOpening",&QueenBackRankOpening,2,true));
++	parameters.push_back(parameter("BishopOnBigDiagonals",&BishopOnBigDiagonals,2,true));

+	parameters.push_back(parameter("queenMobilityPars",&queenMobilityPars,4,true));
+	parameters.push_back(parameter("rookMobilityPars",&rookMobilityPars,4,true));
+	parameters.push_back(parameter("bishopMobilityPars",&bishopMobilityPars,4,true));
+	parameters.push_back(parameter("knightMobilityPars",&knightMobilityPars,4,true));
++	parameters.push_back(parameter("isolatedPawnPenalty",&isolatedPawnPenalty,2));
++	parameters.push_back(parameter("isolatedPawnPenaltyOpp",&isolatedPawnPenaltyOpp,2));
++	parameters.push_back(parameter("doubledPawnPenalty",&doubledPawnPenalty,2));
++	parameters.push_back(parameter("backwardPawnPenalty",&backwardPawnPenalty,2));*/
	parameters.push_back(parameter("chainedPawnBonus",&chainedPawnBonus,2));
	parameters.push_back(parameter("chainedPawnBonusOffset",&chainedPawnBonusOffset,2));
	parameters.push_back(parameter("chainedPawnBonusOpp",&chainedPawnBonusOpp,2));
	parameters.push_back(parameter("chainedPawnBonusOffsetOpp",&chainedPawnBonusOffsetOpp,2));
/*	parameters.push_back(parameter("passedPawnFileAHPenalty",&passedPawnFileAHPenalty,2));
	parameters.push_back(parameter("passedPawnSupportedBonus",&passedPawnSupportedBonus,2));
++	parameters.push_back(parameter("candidateBonus",&candidateBonus,2));
	parameters.push_back(parameter("passedPawnBonus",&passedPawnBonus,2));
	parameters.push_back(parameter("passedPawnUnsafeSquares",&passedPawnUnsafeSquares,2));
	parameters.push_back(parameter("passedPawnBlockedSquares",&passedPawnBlockedSquares,2));
	parameters.push_back(parameter("passedPawnDefendedSquares",&passedPawnDefendedSquares,2));
	parameters.push_back(parameter("passedPawnDefendedBlockingSquare",&passedPawnDefendedBlockingSquare,2));
	parameters.push_back(parameter("unstoppablePassed",&unstoppablePassed,2));
	parameters.push_back(parameter("rookBehindPassedPawn",&rookBehindPassedPawn,2));
	parameters.push_back(parameter("EnemyRookBehindPassedPawn",&EnemyRookBehindPassedPawn,2));
++	parameters.push_back(parameter("holesPenalty",&holesPenalty,2));
++	parameters.push_back(parameter("pawnCenterControl",&pawnCenterControl,2));
++	parameters.push_back(parameter("pawnBigCenterControl",&pawnBigCenterControl,2));

+	parameters.push_back(parameter("pieceCoordination[2]",&pieceCoordination[2],2));
+	parameters.push_back(parameter("pieceCoordination[3]",&pieceCoordination[3],2));
+	parameters.push_back(parameter("pieceCoordination[4]",&pieceCoordination[4],2));
+	parameters.push_back(parameter("pieceCoordination[5]",&pieceCoordination[5],2));
	
+	parameters.push_back(parameter("piecesCenterControl[2]",&piecesCenterControl[2],2));
+	parameters.push_back(parameter("piecesCenterControl[3]",&piecesCenterControl[3],2));
+	parameters.push_back(parameter("piecesCenterControl[4]",&piecesCenterControl[4],2));
+	parameters.push_back(parameter("piecesCenterControl[5]",&piecesCenterControl[5],2));
	
+	parameters.push_back(parameter("piecesBigCenterControl[2]",&piecesBigCenterControl[2],2));
+	parameters.push_back(parameter("piecesBigCenterControl[3]",&piecesBigCenterControl[3],2));
+	parameters.push_back(parameter("piecesBigCenterControl[4]",&piecesBigCenterControl[4],2));
+	parameters.push_back(parameter("piecesBigCenterControl[5]",&piecesBigCenterControl[5],2));
	
+	parameters.push_back(parameter("rookOn7Bonus",&rookOn7Bonus,2));
+	parameters.push_back(parameter("rookOnPawns",&rookOnPawns,2));
+	parameters.push_back(parameter("queenOn7Bonus",&queenOn7Bonus,2));
+	parameters.push_back(parameter("queenOnPawns",&queenOnPawns,2));
+	parameters.push_back(parameter("rookOnOpen",&rookOnOpen,2));
+	parameters.push_back(parameter("rookOnSemi",&rookOnSemi,2));
+	parameters.push_back(parameter("rookTrapped",&rookTrapped,2));
+	parameters.push_back(parameter("rookTrappedKingWithoutCastling",&rookTrappedKingWithoutCastling,2));
+	parameters.push_back(parameter("knightOnOutpost",&knightOnOutpost,2));
+	parameters.push_back(parameter("knightOnOutpostSupported",&knightOnOutpostSupported,2));
+	parameters.push_back(parameter("knightOnHole",&knightOnHole,2));
+	parameters.push_back(parameter("KnightAttackingWeakPawn",&KnightAttackingWeakPawn,2));
+	parameters.push_back(parameter("bishopOnOutpost",&bishopOnOutpost,2));
+	parameters.push_back(parameter("bishopOnOutpostSupported",&bishopOnOutpostSupported,2));
+	parameters.push_back(parameter("bishopOnHole",&bishopOnHole,2));
+	parameters.push_back(parameter("badBishop",&badBishop,2));
++	parameters.push_back(parameter("tempo",&tempo,2));
++	parameters.push_back(parameter("bishopPair",&bishopPair,2));
++	parameters.push_back(parameter("queenVsRook2MinorsImbalance",&queenVsRook2MinorsImbalance,2));
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

+	parameters.push_back(parameter("mobilityBonus[Position::Knights][0]",&mobilityBonus[Position::Knights][0],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][1]",&mobilityBonus[Position::Knights][1],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][2]",&mobilityBonus[Position::Knights][2],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][3]",&mobilityBonus[Position::Knights][3],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][4]",&mobilityBonus[Position::Knights][4],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][5]",&mobilityBonus[Position::Knights][5],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][6]",&mobilityBonus[Position::Knights][6],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][7]",&mobilityBonus[Position::Knights][7],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Knights][8]",&mobilityBonus[Position::Knights][8],2));
	
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][0]",&mobilityBonus[Position::Bishops][0],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][1]",&mobilityBonus[Position::Bishops][1],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][2]",&mobilityBonus[Position::Bishops][2],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][3]",&mobilityBonus[Position::Bishops][3],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][4]",&mobilityBonus[Position::Bishops][4],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][5]",&mobilityBonus[Position::Bishops][5],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][6]",&mobilityBonus[Position::Bishops][6],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][7]",&mobilityBonus[Position::Bishops][7],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][8]",&mobilityBonus[Position::Bishops][8],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][9]",&mobilityBonus[Position::Bishops][9],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][10]",&mobilityBonus[Position::Bishops][10],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][11]",&mobilityBonus[Position::Bishops][11],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][12]",&mobilityBonus[Position::Bishops][12],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][13]",&mobilityBonus[Position::Bishops][13],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][14]",&mobilityBonus[Position::Bishops][14],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][15]",&mobilityBonus[Position::Bishops][15],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Bishops][16]",&mobilityBonus[Position::Bishops][16],2));

+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][0]",&mobilityBonus[Position::Rooks][0],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][1]",&mobilityBonus[Position::Rooks][1],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][2]",&mobilityBonus[Position::Rooks][2],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][3]",&mobilityBonus[Position::Rooks][3],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][4]",&mobilityBonus[Position::Rooks][4],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][5]",&mobilityBonus[Position::Rooks][5],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][6]",&mobilityBonus[Position::Rooks][6],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][7]",&mobilityBonus[Position::Rooks][7],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][8]",&mobilityBonus[Position::Rooks][8],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][9]",&mobilityBonus[Position::Rooks][9],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][10]",&mobilityBonus[Position::Rooks][10],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][11]",&mobilityBonus[Position::Rooks][11],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][12]",&mobilityBonus[Position::Rooks][12],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][13]",&mobilityBonus[Position::Rooks][13],2));
+	parameters.push_back(parameter("mobilityBonus[Position::Rooks][14]",&mobilityBonus[Position::Rooks][14],2));
	
*/


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
	
	/*for( int i = 0; i < arraySize; ++i )
	{
	std::cout<<"["<<arrayScaling*(i)<<";"<<arrayScaling*(i+1)<<") "<<array[i]<<std::endl;
	}
	return 0;*/

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
		Position::initScoreValues();
		


		error = calcError2();

		std::cout<<"newError "<<error<<std::endl;
		if( error < minValue)
		{
			for(auto& p : parameters)
			{
				for( unsigned int i = 0; i < p.count; ++i)
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


