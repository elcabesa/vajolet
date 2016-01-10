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

#include <random>
#include <ctime>
#include <chrono>
#include <vector>
#include <list>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include <atomic>
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "history.h"
#include "book.h"
#include "thread.h"

#ifdef DEBUG_EVAL_SIMMETRY
	Position ppp;
#endif

std::vector<rootMove> search::rootMoves;
std::atomic<unsigned long long> search::visitedNodes;


inline signed int razorMargin(unsigned int depth)__attribute__((const));
inline signed int razorMargin(unsigned int depth){
	return 20000+depth*78;
}

void search::printPVs(unsigned int count){

	auto end = std::next(rootMoves.begin(), count);
	int i= 0;
	std::for_each(rootMoves.begin(),end, [&](rootMove& rm)
	{
		if(rm.nodes)
		{
			printPV(rm.score,rm.depth,rm.selDepth,-SCORE_INFINITE,SCORE_INFINITE,rm.time,i,rm.PV,rm.nodes);
		}
		i++;
	});
}

void search::printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, long long time,unsigned int count,std::list<Move>& PV,unsigned long long nodes){

	sync_cout<<"info multipv "<< (count+1) << " depth "<<(depth)<<" seldepth "<<seldepth <<" score ";

	if(abs(res) >SCORE_MATE_IN_MAX_PLY){
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else{
		int satRes= std::min(res,SCORE_MAX_OUTPUT_VALUE);
		satRes= std::max(satRes,SCORE_MIN_OUTPUT_VALUE);
		std::cout<< "cp "<<(int)((float)satRes/100.0);
	}

	std::cout<<(res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");

	std::cout<<" nodes "<<nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout<<" nps "<<(unsigned int)((double)nodes*1000/(double)time)<<" time "<<(long long int)(time);
#endif

	std::cout<<" pv ";
	for_each(PV.begin(), PV.end(), [&](Move &m){std::cout<<Position::displayUci(m)<<" ";});
	std::cout<<sync_endl;
}


Score search::futility[5]={0,6000,20000,30000,40000};
Score search::futilityMargin[7]={0,10000,20000,30000,40000,50000,60000};
unsigned int search::FutilityMoveCounts[11]={5,10,17,26,37,50,66,85,105,130,151};
Score search::PVreduction[32*ONE_PLY][64];
Score search::nonPVreduction[32*ONE_PLY][64];
unsigned int search::threads=1;
unsigned int search::multiPVLines=1;
unsigned int search::limitStrength=0;
unsigned int search::eloStrenght=3000;
bool search::useOwnBook=true;
bool search::bestMoveBook=false;
bool search::showCurrentLine=false;

Score search::startThinking(searchLimits & l){
	Score res=0;
	signals.stop=false;
	TT.newSearch();
	history.clear();
	visitedNodes=0;
	std::vector<search> helperSearch(threads-1);

	limits=l;
	rootMoves.clear();
	//--------------------------------
	//	generate the list of root moves to be searched
	//--------------------------------
	if(limits.searchMoves.size()==0){
		Move m(Movegen::NOMOVE);
		Movegen mg(pos,history,m);
		while ((m=mg.getNextMove())!= Movegen::NOMOVE){
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=m;
			rm.selDepth=0;
			rm.depth=0;
			rm.nodes=0;
			rm.time=0;
			rm.PV.clear();

			rootMoves.push_back(rm);
		}
	}
	else{
		for_each(limits.searchMoves.begin(), limits.searchMoves.end(),
			[&](Move &m)
			{
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=m;
			rm.selDepth=0;
			rm.depth=0;
			rm.nodes=0;
			rm.time=0;
			rm.PV.clear();
			rootMoves.push_back(rm);
			}
		);
	}

	//sync_cout<<"legal moves ="<<rootMoves.size()<<sync_endl;
	unsigned int linesToBeSearched = search::multiPVLines;

	if(limitStrength){
		int lines= (int)((-8.0/2000.0)*(eloStrenght-1000)+10.0);
		unsigned int s=std::max(lines,4);
		linesToBeSearched=	std::max(linesToBeSearched,s);
	}
	linesToBeSearched = std::min(linesToBeSearched, (unsigned int)rootMoves.size());
	//sync_cout<<"linesToBeSearched ="<<linesToBeSearched<<sync_endl;
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Move m(Movegen::NOMOVE),oldBestMove(Movegen::NOMOVE);

	Movegen mg(pos,history,m);

	Move lastLegalMove;
	unsigned int legalMoves=0;

	while((m=mg.getNextMove())!=Movegen::NOMOVE){
		legalMoves++;
		lastLegalMove=m;
	}

	if(legalMoves==0){
		while((limits.infinite && !signals.stop) || limits.ponder){}
		sync_cout<<"info depth 0 score cp 0"<<sync_endl;
		sync_cout<<"bestmove 0000"<<sync_endl;
		return res;
	}else if(legalMoves==1){
		if(!limits.infinite)
		{
			sync_cout<<"info pv "<<pos.displayUci(lastLegalMove)<<sync_endl;
			while(limits.ponder){}
			sync_cout<<"bestmove "<<pos.displayUci(lastLegalMove);


			pos.doMove(lastLegalMove);

			ttEntry* tte = TT.probe(pos.getKey());

			pos.undoMove(lastLegalMove);
			if(tte && tte->getPackedMove()){
				Move m;
				m.packed=tte->getPackedMove();
				std::cout<<" ponder "<<pos.displayUci(m);
			}

			std::cout<<sync_endl;
			return res;
		}
	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------

	PolyglotBook pol;
	//std::cout<<"polyglot testing"<<std::endl;
	if(useOwnBook && !limits.infinite ){
		Move bookM=pol.probe(pos,"book.bin",bestMoveBook);
		if(bookM.packed){
			sync_cout<<"info pv "<<pos.displayUci(bookM)<<sync_endl;
			while((limits.infinite && !signals.stop) || limits.ponder){}
			sync_cout<<"bestmove "<<pos.displayUci(bookM)<<sync_endl;
			return res;
		}
	}



	unsigned int selDepthBase=pos.getPly();

	startTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
	std::list<Move> newPV;
	unsigned int depth=1;

	Score alpha=-SCORE_INFINITE,beta =SCORE_INFINITE;
	Score delta=1600;
	selDepth=selDepthBase;

	do{

		for (rootMove& rm : rootMoves){
			rm.previousScore = rm.score;
		}


		for (indexPV = 0; indexPV < linesToBeSearched; indexPV++)
		{
			for (unsigned int x =indexPV; x<linesToBeSearched ; x++){
				rootMoves[x].score=-SCORE_INFINITE;
			}

			//sync_cout<<"PVIdx="<<PVIdx<<sync_endl;


			if (depth >= 5)
			{
				delta = 800;
				alpha = (Score) std::max((signed long long int)(rootMoves[indexPV].previousScore) - delta,(signed long long int)-SCORE_INFINITE);
				beta  = (Score) std::min((signed long long int)(rootMoves[indexPV].previousScore) + delta,(signed long long int) SCORE_INFINITE);
			}


			unsigned int reduction=0;

			// reload the last PV in the transposition table
			for(unsigned int i =0; i<=indexPV; i++){
				//sync_cout<<"LENGHT "<<rootMoves[i].PV.lenght<<sync_endl;
				if(rootMoves[i].PV.size()>0){
					const ttEntry* tte;

					auto it = begin(rootMoves[i].PV);
					//sync_cout<<"SCORE "<<rootMoves[i].score<<sync_endl;
					for (;it!=end(rootMoves[i].PV);it++)
					{
						if(!pos.isMoveLegal(*it))
						{
							//sync_cout<<"ERRORE ILLLEGAL MOVE IN PV"<<sync_endl;
							break;
						}
						tte = TT.probe(pos.getKey());

						if (!tte || tte->getPackedMove() != (*it).packed)
						{// Don't overwrite correct entries
							TT.store(pos.getKey(), SCORE_NONE,typeExact,-1000, (*it).packed, pos.eval<false>());
						}

						//sync_cout<<"insert in TT "<<p.displayUci(*it)<<sync_endl;
						pos.doMove(*it);

					}
					for (;it!=begin(rootMoves[i].PV);)
					{
						--it;
						//sync_cout<<"undo move "<<p.displayUci(rootMoves[i].PV[n])<<sync_endl;
						pos.undoMove(*it);
					}
				}
			}



			do{



				//sync_cout<<"SEARCH"<<sync_endl;
				selDepth=selDepthBase;
				newPV.clear();
				pos.cleanStateInfo();
				std::vector<std::list<Move>> pvl2(threads-1);
				std::vector<std::thread> helperThread;

				for(unsigned int i=0;i<(threads-1);i++)
				{
					helperSearch[i].signals.stop =false;
					helperSearch[i].pos=pos;
					helperThread.push_back(std::thread(alphaBeta<search::nodeType::HELPER_ROOT_NODE>,&helperSearch[i],0,(depth-reduction+((i+1)%2))*ONE_PLY,alpha,beta,std::ref(pvl2[i])));
				}


				//std::thread helperThread(&alphaBelli<search::nodeType::ROOT_NODE>,&helperSearch,0,std::ref(pos2),(depth-reduction)*ONE_PLY,alpha,beta,&newPV);
				res=alphaBeta<search::nodeType::ROOT_NODE>(0,(depth-reduction)*ONE_PLY,alpha,beta,newPV);
				for(unsigned int i=0;i<(threads-1);i++)
				{
					helperSearch[i].signals.stop =true;
				}
				for(auto &t : helperThread)
				{
					t.join();
				}





				//sync_cout<<"FINISHED SEARCH"<<sync_endl;
				//sync_cout<<"PVsize "<<newPV.size()<<sync_endl;

				if(depth!=1 && signals.stop){
					//sync_cout<<"iterative deepening Stop"<<sync_endl;
					break;
				}
				if(newPV.size()!=0 &&  res <= alpha){
					sync_cout<<"ERRORE NEWPV"<<sync_endl;
				}
				long long int now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();



				if(newPV.size()!=0 && res > alpha /*&& res < beta*/){
					std::vector<rootMove>::iterator it=std::find(rootMoves.begin()+indexPV,rootMoves.end(),newPV.front());
					if(it->firstMove==newPV.front()){

						//if(res<beta)
						{
							it->PV = newPV;
						}

						it->score=res;
						it->selDepth=selDepth-selDepthBase;
						it->depth=depth;
						it->nodes=visitedNodes;
						it->time= now-startTime;
						std::iter_swap( it, rootMoves.begin()+indexPV);

					}
					else{
						sync_cout<<"ERRORE NOT FOUND NEW_PV"<<sync_endl;
					}

				}






				if (res <= alpha)
				{
					//sync_cout<<"res<=alpha "<<sync_endl;

					//my_thread::timeMan.idLoopRequestToExtend=true;
					newPV.clear();
					newPV.push_back(rootMoves[indexPV].PV.front());
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, now-startTime,indexPV,newPV,visitedNodes);
					alpha = (Score) std::max((signed long long int)(res) - delta,(signed long long int)-SCORE_INFINITE);

					reduction = 0;

				}
				else if (res >= beta){
					if(oldBestMove.packed && oldBestMove!=newPV.front()){
						my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
						//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
					}
					//sync_cout<<"res>=beta "<<sync_endl;
					printPV(res,depth,selDepth-selDepthBase,alpha,beta, now-startTime,indexPV,newPV,visitedNodes);
					beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
					if(depth>1){
						reduction=1;
					}
					//sync_cout<<"new beta "<<beta<<sync_endl;
				}else
				{
					break;
				}


				delta += delta / 2;



			}while(1);
			//sync_cout<<"aspiration window ok "<<sync_endl;
			if(!signals.stop){

				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + indexPV + 1);
				//sync_cout<<"stable sort ok "<<sync_endl;
/*				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (PVIdx + 1 == linesToBeSearched
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
					|| now - startTime > 3000
#endif
				)*/{
					//sync_cout<<"print pv "<<sync_endl;
					printPVs(indexPV+1);

				}
			}
		}
		//-----------------------
		//	single good move at root
		//-----------------------
		if (depth >= 12
			&& !signals.stop
			&&  linesToBeSearched == 1
			&&  res > SCORE_MATED_IN_MAX_PLY)
		{

			Score rBeta = res - 20000;
			pos.getActualState().excludedMove=newPV.front();
			pos.getActualState().skipNullMove=true;
			std::list<Move> locChildPV;
			Score temp = alphaBeta<search::nodeType::ALL_NODE>(0,(depth-3)*ONE_PLY,rBeta-1,rBeta,locChildPV);
			pos.getActualState().skipNullMove=false;
			pos.getActualState().excludedMove.packed=0;

			if(temp < rBeta){
				my_thread::timeMan.singularRootMoveCount++;
			}
		}

		if(oldBestMove.packed && oldBestMove!=newPV.front()){
			my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
			//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
		}
		oldBestMove=newPV.front();

		my_thread::timeMan.depth=depth;
		unsigned long time=my_thread::timeMan.minSearchTime;
		my_thread::timeMan.allocatedTime=std::max(time,(unsigned long int)(my_thread::timeMan.allocatedTime*0.90));
		//sync_cout<<"nuovo tempo allocato="<<my_thread::timeMan.allocatedTime<<sync_endl;

		my_thread::timeMan.idLoopIterationFinished=true;




		depth+=1;

	}while(depth<=(limits.depth? limits.depth:100) && !signals.stop);

	//sync_cout<<"print final bestMove "<<sync_endl;

	while(limits.ponder){
	}

	unsigned int bestMoveLine=0;
	if(limitStrength){
		double lambda=(eloStrenght-1000.0)*(0.8/2000)+0.2;

		std::mt19937_64 rnd;
		std::exponential_distribution<> uint_dist(lambda);
		long long int now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		rnd.seed(now);
		double dres = uint_dist(rnd);

		bestMoveLine =std::min((unsigned int)dres,linesToBeSearched-1);
	}
	//sync_cout<<"bestMove index "<<bestMoveLine<<sync_endl;


	sync_cout<<"bestmove "<<pos.displayUci(rootMoves[bestMoveLine].PV.front());
	if(rootMoves[bestMoveLine].PV.size()>1)
	{
		std::list<Move>::iterator it = rootMoves[bestMoveLine].PV.begin();
		std::advance(it, 1);
		std::cout<<" ponder "<<pos.displayUci(*it);
	}
	else{
		pos.doMove(rootMoves[bestMoveLine].PV.front());
		ttEntry* tte = TT.probe(pos.getKey());
		pos.undoMove(rootMoves[bestMoveLine].PV.front());
		if(tte && tte->getPackedMove()){
			Move m;
			m.packed=tte->getPackedMove();
			std::cout<<" ponder "<<pos.displayUci(m);
		}

	}
	std::cout<<sync_endl;

	return res;



}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,std::list<Move>& pvLine){

	//bool verbose=false;
	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);

	Position::state& st = pos.getActualState();


	/*if(visitedNodes>709000 && visitedNodes<710000){
		sync_cout<<visitedNodes<<" AB "<<"ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	}*/
	//sync_cout<<"AB "<<"ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	/*if(pos.displayFen()=="rn1qkb1r/ppp2ppp/4bn2/1B6/8/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 5"){
		sync_cout<<"eccomi"<<sync_endl;
		verbose=true;
		pos.display();
	}*/
	//if(verbose){sync_cout<<"eccomi"<<sync_endl;}
	visitedNodes++;
	const bool PVnode=(type==search::nodeType::PV_NODE || type==search::nodeType::ROOT_NODE  || type==search::nodeType::HELPER_ROOT_NODE);
	const bool inCheck = pos.isInCheck();
	Move threatMove;
	threatMove=0;

	if(PVnode)
	{
		//assert(pvLine);
		pvLine.clear();
	}

	if( showLine && depth <=ONE_PLY)
	{
		// TODO questo mostra la linea dallo startpoosition.... errore  va modificato
		showLine=false;
		sync_cout<<"info currline";
		for (unsigned int i = 1; i<= pos.getStateIndex()/2;i++){ // show only half of
			std::cout<<" "<<pos.displayUci(pos.getState(i).currentMove);

		}
		std::cout<<sync_endl;
	}


	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;

	if(type !=search::nodeType::ROOT_NODE  && type !=search::nodeType::HELPER_ROOT_NODE){
		if(pos.isDraw(PVnode) || signals.stop){
			if(PVnode){
				pvLine.clear();
			}
			//if(signals.stop){sync_cout<<"alpha beta initial Stop"<<sync_endl;}
			return std::max((int)0,(int)(-5000 + pos.getPly()*7));
		}

		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta){
			return alpha;
		}

	}

	Move excludedMove=st.excludedMove;

	U64 posKey=excludedMove.packed?pos.getExclusionKey() :pos.getKey();
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte!=nullptr ? tte->getPackedMove() : 0;
	Score ttValue = tte!=nullptr ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (type!=search::nodeType::ROOT_NODE && type!=search::nodeType::HELPER_ROOT_NODE
			&& tte!=nullptr
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    //&& (	PVnode ?  tte->getType() == typeExact
		    // TODO vedere se nei PV node in cui ho un beta cutoff o un alpha cutoff ritornare il valore del TT
		    && (	PVnode ?  false
		            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		                              : (tte->getType() ==  typeScoreLowerThanAlpha || tte->getType() == typeExact)))
	{
		TT.refresh(tte);

		if (ttValue >= beta &&
			ttMove.packed &&
			!pos.isCaptureMoveOrPromotion(ttMove) &&
			!inCheck)
		{
			pos.saveKillers(ttMove);
		}



		if(PVnode)
		{
			if(ttMove.packed && pos.isMoveLegal(ttMove))
			{
				pvLine.clear();
				pvLine.push_back(ttMove);
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}

	Score staticEval;
	Score eval;
	if(inCheck){
		staticEval=pos.eval<false>();
		eval=staticEval;
#ifdef DEBUG_EVAL_SIMMETRY
		ppp.setupFromFen(pos.getSymmetricFen());
		Score test=ppp.eval<false>();
		if(test!=eval){
			sync_cout<<1<<" "<<test<<" "<<eval<<sync_endl;
			pos.display();
			while(1);
		}
#endif
	}
	else{
		if(tte!=nullptr)
		{
			staticEval=tte->getStaticValue();
			assert(staticEval<SCORE_INFINITE);
			assert(staticEval>-SCORE_INFINITE);
			eval=staticEval;
			if (ttValue != SCORE_NONE){
				if (
						((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > eval) )
						|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < eval) )
					)
				{
					eval = ttValue;
				}
			}

		}
		else
		{
			staticEval=pos.eval<false>();
			eval=staticEval;
#ifdef DEBUG_EVAL_SIMMETRY
			ppp.setupFromFen(pos.getSymmetricFen());
			Score test=ppp.eval<false>();
			if(test!=eval){
				sync_cout<<2<<" "<<test<<" "<<eval<<sync_endl;
				pos.display();
				while(1);
			}
#endif
		}

	}
	//------------------------
	// razoring
	//------------------------
	// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
	//------------------------
	if (!PVnode
		&& !inCheck
		&&  depth < 4 * ONE_PLY
		&&  eval + razorMargin(depth) <= alpha
		&&  alpha >= -SCORE_INFINITE+razorMargin(depth)
		//&&  abs(alpha) < SCORE_MATE_IN_MAX_PLY // implicito nell riga precedente
		&&  ((!ttMove.packed ) || type==ALL_NODE)
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&& !((pos.getNextTurn() && (pos.getBitmap(Position::blackPawns) & RANKMASK[A2])) || (!pos.getNextTurn() && (pos.getBitmap(Position::whitePawns) & RANKMASK[A7]) ) )
	)
	{
		Score ralpha = alpha - razorMargin(depth);
		assert(ralpha>=-SCORE_INFINITE);

		std::list<Move> childPV;
		Score v = qsearch<CUT_NODE>(ply,0, ralpha, ralpha+1, childPV);
		if (v <= ralpha)
		{
			return v;
		}
	}

	//---------------------------
	//	 STATIC NULL MOVE PRUNING
	//---------------------------
	//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
	//---------------------------
	if (!PVnode
		&& !inCheck
		&& !st.skipNullMove
		&&  depth < 4 * ONE_PLY
		&& eval >-SCORE_INFINITE + futility[depth>>ONE_PLY_SHIFT]
		&&  eval - futility[depth>>ONE_PLY_SHIFT] >= beta
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&&  abs(eval) < SCORE_KNOWN_WIN
		&&  ((pos.getNextTurn() && st.nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getNextTurn() && st.nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0])))
	{
		assert((eval -futility[depth>>ONE_PLY_SHIFT] >-SCORE_INFINITE));
		return eval - futility[depth>>ONE_PLY_SHIFT];
	}


	//---------------------------
	//	 NULL MOVE PRUNING
	//---------------------------
	// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
	// this search let us know about threat move by the opponent.
	//---------------------------

	if(!PVnode
		&& !inCheck
		&& depth>=ONE_PLY
		&& eval>=beta
		&& !st.skipNullMove
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&&((pos.getNextTurn() && st.nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getNextTurn() && st.nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0]))
	){
		// Null move dynamic reduction based on depth
		int red = 3 * ONE_PLY + depth / 4;

		// Null move dynamic reduction based on value
		if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta){
			red += ONE_PLY;
		}
		pos.doNullMove();
		U64 nullKey= pos.getKey();


		Score nullVal;
		std::list<Move> childPV;
		if(depth-red<ONE_PLY ){
			nullVal = -qsearch<childNodesType>(ply+1,0,-beta,-beta+1,childPV);
		}else
		{
			nullVal = -alphaBeta<childNodesType>(ply+1, depth - red, -beta, -beta+1,childPV);
		}
		pos.undoNullMove();

		if (nullVal >= beta)
		{


			// Do not return unproven mate scores
			if (nullVal >= SCORE_MATE_IN_MAX_PLY)
			{
				nullVal = beta;
			}
			//return nullVal; TODO da testare se da vantaggi o no, semplifica il codice

			if (depth < 12 * ONE_PLY)
			{
				return nullVal;
			}

			// Do verification search at high depths
			st.skipNullMove=true;
			assert(depth - red>=ONE_PLY);
			Score val;
			if(depth-red<ONE_PLY){
				val = qsearch<childNodesType>(ply, depth-red, beta-1, beta,childPV);
			}
			else{
				val = alphaBeta<childNodesType>(ply, depth - red, beta-1, beta, childPV);
			}
			st.skipNullMove=false;
			if (val >= beta)
			{
				return nullVal;
			}

		}
		else
		{
			ttEntry * tte=TT.probe(nullKey);
			threatMove=tte!=nullptr? tte->getPackedMove(): 0;
		}

	}

	//------------------------
	//	PROB CUT
	//------------------------
	//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
	//------------------------

	if(!PVnode &&
		!inCheck &&
		depth>=5*ONE_PLY &&
		!st.skipNullMove &&
		//abs(beta)<SCORE_KNOWN_WIN
		//eval> beta-40000
		abs(beta)<SCORE_MATE_IN_MAX_PLY
	){
		Score s;
		Score rBeta=beta+8000;
		int rDepth=depth -ONE_PLY- 3*ONE_PLY;
		Movegen mg(pos,history,ttMove);
		mg.setupProbCutSearch(pos.getCapturedPiece());

		Move m;
		std::list<Move> childPV;
		while((m=mg.getNextMove())!=Movegen::NOMOVE){
			pos.doMove(m);
			assert(rDepth>=ONE_PLY);
			s=-alphaBeta<childNodesType>(ply+1,rDepth,-rBeta,-rBeta+1,childPV);
			pos.undoMove(m);
			if(s>=rBeta){

				return s;
			}

		}
	}

	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? 5 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed == 0
		&& (PVnode || staticEval+10000>= beta))
	{
		int d = depth - 2 * ONE_PLY - (PVnode ? 0 : depth / 4);

		bool skipBackup = st.skipNullMove;
		st.skipNullMove = true;

		std::list<Move> childPV;
		const search::nodeType iidType=type;
		assert(d>=ONE_PLY);
		alphaBeta<iidType>(ply, d, alpha, beta, childPV);

		st.skipNullMove = skipBackup;

		tte = TT.probe(posKey);
		ttMove= tte!=nullptr ? tte->getPackedMove():0;
	}




	Score bestScore=-SCORE_INFINITE;

	Move bestMove;
	bestMove=0;
	Move m;
	Movegen mg(pos,history,ttMove);
	unsigned int moveNumber=0;
	unsigned int quietMoveCount =0;
	Move quietMoveList[64];

	bool singularExtensionNode=
		type!=search::nodeType::ROOT_NODE
		&& type!=search::nodeType::HELPER_ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed != 0
		&& !excludedMove.packed // Recursive singular search is not allowed
		&& tte!=nullptr
		&& (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		&&  tte->getDepth()>= depth - 3 * ONE_PLY;

	//sync_cout<<"iterating moves"<<sync_endl;
	while (bestScore <beta  && (m=mg.getNextMove())!=Movegen::NOMOVE) {

		/*if(verbose){
			sync_cout<<"move "<<pos.displayUci(m)<<sync_endl;
		}*/

		assert(m.packed);
		if(m== excludedMove){
			continue;
		}


		// search only the moves in the search list
		if((type==search::nodeType::ROOT_NODE || type==search::nodeType::HELPER_ROOT_NODE) && !std::count(rootMoves.begin()+indexPV,rootMoves.end(),m)){
			//sync_cout<<"avoid move "<<pos.displayUci(m)<<sync_endl;
			continue;
		}

		moveNumber++;


		bool captureOrPromotion =pos.isCaptureMoveOrPromotion(m);
		if(!captureOrPromotion && quietMoveCount < 64){
			quietMoveList[quietMoveCount++]=m;
		}

		bool moveGivesCheck=pos.moveGivesCheck(m);
		bool isDangerous=moveGivesCheck || pos.isCastleMove(m) || pos.isPassedPawnMove(m);

		int ext=0;
		if(PVnode && isDangerous){
			ext = ONE_PLY;
		}else if(moveGivesCheck && pos.seeSign(m) >= 0){
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if(singularExtensionNode
			&& !ext
			&&  m == ttMove
			&&  abs(ttValue) < SCORE_KNOWN_WIN
		)
		{

			std::list<Move> childPv;
			Score rBeta = ttValue - int(depth*20);
			st.excludedMove=m;
			bool backup=st.skipNullMove;
			st.skipNullMove=true;
			Score temp = alphaBeta<ALL_NODE>(ply,depth/2,rBeta-1,rBeta,childPv);
			st.skipNullMove=backup;
			st.excludedMove = Movegen::NOMOVE;

			if(temp < rBeta){
				ext = ONE_PLY;
		    }
		}

		int newDepth= depth-ONE_PLY+ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if(!PVnode
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore>SCORE_MATED_IN_MAX_PLY
		){
			assert(moveNumber>1);

			if(newDepth < 11*ONE_PLY
				&& moveNumber >= FutilityMoveCounts[newDepth>>ONE_PLY_SHIFT]
				&& (!threatMove.packed)
				)
			{
				assert((newDepth>>ONE_PLY_SHIFT)<11);
				continue;
			}


			if(newDepth<7*ONE_PLY){
				Score localEval= eval + futilityMargin[newDepth>>ONE_PLY_SHIFT];
				if(localEval<beta){
					bestScore = std::max(bestScore, localEval);
					assert((newDepth>>ONE_PLY_SHIFT)<7);
					continue;
				}
			}

			if(newDepth < 4 * ONE_PLY
				&& pos.seeSign(m) < 0)
			{
				continue;
			}



		}

		if(type==ROOT_NODE){
			long long int elapsed=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count()-startTime;
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!signals.stop
				){
				sync_cout<<"info currmovenumber "<<moveNumber<<" currmove "<<pos.displayUci(m)<<" nodes "<<visitedNodes<<
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
						" time "<<elapsed <<
#endif
						sync_endl;
			}
		}

		pos.doMove(m);

		Score val;
		std::list<Move> childPV;
		if(PVnode){
			if(moveNumber==1){
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"FIRST alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
				}
#endif

				childPV.clear();
				if(newDepth<ONE_PLY){
					val=-qsearch<search::nodeType::PV_NODE>(ply+1,newDepth,-beta,-alpha,childPV);
				}else{
					val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,newDepth,-beta,-alpha,childPV);
				}
#ifdef PRINT_PV_CHANGES
				sync_cout<<"FIRST ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
			}
			else{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch=true;
				if(depth>=3*ONE_PLY
					&& !captureOrPromotion
					&& !isDangerous
					&& m != ttMove
					&& !mg.isKillerMove(m)
				)
				{
					assert(moveNumber!=0);

					int reduction = PVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction!=0){
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"LMR alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<" red "<<(float)(reduction)/ONE_PLY<<sync_endl;
					}
#endif
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,d,-alpha-1,-alpha,childPV);
						if(val<=alpha){
							doFullDepthSearch=false;
						}
					}
				}


				if(doFullDepthSearch){

#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
					}
