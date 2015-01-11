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
#include "tune.h"
#include <vector>
#include <iomanip>
#include <cmath>
#include "eval.h"
#include "position.h"
#include "search.h"
#include "transposition.h"

double Tuner::parseEpd(bool save=false){
	Position::initScoreValues();
	TT.clear();
	initMobilityBonus();
	//sync_cout<<"start parsing epd"<<sync_endl;
	std::ifstream ifs (epdFile, std::ifstream::in);
	if ( ifs.fail()){
		sync_cout << "epd opening error"<<sync_endl;
		return 0;
	}
	char fenstr[256];


	std::string newGameString("new game");
	Position pos;
	search searcher;
	searcher.history.clear();
	double sum=0;
	unsigned long positions=0;
	unsigned long counter=0;
	do{
		ifs.getline(fenstr,256);
		if(newGameString.compare(fenstr)!=0){
			std::string fen=fenstr;
			if(fen.length()!=0){
				std::string clearFen=fen.substr(0,fen.find("result")-1);
				std::string result=fen.substr(fen.find("result")+7);
				double scoreRes= stod(result);

				//double modifiedScoreRes=std::min(std::max((0.5+(scoreRes-0.5)*0.005*positions),0.0),1.0);


				pos.setupFromFen(clearFen);
				PVline childPV;
				Score score=searcher.qsearch<search::nodeType::PV_NODE>(0,pos,0,-SCORE_INFINITE,SCORE_INFINITE,&childPV);

				double error=std::pow(scoreRes-(1.0/(1+std::pow(2.71828182846,-score/scaling))),2);
				if(save && std::abs(error)>1.1){
					sync_cout<<pos.displayFen()<<sync_endl;
				}
				sum+=error;
				++positions;
				++counter;
			}
		}
		else{
			positions=0;
		}


	}while(ifs.good());
	sum/=counter;
	double goodness=std::sqrt(sum);
	//sync_cout<<"goodness:"<<goodness<<sync_endl;
	//std::cout<<"feature tested #"<<Position::testPointCounter<<" times"<<std::endl;
	//Position::testPointCounter=0;
	return goodness;
	//sync_cout<<"finish"<<sync_endl;
}


void Tuner::drawSigmoid(void){
	double sum[200]={0};
	unsigned long count[200]={0};
	//sync_cout<<"start parsing epd"<<sync_endl;
	std::ifstream ifs (epdFile, std::ifstream::in);
	if ( ifs.fail()){
		sync_cout << "epd opening error"<<sync_endl;
	}

	char fenstr[256];

	std::string newGameString("new game");
	Position pos;
	search searcher;
	//unsigned long positions=0;
	do{
		ifs.getline(fenstr,256);

		if(newGameString.compare(fenstr)!=0){
			std::string fen=fenstr;
			if(fen.length()!=0){
				std::string clearFen=fen.substr(0,fen.find("result")-1);
				std::string result=fen.substr(fen.find("result")+7);
				double scoreRes= stod(result);



				pos.setupFromFen(clearFen);
				PVline childPV;
				Score score=searcher.qsearch<search::nodeType::PV_NODE>(0,pos,0,-SCORE_INFINITE,SCORE_INFINITE,&childPV);
				unsigned int index=0;

				//sync_cout<<"fen "<<clearFen<<" result "<<scoreRes<< " score "<< score<<sync_endl;
				if(score<=-100000){
					index=0;
				}else if(score>=99000){
					index=199;
				}
				else{

					index=100+score/1000;
					//sync_cout<<score<<" "<< index<<sync_endl;
				}
				//sync_cout<<"index "<<index<<sync_endl;

				sum[index]+=scoreRes;
				count[index]+=1;

				//++positions;
			}

		}


	}while(ifs.good());

	for(int i=0;i<200;i++){
		sum[i]/=count[i];
	}

	for(int i=0;i<200;i++){
		double eval= (i-100)/10.0;
		sync_cout<<eval<<" "<<sum[i]<<sync_endl;
	}


}


