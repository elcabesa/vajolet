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
#include "syzygy/tbprobe.h"
#include "eval.h"

#include <fstream>
#include <random>

std::ifstream infile("quiet-labeled.epd");
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
			std::size_t delimiter = line.find("c9 \"");
			std::string FEN = line.substr(0, delimiter);
			std::size_t end = line.find("\"", delimiter + 4);
			std::string RESULT = line.substr(delimiter + 4, end - delimiter - 4);
			if(RESULT == "1-0")
			{
				res = 1.0;
				//std::cout << RESULT <<": 1"<< std::endl;
			}
			else if(RESULT == "0-1")
			{
				res = 0.0;
				//std::cout << RESULT <<": 0"<< std::endl;
			}else if (RESULT == "1/2-1/2")
			{
				res = 0.5;
				//std::cout << RESULT <<": 0.5"<< std::endl;
			}
			else
			{
				std::cout << "ERRORE" << std::endl;
				return -1;
			}

			positions.emplace_back(results(FEN, res));
			//std::cout << RESULT << std::endl;
			/*std::cout << delimiter << std::endl;
			 std::cout << end << std::endl;
			 std::cout << RESULT << std::endl;
			 return 0;*/
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

long double calcError(void)
{
	Position p;
	initMobilityBonus();
	long double totalError = 0.0;
	const long double k = 3.7e-5;

	unsigned int counter = 0;
	for(const auto& v : positions)
	{
		++counter;
		std::string FEN = v.FEN;
		long double res = v.res;
		p.setupFromFen(FEN);
		long double eval = p.eval<false>();
		if(p.getNextTurn() == Position::blackTurn)
		{
			eval *= -1.0;
		}
		long double sigmoid = 1.0l / (1.0l + std::pow(10.0, -k * eval));
		long double error = res - sigmoid;
		totalError += std::pow(error, 2);
		/*std::cout<<FEN<<std::endl;
		 std::cout<<res<<std::endl;
		 std::cout<<eval<<std::endl;
		 std::cout<<sigmoid<<std::endl;
		 std::cout<<error<<std::endl;
		 std::cout<<"-------------------------"<<std::endl;*/
		if( counter == 1000000)
		{
			break;
		}
	}
	return totalError;
}

struct parameter
{
	std::string name;
	long double value;
	int* pointer;
	long double partialDerivate;
	long double totalGradient;
	parameter(std::string _name, int* _pointer):name(_name),pointer(_pointer){value = *pointer;partialDerivate=0;totalGradient=1e-8;}
};

std::vector<parameter> parameters;

