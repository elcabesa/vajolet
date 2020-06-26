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

#include "movepicker.h"
#include "searcher.h"
#include "searchImpl.h"
#include "syzygy/syzygy.h"
#include "vajo_io.h"

std::mutex  Searcher::_mutex;

Searcher::Searcher(Search::impl& father, SearchTimer& st, SearchLimits& sl, transpositionTable& tt, SearchParameters& sp, timeManagement& tm, rootMovesToBeSearched(rm), PVline pvToBeFollowed, UciOutput::type UOI):
    _UOI(UciOutput::create(UOI)),
    _rootMovesToBeSearched(rm),
    _sp(sp),
    _sl(sl),
    _st(st),
    _tt(tt),
    _tm(tm),
    _father(father) {
    _pos = father.getPosition();
    _pvLineFollower.setPVline(pvToBeFollowed);
    _initialTurn = _pos.getNextTurn();
}

Searcher::Searcher(const Searcher& other) :
    _UOI(UciOutput::create(other._UOI->getType())),
    _pvLineFollower(other._pvLineFollower),
    _rootMovesToBeSearched(other._rootMovesToBeSearched),
    _pos(other._pos),
    _initialTurn(other._initialTurn),
    _sp(other._sp),
    _sl(other._sl),
    _st(other._st),
    _tt(other._tt),
    _tm(other._tm),
    _father(other._father)
    {}


void Searcher::showLine() {
    _showLine = true;
}

void Searcher::stopSearch() {
    _stop = true;
}

void Searcher::_resetStopCondition() {
    _stop = false;
}

void Searcher::searchManager(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove)
{	
    _resetStopCondition();
    _cleanMemoryBeforeStartingNewSearch();
	_idLoop(temporaryResults, index, toBeExcludedMove, 1, -SCORE_INFINITE, SCORE_INFINITE,  index == 0);
	_finished = true;
	_father._waitStopCV.notify_one();	
}

void Searcher::_cleanMemoryBeforeStartingNewSearch()
{
	_sd.cleanData();
	_visitedNodes = 0;
	_tbHits = 0;
	_multiPVmanager.clean();
}

void Searcher::_idLoop(std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, int depth, Score alpha, Score beta, bool masterThread)
{
	long long int lastPvPrint = _st.getElapsedTime();

	//_rootMovesToBeSearched.print();
	rootMove& bestMove = temporaryResults[index];

	// manage multi PV moves
	_multiPVmanager.setLinesToBeSearched(std::min( uciParameters::multiPVLines, (unsigned int)_rootMovesToBeSearched.size()));

	// ramdomly initialize the bestmove
	bestMove = rootMove(_rootMovesToBeSearched.getMove(0));
	
	do
	{
		_UOI->setDepth(depth);
		_UOI->printDepth();

		//----------------------------
		// exclude root moves in multithread search
		//----------------------------
		_excludeRootMoves(temporaryResults, index, toBeExcludedMove, masterThread);

		//----------------------------
		// iterative loop
		//----------------------------
		
		//----------------------------------
		// multi PV loop
		//----------------------------------
		for ( _multiPVmanager.startNewIteration(); _multiPVmanager.thereArePvToBeSearched(); _multiPVmanager.goToNextPV() )
		{
			_UOI->setPVlineIndex(_multiPVmanager.getPVNumber());
			//----------------------------------
			// reload PV
			//----------------------------------
			if( rootMove rm(Move::NOMOVE); _multiPVmanager.getNextRootMove(rm) )
			{
				_expectedValue = rm.score;
				_pvLineFollower.setPVline(rm.PV);
			}
			else
			{
				_expectedValue = -SCORE_INFINITE;
				_pvLineFollower.clear();
			}

			//----------------------------------
			// aspiration window
			//----------------------------------
			rootMove res = _aspirationWindow<false>(depth, alpha, beta, masterThread);
			if( res.firstMove != Move::NOMOVE )
			{
				bestMove = res;
				_multiPVmanager.insertMove(bestMove);
			}

			// at depth 1 only print the PV at the end of search
			if(!_stop && depth == 1)
			{
				_UOI->printPV(res.score, _maxPlyReached, _st.getElapsedTime(), res.PV, _father.getVisitedNodes(), _pos.isChess960());
			}
			if(!_stop && uciParameters::isMultiPvSearch())
			{
				
				auto mpRes = _multiPVmanager.get();
				bestMove = mpRes[0];
				long long int elapsed = _st.getElapsedTime();
				if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
					elapsed - lastPvPrint > 1000 ||
#endif				
					_multiPVmanager.isLastLine()
				)
				{
					_UOI->printPVs(mpRes, _pos.isChess960());
					lastPvPrint = elapsed;
				}
			}
		}

		if( masterThread )
		{
			_tm.notifyIterationHasBeenFinished();
		}
	}
	while(++depth <= (_sl.isDepthLimitedSearch() ? _sl.getDepth() : 100) && !_stop);
}

void Searcher::_excludeRootMoves( std::vector<rootMove>& temporaryResults, unsigned int index, std::vector<Move>& toBeExcludedMove, bool masterThread )
{
	if( masterThread )
	{
		std::lock_guard<std::mutex> lock(_mutex);

		// get temporary results from all the threads
		std::map<unsigned short, unsigned int> tempBestMoves;
		for( auto& m: temporaryResults)
		{
			tempBestMoves[m.firstMove.getPacked()]++;
		}

		Move mostSearchedMove = Move(tempBestMoves.begin()->first);
		unsigned int max = tempBestMoves.begin()->second;
		for( const auto& m: tempBestMoves )
		{
			if( m.second > max)
			{
				max = m.second;
				mostSearchedMove = Move(m.first);
			}
		}

		unsigned int threshold = uciParameters::threads * 0.75;
		// and make some of search alternative moves
		for( unsigned int i = 1; i < uciParameters::threads; ++i)
		{
			if( i >= threshold)
			{
				toBeExcludedMove[i] = mostSearchedMove;
			}
			else
			{
				toBeExcludedMove[i] = Move::NOMOVE;
			}
		}
	}
	else
	{
		std::lock_guard<std::mutex> lock(_mutex);
		// filter out some root move to search alternatives
		_sd.setExcludedMove(0, toBeExcludedMove[index]);
	}
}

