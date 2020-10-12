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
#ifndef POSITION_H_
#define POSITION_H_

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <set>

#include "bitBoardIndex.h"
#include "data.h"
#include "eCastle.h"
#include "hashKey.h"
#include "movegen.h"
#include "move.h"
#include "parameters.h"
#include "score.h"
#include "state.h"
#include "vajolet.h"
//---------------------------------------------------
// forward declarations
//---------------------------------------------------
class pawnEntry;
class pawnTable;
class NNUE;

//---------------------------------------------------
//	class
//---------------------------------------------------
class Position
{
public:
	//--------------------------------------------------------
	// enum definition
	//--------------------------------------------------------
	enum class pawnHash{
		on,
		off
	};

	enum class nnueConfig{
		on,
		off
	};

	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------
	static void initMaterialKeys(void);
	
	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	
	/*! \brief constructor
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	explicit Position(const nnueConfig nnue = nnueConfig::off, const pawnHash usePawnHash = pawnHash::on, const EvalParameters& eParm = EvalParameters());
	explicit Position(const Position& other, const nnueConfig nnue = nnueConfig::off, const pawnHash usePawnHash = pawnHash::on);
	~Position();
	Position& operator=(const Position& other);
	
	
	void setupCastleData (const eCastle cr, const tSquare kFrom, const tSquare kTo, const tSquare rFrom, const tSquare rTo);
	bitMap initCastlePath(const tSquare kSqFrom, const tSquare kSqTo, const tSquare rSqFrom, const tSquare rSqTo);
	bitMap initKingPath(const tSquare kSqFrom, const tSquare kSqTo);
	

	inline bitMap getOccupationBitmap() const
	{
		return _bitBoard[occupiedSquares];
	}
	inline bitMap getBitmap(const bitboardIndex in) const
	{
		return _bitBoard[in];
	}
	inline unsigned int getPieceCount(const bitboardIndex in) const
	{
		return bitCnt( getBitmap( in ) );
	}

	inline bitboardIndex getPieceAt(const tSquare sq) const
	{
		return _squares[sq];
	}

	inline bitboardIndex getPieceTypeAt(const tSquare sq) const
	{
		return getPieceType(_squares[sq]);
	}

	inline tSquare getSquareOfThePiece(const bitboardIndex piece) const
	{
		return firstOne(getBitmap(piece));
	}

	inline tSquare getSquareOfOurKing() const
	{
		return firstOne( getOurBitmap( King ) );
	}

	inline tSquare getSquareOfTheirKing() const
	{
		return firstOne( getTheirBitmap( King ) );
	}


	inline bitMap getOurBitmap(const bitboardIndex piece)const { return Us[piece];}
	inline bitMap getTheirBitmap(const bitboardIndex piece)const { return Them[piece];}


	unsigned int getStateSize() const
	{
		return _stateInfo.size();
	}
	
	unsigned int getNumberOfLegalMoves() const;
	MoveList<MAX_MOVE_PER_POSITION> getLegalMoves() const;
	
	void display(void) const;
	std::string getFen(void) const;
#ifdef DEBUG_EVAL_SIMMETRY
	std::string getSymmetricFen() const;
#endif

	Position& setupFromFen(const std::string& fenStr);
	const Position& setup(const std::string& code, const Color c);

	void doNullMove();
	void doMove(const Move &m);
	void undoMove();
	void undoNullMove();


	template<bool trace>Score eval(void) const;
	bool isDraw(bool isPVline) const;
	bool hasRepeated(bool isPVline = false) const;


	bool moveGivesCheck(const Move& m)const ;
	bool moveGivesDoubleCheck(const Move& m)const;
	bool moveGivesSafeDoubleCheck(const Move& m)const;
	bool isCheckMate() const;
	bool isStaleMate() const;
	
	Score see(const Move& m) const;
	Score seeSign(const Move& m) const;
	
	const HashKey& getKey(void) const
	{
		return getActualState().getKey();
	}

	/*const HashKey getKeyAfterMove(const Move& m) const
	{
		auto k = getActualState().getKey();
		const tSquare from = m.getFrom();
		const tSquare to = m.getTo();
		const bitboardIndex piece = getPieceAt(from);
		const bitboardIndex captured = getPieceAt(to);

		k.changeSide();

		if(captured){
			k.updatePiece(to,captured);
		}
		k.updatePiece( from, to, piece );
		return k;	
	}*/

	const HashKey getExclusionKey(void) const
	{
		return getActualState().getKey().getExclusionKey();
	}

	const HashKey& getPawnKey(void) const
	{
		return getActualState().getPawnKey();
	}

