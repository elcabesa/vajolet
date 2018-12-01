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
#include <unordered_map>
#include <vector>

#include "bitBoardIndex.h"
#include "data.h"
#include "eCastle.h"
#include "hashKey.h"
#include "movegen.h"
#include "move.h"
#include "tables.h"
#include "score.h"
#include "state.h"
#include "vajolet.h"

//---------------------------------------------------
//	class
//---------------------------------------------------



class Position
{
public:

	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------
	void static initMaterialKeys(void);
	static void initCastleRightsMask(void);
	static void initScoreValues(void);
	static void initPstValues(void);
	
	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	
	/*! \brief constructor
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	Position();
	Position(const Position& other);
	Position& operator=(const Position& other);
	

	inline bitMap getOccupationBitmap() const
	{
		return bitBoard[occupiedSquares];
	}
	inline bitMap getBitmap(const bitboardIndex in) const
	{
		return bitBoard[in];
	}
	inline unsigned int getPieceCount(const bitboardIndex in) const
	{
		return bitCnt( getBitmap( in ) );
	}

	inline bitboardIndex getPieceAt(const tSquare sq) const
	{
		return squares[sq];
	}

	inline bitboardIndex getPieceTypeAt(const tSquare sq) const
	{
		return getPieceType( squares[sq] );
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
		return stateInfo.size();
	}
	
	unsigned int getNumberOfLegalMoves() const;
	
	void display(void) const;
	std::string getFen(void) const;
#ifdef DEBUG_EVAL_SIMMETRY
	std::string getSymmetricFen() const;
#endif

	void setupFromFen(const std::string& fenStr);
	void setup(const std::string& code, Color c);

	unsigned long long perft(unsigned int depth);
	unsigned long long divide(unsigned int depth);

	void doNullMove();
	void doMove(const Move &m);
	void undoMove();
	void undoNullMove();


	template<bool trace>Score eval(void);
	bool isDraw(bool isPVline) const;


	bool moveGivesCheck(const Move& m)const ;
	bool moveGivesDoubleCheck(const Move& m)const;
	bool moveGivesSafeDoubleCheck(const Move& m)const;
	Score see(const Move& m) const;
	Score seeSign(const Move& m) const;
	
	const HashKey& getKey(void) const
	{
		return getActualStateConst().getKey();
	}

	const HashKey getExclusionKey(void) const
	{
		return getActualStateConst().getKey().getExclusionKey();
	}

	const HashKey& getPawnKey(void) const
	{
		return getActualStateConst().getPawnKey();
	}

	const HashKey& getMaterialKey(void) const
	{
		return getActualStateConst().getMaterialKey();
	}

	eNextMove getNextTurn(void) const
	{
		return getActualStateConst().getNextTurn();
	}

	inline bool isBlackTurn() const
	{
		return getActualStateConst().isBlackTurn();
	}

	inline bool isWhiteTurn() const
	{
		return getActualStateConst().isWhiteTurn();
	}

	tSquare getEpSquare(void) const
	{
		return getActualStateConst().getEpSquare();
	}

	bool hasEpSquare(void) const
	{
		return getActualStateConst().hasEpSquare();
	}

	eCastle getCastleRights(void) const
	{
		return getActualStateConst().getCastleRights();
	}

	unsigned int getPly(void) const
	{
		return _ply;
	}

	bitboardIndex getCapturedPiece(void) const
	{
		return getActualStateConst().getCapturedPiece();
	}

	bool isInCheck(void) const
	{
		return getActualStateConst().getCheckers();
	}
	
	/*! \brief return a reference to the actual state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline const state& getActualStateConst(void)const
	{
		return stateInfo.back();
	}
	
	inline state& getActualState(void)
	{
		return stateInfo.back();
	}

	inline const state& getState(unsigned int n)const
	{
		return stateInfo[n];	
	}
	
	bool isMoveLegal(const Move &m) const;

	/*! \brief return a bitmap with all the attacker/defender of a given square
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline bitMap getAttackersTo(const tSquare to) const
	{
		return getAttackersTo(to, bitBoard[occupiedSquares]);
	}

	bitMap getAttackersTo(const tSquare to, const bitMap occupancy) const;


	/*! \brief return the mvvlva score
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline Score getMvvLvaScore(const Move & m) const
	{
		Score s = pieceValue[ getPieceAt( m.getTo() ) ][0] + getPieceAt( m.getFrom() );
		if( m.isEnPassantMove() )
		{
			s += pieceValue[ whitePawns ][0];
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
		return getPieceAt( m.getTo() ) !=empty || m.isEnPassantMove() ;
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
			bitMap theirPawns = color? bitBoard[whitePawns]:bitBoard[blackPawns];
			bitMap ourPawns = color? bitBoard[blackPawns]:bitBoard[whitePawns];
			return !(theirPawns & PASSED_PAWN[color][m.getFrom()]) && !(ourPawns & SQUARES_IN_FRONT_OF[color][m.getFrom()]);
		}
		return false;
	}
	
	//--------------------------------------------------------
	// public members
	//--------------------------------------------------------
	static bool perftUseHash;
	static simdScore pieceValue[lastBitboard];

	// todo private?
	Movegen mg;

private:


	Position& operator=(Position&& ) noexcept = delete;
	Position(Position&& ) noexcept = delete;

	//--------------------------------------------------------
	// private struct
	//--------------------------------------------------------
	struct materialStruct
	{
		using tType = enum
		{
			exact,
			multiplicativeFunction,
			exactFunction,
			saturationH,
			saturationL,
		};
		tType type;
		bool (Position::*pointer)(Score &);
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
	static eCastle castleRightsMask[squareNumber];
	static simdScore pstValue[lastBitboard][squareNumber];
	static simdScore nonPawnValue[lastBitboard];
	std::unordered_map<tKey, materialStruct> static materialKeyMap;
	
	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------	
	unsigned int _ply;


	/*used for search*/
	pawnTable pawnHashTable;