template<bool log>
rootMove Searcher::_aspirationWindow(const int depth, Score alpha, Score beta, const bool masterThread)
{
	rootMove bestMove(Move::NOMOVE);
	Score delta = 800;
	//----------------------------------
	// prepare alpha & beta
	//----------------------------------
	if (depth >= 5)
	{
		delta = 800;
		alpha = (Score) std::max((signed long long int)_expectedValue - delta,(signed long long int) SCORE_MATED);
		beta  = (Score) std::min((signed long long int)_expectedValue + delta,(signed long long int) SCORE_MATE);
	}

	int globalReduction = 0;
	unsigned int iteration = 0;

	//----------------------------------
	// aspiration window
	//----------------------------------
	do
	{	++iteration;
		_maxPlyReached = 0;
		_validIteration = false;
		_pvLineFollower.restart();
		PVline newPV;
		newPV.clear();

		if(log) _lw = std::unique_ptr<logWriter>(new logWriter(_pos.getFen(), depth, iteration));
		Score res = _alphaBeta<nodeType::ROOT_NODE, log>(0, (depth - globalReduction) * ONE_PLY, alpha, beta, newPV);

		if(_validIteration || !_stop)
		{
			long long int elapsedTime = _st.getElapsedTime();

			if (res <= alpha)
			{
				if(!uciParameters::isMultiPvSearch())
				{
					_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, _father.getVisitedNodes(), _pos.isChess960(), UciOutput::PVbound::upperbound);
				}

				alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

				globalReduction = 0;
				if( masterThread )
				{
					_tm.notifyFailLow();
				}
			}
			else if (res >= beta)
			{
				if(!uciParameters::isMultiPvSearch())
				{
					_UOI->printPV(res, _maxPlyReached, elapsedTime, newPV, _father.getVisitedNodes(), _pos.isChess960(), UciOutput::PVbound::lowerbound);
				}

				beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
				if(depth > 1)
				{
					globalReduction = 1;
				}
				if( masterThread )
				{
					_tm.notifyFailOver();
				}
				_pvLineFollower.setPVline(newPV);

				bestMove = rootMove(newPV.getMove(0), newPV, res, _maxPlyReached, depth, _father.getVisitedNodes(), elapsedTime);
			}
			else
			{
				bestMove = rootMove(newPV.getMove(0), newPV, res, _maxPlyReached, depth, _father.getVisitedNodes(), elapsedTime);
				return bestMove;
			}
			
			delta += delta / 2;
		}
	}
	while(!_stop);

	return bestMove;

}