	const HashKey& getMaterialKey(void) const
	{
		return getActualState().getMaterialKey();
	}

	eNextMove getNextTurn(void) const
	{
		return getActualState().getNextTurn();
	}

	inline bool isBlackTurn() const
	{
		return getActualState().isBlackTurn();
	}

	inline bool isWhiteTurn() const
	{
		return getActualState().isWhiteTurn();
	}
	
	inline bool isTurn(const eNextMove t) const
	{
		return getActualState().getNextTurn() == t;
	}

	tSquare getEpSquare(void) const
	{
		return getActualState().getEpSquare();
	}

	bool hasEpSquare(void) const
	{
		return getActualState().hasEpSquare();
	}

	eCastle getCastleRights(void) const
	{
		return getActualState().getCastleRights();
	}

	unsigned int getPly(void) const
	{
		return _ply;
	}

	bitboardIndex getCapturedPiece(void) const
	{
		return getActualState().getCapturedPiece();
	}

	bool isInCheck(void) const
	{
		return getActualState().getCheckers();
	}
	
	/*! \brief return a reference to the actual state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline const state& getActualState(void)const
	{
		return _stateInfo.back();
	}
	
	inline state& getActualState(void)
	{
		return _stateInfo.back();
	}

	inline const state& getState(unsigned int n)const
	{
		return _stateInfo[n];	
	}
	
	bool isMoveLegal(const Move &m) const;

	/*! \brief return a bitmap with all the attacker/defender of a given square
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline bitMap getAttackersTo(const tSquare to) const
	{
		return getAttackersTo(to, _bitBoard[occupiedSquares]);
	}

	bitMap getAttackersTo(const tSquare to, const bitMap occupancy) const;


	/*! \brief return the mvvlva score
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline Score getMvvLvaScore(const Move & m) const
	{
		Score s = _eParm._pieceValue[ getPieceAt( m.getTo() ) ][0] + getPieceAt( m.getFrom() );
		if( m.isEnPassantMove() )
		{
			s += _eParm._pieceValue[ whitePawns ][0];
		}
		return s;
	}

	/*! \brief return the gamephase for interpolation
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline unsigned int getGamePhase( const state& s ) const
	{
		const int opening = 700000;
		const int endgame = 100000;

		Score tot = s.getNonPawnValue()[0] + s.getNonPawnValue()[2];
		if(tot>opening) //opening
		{
			return 0;

		}
		if(tot<endgame)	//endgame
		{
			return 65536;
		}
		return (unsigned int)((float)(opening-tot)*(65536.0f/(float)(opening-endgame)));
	}

	inline bool isCaptureMove(const Move & m) const
	{
		return !m.isCastleMove() && ( getPieceAt( m.getTo() ) !=empty  || m.isEnPassantMove());
	}
	inline bool isCaptureMoveOrPromotion(const Move & m) const
	{
		return isCaptureMove(m) || m.isPromotionMove();
	}
	inline bool isPassedPawnMove(const Move & m) const
	{
		if(isPawn(getPieceAt(m.getFrom())))
		{
			bool color = isBlackPiece( getPieceAt( m.getFrom() ) );
			bitMap theirPawns = color? _bitBoard[whitePawns]:_bitBoard[blackPawns];
			bitMap ourPawns = color? _bitBoard[blackPawns]:_bitBoard[whitePawns];
			return !(theirPawns & PASSED_PAWN[color][m.getFrom()]) && !(ourPawns & SQUARES_IN_FRONT_OF[color][m.getFrom()]);
		}
		return false;
	}
	
	inline bool hasActivePlayerNonPawnMaterial() const
	{
		return getActualState().getNonPawnValue()[ isBlackTurn()? 2 : 0 ] >= _eParm._pieceValue[whiteKnights][0];
	}
	
	bitMap _CastlePathOccupancyBitmap( const eCastle c ) const;
	bool isCastlePathFree( const eCastle c ) const;
	
	bitMap getCastleKingPath(const eCastle c ) const;
	tSquare getCastleRookInvolved(const eCastle c ) const;

	const Movegen& getMoveGen() const
	{
		return _mg;
	}
	
	bool isOppositeBishops() const
	{
		return 
			(getPieceCount(whiteBishops) == 1)
			&& (getPieceCount(blackBishops) == 1)
			&& (getSquareColor(getSquareOfThePiece(blackBishops)) != getSquareColor(getSquareOfThePiece(whiteBishops)));
	}

	bool isChess960() const {return _isChess960;}

	simdScore getPieceValue(const bitboardIndex b) const { return _eParm._pieceValue[b];}
	std::unique_ptr<NNUE>& nnue() { return _nnue; };

	//--------------------------------------------------------
	// public members
	//--------------------------------------------------------
private:


	Position& operator=(Position&& ) noexcept = delete;
	Position(Position&& ) noexcept = delete;

	//--------------------------------------------------------
	// private structPosition
	//--------------------------------------------------------
	struct materialStruct
	{
		using tType = enum class type
		{
			exact,
			multiplicativeFunction,
			exactFunction,
			saturationH,
			saturationL,
		};
		tType type;
		bool (Position::*pointer)(Score &) const;
		Score val;

	};
	
	//--------------------------------------------------------
	// private static members
	//--------------------------------------------------------	
	
	/*! \brief helper mask used to speedup castle right management
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	std::unordered_map<tKey, materialStruct> static _materialKeyMap;
	
	
	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------	
	std::array<eCastle, squareNumber> _castleRightsMask;
	
	std::array<bitMap ,9> _castlePath;				// path that need to be free to be able to castle
	std::array<bitMap, 9> _castleKingPath;			// path to be traversed by the king when castling
	std::array<tSquare ,9> _castleRookInvolved;		// rook involved in the castling
	std::array<tSquare ,9> _castleKingFinalSquare;	// king destination square of castling
	std::array<tSquare ,9> _castleRookFinalSquare;	// rook destination square of castling
	
	unsigned int _ply;
	const Movegen _mg;


	/*used for search*/
	mutable std::unique_ptr<pawnTable> _pawnHashTable;