	std::vector<state> stateInfo;

	/*! \brief board rapresentation
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	std::array<bitboardIndex,squareNumber> squares;		// board square rapresentation to speed up, it contain pieces indexed by square
	std::array<bitMap,lastBitboard> bitBoard;			// bitboards indexed by bitboardIndex enum
	bitMap *Us,*Them;	/*!< pointer to our & their pieces bitboard*/

	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------

	inline void insertState( state & s );
	inline void removeState();

	void updateUsThem();


	HashKey calcKey(void) const;
	HashKey calcPawnKey(void) const;
	HashKey calcMaterialKey(void) const;
	simdScore calcMaterialValue(void) const;
	simdScore calcNonPawnMaterialValue(void) const;
#ifdef	ENABLE_CHECK_CONSISTENCY
	bool checkPosConsistency(int nn) const;
#endif
	void clear();
	inline void calcCheckingSquares(void);
	template<bool our>
	bitMap getHiddenCheckers() const;

	void putPiece(const bitboardIndex piece, const tSquare s);
	void movePiece(const bitboardIndex piece, const tSquare from, const tSquare to);
	void removePiece(const bitboardIndex piece, const tSquare s);


	template<Color c> simdScore evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const;
	template<Color c> simdScore evalPassedPawn(bitMap pp, bitMap * attackedSquares) const;
	template<bitboardIndex piece> simdScore evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap const blockedPawns, bitMap * const kingRing, unsigned int * const kingAttackersCount, unsigned int * const kingAttackersWeight, unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const;

	template<Color c> Score evalShieldStorm(tSquare ksq) const;
	template<Color c> simdScore evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const;

	const materialStruct* getMaterialData();
	bool evalKxvsK(Score& res);
	bool evalKBPsvsK(Score& res);
	bool evalKQvsKP(Score& res);
	bool evalKRPvsKr(Score& res);
	bool evalKBNvsK(Score& res);
	bool evalKQvsK(Score& res);
	bool evalKRvsK(Score& res);
	bool kingsDirectOpposition();
	bool evalKPvsK(Score& res);
	bool evalKPsvsK(Score& res);
	bool evalOppositeBishopEndgame(Score& res);
	bool evalKRvsKm(Score& res);
	bool evalKNNvsK(Score& res);
	bool evalKNPvsK(Score& res);

	static std::string _printEpSquare( const state& st );

};


#endif /* POSITION_H_ */