template<Searcher::nodeType type, bool log> Score Searcher::_alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{
	std::unique_ptr<logNode> ln;
	if(log) ln =  std::unique_ptr<logNode>(new logNode(*_lw, ply, depth, alpha, beta, "alphaBeta"));
	//--------------------------------------
	// node asserts
	//--------------------------------------
	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);

	//--------------------------------------
	// initialize node constants
	//--------------------------------------
	const bool rootNode = (type == nodeType::ROOT_NODE);
	const bool PVnode = (type == nodeType::PV_NODE || type == nodeType::ROOT_NODE);
	const bool inCheck = _pos.isInCheck();
	_sd.setInCheck(ply, inCheck);

	_updateNodeStatistics(ply);
	_sd.clearKillers(ply+1);

	//--------------------------------------
	// show current line if needed
	//--------------------------------------
	_showCurrenLine( ply, depth );

	//--------------------------------------
	// choose node type
	//--------------------------------------
	const nodeType childNodesType =
		type == nodeType::ALL_NODE ?
			nodeType::CUT_NODE :
		type == nodeType::CUT_NODE ?
			nodeType::ALL_NODE :
			nodeType::PV_NODE;
	(void)childNodesType;	// to suppress warning in root node and PV nodes

	if (log) ln->startSection("early returns");
	if(!rootNode)
	{
		//---------------------------------------
		//	Manage Draw
		//---------------------------------------
		if (log) ln->test("IsDraw");
		if( _manageDraw( PVnode, pvLine) ) {
			if (log) ln->logReturnValue(_getDrawValue());
			if (log) ln->endSection();
			return _getDrawValue();
		}
		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		if (log) ln->test("MateDistancePruning");
		if( _MateDistancePruning( ply, alpha, beta) ) {
			if (log) ln->logReturnValue(alpha);
			if (log) ln->endSection();
			return alpha;
		}
	}

	const Move& excludedMove = _sd.getExcludedMove(ply);
	const HashKey& posKey = _getSearchKey( (bool)excludedMove );

	//--------------------------------------
	// test the transposition table
	//--------------------------------------
	ttEntry* tte = _tt.probe( posKey );
	Move ttMove( tte->getPackedMove() );
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(), ply);

	if (log) ln->test("CanUseTT");
	if (	!rootNode
			&& _canUseTTeValue( PVnode, beta, ttValue, tte, depth )
		)
	{
		_tt.refresh(*tte);
		
		if constexpr (PVnode)
		{
			_appendTTmoveIfLegal( ttMove, pvLine);
		}

		//save killers
		if (ttValue >= beta
			&& ttMove
			&& !_pos.isCaptureMoveOrPromotion(ttMove)
			&& !inCheck)
		{
			_sd.saveKillers(ply, ttMove);
			_updateCounterMove( ttMove );
		}
		if (log) ln->logReturnValue(ttValue);
		if (log) ln->endSection();
		return ttValue;
	}
	
	//--------------------------------------
	// overwrite ttMove with move from move from PVlineToBeFollowed
	//--------------------------------------
	if constexpr ( PVnode )
	{
		if (_pvLineFollower.getNextMove(ply, ttMove))
		{
			assert(_pos.isMoveLegal(ttMove));
		}
		
	}

	//--------------------------------------
	// table base prove
	//--------------------------------------
	if constexpr (!PVnode)
	{
		if (log) ln->test("CheckTablebase");
		auto res = _checkTablebase(ply, depth);
		if( res.value != SCORE_NONE )
		{
			if(	res.TTtype == typeExact || (res.TTtype == typeScoreHigherThanBeta  && res.value >=beta) || (res.TTtype == typeScoreLowerThanAlpha && res.value <=alpha)	)
			{
				_tt.store(posKey,
					transpositionTable::scoreToTT(res.value, ply),
					res.TTtype,
					std::min( 100 * ONE_PLY , depth + 6 * ONE_PLY),
					ttMove,
					_pos.eval<false>());
				if (log) ln->logReturnValue(res.value);
				if (log) ln->endSection();
				return res.value;
			}
		}
	}
	
	if (log) ln->endSection();
	if (log) ln->startSection("calc eval");
	//---------------------------------
	// calc the eval & static eval
	//---------------------------------

	Score staticEval;
	Score eval;
	if(inCheck || tte->getType() == typeVoid)
	{
		staticEval = _pos.eval<false>();
		eval = staticEval;
		if (log) ln->calcStaticEval(staticEval);

#ifdef DEBUG_EVAL_SIMMETRY
		_testSimmetry();
#endif
	}
	else
	{
		staticEval = tte->getStaticValue();
		eval = staticEval;
		assert(staticEval < SCORE_INFINITE);
		assert(staticEval > -SCORE_INFINITE);
		if (log) ln->calcStaticEval(staticEval);

		if (ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > eval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < eval) )
				)
			{
				if (log) ln->refineEval(ttValue);
				eval = ttValue;
			}
		}
	}
	
	_sd.setStaticEval(ply, staticEval);
	
	bool improving = false;
	if( ply <2 || inCheck || ( ply >=2 && _sd.getInCheck(ply - 2) ) || ( ply >=2 && _sd.getStaticEval(ply) >= _sd.getStaticEval(ply-2) ) )
	{
		improving = true;
	}
	
	if (log) ln->endSection();
	//-----------------------------
	// reduction && pruning
	//-----------------------------
	if constexpr ( !PVnode )
	{
		if (log) ln->startSection("pruning");
		if(!inCheck )
		{
			//------------------------
			// razoring
			//------------------------
			// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
			//------------------------
			if (log) ln->test("Razoring");
			if (!_sd.skipNullMove(ply)
				&& depth < _sp.razorDepth
				&& eval + _razorMargin(depth, type == nodeType::CUT_NODE) <= alpha
				&& alpha >= -SCORE_INFINITE + _razorMargin(depth, type == nodeType::CUT_NODE)
				&& ( !ttMove || type == nodeType::ALL_NODE)
			)
			{
				PVline childPV;
				Score v = _qsearch<nodeType::CUT_NODE, log>(ply,0 , alpha, alpha + 1, childPV);
				if (v <= alpha || _sp.razorReturn)
				{
					if (log) ln->logReturnValue(v);
					if (log) ln->endSection();
					return v;
				}
			}

			//---------------------------
			//	 STATIC NULL MOVE PRUNING
			//---------------------------
			//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
			//---------------------------
			if (log) ln->test("StaticNullMovePruning");
			if (!_sd.skipNullMove(ply)
				&& depth < _sp.staticNullMovePruningDepth
				&& eval - _futility(depth, improving) >= beta
				&& eval < SCORE_KNOWN_WIN
				&& _pos.hasActivePlayerNonPawnMaterial()
			)
			{
				assert((depth>>ONE_PLY_SHIFT)<9);
				if (log) ln->logReturnValue(eval);
				if (log) ln->endSection();
				return eval;
			}


			//---------------------------
			//	 NULL MOVE PRUNING
			//---------------------------
			// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
			// this search let us know about threat move by the opponent.
			//---------------------------
			if (log) ln->test("NullMovePruning");
			if( depth >= _sp.nullMovePruningDepth
				&& eval >= beta
				&& !_sd.skipNullMove(ply)
				&& _pos.hasActivePlayerNonPawnMaterial()
			){
				int newPly = ply + 1;
				// Null move dynamic reduction based on depth
				int red = _sp.nullMovePruningReduction + depth * _sp.nullMovePruningBonusDepth;

				// Null move dynamic reduction based on value
				if (eval - _sp.nullMovePruningBonusThreshold > -SCORE_INFINITE && eval - _sp.nullMovePruningBonusThreshold > beta)
				{
					red += _sp.nullMovePruningBonusAdditionalRed;
				}
				
				_pos.doNullMove();
				if (log) ln->doMove(Move::NOMOVE);
				_sd.setSkipNullMove(newPly, true);

				//uint64_t nullKey = _pos.getKey();
				Score nullVal;
				PVline childPV;
				if( depth - red < ONE_PLY )
				{
					nullVal = -_qsearch<childNodesType, log>(newPly, 0, -beta, -beta + 1, childPV);
				}
				else
				{
					nullVal = -_alphaBeta<childNodesType, log>(newPly, depth - red, -beta, -beta + 1, childPV);
				}

				_pos.undoNullMove();
				if (log) ln->undoMove();
				
				_sd.setSkipNullMove(newPly, false);

				if (nullVal >= beta)
				{
					// Do not return unproven mate scores
					if (nullVal >= SCORE_MATE_IN_MAX_PLY)
					{
						nullVal = beta;
					}

					if (depth < _sp.nullMovePruningVerificationDepth)
					{
						if (log) ln->logReturnValue(nullVal);
						if (log) ln->endSection();
						return nullVal;
					}

					if (log) ln->test("DoVerification");
					// Do verification search at high depths
					_sd.setSkipNullMove(ply, true);
					assert(depth - red >= ONE_PLY);
					Score val;
					val = _alphaBeta<childNodesType, log>(ply, depth - red, beta - 1, beta, childPV);
					_sd.setSkipNullMove(ply, false);
					if (val >= beta)
					{
						if (log) ln->logReturnValue(nullVal);
						if (log) ln->endSection();
						return nullVal;
					}

				}

			}

			//------------------------
			//	PROB CUT
			//------------------------
			//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
			//------------------------
			if (log) ln->test("ProbCut");
			if( depth >= _sp.probCutDepth
				&&  !_sd.skipNullMove(ply)
				// && abs(beta)<SCORE_KNOWN_WIN
				// && eval> beta-40000
				&& abs(beta) < SCORE_MATE_IN_MAX_PLY
			){
				Score s;
				Score rBeta = std::min(beta + _sp.probCutDelta, SCORE_INFINITE);
				int rDepth = depth - ONE_PLY - _sp.probCutDepthRed;

				MovePicker mp(_pos, _sd, ply, ttMove);
				mp.setupProbCutSearch( _pos.getCapturedPiece() );

				Move m;
				PVline childPV;
				unsigned int pbCount = 0u;
				while( ( m = mp.getNextMove() ) && ( pbCount < 3 ) )
				{
					if( m == excludedMove )
					{
						continue;
					}
					
					++pbCount;
					
					_pos.doMove(m);
					if (log) ln->doMove(m);

					assert(rDepth>=ONE_PLY);
					s = -_alphaBeta<childNodesType, log>(ply + 1, rDepth, -rBeta, -rBeta + 1, childPV);

					_pos.undoMove();
					if (log) ln->undoMove();

					if(s >= rBeta)
					{
						if (log) ln->logReturnValue(s);
						if (log) ln->endSection();
						return s;
					}

				}
			}
		}

		if (log) ln->endSection();
	}
	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? _sp.iidDepthPv : _sp.iidDepthNonPv)
		&& !ttMove
		&& (PVnode || staticEval + _sp.iidStaticEvalBonus >= beta))
	{
		if (log) ln->test("IID");
		int d = depth - _sp.iidDepthRed - (PVnode ? 0 : depth / _sp.iidDepthRedFactor);

		bool skipBackup = _sd.skipNullMove(ply);
		_sd.setSkipNullMove(ply, true);

		PVline childPV;
		const nodeType iidType = type;
		assert(d >= ONE_PLY);
		_alphaBeta<iidType, log>(ply, d, alpha, beta, childPV);

		_sd.setSkipNullMove(ply, skipBackup);

		tte = _tt.probe(posKey);
		ttMove = tte->getPackedMove();
	}



	if (log) ln->startSection("move loop");

	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Move::NOMOVE);

	Move m;
	MovePicker mp(_pos, _sd, ply, ttMove);
	unsigned int moveNumber = 0;
	unsigned int quietMoveCount = 0;
	constexpr int quietMoveListSize = 64;
	constexpr int captureMoveListSize = 32;
	Move quietMoveList[quietMoveListSize];
	unsigned int captureMoveCount = 0;
	Move captureMoveList[captureMoveListSize];

	bool singularExtensionNode =
		!rootNode
		&& depth >= (PVnode ? _sp.singularExtensionPVDepth : _sp.singularExtensionNonPVDepth)
		&& ttMove
		&& !excludedMove // Recursive singular Search is not allowed
		&& tte != nullptr
		&& tte->isTypeGoodForBetaCutoff()
		&& tte->getDepth() >= depth - _sp.singularExtensionTtDepth;
	
	long long int lastCurrMovePrint = _st.getElapsedTime();
	while (bestScore <beta  && ( m = mp.getNextMove() ) )
	{
		assert( m );
		if(m == excludedMove)
		{
			if (log) ln->skipMove(m, "Excluded Move");
			continue;
		}

		// Search only the moves in the Search list
		if(rootNode && ( _multiPVmanager.alreadySearched(m) || !_rootMovesToBeSearched.contain(m)))
		{
			if (log) ln->skipMove(m, " not in the root nodes");
			continue;
		}
		++moveNumber;


		bool captureOrPromotion = _pos.isCaptureMoveOrPromotion(m);


		bool moveGivesCheck = _pos.moveGivesCheck(m);
		bool isDangerous = moveGivesCheck || m.isCastleMove() || _pos.isPassedPawnMove(m);
		bool FutilityMoveCountFlag = (depth < _sp.FutilityMoveCountsDepth * ONE_PLY ) && (moveNumber >= _sp.FutilityMoveCounts[improving][depth >> ONE_PLY_SHIFT]);

		int ext = 0;
		if(PVnode && isDangerous)
		{
			if (log) ln->ExtendedDepth();
			ext = _sp.dangerousMoveExtension;
		}
		else if(moveGivesCheck && _pos.seeSign(m) >= 0 && !FutilityMoveCountFlag)
		{
			if (log) ln->ExtendedDepth();
			ext = _sp.checkMoveExtension;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if( singularExtensionNode
			&& !ext
			&& m == ttMove
			//&&  abs(ttValue) < SCORE_KNOWN_WIN
			&& abs(beta) < SCORE_MATE_IN_MAX_PLY
		)
		{
			if (log) ln->test("SingularExtension");

			PVline childPv;

			Score rBeta = ttValue - depth * _sp.singularExtensionScoreDepthBonus;
			rBeta = std::max( rBeta, -SCORE_MATE + 1 );

			_sd.setExcludedMove(ply, m);
			Score temp = _alphaBeta<nodeType::ALL_NODE, log>(ply, depth/2, rBeta - 1, rBeta, childPv);
			_sd.setExcludedMove(ply, Move::NOMOVE);

			if(temp < rBeta)
			{
				if (log) ln->ExtendedDepth();
				ext = _sp.singularExtensionExt;
		    }
		}

		int newDepth = depth - ONE_PLY + ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if( !rootNode
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore > SCORE_MATED_IN_MAX_PLY
		){
			if (log) ln->test("Pruning");
			assert(moveNumber > 1);

			if(FutilityMoveCountFlag)
			{
				assert((newDepth>>ONE_PLY_SHIFT)<11);
				if (log) ln->skipMove(m, "futility move Count flag");
				continue;
			}

			if(newDepth < _sp.futilityDepth)
			{
				Score localEval = staticEval + _sp.futilityMargin[newDepth >> ONE_PLY_SHIFT];
				if(localEval <= alpha)
				{
					if constexpr ( !PVnode )
					{
						bestScore = std::max(bestScore, localEval);
					}
					assert((newDepth>>ONE_PLY_SHIFT)<7);
					if (log) ln->skipMove(m, "futiliy margin");
					continue;
				}
			}

			if(newDepth < _sp.negativeSeeSkipDepth && _pos.seeSign(m) < 0)
			{
				if (log) ln->skipMove(m, "negative see");
				continue;
			}
		}

		if(rootNode && depth >= 10 * ONE_PLY)
		{
			long long int elapsed = _st.getElapsedTime();
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				(elapsed - lastCurrMovePrint > 1000  || moveNumber == 1) &&
#endif
				!_stop
				)
			{
				_UOI->printCurrMoveNumber(moveNumber, m, _father.getVisitedNodes(), elapsed, _pos.isChess960());
				lastCurrMovePrint = elapsed;
			}
		}


		_pos.doMove(m);
		__builtin_prefetch(_tt.findCluster(_pos.getKey().getKey()));
		if (log) ln->doMove(m);
		Score val;
		PVline childPV;

		if constexpr (PVnode)
		{
			if(moveNumber==1)
			{
				if(newDepth < ONE_PLY)
				{
					val = -_qsearch<nodeType::PV_NODE, log>(ply + 1, newDepth, -beta, -alpha, childPV);
				}
				else
				{
					val = -_alphaBeta<nodeType::PV_NODE, log>(ply + 1, newDepth, -beta, -alpha, childPV);
				}
			}
			else
			{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch = true;
				if( depth >= _sp.lmrDepthLimitInf
					&& (!captureOrPromotion || FutilityMoveCountFlag )
					&& !isDangerous
					&& m != ttMove
					&& !mp.isKillerMove(m)
				)
				{
					assert(moveNumber != 0);

					int reduction = _sp.PVreduction[improving][ std::min(depth, int((_sp.LmrLimit * ONE_PLY) - 1)) ][ std::min(moveNumber, _sp.LmrLimitMove - 1) ];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction != 0)
					{
						if (log) ln->doLmrSearch();
						val = -_alphaBeta<nodeType::CUT_NODE, log>(ply + 1, d, -alpha - 1, -alpha, childPV);
						if(val <= alpha)
						{
							doFullDepthSearch = false;
						}
					}
				}


				if(doFullDepthSearch)
				{
					
					if (log) ln->doFullDepthSearchSearch();
					if(newDepth < ONE_PLY)
					{
						val = -_qsearch<nodeType::CUT_NODE, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -_alphaBeta<nodeType::CUT_NODE, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}

					if( val > alpha && val < beta )
					{
						if (log) ln->doFullWidthSearchSearch();
						if( newDepth < ONE_PLY )
						{
							val = -_qsearch<nodeType::PV_NODE, log>(ply + 1, newDepth, -beta, -alpha, childPV);
						}
						else
						{
							val = -_alphaBeta<nodeType::PV_NODE, log>(ply + 1, newDepth, -beta, -alpha, childPV);
						}
					}
				}
			}
		}
		else
		{

			//------------------------------
			//	LMR
			//------------------------------
			bool doFullDepthSearch = true;
			if( depth >= _sp.lmrDepthLimitInf
				&& (!captureOrPromotion || FutilityMoveCountFlag )
				&& !isDangerous
				&& m != ttMove
				&& !mp.isKillerMove(m)
			)
			{
				int reduction = _sp.nonPVreduction[improving][std::min(depth, int((_sp.LmrLimit * ONE_PLY) - 1))][std::min(moveNumber, _sp.LmrLimitMove - 1)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction != 0)
				{
					if (log) ln->doLmrSearch();
					val = -_alphaBeta<childNodesType, log>(ply + 1, d, -alpha - 1, -alpha, childPV);
					if(val <= alpha)
					{
						doFullDepthSearch = false;
					}
				}
			}

			if(doFullDepthSearch)
			{
				if (log) ln->doFullDepthSearchSearch();
				if(moveNumber<5)
				{
					if(newDepth < ONE_PLY)
					{
						val = -_qsearch<childNodesType, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -_alphaBeta<childNodesType, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
				}
				else
				{
					if(newDepth < ONE_PLY)
					{
						val = -_qsearch<nodeType::CUT_NODE, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
					else
					{
						val = -_alphaBeta<nodeType::CUT_NODE, log>(ply + 1, newDepth, -alpha - 1, -alpha, childPV);
					}
				}
			}
		}

		_pos.undoMove();
		if (log) ln->undoMove();

		if(!_stop && val > bestScore)
		{
			if (log) ln->raisedbestScore();
			bestScore = val;

			if(bestScore > alpha)
			{
				if (log) ln->raisedAlpha();
				bestMove = m;
				if constexpr (PVnode)
				{
					alpha = bestScore;
					pvLine.appendNewPvLine( bestMove, childPV);
					if(rootNode && !uciParameters::isMultiPvSearch())
					{
						if(val < beta && depth > 1 * ONE_PLY)
						{
							_UOI->printPV(val, _maxPlyReached, _st.getElapsedTime(), pvLine, _father.getVisitedNodes(), _pos.isChess960());
						}
						if(val > _expectedValue - 800)
						{
							_validIteration = true;
						}
					}
				}
			}
		}
		
		if( m != bestMove )
		{
			if(!captureOrPromotion)
			{
				if(quietMoveCount < quietMoveListSize)
				{
					quietMoveList[quietMoveCount++] = m;
				}
			}
			else
			{
				if(captureMoveCount < captureMoveListSize)
				{
					captureMoveList[captureMoveCount++] = m;
				}
			}
		}
	}

	if (log) ln->endSection();
	// draw
	if(!moveNumber)
	{
		if (log) ln->test("IsMated");
		if( excludedMove )
		{
			if (log) ln->logReturnValue(alpha);
			return alpha;
		}
		else if(!inCheck)
		{
			bestScore = _getDrawValue();
		}
		else
		{
			bestScore = matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;

	if(!_stop)
	{
		_tt.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove ) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove, staticEval);
	}

	// save killer move & update history
	if (bestScore >= beta && !inCheck)
	{
		if (!_pos.isCaptureMoveOrPromotion(bestMove))
		{
			_sd.saveKillers(ply, bestMove);

			// update history
			int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
			Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY * ONE_PLY);

			auto& history = _sd.getHistory();
			history.update( _pos.isWhiteTurn() ? white: black, bestMove, bonus);

			for (unsigned int i = 0; i < quietMoveCount; ++i) {
				history.update( _pos.isWhiteTurn() ? white: black, quietMoveList[i], -bonus);
			}
			
			_updateCounterMove( bestMove );
		}
		else
		{
			//if( _pos.isCaptureMove( bestMove ) )
			{
				// update capture history
				int loc_depth = (depth > ( 17 * ONE_PLY) ) ? 0 : depth;
				Score bonus = Score(loc_depth * loc_depth)/(ONE_PLY * ONE_PLY);

				auto & capt= _sd.getCaptureHistory();
				capt.update( _pos.getPieceAt(bestMove.getFrom()), bestMove, _pos.getPieceAt(bestMove.getTo()), bonus);

				for (unsigned int i = 0; i < captureMoveCount; i++)
				{
					Move mm = captureMoveList[i];
					//if( _pos.isCaptureMove( mm ) )
					{
						capt.update( _pos.getPieceAt(mm.getFrom()), mm, _pos.getPieceAt(mm.getTo()), -bonus);
					}
				}
			}
		}
	}
	if (log) ln->logReturnValue(bestScore);
	return bestScore;

}