void Tuner::drawAverageEvolution(void){
	double sum[200]={0};
	unsigned long count[200]={0};
	//sync_cout<<"start parsing epd"<<sync_endl;
	std::ifstream ifs (epdFile, std::ifstream::in);
	if ( ifs.fail()){
		sync_cout << "epd opening error"<<sync_endl;
	}

	char fenstr[256];

	std::string newGameString("new game");
	Position pos;
	search searcher;
	unsigned long positions=0;
	do{
		ifs.getline(fenstr,256);

		if(newGameString.compare(fenstr)!=0){
			std::string fen=fenstr;
			if(fen.length()!=0){
				std::string clearFen=fen.substr(0,fen.find("result")-1);
				std::string result=fen.substr(fen.find("result")+7);
				double scoreRes= stod(result);



				pos.setupFromFen(clearFen);
				PVline childPV;
				Score score=searcher.qsearch<search::nodeType::PV_NODE>(0,pos,0,-SCORE_INFINITE,SCORE_INFINITE,&childPV);
				if(scoreRes!=0.5){
					double error=std::abs(scoreRes-(1.0/(1+std::pow(2.71828182846,-score/scaling))));
					unsigned int index=std::min(positions,(unsigned long int)199);
					sum[index]+=error;
					count[index]+=1;
				}

				++positions;
			}

		}
		else{
			positions=0;
		}


	}while(ifs.good());

	for(int i=0;i<200;i++){
		sum[i]/=count[i];
	}

	for(int i=0;i<200;i++){
		sync_cout<<i<<" "<<sum[i]<<sync_endl;
	}


}

void Tuner::createEpd(void){
	searchLimits sl;

	search searcher;
	sync_cout<<"start parsing epd"<<sync_endl;
	std::ifstream ifs ("out.epd", std::ifstream::in);
	std::ifstream ifs2 ("out.epd", std::ifstream::in);
	std::ofstream ofs ("out2.epd", std::ifstream::out);


	if ( ifs.fail()){
		sync_cout << "epd opening error"<<sync_endl;
		return;
	}
	std::string initialFen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

	std::string lastFen="";
	float result=0;
	bool draw=false;
	unsigned long line=0;
	unsigned long line2=0;
	char fenstr[256];
	unsigned int games=0;
	do{
		ifs.getline(fenstr,256);
		++line;
		std::string fen=fenstr;
		std::string clearFen=fen.substr(0,fen.find("bm")-1);
		if(initialFen.compare(clearFen)==0){
			if(lastFen!=""){// discard first position found
				////////////////////////////////////////////
				//detect the result of the previous game
				////////////////////////////////////////////
				std::string tempFen=lastFen;
				std::string clearTempFen=tempFen.substr(0,tempFen.find("bm")-1);
				Position pos;
				pos.setupFromFen(clearTempFen);
				sl.depth=10;
				Score searchRes=searcher.startThinking(pos,sl);
				unsigned int res=lastFen.find('w');

				if(abs(searchRes)<10000){
					draw=true;
					result=0.5;
				}
				else{
					draw=false;
					sync_cout<<"game result "<<searchRes<<sync_endl;
					if(res!=std::string::npos && searchRes>=10000){ // white
						result=1;
						//ofs.write("white win\n",9);
					}
					else{
						//ofs.write("black win\n",9);
						result=0;
					}
				}
				/*unsigned int res=lastFen.find('#');
				if(res!=std::string::npos){
					res=lastFen.find('w');
					if(res!=std::string::npos){
						sync_cout<<"white mate"<<sync_endl;
						result=1;
					}
					else{
						sync_cout<<"black mate"<<sync_endl;
						result=0;
					}
					draw=false;
				}
				else{
					sync_cout<<"draw"<<sync_endl;
					result=0.5;
					draw=true;
				}*/
				unsigned long startLine=line2;
				////////////////////////////////////////////////////////////////////////////////
				// now that i know the result i can create the new file with the result appended
				////////////////////////////////////////////////////////////////////////////////
				ofs.write("new game\n",9);
				sl.depth=5;
				while(line2<line-1){





					ifs2.getline(fenstr,256);
					std::string fen=fenstr;

					std::string clearFen=fen.substr(0,fen.find("bm")-1);
					if((signed long)(line2-startLine)>=0 && (draw || (signed long)(line-line2)>=0)){
						Position pos;
						pos.setupFromFen(clearFen);
						Score searchRes;
						if(result==0){
							searchRes=searcher.startThinking(pos,sl);
							searchRes-=5000.0+10000.0*(line2-startLine)/(line-startLine);
						}
						else if (result==1)
						{
							searchRes=searcher.startThinking(pos,sl);
							searchRes+=5000.0+10000.0*(line2-startLine)/(line-startLine);
						}
						else{
							searchRes=0.5;
						}

						double sigmoid=1.0/(1+std::pow(2.71828182846,-searchRes/scaling));
						ofs.write(clearFen.c_str(),clearFen.length());
						ofs.write(" result ",8);
						std::string strRes=std::to_string(sigmoid);
						//sync_cout<<line2<<" * "<<sync_endl;
						ofs.write(strRes.c_str(),strRes.length());
						ofs.write("\n",1);
					}
					/*else{
						sync_cout<<line2<<sync_endl;
					}*/
					++line2;
					result=1-result;
				}
			}
			sync_cout<<"new game "<<(++games)<<sync_endl;


		}
		//update last fen
		lastFen=std::string(fen);

	}while(ifs.good());
	ifs.close();
	ifs2.close();
	ofs.close();
	sync_cout<<"finish"<<sync_endl;
}

