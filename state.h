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
#ifndef STATE_H_
#define STATE_H_

#include "bitBoardIndex.h"
#include "eCastle.h"
#include "hashKey.h"
#include "move.h"
#include "score.h"

//--------------------------------------------------------
// struct
//--------------------------------------------------------
/*! \brief define the state of the board
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
struct state
{

public:
	// todo modifiche a keys inseririle dentro alle altre chiamate

	state(){}

	inline HashKey& getKey()
	{
		return _key;
	}

	inline const HashKey& getKey() const
	{
		return _key;
	}

	inline void setKey( const HashKey& k )
	{
		_key = k;
	}

	inline HashKey& getPawnKey()
	{
		return _pawnKey;
	}

	inline const HashKey& getPawnKey() const
	{
		return _pawnKey;
	}

	inline void setPawnKey( const HashKey& k )
	{
		_pawnKey = k;
	}

	inline HashKey& getMaterialKey()
	{
		return _materialKey;
	}

	inline const HashKey& getMaterialKey() const
	{
		return _materialKey;
	}

	inline void setMaterialKey( const HashKey& k )
	{
		_materialKey = k;
	}


	inline bool hasEpSquare() const
	{
		return _epSquare != squareNone;
	}

	inline bool isEpSquare( const tSquare s) const
	{
		return _epSquare == s;
	}

	inline tSquare getEpSquare() const
	{
		assert( _epSquare < squareNumber );
		return _epSquare;
	}

	inline void resetEpSquare()
	{
		_epSquare = squareNone;
	}

	inline void setEpSquare( const tSquare s )
	{
		_epSquare = s;
	}

	inline unsigned int getPliesFromNullCount() const
	{
		return _pliesFromNull;
	}
	inline void resetPliesFromNullCount()
	{
		_pliesFromNull = 0;
	}

	inline void incrementPliesFromNullCount()
	{
		++_pliesFromNull;
	}

	inline unsigned int getIrreversibleMoveCount() const
	{
		return _fiftyMoveCnt;
	}

	inline void setIrreversibleMoveCount(unsigned int x)
	{
		_fiftyMoveCnt = x;
	}

	inline void resetIrreversibleMoveCount()
	{
		_fiftyMoveCnt = 0;
	}

	inline void incrementIrreversibleMoveCount()
	{
		++_fiftyMoveCnt;
	}

	inline simdScore getNonPawnValue() const
	{
		return _nonPawnMaterial;
	}

	inline void setNonPawnValue( const simdScore& sc)
	{
		_nonPawnMaterial = sc;
	}

	inline void addNonPawnMaterial( const simdScore& sc)
	{
		_nonPawnMaterial += sc;
	}

	inline void removeNonPawnMaterial( const simdScore& sc)
	{
		_nonPawnMaterial -= sc;
	}


	inline simdScore getMaterialValue() const
	{
		return _material;
	}

	inline void setMaterialValue( const simdScore& sc)
	{
		_material = sc;
	}

	inline void addMaterial( const simdScore& sc)
	{
		_material += sc;
	}

	inline void removeMaterial( const simdScore& sc)
	{
		_material -= sc;
	}

	static inline eCastle calcCastleRight( const eCastle cr, const Color c )
	{
		return eCastle( cr << (2 * c ) );
	}

	inline bool hasCastleRight( const eCastle cr )const
	{
		return _castleRights & cr;
	};

	inline bool hasCastleRight( const eCastle cr, const Color c )const
	{
		return _castleRights & calcCastleRight( cr, c );
	};

	inline const eCastle& getCastleRights() const
	{
		return _castleRights;
	}

	inline bool hasCastleRights() const
	{
		return _castleRights;
	}

	inline void clearCastleRight()
	{
		_castleRights = (eCastle)0;
	}

	inline void clearCastleRight( const eCastle c )
	{
		_castleRights = (eCastle)(_castleRights & ( ~c ) );
	}

	inline void setCastleRight( const eCastle c )
	{
		_castleRights = (eCastle)( _castleRights | c );
	}

	inline const Move& getCurrentMove() const
	{
		return _currentMove;
	}

	inline void setCurrentMove( const Move & m )
	{
		_currentMove = m;
	}

	inline bool isInCheck() const
	{
		return _checkers;
	}

	inline bool isInDoubleCheck() const
	{
		return moreThanOneBit( _checkers );
	}

	inline const bitMap& getCheckers() const
	{
		return _checkers;
	}

	inline void setCheckers( const bitMap & b )
	{
		_checkers = b;
	}

	inline void addCheckers( const bitMap & b )
	{
		_checkers |= b;
	}

	inline void setPinnedPieces( const bitMap & b )
	{
		_pinnedPieces = b;
	}

	inline bool isPinned( const tSquare& sq ) const
	{
		return isSquareSet( _pinnedPieces, sq );
	}

	inline void setNextTurn( const eNextMove nm )
	{
		_nextMove = nm;
	}

	inline eNextMove getNextTurn() const
	{
		return _nextMove;
	}

	inline bitboardIndex getPiecesOfActivePlayer() const
	{
		return (bitboardIndex)(whitePieces + _nextMove);
	}

	inline bitboardIndex getPiecesOfOtherPlayer() const
	{
		return (bitboardIndex)(blackPieces - _nextMove);
	}

	inline bitboardIndex getKingOfActivePlayer() const
	{
		return (bitboardIndex)(whiteKing + _nextMove);
	}

	inline bitboardIndex getKingOfOtherPlayer() const
	{
		return (bitboardIndex)(blackKing - _nextMove);
	}

	inline bitboardIndex getPawnsOfActivePlayer() const
	{
		return (bitboardIndex)(whitePawns + _nextMove);
	}

	inline bitboardIndex getPawnsOfOtherPlayer() const
	{
		return (bitboardIndex)(blackPawns - _nextMove);
	}

	inline bool isBlackTurn() const
	{
		return _nextMove;
	}

	inline bool isWhiteTurn() const
	{
		return !isBlackTurn();
	}

	inline void changeNextTurn()
	{
		_nextMove = getSwitchedTurn();
	}

	inline eNextMove getSwitchedTurn() const
	{
		return (eNextMove)( blackTurn - _nextMove );
	}

	inline bool thereAreHiddenCheckers() const
	{
		return _hiddenCheckersCandidate;
	}
	inline void setHiddenCheckers( const bitMap & b )
	{
		_hiddenCheckersCandidate = b;
	}

	inline bool isHiddenChecker( const tSquare& sq ) const
	{
		return isSquareSet(_hiddenCheckersCandidate, sq );
	}
	
	inline void setHiddenChecker( const bitMap & b )
	{
		_hiddenCheckersCandidate = b;
	}
	
	inline void setCheckingSquares( const bitboardIndex piece, const bitMap & b )
	{
		_checkingSquares[ piece ] = b;
	}
	
	inline void resetCheckingSquares( const bitboardIndex piece )
	{
		_checkingSquares[ piece ] = 0;
	}
	
	inline const bitMap& getCheckingSquares( const bitboardIndex piece ) const
	{
		return _checkingSquares[ piece ];
	}
	
	inline const bitboardIndex& getCapturedPiece() const
	{
		return _capturedPiece;
	}
	
	inline void setCapturedPiece( const bitboardIndex p )
	{
		_capturedPiece = p;
	}
	
	inline void resetCapturedPiece()
	{
		_capturedPiece = empty;
	}
	
	
	

private:
	eCastle _castleRights; /*!<  actual castle rights*/
	Move _currentMove;
	bitMap _pinnedPieces;	/*!< pinned pieces*/
	bitMap _checkers;	/*!< checking pieces*/
	eNextMove _nextMove; /*!< who is the active player*/
	bitMap _hiddenCheckersCandidate;	/*!< pieces who can make a discover check moving*/
	simdScore _material;
	simdScore _nonPawnMaterial; /*!< four score used for white/black opening/endgame non pawn material sum*/
	unsigned int _fiftyMoveCnt;	/*!<  50 move count used for draw rule*/
	unsigned int _pliesFromNull;	/*!<  plies from null move*/
	bitMap _checkingSquares[lastBitboard]; /*!< squares of the board from where a king can be checked*/
	bitboardIndex _capturedPiece; /*!<  index of the captured piece for unmakeMove*/
	tSquare _epSquare;	/*!<  en passant square*/
	HashKey _key,		/*!<  hashkey identifying the position*/
			_pawnKey,	/*!<  hashkey identifying the pawn formation*/
			_materialKey;/*!<  hashkey identifying the material signature*/

};

#endif