template<Searcher::nodeType type, bool log> Score Searcher::_qsearch(unsigned int ply, int depth, Score alpha, Score beta, PVline& pvLine)
{
	std::unique_ptr<logNode> ln;
	if(log) ln =  std::unique_ptr<logNode>(new logNode(*_lw, ply, depth, alpha, beta, "qSearch"));
	//---------------------------------------
	//	node asserts
	//---------------------------------------
	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);
	//---------------------------------------
	//	initialize constants
	//---------------------------------------
	const bool PVnode = (type == nodeType::PV_NODE);
	assert( PVnode || alpha + 1 == beta );

	bool inCheck = _pos.isInCheck();
	_sd.setInCheck(ply, inCheck);

	_updateNodeStatistics(ply);
	if (log) ln->startSection("early returns");
	if (log) ln->test("IsDraw");
	if( _manageDraw( PVnode, pvLine) ) {
		if (log) ln->logReturnValue(_getDrawValue());
		if (log) ln->endSection();
		return _getDrawValue();
	}
	//---------------------------------------
	//	MATE DISTANCE PRUNING
	//---------------------------------------
	//if( _MateDistancePrunqsearing( ply, alpha, beta) ) return alpha;

	//----------------------------
	//	next node type
	//----------------------------
	const nodeType childNodesType =
		type == nodeType::ALL_NODE ?
			nodeType::CUT_NODE :
		type == nodeType::CUT_NODE ?
			nodeType::ALL_NODE :
			nodeType::PV_NODE;


	const HashKey& posKey = _getSearchKey();
	ttEntry* const tte = _tt.probe( _pos.getKey() );
	if (log) ln->logTTprobe(*tte);
	Move ttMove( tte->getPackedMove() );
	if(!_pos.isMoveLegal(ttMove)) {
		ttMove = Move::NOMOVE;
	}
	
	// overwrite ttMove with move from move from PVlineToBeFollowed
	if constexpr ( PVnode )
	{
		if (_pvLineFollower.getNextMove(ply, ttMove))
		{
			assert(_pos.isMoveLegal(ttMove));
		}
	}
	

	MovePicker mp(_pos, _sd, ply, ttMove);
	
	short int TTdepth = mp.setupQuiescentSearch(inCheck, depth) * ONE_PLY;
	Score ttValue = transpositionTable::scoreFromTT(tte->getValue(), ply);

	if (log) ln->test("CanUseTT");
	if( _canUseTTeValue( PVnode, beta, ttValue, tte, TTdepth ) )
	{
		_tt.refresh(*tte);
		if constexpr (PVnode)
		{
			_appendTTmoveIfLegal( ttMove, pvLine);
		}
		if (log) ln->logReturnValue(ttValue);
		if (log) ln->endSection();
		return ttValue;
	}

	if (log) ln->endSection();

	ttType TTtype = typeScoreLowerThanAlpha;

	if (log) ln->startSection("calc eval");

	Score staticEval = (tte->getType() != typeVoid) ? tte->getStaticValue() : _pos.eval<false>();
	if (log) ln->calcStaticEval(staticEval);