#endif
					if(newDepth<ONE_PLY){
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}else{
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}

					if(val>alpha && val < beta ){
#ifdef DEBUG1
						if(type==ROOT_NODE){
							sync_cout<<"info currmove "<<pos.displayUci(m)<<" val "<<val/10000.0<<" nodes "<<visitedNodes<< sync_endl;
						}
#endif
						childPV.clear();
#ifdef DEBUG1
						if(type==search::nodeType::ROOT_NODE){
							sync_cout<<"RESEARCH alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
						}
#endif
						if(newDepth<ONE_PLY){
							val=-qsearch<search::nodeType::PV_NODE>(ply+1,newDepth,-beta,-alpha,childPV);
						}
						else{
							val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,newDepth,-beta,-alpha,childPV);
						}
#ifdef PRINT_PV_CHANGES
						sync_cout<<"OTHER ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
					}
				}

			}
		}
		else{

			//------------------------------
			//	LMR
			//------------------------------
			bool doFullDepthSearch=true;
			if(depth>=3*ONE_PLY
				&& !captureOrPromotion
				&& !isDangerous
				&& m != ttMove
				&& !mg.isKillerMove(m)
			)
			{
				int reduction = nonPVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction!=0){
					/*if(verbose){
						sync_cout<<"LMR search depth "<<d<<sync_endl;
					}*/
					val=-alphaBeta<childNodesType>(ply+1,d,-alpha-1,-alpha,childPV);
					if(val<=alpha){
						doFullDepthSearch=false;
					}
				}
			}




			if(doFullDepthSearch){

				if(moveNumber<5){
					if(newDepth<ONE_PLY){
						/*if(verbose){
							sync_cout<<"qsearch depth "<<newDepth<<sync_endl;
						}*/
						val=-qsearch<childNodesType>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}else{
						/*if(verbose){
							sync_cout<<"search depth "<<newDepth<<sync_endl;
						}*/
						val=-alphaBeta<childNodesType>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}
				}
				else{
					if(newDepth<ONE_PLY){
						/*if(verbose){
							sync_cout<<"qsearch depth "<<newDepth<<sync_endl;
						}*/
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}else{
						/*if(verbose){
							sync_cout<<"search depth "<<newDepth<<sync_endl;
						}*/
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,newDepth,-alpha-1,-alpha,childPV);
					}

				}

			}
		}



		pos.undoMove(m);




		if(val>bestScore){
			bestScore=val;

			if(val>alpha){
				bestMove=m;
				if(PVnode)
				{
					alpha =val;
				}
				if(type ==search::nodeType::ROOT_NODE || type ==search::nodeType::HELPER_ROOT_NODE|| (PVnode &&!signals.stop)){
					if(PVnode){
						pvLine.clear();
						pvLine.push_back(bestMove);
						pvLine.splice(pvLine.end(),childPV);
						//std::copy(childPV.begin(),childPV.end(),back_inserter(pvLine));
					}
				}
			}
		}
	}


	// draw

	if(!moveNumber){
		if( excludedMove.packed){
			return alpha;
		}else if(!inCheck){
			bestScore = std::max((int)0,(int)(-5000 + pos.getPly()*7));
		}
		else{
			bestScore=matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;





	if(!signals.stop){
	//Statistics::instance().gatherNodeTypeStat(type,bestScore >= beta?CUT_NODE:PVnode && bestMove.packed? PV_NODE:ALL_NODE );
	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove.packed/*?bestMove.packed:ttMove.packed*/, staticEval);
	}


	// TODO fare > originalAlpha
	// save killer move & update history
	if (bestScore >= beta &&
			// TODO provare a fare solamente pos.isCaptureMove
		!pos.isCaptureMoveOrPromotion(bestMove) &&
		!inCheck)
	{
		pos.saveKillers(bestMove);



		// update history
		// todo controllare se fare +=depth^2 e -=(depth^2)/(numero di mosse quiet) per avere media nulla
		// todo controllare se usare depth o qualche depth scalata
		Score bonus = Score(depth * depth)/(ONE_PLY*ONE_PLY);
		//sync_cout<<bonus<<sync_endl;
		history.update(pos.getPieceAt((tSquare)bestMove.bit.from),(tSquare) bestMove.bit.to, bonus);
		if(quietMoveCount>1){
			for (unsigned int i = 0; i < quietMoveCount - 1; i++){
				Move m;
				m= quietMoveList[i];
				history.update(pos.getPieceAt((tSquare)m.bit.from),(tSquare) m.bit.to, -bonus);
			}
		}
	}
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(unsigned int ply,int depth,Score alpha,Score beta,std::list<Move>& pvLine){

	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);




	/*if(visitedNodes>599000 && visitedNodes<800000){
		sync_cout<<"Q ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	}*/
	//sync_cout<<"Q ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;

	const bool PVnode=(type==search::nodeType::PV_NODE);
	assert(PVnode || alpha+1==beta);

	if(PVnode){
		//assert(pvLine);
		pvLine.clear();
	}

	bool inCheck=pos.isInCheck();

	selDepth=std::max(pos.getPly(),selDepth);
	visitedNodes++;


	if(pos.isDraw(PVnode) || signals.stop){
		if(PVnode){
			pvLine.clear();
		}
		return std::max((int)0,(int)(-5000 + pos.getPly()*7));
	}





	//----------------------------
	//	next node type
	//----------------------------
	const search::nodeType childNodesType=
		type==search::nodeType::ALL_NODE?
			search::nodeType::CUT_NODE:
			type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
				search::nodeType::PV_NODE;


	ttEntry* tte = TT.probe(pos.getKey());
	Move ttMove;
	ttMove=tte ? tte->getPackedMove() : Movegen::NOMOVE;
	Movegen mg(pos,history,ttMove);
	int TTdepth=mg.setupQuiescentSearch(inCheck,depth);
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (tte
		&& tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false/*tte->getType() == typeExact*/
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta|| tte->getType() == typeExact)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha|| tte->getType() == typeExact)))
	{
		TT.refresh(tte);



		if(PVnode)
		{
			if(ttMove.packed && pos.isMoveLegal(ttMove))
			{
				pvLine.clear();
				pvLine.push_back(ttMove);
			}
			else
			{
				pvLine.clear();
			}
		}
		return ttValue;
	}

	ttType TTtype=typeScoreLowerThanAlpha;


	Score staticEval= tte?tte->getStaticValue():pos.eval<false>();