	std::vector<state> _stateInfo;

	/*! \brief board rapresentation
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	std::array<bitboardIndex,squareNumber> _squares;	// board square rapresentation to speed up, it contain pieces indexed by square
	std::array<bitMap,lastBitboard> _bitBoard;			// bitboards indexed by bitboardIndex enum
	bitMap *Us, *Them;	/*!< pointer to our & their pieces _bitBoard*/
	bool _isChess960;
	EvalParameters _eParm;

	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------	

	inline void _insertState( state & s );
	inline void _removeState();

	void _updateUsThem();


	HashKey _calcKey(void) const;
	HashKey _calcPawnKey(void) const;
	HashKey _calcMaterialKey(void) const;
	simdScore _calcMaterialValue(void) const;
	simdScore _calcNonPawnMaterialValue(void) const;
#ifdef	ENABLE_CHECK_CONSISTENCY
	enum class checkPhase{
	setup,
	doMove,
	doNullMove,
	undoMove,
	undoNullMove
};
	void _checkPosConsistency(checkPhase nn) const;
	void _block( const std::string& errorString, checkPhase type ) const;
#endif
	void _clear();
	inline void _calcCheckingSquares(void);
	template<bool our>
	bitMap _getHiddenCheckers() const;

	void _putPiece(const bitboardIndex piece, const tSquare s);
	void _movePiece(const bitboardIndex piece, const tSquare from, const tSquare to);
	void _removePiece(const bitboardIndex piece, const tSquare s);


	template<Color c> simdScore _evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const;
	template<Color c> simdScore _evalPassedPawn(bitMap pp, bitMap * attackedSquares) const;
	template<bitboardIndex piece> simdScore _evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap const blockedPawns, bitMap * const kingRing, unsigned int * const kingAttackersCount, unsigned int * const kingAttackersWeight, unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const;

	template<Color c> Score _evalShieldStorm(tSquare ksq) const;
	template<Color c> simdScore _evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const;
	
	simdScore _calcPawnValues(bitMap& weakPawns, bitMap& passedPawns, bitMap * const attackedSquares , bitMap * const weakSquares, bitMap * const holes) const;

	const materialStruct* _getMaterialData() const;
	bool _evalKxvsK(Score& res) const;
	bool _evalKBPsvsK(Score& res) const;
	bool _evalKQvsKP(Score& res) const;
	bool _evalKRPvsKr(Score& res) const;
	bool _evalKBNvsK(Score& res) const;
	bool _evalKQvsK(Score& res) const;
	bool _evalKRvsK(Score& res) const;
	bool _kingsDirectOpposition() const;
	bool _evalKPvsK(Score& res) const;
	bool _evalKPsvsK(Score& res) const;
	bool _evalOppositeBishopEndgame(Score& res) const;
	bool _evalKRvsKm(Score& res) const;
	bool _evalKNNvsK(Score& res) const;
	bool _evalKNPvsK(Score& res) const;

	static std::string _printEpSquare( const state& st );
    
    std::unique_ptr<NNUE> _nnue;
};


#endif /* POSITION_H_ */