#ifdef DEBUG_EVAL_SIMMETRY
	_testSimmetry();
#endif

	//----------------------------
	//	stand pat score
	//----------------------------
	Score bestScore;
	Score futilityBase;
	if(!inCheck)
	{
		bestScore = staticEval;
		// todo trovare un valore buono per il futility

		if( /*!PVnode && */ttValue != SCORE_NONE)
		{
			if (
					( tte->isTypeGoodForBetaCutoff() && (ttValue > staticEval) )
					|| (tte->isTypeGoodForAlphaCutoff() && (ttValue < staticEval) )
			)
			{
				bestScore = ttValue;
			}
		}
		if (log) ln->calcBestScore(bestScore);

		if(bestScore > alpha)
		{
			assert(!inCheck);

			// TODO testare se la riga TTtype=typeExact; ha senso
			if constexpr (PVnode)
			{
				pvLine.clear();
			}

			if (log) ln->test("StandPat");
			if( bestScore >= beta)
			{
				if( !_pos.isCaptureMoveOrPromotion(ttMove) )
				{
					_sd.saveKillers(ply, ttMove);
				}
				if(!_stop)
				{
					_tt.store(posKey, transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, ttMove, staticEval);
				}
				if (log) ln->logReturnValue(bestScore);
				if (log) ln->endSection();
				return bestScore;
			}
			if (log) ln->raisedAlpha();
			alpha = bestScore;
			TTtype = typeExact;

		}
		
		futilityBase = bestScore + 5050;


	}
	else
	{
		bestScore = -SCORE_INFINITE;
		futilityBase = -SCORE_INFINITE;
	}
	
	if (log) ln->endSection();
	if (log) ln->startSection("move loop");


	//----------------------------
	//	try the captures
	//----------------------------
	Move m;
	Move bestMove(Move::NOMOVE);

	PVline childPV;

	while ( ( m = mp.getNextMove() ) )
	{
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert( m );


		if(!inCheck)
		{
			// allow only queen promotion at deeper search
			if( (TTdepth < -1 * ONE_PLY) && ( m.isPromotionMove() ) && (m.getPromotionType() != Move::promQueen))
			{
				if (log) ln->skipMove(m, "not allowed underpomotion");
				continue;
			}

			// at very deep search allow only recapture
			if(depth < -7 * ONE_PLY && _pos.getActualState().getCurrentMove().getTo() != m.getTo())
			{
				if (log) ln->skipMove(m, "only allowed recapture");
				continue;
			}

			//----------------------------
			//	futility pruning (delta pruning)
			//----------------------------
			//if constexpr ( !PVnode )
			//{
				//if(
					//m != ttMove
					//&& m.bit.flags != Move::fpromotion
					//&& !_pos.moveGivesCheck(m)
					//)
				//{
					bool moveGiveCheck = _pos.moveGivesCheck(m);
					if(
						!moveGiveCheck
						&& !_pos.isPassedPawnMove(m)
						&& futilityBase>-SCORE_KNOWN_WIN
					)
					{
						Score futilityValue = futilityBase
								+ _pos.getPieceValue(_pos.getPieceAt(m.getTo()))[1]
								+ ( m.isEnPassantMove() ? _pos.getPieceValue(whitePawns)[1] : 0);

						if( m.isPromotionMove() )
						{
							futilityValue += _pos.getPieceValue(static_cast<bitboardIndex>(static_cast<int>(m.getPromotionType()) + static_cast<int>(whiteQueens)))[1] - _pos.getPieceValue(whitePawns)[1];
						}

						if (futilityValue <= alpha)
						{
							bestScore = std::max(bestScore, futilityValue);
							if (log) ln->skipMove(m, "futility margin");
							continue;
						}
						
						if (futilityBase <= alpha && _pos.seeSign(m) <= 0)
						{
							bestScore = std::max(bestScore, futilityBase);
							if (log) ln->skipMove(m, "futile & not gaining");
							continue;
						}

					}


					//----------------------------
					//	don't check moves with negative see
					//----------------------------

					// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
					// TODO testare se aggiungere o no !movegivesCheck() &&
					if(
							//!moveGiveCheck &&
							_pos.seeSign(m) < 0)
					{
						if (log) ln->skipMove(m, "negative see");
						continue;
					}
				//}
			//}

		}
		
		_pos.doMove(m);
		__builtin_prefetch(_tt.findCluster(_pos.getKey().getKey()));
		if (log) ln->doMove(m);
		Score val = -_qsearch<childNodesType, log>(ply+1, depth - ONE_PLY, -beta, -alpha, childPV);
		_pos.undoMove();
		if (log) ln->undoMove();

		if(val > bestScore)
		{
			if (log) ln->raisedbestScore();
			bestScore = val;
			if( bestScore > alpha )
			{
				if (log) ln->raisedAlpha();
				bestMove = m;
				TTtype = typeExact;
				alpha = bestScore;

				if(PVnode && !_stop)
				{
					pvLine.appendNewPvLine( bestMove, childPV ); 

				}
				if( bestScore >= beta)
				{
					if( !_pos.isCaptureMoveOrPromotion(bestMove) && !inCheck )
					{
						_sd.saveKillers(ply, bestMove);
					}
					if(!_stop)
					{
						_tt.store(posKey, transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)TTdepth, bestMove, staticEval);
					}
					if (log) ln->logReturnValue(bestScore);
					if (log) ln->endSection();
					return bestScore;
				}
			}
		}
	}

	if (log) ln->endSection();
	if(bestScore == -SCORE_INFINITE)
	{
		if (log) ln->test("IsMated");
		assert(inCheck);
		if constexpr (PVnode)
		{
			pvLine.clear();
		}
		if (log) ln->logReturnValue(matedIn(ply));
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);

	if( !_stop )
	{
		_tt.store(posKey, transpositionTable::scoreToTT(bestScore, ply), TTtype, (short int)TTdepth, bestMove, staticEval);
	}
	if (log) ln->logReturnValue(bestScore);
	return bestScore;

}

