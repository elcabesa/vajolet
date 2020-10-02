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
#include <thread>

#include "book.h"
#include "vajo_io.h"
#include "movepicker.h"
#include "rootMove.h"
#include "searchImpl.h"
#include "thread.h"
#include "uciOutput.h"
#include "uciParameters.h"
#include "searchResult.h"
#include "syzygy/syzygy.h"
#include "vajolet.h"

class voteSystem
{
public:
	explicit voteSystem( const std::vector<rootMove>& res): _results(res) {}

	void print(const std::map<unsigned short, int>& votes, const Score minScore, const rootMove& bm) const
	{
		std::cout<<"----------VOTE SYSTEM-------------"<<std::endl;
		std::cout<<"minScore: "<<minScore<<std::endl;
		std::cout<<"----------------------------------"<<std::endl;
		for (auto &res : votes)
		{
			Move m(res.first);
			// todo shall it know wheher the move is chess960?
			std::cout<<"Move: "<<UciOutput::displayUci(m, false)<<" votes: "<<res.second;
			if( bm == m ) std::cout<<" *****";
			std::cout<<std::endl;
		}
		std::cout<<"----------------------------------"<<std::endl;

		for (auto &res : _results)
		{
			if(res == bm)
			{
				std::cout<<"bestMove: "<<UciOutput::displayUci(res.firstMove, false)<<" *****"<<std::endl;
			}
			else
			{
				std::cout<<"bestMove: "<<UciOutput::displayUci(res.firstMove, false)<<std::endl;
			}
			std::cout<<"score: "<<res.score<<std::endl;
			std::cout<<"depth: "<<res.depth<<std::endl;
			std::cout<<"votes: "<<votes.at( res.firstMove.getPacked() )<<std::endl;
		}
	}

	const rootMove& getBestMove( bool verbose = false) const
	{

		std::map<unsigned short, int> votes;

		//////////////////////////////////////////
		// calc the min value
		//////////////////////////////////////////
		Score minScore = _results[0].score;
		for (auto &res : _results)
		{
			minScore = std::min(minScore, res.score);
			votes[res.firstMove.getPacked()] = 0;
		}

		//////////////////////////////////////////
		// calc votes
		//////////////////////////////////////////
		for (auto &res : _results)
		{
			votes[res.firstMove.getPacked()] += (int)(res.score - minScore) + 40 * res.depth;
		}

		//////////////////////////////////////////
		// find the maximum
		//////////////////////////////////////////
		const rootMove* bestMove = &_results[0];
		int bestResult = votes[_results[0].firstMove.getPacked()];

		for (auto &res : _results)
		{
			if (votes[ res.firstMove.getPacked() ] > bestResult)
			{
				bestResult = votes[res.firstMove.getPacked()];
				bestMove = &res;
			}
		}

		if( verbose ) print(votes, minScore, *bestMove);

		return *bestMove;
	}

private:
	const std::vector<rootMove>& _results;
};

unsigned long long Search::impl::getVisitedNodes() const
{
	//TODO replace by algorithm??
	unsigned long long n = 0;
	for (auto& hs : _searchers)
		n += hs.getVisitedNodes();
	return n;
}

unsigned long long Search::impl::getTbHits() const
{
	//TODO replace by algorithm??
	unsigned long long n = 0;
	for (auto& hs : _searchers)
		n += hs.getTbHits();
	return n;
}

void Search::impl::_generateRootMovesList(const std::list<Move>& ml)
{
	if(ml.size() == 0)	// all the legal moves
	{
		_rootMovesToBeSearched.fill(_pos.getLegalMoves());
	}
	else //only selected moves
	{
		_rootMovesToBeSearched.fill(ml);
	}
}

SearchResult Search::impl::_manageQsearch(timeManagement & tm)
{
	PVline pvLine;
	Searcher s(*this, _st, _sl, _tt, _sp, tm, _rootMovesToBeSearched, pvLine, UciOutput::type::mute );

	Score res = s.performQsearch();
	
	_UOI->printScore( res/100 );
	
	return SearchResult( -SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, res );
}