long double calcPartialDerivate(parameter& p)
{


	const long double delta = 1.0;

	*(p.pointer) = int (p.value + delta);

	long double ep = calcError();

	*(p.pointer) = int (p.value - delta);
	long double em = calcError();

	long double pd = (ep - em)/(2.0* delta);
	*(p.pointer) = int (p.value);
	return pd;

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

	//----------------------------------
	//	main loop
	//----------------------------------
	printStartInfo();


	readFile();


	parameters.push_back(parameter("queenMobilityPars[0]",&queenMobilityPars[0]));
	parameters.push_back(parameter("queenMobilityPars[1]",&queenMobilityPars[1]));
	parameters.push_back(parameter("queenMobilityPars[2]",&queenMobilityPars[2]));
	parameters.push_back(parameter("queenMobilityPars[3]",&queenMobilityPars[3]));

	parameters.push_back(parameter("rookMobilityPars[0]",&rookMobilityPars[0]));
	parameters.push_back(parameter("rookMobilityPars[1]",&rookMobilityPars[1]));
	parameters.push_back(parameter("rookMobilityPars[2]",&rookMobilityPars[2]));
	parameters.push_back(parameter("rookMobilityPars[3]",&rookMobilityPars[3]));

	parameters.push_back(parameter("bishopMobilityPars[0]",&bishopMobilityPars[0]));
	parameters.push_back(parameter("bishopMobilityPars[1]",&bishopMobilityPars[1]));
	parameters.push_back(parameter("bishopMobilityPars[2]",&bishopMobilityPars[2]));
	parameters.push_back(parameter("bishopMobilityPars[3]",&bishopMobilityPars[3]));

	parameters.push_back(parameter("knightMobilityPars[0]",&knightMobilityPars[0]));
	parameters.push_back(parameter("knightMobilityPars[1]",&knightMobilityPars[1]));
	parameters.push_back(parameter("knightMobilityPars[2]",&knightMobilityPars[2]));
	parameters.push_back(parameter("knightMobilityPars[3]",&knightMobilityPars[3]));


	parameters.push_back(parameter("isolatedPawnPenalty[0]",&isolatedPawnPenalty[0]));
	parameters.push_back(parameter("isolatedPawnPenalty[1]",&isolatedPawnPenalty[1]));
	parameters.push_back(parameter("isolatedPawnPenaltyOpp[0]",&isolatedPawnPenaltyOpp[0]));
	parameters.push_back(parameter("isolatedPawnPenaltyOpp[1]",&isolatedPawnPenaltyOpp[1]));
	parameters.push_back(parameter("doubledPawnPenalty[0]",&doubledPawnPenalty[0]));
	parameters.push_back(parameter("doubledPawnPenalty[1]",&doubledPawnPenalty[1]));
	parameters.push_back(parameter("backwardPawnPenalty[0]",&backwardPawnPenalty[0]));
	parameters.push_back(parameter("backwardPawnPenalty[1]",&backwardPawnPenalty[1]));
	parameters.push_back(parameter("chainedPawnBonus[0]",&chainedPawnBonus[0]));
	parameters.push_back(parameter("chainedPawnBonus[1]",&chainedPawnBonus[1]));
	parameters.push_back(parameter("passedPawnFileAHPenalty[0]",&passedPawnFileAHPenalty[0]));
	parameters.push_back(parameter("passedPawnFileAHPenalty[1]",&passedPawnFileAHPenalty[1]));
	parameters.push_back(parameter("passedPawnSupportedBonus[0]",&passedPawnSupportedBonus[0]));
	parameters.push_back(parameter("passedPawnSupportedBonus[1]",&passedPawnSupportedBonus[1]));
	parameters.push_back(parameter("candidateBonus[0]",&candidateBonus[0]));
	parameters.push_back(parameter("candidateBonus[1]",&candidateBonus[1]));
	parameters.push_back(parameter("passedPawnBonus[0]",&passedPawnBonus[0]));
	parameters.push_back(parameter("passedPawnBonus[1]",&passedPawnBonus[1]));
	parameters.push_back(parameter("passedPawnUnsafeSquares[0]",&passedPawnUnsafeSquares[0]));
	parameters.push_back(parameter("passedPawnUnsafeSquares[1]",&passedPawnUnsafeSquares[1]));
	parameters.push_back(parameter("passedPawnBlockedSquares[0]",&passedPawnBlockedSquares[0]));
	parameters.push_back(parameter("passedPawnBlockedSquares[1]",&passedPawnBlockedSquares[1]));
	parameters.push_back(parameter("passedPawnDefendedSquares[0]",&passedPawnDefendedSquares[0]));
	parameters.push_back(parameter("passedPawnDefendedSquares[1]",&passedPawnDefendedSquares[1]));
	parameters.push_back(parameter("passedPawnDefendedBlockingSquare[0]",&passedPawnDefendedBlockingSquare[0]));
	parameters.push_back(parameter("passedPawnDefendedBlockingSquare[1]",&passedPawnDefendedBlockingSquare[1]));
	parameters.push_back(parameter("unstoppablePassed[0]",&unstoppablePassed[0]));
	parameters.push_back(parameter("unstoppablePassed[1]",&unstoppablePassed[1]));
	parameters.push_back(parameter("rookBehindPassedPawn[0]",&rookBehindPassedPawn[0]));
	parameters.push_back(parameter("rookBehindPassedPawn[1]",&rookBehindPassedPawn[1]));
	parameters.push_back(parameter("EnemyRookBehindPassedPawn[0]",&EnemyRookBehindPassedPawn[0]));
	parameters.push_back(parameter("EnemyRookBehindPassedPawn[1]",&EnemyRookBehindPassedPawn[1]));

	parameters.push_back(parameter("holesPenalty[0]",&holesPenalty[0]));
	parameters.push_back(parameter("holesPenalty[1]",&holesPenalty[1]));
	parameters.push_back(parameter("pawnCenterControl[0]",&pawnCenterControl[0]));
	parameters.push_back(parameter("pawnCenterControl[1]",&pawnCenterControl[1]));
	parameters.push_back(parameter("pawnBigCenterControl[0]",&pawnBigCenterControl[0]));
	parameters.push_back(parameter("pawnBigCenterControl[1]",&pawnBigCenterControl[1]));
	parameters.push_back(parameter("pieceCoordination[0]",&pieceCoordination[0]));
	parameters.push_back(parameter("pieceCoordination[1]",&pieceCoordination[1]));

	parameters.push_back(parameter("piecesCenterControl[0]",&piecesCenterControl[0]));
	parameters.push_back(parameter("piecesCenterControl[1]",&piecesCenterControl[1]));

	parameters.push_back(parameter("piecesBigCenterControl[0]",&piecesBigCenterControl[0]));
	parameters.push_back(parameter("piecesBigCenterControl[1]",&piecesBigCenterControl[1]));
	parameters.push_back(parameter("rookOn7Bonus[0]",&rookOn7Bonus[0]));
	parameters.push_back(parameter("rookOn7Bonus[1]",&rookOn7Bonus[1]));
	parameters.push_back(parameter("rookOnPawns[0]",&rookOnPawns[0]));
	parameters.push_back(parameter("rookOnPawns[1]",&rookOnPawns[1]));

	parameters.push_back(parameter("queenOn7Bonus[0]",&queenOn7Bonus[0]));
	parameters.push_back(parameter("queenOn7Bonus[1]",&queenOn7Bonus[1]));
	parameters.push_back(parameter("queenOnPawns[0]",&queenOnPawns[0]));
	parameters.push_back(parameter("queenOnPawns[1]",&queenOnPawns[1]));
	parameters.push_back(parameter("rookOnOpen[0]",&rookOnOpen[0]));
	parameters.push_back(parameter("rookOnOpen[1]",&rookOnOpen[1]));
	parameters.push_back(parameter("rookOnSemi[0]",&rookOnSemi[0]));
	parameters.push_back(parameter("rookOnSemi[1]",&rookOnSemi[1]));
	parameters.push_back(parameter("rookTrapped[0]",&rookTrapped[0]));
	parameters.push_back(parameter("rookTrapped[1]",&rookTrapped[1]));

	parameters.push_back(parameter("rookTrappedKingWithoutCastling[0]",&rookTrappedKingWithoutCastling[0]));
	parameters.push_back(parameter("rookTrappedKingWithoutCastling[1]",&rookTrappedKingWithoutCastling[1]));
	parameters.push_back(parameter("knightOnOutpost[0]",&knightOnOutpost[0]));
	parameters.push_back(parameter("knightOnOutpost[1]",&knightOnOutpost[1]));
	parameters.push_back(parameter("knightOnOutpostSupported[0]",&knightOnOutpostSupported[0]));
	parameters.push_back(parameter("knightOnOutpostSupported[1]",&knightOnOutpostSupported[1]));
	parameters.push_back(parameter("knightOnHole[0]",&knightOnHole[0]));
	parameters.push_back(parameter("knightOnHole[1]",&knightOnHole[1]));
	parameters.push_back(parameter("KnightAttackingWeakPawn[0]",&KnightAttackingWeakPawn[0]));
	parameters.push_back(parameter("KnightAttackingWeakPawn[1]",&KnightAttackingWeakPawn[1]));
	parameters.push_back(parameter("bishopOnOutpost[0]",&bishopOnOutpost[0]));
	parameters.push_back(parameter("bishopOnOutpost[1]",&bishopOnOutpost[1]));
	parameters.push_back(parameter("bishopOnOutpostSupported[0]",&bishopOnOutpostSupported[0]));
	parameters.push_back(parameter("bishopOnOutpostSupported[1]",&bishopOnOutpostSupported[1]));
	parameters.push_back(parameter("bishopOnHole[0]",&bishopOnHole[0]));
	parameters.push_back(parameter("bishopOnHole[1]",&bishopOnHole[1]));
	parameters.push_back(parameter("badBishop[0]",&badBishop[0]));
	parameters.push_back(parameter("badBishop[1]",&badBishop[1]));

	parameters.push_back(parameter("tempo[0]",&tempo[0]));
	parameters.push_back(parameter("tempo[1]",&tempo[1]));
	parameters.push_back(parameter("bishopPair[0]",&bishopPair[0]));
	parameters.push_back(parameter("bishopPair[1]",&bishopPair[1]));
	parameters.push_back(parameter("ownKingNearPassedPawn[0]",&ownKingNearPassedPawn[0]));
	parameters.push_back(parameter("ownKingNearPassedPawn[1]",&ownKingNearPassedPawn[1]));
	parameters.push_back(parameter("enemyKingNearPassedPawn[0]",&enemyKingNearPassedPawn[0]));
	parameters.push_back(parameter("enemyKingNearPassedPawn[1]",&enemyKingNearPassedPawn[1]));
	parameters.push_back(parameter("spaceBonus[0]",&spaceBonus[0]));
	parameters.push_back(parameter("spaceBonus[1]",&spaceBonus[1]));
	parameters.push_back(parameter("undefendedMinorPenalty[0]",&undefendedMinorPenalty[0]));
	parameters.push_back(parameter("undefendedMinorPenalty[1]",&undefendedMinorPenalty[1]));



	/*
	for(auto& p : parameters)
	{
		std::cout<<p.name<<" "<<p.value<<std::endl;
	}*/

	std::vector<parameter> bestParameters;

	long double learningRate = 0.1;
	long double minValue = 1e6;


	std::random_device rd;
	std::mt19937 g(rd());
	unsigned long iteration = 0;
	unsigned long bestIteration = 0;

	bool stop = false;
	while(!stop)
	{
		++iteration;
		std::cout<<"iteration #"<<iteration<<std::endl;
		std::cout<<"shuffle"<<std::endl;
		std::shuffle(positions.begin(), positions.end(), g);

		std::cout<<"--------------------------------------------------------------------"<<std::endl;
		std::cout<<"calc gradient"<<std::endl;
		long double gradientMagnitude = 0.0;
		unsigned int par = 0;
		for(auto& p : parameters)
		{
			++par;
			//std::cout<<"calc gradient "<<par<<"/"<<parameters.size()<<std::endl;
			p.partialDerivate = calcPartialDerivate(p);
			p.totalGradient += std::pow(p.partialDerivate, 2.0);
			gradientMagnitude += std::pow(p.partialDerivate, 2.0);
			std::cout<<"dy/d"<<p.name<<" "<<p.partialDerivate<<std::endl;
			std::cout<<p.name<<" "<<p.totalGradient<<std::endl;
		}
		std::cout<<"\ngradient magnitude "<<gradientMagnitude<<std::endl;
		if(gradientMagnitude < 1e-6)
		{
			stop = true;
		}


		std::cout<<"--------------------------------------------------------------------"<<std::endl;
		std::cout<<"newValues:"<<std::endl;
		for(auto& p : parameters)
		{
			std::cout<<"new learning rate  "<<learningRate/(std::pow(p.totalGradient,0.5))<<std::endl;
			p.value -= learningRate/(std::pow(p.totalGradient,0.5)) * p.partialDerivate;
			std::cout<<p.name<<" "<<p.value<<std::endl;
		}
		double error = calcError();
		if( error < minValue)
		{
			std::cout<<"--------------------------------------------------------------------"<<std::endl;
			std::cout<<"###### NEW BEST ITERATION ####"<<std::endl;
			minValue = error;
			bestIteration = iteration;
			std::copy(parameters.begin(), parameters.end(),std::back_inserter(bestParameters));

		}
		std::cout<<"--------------------------------------------------------------------"<<std::endl;


		std::cout<<"newError "<<error<<std::endl;
		std::cout<<"bestIteration "<<bestIteration<<" minError "<<minValue<<std::endl;

	}


	return 0;
}