Score Searcher::_futility(int depth, bool improving )
{
	return _sp.staticNullMovePruningValue * depth - _sp.staticNullMovePruningImprovingBonus * improving;
}

inline Score Searcher::_getDrawValue() const
{
	int contemptSign = _pos.isTurn(_initialTurn) ? 1 : -1;
	return contemptSign * std::min( (int)0, (int)(-5000 + _pos.getPly()*250) );
}

inline void Searcher::_updateCounterMove(const Move& m)
{
	if( const Move& previousMove = _pos.getActualState().getCurrentMove() )
	{
		_sd.getCounterMove().update( _pos.getPieceAt( previousMove.getTo() ), previousMove.getTo(), m );
	}
}

inline void Searcher::_updateNodeStatistics(const unsigned int ply)
{
	_maxPlyReached = std::max(ply, _maxPlyReached);
	++_visitedNodes;
}

bool Searcher::_manageDraw(const bool PVnode, PVline& pvLine)
{
	if(_pos.isDraw(PVnode) || _stop)
	{
		if(PVnode)
		{
			pvLine.clear();
		}
		return true;
	}
	return false;
}

void Searcher::_showCurrenLine(const unsigned int ply, const int depth)
{
	if( _showLine && depth <= ONE_PLY)
	{
		_showLine = false;
		_UOI->showCurrLine(_pos, ply);
	}
}