void Search::impl::_filterRootMovesByTablebase()
{
	if(_rootMovesToBeSearched.size() > 0) {
		
		std::vector<extMove> rm2;
		for (auto m: _rootMovesToBeSearched.getAll()) {
			extMove em(m);
			rm2.push_back(em);
		}

		unsigned int piecesCnt = bitCnt (_pos.getBitmap(whitePieces) | _pos.getBitmap(blackPieces));
		Syzygy& szg = Syzygy::getInstance();
		if (piecesCnt <= szg.getMaxCardinality() && _pos.getCastleRights() == noCastle) {
			
			bool found = szg.rootProbe(_pos, rm2) || szg.rootProbeWdl(_pos, rm2);
			
			if (found) {
				std::sort(rm2.begin(), rm2.end());
				std::reverse(rm2.begin(), rm2.end());
				Score Max = rm2[0].getScore();
				
				for (auto m: rm2) {
					if (m.getScore() < Max) {
						_rootMovesToBeSearched.remove(m);
					}
				}
			}	
		}
	}
}


SearchResult Search::impl::_go(timeManagement& tm, /*int depth,*/ Score alpha, Score beta, PVline pvToBeFollowed)
{
	// manage depth 0 search ( return qsearch )
	if(_sl.getDepth() == 0)
	{
		return _manageQsearch(tm);
	}
	//------------------------------------
	//init the new search
	//------------------------------------
	
	//clean transposition table
	_tt.newSearch();
	
	//--------------------------------
	// generate the list of root moves to be searched
	//--------------------------------
	_generateRootMovesList(_sl.getMoveList());
	
	//--------------------------------
	//	tablebase probing, filtering rootmoves to be searched
	//--------------------------------
	if(!_sl.isSearchMovesMode() && !uciParameters::isMultiPvSearch())
	{
		_filterRootMovesByTablebase();
	}

	// setup other threads
	_searchers.clear();
	for(unsigned int i = 0; i < uciParameters::threads; ++i) {
		_searchers.emplace_back(
			*this,
			_st,
			_sl,
			_tt,
			_sp,
			tm,
			_rootMovesToBeSearched,
			pvToBeFollowed,
			i == 0 ? _UOI->getType() : UciOutput::type::mute
			);
	}
	//----------------------------------
	// we can start the real search
	//----------------------------------

	//----------------------------
	// multithread : lazy smp threads
	//----------------------------

	
	std::vector<std::thread> helperThread;
	Move m(0);
	rootMove rm(m);
	std::vector<rootMove> helperResults( uciParameters::threads, rm);
	std::vector<Move> toBeExcludedMove( uciParameters::threads, Move::NOMOVE);

	// launch helper threads
	//----------------------------------
	// iterative deepening loop
	//----------------------------------
	for( unsigned int i = 0; i < ( uciParameters::threads); ++i)
	{
		helperResults[i].firstMove = m;
		helperThread.emplace_back( std::thread(&Searcher::searchManager, &_searchers[i], std::ref(helperResults), i, std::ref(toBeExcludedMove)));
	}
  	helperThread[0].join();
	// _stop helper threads
	for(auto &s : _searchers)
	{
		s.stopSearch();
	}

	for( unsigned int i = 1; i < ( uciParameters::threads); ++i)
	{
		helperThread[i].join();
	}
	//----------------------------------
	// gather results
	//----------------------------------
	const rootMove& bestMove = voteSystem(helperResults).getBestMove();
	
	return SearchResult( alpha, beta, bestMove.depth, bestMove.PV, bestMove.score);

}



void Search::impl::setUOI( UciOutput::type UOI )
{
	// manage output syncronization
	sync_cout;
	_UOI = UciOutput::create(UOI);
	std::cout<<sync_noNewLineEndl;
}