#ifdef DEBUG_EVAL_SIMMETRY
	ppp.setupFromFen(pos.getSymmetricFen());
	Score test=ppp.eval<false>();
	if(test!=staticEval){
		sync_cout<<3<<" "<<test<<" "<<staticEval<<" "<<pos.eval<false>()<<sync_endl;
		pos.display();
		ppp.display();

		while(1);
	}
#endif


	//----------------------------
	//	stand pat score
	//----------------------------
	Score bestScore = -SCORE_INFINITE;
	if(!inCheck){
		bestScore=staticEval;

		/*if (ttValue != SCORE_NONE){
			if (
					((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > staticEval) )
					|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < staticEval) )
				)
			{
				bestScore = ttValue;
			}
		}*/
	}


	if(bestScore>alpha){
		assert(!inCheck);
		alpha=bestScore;
		// TODO testare se la riga TTtype=typeExact; ha senso
		TTtype=typeExact;

	}
	// todo trovare un valore buono per il futility
	Score futilityBase=bestScore+5000;

	//----------------------------
	//	try the captures
	//----------------------------
	unsigned int moveNumber=0;
	Move m;
	Move bestMove=ttMove;
	//bestMove=0;
	std::list<Move> childPV;

	Position::state &st = pos.getActualState();
	while (bestScore <beta  &&  (m=mg.getNextMove())!=Movegen::NOMOVE) {
		assert(alpha<beta);
		assert(beta<=SCORE_INFINITE);
		assert(alpha>=-SCORE_INFINITE);
		moveNumber++;

		assert(m.packed);

		if(!inCheck && (TTdepth<-1* ONE_PLY) && m.bit.flags==Move::fpromotion && m.bit.promotion!= Move::promQueen){
			continue;
		}

		if(depth<-7*ONE_PLY && !inCheck){
			if(st.currentMove.bit.to!= m.bit.to){
				continue;
			}
		}

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		if(	!PVnode
			&& !inCheck
			&& m != ttMove
			&& m.bit.flags != Move::fpromotion
			//&& !pos.moveGivesCheck(m)
			)
		{
			if(
					!pos.moveGivesCheck(m)
					&& !pos.isPassedPawnMove(m)
					&& abs(staticEval)<SCORE_KNOWN_WIN
			)
			{
				Score futilityValue = futilityBase
						+ Position::pieceValue[pos.getPieceAt((tSquare)m.bit.to)][1]
						+ (m.bit.flags == Move::fenpassant ? Position::pieceValue[Position::whitePawns][1] : 0);

				if (futilityValue < beta)
				{
					bestScore = std::max(bestScore, futilityValue);
					continue;
				}
				if (futilityBase < beta && pos.seeSign(m)<=0)
				{
					bestScore = std::max(bestScore, futilityBase);
					continue;
				}

			}


			//----------------------------
			//	don't check moves with negative see
			//----------------------------

			// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
			// TODO testare se aggiungere o no !movegivesCheck() &&
			if(
					//!pos.moveGivesCheck(m) && -50ELO
					pos.seeSign(m)<0)
			{
				continue;
			}
		}

		pos.doMove(m);
		Score val;


		if(PVnode){
			childPV.clear();
			val=-qsearch<childNodesType>(ply+1,depth-ONE_PLY,-beta,-alpha,childPV);
		}
		else{
			val=-qsearch<childNodesType>(ply+1,depth-ONE_PLY,-beta,-alpha,childPV);
		}

		pos.undoMove(m);


		if(val>bestScore){
			bestScore=val;
			if(val>alpha){
				bestMove=m;
				TTtype=typeExact;
				alpha =val;
				if(PVnode &&!signals.stop){
					pvLine.clear();
					pvLine.push_back(bestMove);
					pvLine.splice(pvLine.end(),childPV);

				}
			}
		}

	}

	if (bestScore >= beta &&
			// TODO provare a fare solamente pos.isCaptureMove
		!pos.isCaptureMoveOrPromotion(bestMove) &&
		!inCheck)
	{
		//sync_cout<<pos.displayUci(bestMove)<<sync_endl;
		pos.saveKillers(bestMove);


	}



	if(bestScore == -SCORE_INFINITE){
		assert(inCheck);
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);




	if(!signals.stop){
		TT.store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta ? typeScoreHigherThanBeta : TTtype,
					(short int)TTdepth, bestMove.packed, staticEval);
	}

	return bestScore;

}