void Tuner::showValues(const std::vector<parameterStruct>& parameters) {
	sync_cout<<"---------PARAMETERS---------"<<sync_endl;
	for(unsigned int index=0;index<parameters.size();index++) {

		sync_cout<<parameters[index].name<<":"<<(*parameters[index].parameter)[parameters[index].index]<<sync_endl;

	}
	sync_cout<<"----------------------------"<<sync_endl;
}

void Tuner::tuneParameters(void){

	enum searchState{
		initialization,
		findDirection,
		iterateDirection,
		finalize,
		end
	}state;
	std::vector<parameterStruct> parameters;

	parameters.push_back(parameterStruct("queen opening value",&initialPieceValue[Position::whiteQueens],0,1000));
	parameters.push_back(parameterStruct("queen endgame value",&initialPieceValue[Position::whiteQueens],1,1000));
	parameters.push_back(parameterStruct("rook opening value",&initialPieceValue[Position::whiteRooks],0,1000));
	parameters.push_back(parameterStruct("rook endgame value",&initialPieceValue[Position::whiteRooks],1,1000));
	parameters.push_back(parameterStruct("bishop opening value",&initialPieceValue[Position::whiteBishops],0,100));
	parameters.push_back(parameterStruct("bishop endgame value",&initialPieceValue[Position::whiteBishops],1,100));
	parameters.push_back(parameterStruct("knight opening value",&initialPieceValue[Position::whiteKnights],0,100));
	parameters.push_back(parameterStruct("knight endgame value",&initialPieceValue[Position::whiteKnights],1,100));
	parameters.push_back(parameterStruct("pawn opening value",&initialPieceValue[Position::whitePawns],0,100));
/*
	parameters.push_back(parameterStruct("PawnD3 opening bonus",&PawnD3,0,10));
//	parameters.push_back(parameterStruct("PawnD3 endgame bonus",&PawnD3,1,10));
	parameters.push_back(parameterStruct("PawnD4 opening bonus",&PawnD4,0,10));
//	parameters.push_back(parameterStruct("PawnD4 endgame bonus",&PawnD4,1,10));
	parameters.push_back(parameterStruct("PawnD5 opening bonus",&PawnD5,0,10));
//	parameters.push_back(parameterStruct("PawnD5 endgame bonus",&PawnD5,1,10));
	parameters.push_back(parameterStruct("PawnE3 opening bonus",&PawnE3,0,10));
//	parameters.push_back(parameterStruct("PawnE3 endgame bonus",&PawnE3,1,10));
	parameters.push_back(parameterStruct("PawnE4 opening bonus",&PawnE4,0,10));
//	parameters.push_back(parameterStruct("PawnE4 endgame bonus",&PawnE4,1,10));
	parameters.push_back(parameterStruct("PawnE5 opening bonus",&PawnE5,0,10));
//	parameters.push_back(parameterStruct("PawnE5 endgame bonus",&PawnE5,1,10));
	parameters.push_back(parameterStruct("PawnCentering opening bonus",&PawnCentering,0,10));
	parameters.push_back(parameterStruct("PawnCentering endgame bonus",&PawnCentering,1,10));
	parameters.push_back(parameterStruct("PawnRank opening bonus",&PawnRankBonus,0,10));
	parameters.push_back(parameterStruct("PawnRank endgame bonus",&PawnRankBonus,1,10));
	parameters.push_back(parameterStruct("KnightPST opening bonus",&KnightPST,0,10));
	parameters.push_back(parameterStruct("KnightPST endgame bonus",&KnightPST,1,10));
	parameters.push_back(parameterStruct("BishopPST opening bonus",&BishopPST,0,10));
	parameters.push_back(parameterStruct("BishopPST endgame bonus",&BishopPST,1,10));
	parameters.push_back(parameterStruct("RookPST opening bonus",&RookPST,0,10));
	parameters.push_back(parameterStruct("RookPST endgame bonus",&RookPST,1,10));
	parameters.push_back(parameterStruct("QueenPST opening bonus",&QueenPST,0,10));
	parameters.push_back(parameterStruct("QueenPST endgame bonus",&QueenPST,1,10));
	parameters.push_back(parameterStruct("KingPST opening bonus",&KingPST,0,10));
	parameters.push_back(parameterStruct("KingPST endgame bonus",&KingPST,1,10));*/
	/*
	parameters.push_back(parameterStruct("BishopBackRankOpening opening penalty",&BishopBackRankOpening,0,100));
	parameters.push_back(parameterStruct("BishopBackRankOpening endgame penalty",&BishopBackRankOpening,1,100));
	parameters.push_back(parameterStruct("KnightBackRankOpening opening penalty",&KnightBackRankOpening,0,100));
	parameters.push_back(parameterStruct("KnightBackRankOpening endgame penalty",&KnightBackRankOpening,1,100));
	parameters.push_back(parameterStruct("RookBackRankOpening opening penalty",&RookBackRankOpening,0,100));
	parameters.push_back(parameterStruct("RookBackRankOpening endgame penalty",&RookBackRankOpening,1,100));
	parameters.push_back(parameterStruct("QueenBackRankOpening opening penalty",&QueenBackRankOpening,0,100));
	parameters.push_back(parameterStruct("QueenBackRankOpening endgame penalty",&QueenBackRankOpening,1,100));
	parameters.push_back(parameterStruct("BishopOnBigDiagonals opening bonus",&BishopOnBigDiagonals,0,100));
	parameters.push_back(parameterStruct("BishopOnBigDiagonals endgame bonus",&BishopOnBigDiagonals,1,100));

	//-------------------------------------------------------------------------------------------------------------

	parameters.push_back(parameterStruct("tempo opening bonus",&tempo,0,10));
	parameters.push_back(parameterStruct("bishopPair opening bonus",&bishopPair,0,10));
	parameters.push_back(parameterStruct("isolatedPawnPenalty opening bonus",&isolatedPawnPenalty,0,10));
	parameters.push_back(parameterStruct("isolatedPawnPenaltyOpp opening bonus",&isolatedPawnPenaltyOpp,0,10));
	parameters.push_back(parameterStruct("doubledPawnPenalty opening bonus",&doubledPawnPenalty,0,10));
	parameters.push_back(parameterStruct("backwardPawnPenalty opening bonus",&backwardPawnPenalty,0,10));
	parameters.push_back(parameterStruct("chainedPawnBonus opening bonus",&chainedPawnBonus,0,10));
	parameters.push_back(parameterStruct("passedPawnFileAHPenalty opening bonus",&passedPawnFileAHPenalty,0,10));
	parameters.push_back(parameterStruct("passedPawnSupportedBonus opening bonus",&passedPawnSupportedBonus,0,10));
	parameters.push_back(parameterStruct("candidateBonus opening bonus",&candidateBonus,0,10));
	parameters.push_back(parameterStruct("holesPenalty opening bonus",&holesPenalty,0,10));
	parameters.push_back(parameterStruct("pawnCenterControl opening bonus",&pawnCenterControl,0,10));
	parameters.push_back(parameterStruct("pawnBigCenterControl opening bonus",&pawnBigCenterControl,0,10));
	parameters.push_back(parameterStruct("pieceCoordination opening bonus",&pieceCoordination,0,10));
	parameters.push_back(parameterStruct("piecesCenterControl opening bonus",&piecesCenterControl,0,10));
	parameters.push_back(parameterStruct("piecesBigCenterControl opening bonus",&piecesBigCenterControl,0,10));

	parameters.push_back(parameterStruct("tempo endgame bonus",&tempo,1,10));
	parameters.push_back(parameterStruct("bishopPair endgame bonus",&bishopPair,1,10));
	parameters.push_back(parameterStruct("isolatedPawnPenalty endgame bonus",&isolatedPawnPenalty,1,10));
	parameters.push_back(parameterStruct("isolatedPawnPenaltyOpp endgame bonus",&isolatedPawnPenaltyOpp,1,10));
	parameters.push_back(parameterStruct("doubledPawnPenalty endgame bonus",&doubledPawnPenalty,1,10));
	parameters.push_back(parameterStruct("backwardPawnPenalty endgame bonus",&backwardPawnPenalty,1,10));
	parameters.push_back(parameterStruct("chainedPawnBonus endgame bonus",&chainedPawnBonus,1,10));
	parameters.push_back(parameterStruct("passedPawnFileAHPenalty endgame bonus",&passedPawnFileAHPenalty,1,10));
	parameters.push_back(parameterStruct("passedPawnSupportedBonus endgame bonus",&passedPawnSupportedBonus,1,10));
	parameters.push_back(parameterStruct("candidateBonus endgame bonus",&candidateBonus,1,10));
	parameters.push_back(parameterStruct("holesPenalty endgame bonus",&holesPenalty,1,10));
	parameters.push_back(parameterStruct("pawnCenterControl endgame bonus",&pawnCenterControl,1,10));
	parameters.push_back(parameterStruct("pawnBigCenterControl endgame bonus",&pawnBigCenterControl,1,10));
	parameters.push_back(parameterStruct("pieceCoordination endgame bonus",&pieceCoordination,1,10));
	parameters.push_back(parameterStruct("piecesCenterControl endgame bonus",&piecesCenterControl,1,10));
	parameters.push_back(parameterStruct("piecesBigCenterControl endgame bonus",&piecesBigCenterControl,1,10));

	//--------------------------------------------------------------------------------

	parameters.push_back(parameterStruct("passedPawnBonus opening bonus",&passedPawnBonus,0,10));
	parameters.push_back(parameterStruct("ownKingNearPassedPawn opening bonus",&ownKingNearPassedPawn,0,10));
	parameters.push_back(parameterStruct("enemyKingNearPassedPawn opening bonus",&enemyKingNearPassedPawn,0,10));
	parameters.push_back(parameterStruct("passedPawnUnsafeSquares opening bonus",&passedPawnUnsafeSquares,0,10));
	parameters.push_back(parameterStruct("passedPawnBlockedSquares opening bonus",&passedPawnBlockedSquares,0,10));
	parameters.push_back(parameterStruct("passedPawnDefendedSquares opening bonus",&passedPawnDefendedSquares,0,10));
	parameters.push_back(parameterStruct("passedPawnDefendedBlockingSquare opening bonus",&passedPawnDefendedBlockingSquare,0,10));

	parameters.push_back(parameterStruct("passedPawnBonus endgame bonus",&passedPawnBonus,1,10));
	parameters.push_back(parameterStruct("ownKingNearPassedPawn endgame bonus",&ownKingNearPassedPawn,1,10));
	parameters.push_back(parameterStruct("enemyKingNearPassedPawn endgame bonus",&enemyKingNearPassedPawn,1,10));
	parameters.push_back(parameterStruct("passedPawnUnsafeSquares endgame bonus",&passedPawnUnsafeSquares,1,10));
	parameters.push_back(parameterStruct("passedPawnBlockedSquares endgame bonus",&passedPawnBlockedSquares,1,10));
	parameters.push_back(parameterStruct("passedPawnDefendedSquares endgame bonus",&passedPawnDefendedSquares,1,10));
	parameters.push_back(parameterStruct("passedPawnDefendedBlockingSquare opening bonus",&passedPawnDefendedBlockingSquare,1,10));

	*/
/*	parameters.push_back(parameterStruct("queenMobilityPars opening offset",&queenMobilityPars,0,1));
	parameters.push_back(parameterStruct("rookMobilityPars opening offset",&rookMobilityPars,0,1));
	parameters.push_back(parameterStruct("bishopMobilityPars opening offset",&bishopMobilityPars,0,1));
	parameters.push_back(parameterStruct("knightMobilityPars opening offset",&knightMobilityPars,0,1));

	parameters.push_back(parameterStruct("queenMobilityPars endgame offset",&queenMobilityPars,1,1));
	parameters.push_back(parameterStruct("rookMobilityPars endgame offset",&rookMobilityPars,1,1));
	parameters.push_back(parameterStruct("bishopMobilityPars endgame offset",&bishopMobilityPars,1,1));
	parameters.push_back(parameterStruct("knightMobilityPars endgame offset",&knightMobilityPars,1,1));
*/
/*	parameters.push_back(parameterStruct("queenMobilityPars opening gain",&queenMobilityPars,2,5));
	parameters.push_back(parameterStruct("rookMobilityPars opening gain",&rookMobilityPars,2,5));
	parameters.push_back(parameterStruct("bishopMobilityPars opening gain",&bishopMobilityPars,2,5));
	parameters.push_back(parameterStruct("knightMobilityPars opening gain",&knightMobilityPars,2,5));

	parameters.push_back(parameterStruct("queenMobilityPars endgame gain",&queenMobilityPars,3,5));
	parameters.push_back(parameterStruct("rookMobilityPars endgame gain",&rookMobilityPars,3,5));
	parameters.push_back(parameterStruct("bishopMobilityPars endgame gain",&bishopMobilityPars,3,5));
	parameters.push_back(parameterStruct("knightMobilityPars endgame gain",&knightMobilityPars,3,5));
*/

/*	parameters.push_back(parameterStruct("rookOn7Bonus opening bonus",&rookOn7Bonus,0,100));
	parameters.push_back(parameterStruct("rookOnPawns opening bonus",&rookOnPawns,0,100));
	parameters.push_back(parameterStruct("queenOn7Bonus opening bonus",&queenOn7Bonus,0,100));
	parameters.push_back(parameterStruct("queenOnPawns opening bonus",&queenOnPawns,0,100));
	parameters.push_back(parameterStruct("rookOnOpen opening bonus",&rookOnOpen,0,100));
	parameters.push_back(parameterStruct("rookOnSemi opening bonus",&rookOnSemi,0,100));
	parameters.push_back(parameterStruct("knightOnOutpost opening bonus",&knightOnOutpost,0,100));
	parameters.push_back(parameterStruct("knightOnOutpostSupported opening bonus",&knightOnOutpostSupported,0,100));
	parameters.push_back(parameterStruct("knightOnHole opening bonus",&knightOnHole,0,100));
	parameters.push_back(parameterStruct("bishopOnOutpost opening bonus",&bishopOnOutpost,0,100));
	parameters.push_back(parameterStruct("bishopOnOutpostSupported opening bonus",&bishopOnOutpostSupported,0,100));
	parameters.push_back(parameterStruct("bishopOnHole opening bonus",&bishopOnHole,0,100));
	parameters.push_back(parameterStruct("badBishop opening bonus",&badBishop,0,100));

	parameters.push_back(parameterStruct("rookOn7Bonus endgame bonus",&rookOn7Bonus,1,100));
	parameters.push_back(parameterStruct("rookOnPawns endgame bonus",&rookOnPawns,1,100));
	parameters.push_back(parameterStruct("queenOn7Bonus endgame bonus",&queenOn7Bonus,1,100));
	parameters.push_back(parameterStruct("queenOnPawns endgame bonus",&queenOnPawns,1,100));
	parameters.push_back(parameterStruct("rookOnOpen endgame bonus",&rookOnOpen,1,100));
	parameters.push_back(parameterStruct("rookOnSemi endgame bonus",&rookOnSemi,1,100));
	parameters.push_back(parameterStruct("knightOnOutpost endgame bonus",&knightOnOutpost,1,100));
	parameters.push_back(parameterStruct("knightOnOutpostSupported endgame bonus",&knightOnOutpostSupported,1,100));
	parameters.push_back(parameterStruct("knightOnHole endgame bonus",&knightOnHole,1,100));
	parameters.push_back(parameterStruct("bishopOnOutpost endgame bonus",&bishopOnOutpost,1,100));
	parameters.push_back(parameterStruct("bishopOnOutpostSupported endgame bonus",&bishopOnOutpostSupported,1,100));
	parameters.push_back(parameterStruct("bishopOnHole endgame bonus",&bishopOnHole,1,100));
	parameters.push_back(parameterStruct("badBishop endgame bonus",&badBishop,1,100));
*/
/*	parameters.push_back(parameterStruct("spaceBonus opening bonus",&spaceBonus,0,100));
	parameters.push_back(parameterStruct("undefendedMinorPenalty opening bonus",&undefendedMinorPenalty,0,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Queens] opening bonus",&attackedByPawnPenalty[Position::Queens],0,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Rooks] opening bonus",&attackedByPawnPenalty[Position::Rooks],0,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Bishops] opening bonus",&attackedByPawnPenalty[Position::Bishops],0,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Knights] opening bonus",&attackedByPawnPenalty[Position::Knights],0,100));

	parameters.push_back(parameterStruct("spaceBonus endgame bonus",&spaceBonus,1,100));
	parameters.push_back(parameterStruct("undefendedMinorPenalty endgame bonus",&undefendedMinorPenalty,1,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Queens] endgame bonus",&attackedByPawnPenalty[Position::Queens],1,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Rooks] endgame bonus",&attackedByPawnPenalty[Position::Rooks],1,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Bishops] endgame bonus",&attackedByPawnPenalty[Position::Bishops],1,100));
	parameters.push_back(parameterStruct("attackedByPawnPenalty[Knights] endgame bonus",&attackedByPawnPenalty[Position::Knights],1,100));
*/

/*
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][queen] opening bonus",&weakPiecePenalty[Position::Queens][Position::Queens],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][rook] opening bonus",&weakPiecePenalty[Position::Queens][Position::Rooks],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][bishop] opening bonus",&weakPiecePenalty[Position::Queens][Position::Bishops],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][knight] opening bonus",&weakPiecePenalty[Position::Queens][Position::Knights],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][pawn] opening bonus",&weakPiecePenalty[Position::Queens][Position::Pawns],0,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][queen] opening bonus",&weakPiecePenalty[Position::Rooks][Position::Queens],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][rook] opening bonus",&weakPiecePenalty[Position::Rooks][Position::Rooks],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][bishop] opening bonus",&weakPiecePenalty[Position::Rooks][Position::Bishops],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][knight] opening bonus",&weakPiecePenalty[Position::Rooks][Position::Knights],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][pawn] opening bonus",&weakPiecePenalty[Position::Rooks][Position::Pawns],0,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][queen] opening bonus",&weakPiecePenalty[Position::Bishops][Position::Queens],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][rook] opening bonus",&weakPiecePenalty[Position::Bishops][Position::Rooks],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][bishop] opening bonus",&weakPiecePenalty[Position::Bishops][Position::Bishops],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][knight] opening bonus",&weakPiecePenalty[Position::Bishops][Position::Knights],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][pawn] opening bonus",&weakPiecePenalty[Position::Bishops][Position::Pawns],0,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][queen] opening bonus",&weakPiecePenalty[Position::Knights][Position::Queens],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][rook] opening bonus",&weakPiecePenalty[Position::Knights][Position::Rooks],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][bishop] opening bonus",&weakPiecePenalty[Position::Knights][Position::Bishops],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][knight] opening bonus",&weakPiecePenalty[Position::Knights][Position::Knights],0,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][pawn] opening bonus",&weakPiecePenalty[Position::Knights][Position::Pawns],0,100));


	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][queen] endgame bonus",&weakPiecePenalty[Position::Queens][Position::Queens],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][rook] endgame bonus",&weakPiecePenalty[Position::Queens][Position::Rooks],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][bishop] endgame bonus",&weakPiecePenalty[Position::Queens][Position::Bishops],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][knight] endgame bonus",&weakPiecePenalty[Position::Queens][Position::Knights],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Queens][pawn] endgame bonus",&weakPiecePenalty[Position::Queens][Position::Pawns],1,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][queen] endgame bonus",&weakPiecePenalty[Position::Rooks][Position::Queens],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][rook] endgame bonus",&weakPiecePenalty[Position::Rooks][Position::Rooks],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][bishop] endgame bonus",&weakPiecePenalty[Position::Rooks][Position::Bishops],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][knight] endgame bonus",&weakPiecePenalty[Position::Rooks][Position::Knights],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Rooks][pawn] endgame bonus",&weakPiecePenalty[Position::Rooks][Position::Pawns],1,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][queen] endgame bonus",&weakPiecePenalty[Position::Bishops][Position::Queens],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][rook] endgame bonus",&weakPiecePenalty[Position::Bishops][Position::Rooks],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][bishop] endgame bonus",&weakPiecePenalty[Position::Bishops][Position::Bishops],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][knight] endgame bonus",&weakPiecePenalty[Position::Bishops][Position::Knights],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Bishops][pawn] endgame bonus",&weakPiecePenalty[Position::Bishops][Position::Pawns],1,100));

	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][queen] endgame bonus",&weakPiecePenalty[Position::Knights][Position::Queens],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][rook] endgame bonus",&weakPiecePenalty[Position::Knights][Position::Rooks],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][bishop] endgame bonus",&weakPiecePenalty[Position::Knights][Position::Bishops],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][knight] endgame bonus",&weakPiecePenalty[Position::Knights][Position::Knights],1,100));
	parameters.push_back(parameterStruct("weakPiecePenalty[Knights][pawn] endgame bonus",&weakPiecePenalty[Position::Knights][Position::Pawns],1,100));*/

	/*parameters.push_back(parameterStruct("kingShieldBonus bonus",&kingShieldBonus,0,100));
	parameters.push_back(parameterStruct("kingFarShieldBonus bonus",&kingFarShieldBonus,0,100));
	parameters.push_back(parameterStruct("kingStormBonus bonus",&kingStormBonus,0,10));
	parameters.push_back(parameterStruct("kingSafetyBonus opening bonus",&kingSafetyBonus,0,1));
	parameters.push_back(parameterStruct("kingSafetyBonus endgame bonus",&kingSafetyBonus,1,1));*/

	while(1){
		showValues(parameters);
		for(unsigned int index=0;index<parameters.size();index++){

			state=initialization;
			Score delta=1000;
			Score bestValue=0;
			double bestScore=0;
			simdScore* parameter=nullptr;
			while(state!=end){
				switch(state){
				case initialization:

					parameter=parameters[index].parameter;
					sync_cout<<std::setprecision(16)<<"-----------------------------------------------"<<sync_endl;
					sync_cout<<"optimizing parameter "<<parameters[index].name<<sync_endl;
					sync_cout<<"start Values: "<<(*parameter)[parameters[index].index]<<sync_endl;
					delta=parameters[index].delta;
					sync_cout<<"delta: "<<delta<<sync_endl;
					bestValue=(*parameter)[parameters[index].index];

					state=findDirection;
					break;

				case findDirection:
				{



					sync_cout<<"--find Direction--"<<sync_endl;

					Score newValueOriginal=bestValue;
					sync_cout<<"test value:"<<newValueOriginal<<" result:";
					(*parameter).insert(parameters[index].index,newValueOriginal);
					double ScoreOriginal=parseEpd();
					std::cout<<ScoreOriginal<<sync_endl;

					Score newValueP=bestValue+delta;
					sync_cout<<"test+ value:"<<newValueP<<" result:";

					(*parameter).insert(parameters[index].index,newValueP);
					double ScoreP=parseEpd();
					std::cout<<ScoreP<<sync_endl;

					Score newValueN=bestValue-delta;
					sync_cout<<"test- value:"<<newValueN<<" result:";


					(*parameter).insert(parameters[index].index,newValueN);
					double ScoreN=parseEpd();
					std::cout<<ScoreN<<sync_endl;

					if(ScoreP<ScoreOriginal || ScoreN<ScoreOriginal ){
						if(ScoreP<ScoreN){
							sync_cout<<"direction +"<<sync_endl;
							//delta=delta;
							bestValue=newValueP;
							bestScore=ScoreP;
						}
						else{
							sync_cout<<"direction -"<<sync_endl;
							delta=-delta;
							bestValue=newValueN;
							bestScore=ScoreN;
						}
						state= iterateDirection;
					}
					else{
						sync_cout<<"no iteration needed"<<sync_endl;
						bestValue=newValueOriginal;
						bestScore=ScoreOriginal;
						state=finalize;
					}

				}
					break;
				case iterateDirection:
				{
					//sync_cout<<"--iterate--"<<sync_endl;
					Score newValue=bestValue+delta;
					sync_cout<<"newValue: "<<newValue<< " result: ";

					(*parameter).insert(parameters[index].index,newValue);
					double Score=parseEpd();
					std::cout<<Score<<sync_endl;
					if(Score<bestScore){
						bestScore=Score;
						bestValue=newValue;
						break;
					}
					else{
						state= finalize;
					}

				}
					break;
				case finalize:
				{
					sync_cout<<"----finalize----"<<sync_endl;
					sync_cout<<"bestValue: "<<bestValue<<sync_endl;
					(*parameter).insert(parameters[index].index,bestValue);
					showValues(parameters);
					state= end;

				}
					break;
				default:
					break;
				}
			}

		}

	}






}