Move Search::impl::_getPonderMoveFromHash(const Move& bestMove )
{
	Move ponderMove(0);
	_pos.doMove( bestMove );
	
	const ttEntry* const tte = _tt.probe(_pos.getKey());
	
	Move m( tte->getPackedMove() );
	if( _pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_pos.undoMove();
	
	return ponderMove;
}

Move Search::impl::_getPonderMoveFromBook(const Move& bookMove )
{
	Move ponderMove(0);
	_pos.doMove( bookMove );
	PolyglotBook pol;
	Move m = pol.probe( _pos, uciParameters::bestMoveBook);
	
	if( _pos.isMoveLegal(m) )
	{
		ponderMove = m;
	}
	_pos.undoMove();
	
	return ponderMove;
}

void Search::impl::_waitStopPondering() const
{
	while(_sl.isPondering()){}
}

Position& Search::impl::getPosition()
{
	return _pos;
}

SearchResult Search::impl::manageNewSearch(timeManagement & tm)
{
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/

	if( _game.isNewGame(_pos))
	{
		_game.CreateNewGame(_pos.isChess960());

	}
	_game.insertNewMoves(_pos);

	unsigned int legalMoves = _pos.getNumberOfLegalMoves();

	if(legalMoves == 0)
	{
		_UOI->printPV( Move::NOMOVE, _pos.isChess960());
		
		_waitStopPondering();

		_UOI->printBestMove( Move::NOMOVE, Move::NOMOVE, _pos.isChess960() );

		PVline pvLine;
		return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );
	}
	
	if( legalMoves == 1 && !_sl.isInfiniteSearch() )
	{
		Move bestMove = MovePicker( _pos ).getNextMove();
		
		_UOI->printPV(bestMove, _pos.isChess960());
		
		_waitStopPondering();
		
		Move ponderMove = _getPonderMoveFromHash( bestMove );
		
		_UOI->printBestMove( bestMove, ponderMove, _pos.isChess960() );

		PVline pvLine;
		pvLine.set(bestMove, ponderMove);
		return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );

	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------
	if( uciParameters::useOwnBook && !_sl.isInfiniteSearch() )
	{
		PolyglotBook pol;
		Move bookM = pol.probe( _pos, uciParameters::bestMoveBook);
		if( bookM )
		{
			_UOI->printPV(bookM, _pos.isChess960());
			
			_waitStopPondering();
			
			Move ponderMove = _getPonderMoveFromBook( bookM );
			
			_UOI->printBestMove(bookM, ponderMove, _pos.isChess960());
			
			PVline pvLine;
			pvLine.set(bookM, ponderMove);
			return SearchResult(-SCORE_INFINITE, SCORE_INFINITE, 0, pvLine, 0 );
		}
	}
	
	//if( game.isPonderRight() )
	//{
	//	Game::GamePosition gp = game.getNewSearchParameters();
	//
	//	PVline newPV;
	//	std::copy( gp.PV.begin(), gp.PV.end(), std::back_inserter( newPV ) );
	//	
	//	newPV.resize(gp.depth/2 + 1);
	//	newPV.pop_front();
	//	newPV.pop_front();
	//	res = src.go( gp.depth/2 + 1, gp.alpha, gp.beta, newPV );
	//}
	//else
	
	SearchResult res = _go(tm);
	
	PVline PV = res.PV;

	_waitStopPondering();

	//-----------------------------
	// print out the choosen line
	//-----------------------------

	_UOI->printGeneralInfo( _tt.getFullness(), getTbHits(), getVisitedNodes(), _st.getElapsedTime());
	
	Move bestMove = PV.getMove(0);
	Move ponderMove = PV.getMove(1);
	if( !ponderMove )
	{
		ponderMove = _getPonderMoveFromHash( bestMove );
	}
	
	_UOI->printBestMove( bestMove, ponderMove, _pos.isChess960() );

	_game.savePV(PV, res.depth, res.alpha, res.beta);
	
	return res;

}

bool Search::impl::setNnue(bool use, std::string path) {
	if(use) {
		return _nnue.init(path);
	}
	else {
		_nnue.clear();
		return true;
	}
	
}


Search::Search( SearchTimer& st, SearchLimits& sl, transpositionTable& tt, std::unique_ptr<UciOutput> UOI):pimpl{std::make_unique<impl>(st, sl, tt, std::move(UOI))}{}
Search::~Search() = default;
void Search::stopSearch(){ pimpl->stopSearch(); }
unsigned long long Search::getVisitedNodes() const{ return pimpl->getVisitedNodes(); }
unsigned long long Search::getTbHits() const{ return pimpl->getTbHits(); }
void Search::showLine(){ pimpl->showLine(); }
SearchResult Search::manageNewSearch(timeManagement & tm){ return pimpl->manageNewSearch(tm); }
Position& Search::getPosition(){ return pimpl->getPosition(); }
void Search::setUOI( UciOutput::type UOI ) { pimpl->setUOI(UOI); }
SearchParameters& Search::getSearchParameters() { return pimpl->getSearchParameters(); }
bool Search::setNnue(bool use, std::string path) { return pimpl->setNnue(use, path); }
