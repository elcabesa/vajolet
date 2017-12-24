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
#include "parameters.h"
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

Search src;
PVline newPV;
long double calcSigleError(Position &p, long double res)
{
	const long double k = 3.7e-5;

	src.pos = p;
	newPV.reset();
	long double eval = src.qsearch<Search::nodeType::PV_NODE>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, newPV);
	//long double eval = p.eval<false>();

	//std::cout<<p.getFen()<<" "<<eval <<" "<<eval2<<" " << eval -eval2<<std::endl;

	if(p.getNextTurn() == Position::blackTurn)
	{
		eval *= -1.0;
	}
	long double sigmoid = 1.0l / (1.0l + std::pow(10.0, -k * eval));
	long double error = res - sigmoid;
	return std::pow(error, 2);
	
}

long double calcError(void)
{
	Position p;
	initMobilityBonus();
	long double totalError = 0.0;

	for(const auto& v : positions)
	{
		p.setupFromFen(v.FEN);
		totalError += calcSigleError(p, v.res);

	}
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
	if(par.requireUpdate)
	{
		Position::initPstValues();
		initMobilityBonus();
	}

}
long double calcPartialDerivate2(parameter& par,unsigned int i, Position & pos, long double res)
{

	const long double delta = 1.0;

	updateParameter(par, i, delta);

	long double ep = calcSigleError(pos, res);

	updateParameter(par, i, -delta);

	long double em = calcSigleError(pos, res);

	long double pd = (ep - em)/(2.0* delta);

	updateParameter(par, i, 0);

	return pd;

}

long double calcGradient2(long double& error)
{
	std::cout<<"calc error and gradient"<<std::endl;
	long double gradientMagnitude = 0.0;
	error= 0.0;
	
	Position pos;
	
	for(auto& par : parameters)
	{
		for( unsigned int i = 0; i < par.count; ++i)
		{
			par.partialDerivate[i] =0.0;
		}
	}
	//unsigned int tot = 0;
	for(const auto& v : positions)
	{
		//tot++;
		//std::cout<<"\r"<<tot;
		pos.setupFromFen(v.FEN);
		long double res = v.res;
		error += calcSigleError(pos, res);
		for(auto& par : parameters)
		{
			for( unsigned int i = 0; i < par.count; ++i)
			{
				par.partialDerivate[i] += calcPartialDerivate2(par,i,pos, res);
			}


		}
		
	}
	
	for(auto& p : parameters)
	{
		for( unsigned int i = 0; i < p.count; ++i)
		{
			long double x = std::pow(p.partialDerivate[i], 2.0);
			p.totalGradient[i] += x;
			gradientMagnitude += x;
		}
		
	}

	return gradientMagnitude;
	
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

/*	parameters.push_back(parameter("initialPieceValue[2]",&initialPieceValue[2],2,true));
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

	parameters.push_back(parameter("weakPawnAttackedByKing",&weakPawnAttackedByKing,2));*/

	parameters.push_back(parameter("kingShieldBonus",&kingShieldBonus,1));
	parameters.push_back(parameter("kingFarShieldBonus",&kingFarShieldBonus,1));
	parameters.push_back(parameter("kingStormBonus",&kingStormBonus,1));

	parameters.push_back(parameter("kingSafetyBonus",&kingSafetyBonus,2));

	parameters.push_back(parameter("kingSafetyScaling",&kingSafetyScaling,1));
	parameters.push_back(parameter("KingSafetyMaxAttack",&KingSafetyMaxAttack,1));
	parameters.push_back(parameter("KingSafetyLinearCoefficent",&KingSafetyLinearCoefficent,1));
	parameters.push_back(parameter("KingAttackUnitWeigth",&KingAttackUnitWeigth,4));
	parameters.push_back(parameter("KingSafetyMaxResult",&KingSafetyMaxResult,1));






	unsigned int totParameters = 0;
	for(auto& p : parameters)
	{
		for( unsigned int i = 0; i < p.count; ++i)
		{
			++totParameters;
		}
	}

	std::cout<<"start to optimizing "<<totParameters<<" parameters"<<std::endl;

	/*
	for(auto& p : parameters)
	{
		std::cout<<p.name<<" "<<p.value<<std::endl;
	}*/

	std::vector<parameter> bestParameters;

	long double learningRate = 1.0;
	long double error = 1e22;// big number
	long double minValue = 1e6;

	std::random_device rd;
	std::mt19937 g(rd());
	unsigned long iteration = 0;
	unsigned long bestIteration = 0;

	bool stop = false;

	while(!stop)
	{
		++iteration;
		std::cout<<"--------------------------------------------------------------------"<<std::endl;
		std::cout<<"iteration #"<<iteration<<std::endl;
		std::cout<<"shuffle"<<std::endl;
		std::shuffle(positions.begin(), positions.end(), g);




		long double gradientMagnitude = calcGradient2(error);

		std::cout<<"newError "<<error<<std::endl;
		if( error < minValue)
		{
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

		std::cout<<"--------------------------------------------------------------------"<<std::endl;
		std::cout<<"UPDATE PARAMETERS"<<std::endl;



		unsigned int par = 0;
		for(auto& p : parameters)
		{

			for( unsigned int i = 0; i < p.count; ++i)
			{
				++par;
				long double oldvalue = p.value[i];
				double lr = learningRate/(std::pow(p.totalGradient[i],0.5));
				p.value[i] -= lr * p.partialDerivate[i];

				std::cout<<par<<"/"<<totParameters<<": dy/d("<<p.name<<"["<<i<<"]) = "<<p.partialDerivate[i]<<" totalGradient = "<<p.totalGradient[i]<<
						" newValue = "<<p.value[i] <<" ("<<oldvalue<<") learning rate = "<<lr<<std::endl;
			}
		}
		std::cout<<"\ngradient magnitude "<<gradientMagnitude<<std::endl;
		if(gradientMagnitude < 1e-6)
		{
			stop = true;
		}






	}


	return 0;
}