inline bool Searcher::_MateDistancePruning(const unsigned int ply, Score& alpha, Score& beta) const
{
	alpha = std::max(matedIn(ply), alpha);
	beta = std::min(mateIn(ply+1), beta);
	if (alpha >= beta)
	{
		return true;
	}
	return false;
}

void Searcher::_appendTTmoveIfLegal(const Move& ttm, PVline& pvLine) const
{
	if( _pos.isMoveLegal(ttm) )
	{
		pvLine.set( ttm );
	}
	else
	{
		pvLine.clear();
	}
}

inline bool Searcher::_canUseTTeValue(const bool PVnode, const Score beta, const Score ttValue, const ttEntry * const tte, short int depth) const
{
	return
		( tte->getDepth() >= depth )
		&& ( ttValue != SCORE_NONE )// Only in case of TT access race
		&& (
			PVnode ?
				false :
			ttValue >= beta ?
				tte->isTypeGoodForBetaCutoff():
				tte->isTypeGoodForAlphaCutoff()
		);
}

inline const HashKey Searcher::_getSearchKey( const bool excludedMove ) const
{
	return excludedMove ? _pos.getExclusionKey() : _pos.getKey();
}


inline Searcher::tableBaseRes Searcher::_checkTablebase(const unsigned int ply, const int depth)
{
	tableBaseRes res{typeScoreLowerThanAlpha, SCORE_NONE};
	Syzygy& szg = Syzygy::getInstance();
	
	if (szg.getMaxCardinality()) {
		
		res.TTtype = typeScoreLowerThanAlpha;
		unsigned int piecesCnt = bitCnt (_pos.getBitmap(whitePieces) | _pos.getBitmap(blackPieces));

		if (piecesCnt <= szg.getMaxCardinality()
			&& (piecesCnt < szg.getMaxCardinality() || depth >= (int)( uciParameters::SyzygyProbeDepth * ONE_PLY ) )
			&& _pos.getActualState().getIrreversibleMoveCount() == 0
			&& _pos.getCastleRights() == noCastle ) {
			
			ProbeState err;			
			WDLScore wdl = szg.probeWdl(_pos, err);
			
			if (err != ProbeState::FAIL) {
				++_tbHits;

				if (uciParameters::Syzygy50MoveRule) {
					switch(wdl)
					{
					case WDLScore::WDLLoss:
						res.value = SCORE_MATED + 100 + ply;
						res.TTtype = typeScoreLowerThanAlpha;
						break;
					case WDLScore::WDLBlessedLoss:
						res.TTtype = typeExact;
						res.value = -100;
						break;
					case WDLScore::WDLDraw:
						res.TTtype = typeExact;
						res.value = 0;
						break;
					case WDLScore::WDLCursedWin:
						res.TTtype = typeExact;
						res.value = 100;
						break;
					case WDLScore::WDLWin:
						res.TTtype = typeScoreHigherThanBeta;
						res.value = SCORE_MATE -100 -ply;
						break;
					default:
						res.value = 0;
					}

				}
				else
				{
					switch(wdl)
					{
					case WDLScore::WDLLoss:
					case WDLScore::WDLBlessedLoss:
						res.TTtype = typeScoreLowerThanAlpha;
						res.value = SCORE_MATED + 100 + ply;
						break;
					case WDLScore::WDLDraw:
						res.TTtype = typeExact;
						res.value = 0;
						break;
					case WDLScore::WDLCursedWin:
					case WDLScore::WDLWin:
						res.TTtype = typeScoreHigherThanBeta;
						res.value = SCORE_MATE - 100 - ply;
						break;
					default:
						res.value = 0;
					}
				}
			}
		}
	}
	return res;
}

#ifdef DEBUG_EVAL_SIMMETRY
void Searcher::_testSimmetry() const
{
	Position ppp(Position::pawnHash::off);

	ppp.setupFromFen(_pos.getSymmetricFen());

	Score staticEval = _pos.eval<false>();
	Score test = ppp.eval<false>();

	if(test != staticEval)
	{
		sync_cout << "eval symmetry problem " << test << ":" << staticEval << sync_endl;
		_pos.display();
		ppp.display();
		exit(-1);
	}
}
#endif

Score Searcher::performQsearch() {
	PVline pv;
	return _qsearch<nodeType::PV_NODE, false>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, pv);
}