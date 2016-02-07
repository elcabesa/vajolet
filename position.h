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

#include <unordered_map>
#include <map>
#include "vajolet.h"
#include "data.h"
#include "vectorclass/vectorclass.h"
#include "hashKeys.h"
#include "move.h"
#include "io.h"
#include "tables.h"




enum color
{
	white = 0,
	black = 1
};

//---------------------------------------------------
//	class
//---------------------------------------------------



class Position
{
	struct materialStruct
	{
		enum
		{
			exact,
			multiplicativeFunction,
			exactFunction,
			saturationH,
			saturationL
		} type ;
		bool (Position::*pointer)(Score &);
		Score val;

	};
public:
	void static initMaterialKeys(void);
	static const int maxNumberOfPieces = 10;

	/*! \brief define the index of the bitboards
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	enum bitboardIndex
	{
		occupiedSquares=0,				//0		00000000
		whiteKing=1,					//1		00000001
		whiteQueens=2,					//2		00000010
		whiteRooks=3,					//3		00000011
		whiteBishops=4,					//4		00000100
		whiteKnights=5,					//5		00000101
		whitePawns=6,					//6		00000110
		whitePieces=7,					//7		00000111

		separationBitmap=8,
		blackKing=9,					//9		00001001
		blackQueens=10,					//10	00001010
		blackRooks=11,					//11	00001011
		blackBishops=12,				//12	00001100
		blackKnights=13,				//13	00001101
		blackPawns=14,					//14	00001110
		blackPieces=15,					//15	00001111




		lastBitboard=16,

		King=whiteKing,
		Queens,
		Rooks,
		Bishops,
		Knights,
		Pawns,
		Pieces,

		empty=occupiedSquares

	};

	enum eNextMove	// color turn. ( it's also used as offset to access bitmaps by index)
	{
		whiteTurn = 0,
		blackTurn=blackKing-whiteKing
	};

	enum eCastle	// castleRights
	{
		wCastleOO=1,
		wCastleOOO=2,
		bCastleOO=4,
		bCastleOOO=8,
	};


	/*! \brief constructor
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	Position()
	{
		stateIndex = 0;
		actualState = &stateInfo[stateIndex];
	}


	Position(const Position& other) : stateIndex(other.stateIndex) // calls the copy constructor of the age
	{

		for(unsigned int i = 0; i <= stateIndex; i++)
		{
			stateInfo[i] = other.stateInfo[i];
		}
		for(int i=0; i<squareNumber; i++)
		{
			squares[i] = other.squares[i];
			index[i] = other.index[i];
		}
		for(int i = 0; i < lastBitboard; i++)
		{
			bitBoard[i] = other.bitBoard[i];
			pieceCount[i] = other.pieceCount[i];
			for(int n = 0; n < maxNumberOfPieces; n++)
			{
				pieceList[i][n] =other.pieceList[i][n];
			}
		}


		actualState = &stateInfo[stateIndex];

		Us=&bitBoard[ getNextTurn() ];
		Them=&bitBoard[blackTurn - getNextTurn()];
	};

	Position& operator=(const Position& other)
	{
		if (this == &other)
			return *this;

		stateIndex = other.stateIndex;

		for(unsigned int i=0; i<=stateIndex; i++)
		{
			stateInfo[i] = other.stateInfo[i];
		}

		for(int i = 0; i < squareNumber; i++)
		{
			squares[i] = other.squares[i];
			index[i] = other.index[i];
		}
		for(int i=0;i<lastBitboard;i++)
		{
			bitBoard[i] = other.bitBoard[i];
			pieceCount[i] = other.pieceCount[i];
			for(int n = 0; n < maxNumberOfPieces; n++)
			{
				pieceList[i][n] = other.pieceList[i][n];
			}
		}


		actualState = &stateInfo[stateIndex];

		Us = &bitBoard[ getNextTurn() ];
		Them = &bitBoard[blackTurn-getNextTurn()];

		return *this;
	};


	/*! \brief define the state of the board
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	struct state
	{
		U64 key,		/*!<  hashkey identifying the position*/
			pawnKey,	/*!<  hashkey identifying the pawn formation*/
			materialKey;/*!<  hashkey identifying the material signature*/

		Score nonPawnMaterial[4]; /*!< four score used for white/black opening/endgame non pawn material sum*/

		eNextMove nextMove; /*!< who is the active player*/
		eCastle castleRights; /*!<  actual castle rights*/
		tSquare epSquare;	/*!<  en passant square*/

		unsigned int fiftyMoveCnt,	/*!<  50 move count used for draw rule*/
			pliesFromNull,	/*!<  plies from null move*/
			ply;			/*!<  ply from the start*/

		bitboardIndex capturedPiece; /*!<  index of the captured piece for unmakeMove*/
		Score material[2];	/*!<  two values for opening/endgame score*/

		bitMap checkingSquares[lastBitboard]; /*!< squares of the board from where a king can be checked*/
		bitMap hiddenCheckersCandidate;	/*!< pieces who can make a discover check moving*/
		bitMap pinnedPieces;	/*!< pinned pieces*/
		bitMap checkers;	/*!< checking pieces*/
		Move currentMove;

		state()
		{
		}


	};

private:

	/*! \brief helper mask used to speedup castle right management
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	static int castleRightsMask[squareNumber];

	static simdScore pstValue[lastBitboard][squareNumber];
	static simdScore nonPawnValue[lastBitboard];


	/*used for search*/
	pawnTable pawnHashTable;

	/*data defining the position*/
	state * actualState;
	unsigned int stateIndex;
	/*! \brief list of the past states, this is the history of the position
		the last element is the actual state
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	state stateInfo[STATE_INFO_LENGTH];

	/*! \brief board rapresentation
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	bitboardIndex squares[squareNumber];		// board square rapresentation to speed up, it contain pieces indexed by square
	bitMap bitBoard[lastBitboard];			// bitboards indexed by bitboardIndex enum
	unsigned int pieceCount[lastBitboard];	// number of pieces indexed by bitboardIndex enum
	tSquare pieceList[lastBitboard][maxNumberOfPieces];	// lista di pezzi indicizzata per tipo di pezzo e numero ( puo contentere al massimo 64 pezzi di ogni tipo)
	unsigned int index[squareNumber];		// indice del pezzo all'interno della sua lista
	bitMap *Us,*Them;	/*!< pointer to our & their pieces bitboard*/








public:
	inline bitMap getOccupationBitmap() const
	{
		return bitBoard[occupiedSquares];
	}
	inline bitMap getBitmap(const bitboardIndex index) const
	{
		return bitBoard[index];
	}
	inline unsigned int getpieceCount(const bitboardIndex index) const
	{
		return pieceCount[index];
	}

	inline bitboardIndex getPieceAt(const tSquare sq) const
	{
		return squares[sq];
	}
	inline tSquare getSquareOfThePiece(const bitboardIndex piece,const unsigned int n = 0) const
	{
		return pieceList[piece][n];
	}
	inline bitMap getOurBitmap(const bitboardIndex piece)const { return Us[piece];}
	inline bitMap getTheirBitmap(const bitboardIndex piece)const { return Them[piece];}


	unsigned int getStateIndex(void)const { return stateIndex;}

	/*! \brief piece values used to calculate scores
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/


	static void initCastleRightsMask(void);
	static void initScoreValues(void);
	static void initPstValues(void);
	static simdScore pieceValue[lastBitboard];



	void display(void) const;
	std::string getFen(void) const;
	std::string getSymmetricFen() const;

	void setupFromFen(const std::string& fenStr);

	unsigned long long perft(unsigned int depth);
	unsigned long long divide(unsigned int depth);

	void doNullMove(void);
	void doMove(const Move &m);
	void undoMove();
	/*! \brief undo a null move
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline void undoNullMove(void)
	{
		removeState();
		std::swap( Us , Them );

#ifdef ENABLE_CHECK_CONSISTENCY
		checkPosConsistency(0);
#endif
	}


	template<bool trace>Score eval(void);
	bool isDraw(bool isPVline) const;


	bool moveGivesCheck(const Move& m)const ;
	bool moveGivesDoubleCheck(const Move& m)const;
	bool moveGivesSafeDoubleCheck(const Move& m)const;
	Score see(const Move& m) const;
	Score seeSign(const Move& m) const;




	/*! \brief tell if the piece is a pawn
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static bool isPawn(bitboardIndex piece)
	{
		return (piece&7) == Pawns;
	}
	/*! \brief tell if the piece is a king
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static bool isKing(bitboardIndex piece)
	{
		return (piece&7) == King;
	}
	/*! \brief tell if the piece is a queen
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static bool isQueen(bitboardIndex piece)
	{
		return (piece&7) == Queens;
	}
	/*! \brief tell if the piece is a rook
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static bool isRook(bitboardIndex piece)
	{
		return (piece&7) == Rooks;
	}
	/*! \brief tell if the piece is a bishop
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static bool isBishop(bitboardIndex piece){
		return (piece&7) == Bishops;
	}
	/*! \brief tell the color of a piece
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static bool isblack(bitboardIndex piece)
	{
		return piece & 8;
	}


	U64 getKey(void) const
	{
		return getActualState().key;
	}

	U64 getExclusionKey(void) const
	{
		return getActualState().key^HashKeys::exclusion;
	}

	U64 getPawnKey(void) const
	{
		return getActualState().pawnKey;
	}

	U64 getMaterialKey(void) const
	{
		return getActualState().materialKey;
	}

	eNextMove getNextTurn(void) const
	{
		return getActualState().nextMove;
	}

	tSquare getEpSquare(void) const
	{
		return getActualState().epSquare;
	}

	eCastle getCastleRights(void) const
	{
		return getActualState().castleRights;
	}

	unsigned int getPly(void) const
	{
		return getActualState().ply;
	}

	bitboardIndex getCapturedPiece(void) const
	{
		return getActualState().capturedPiece;
	}

	bool isInCheck(void) const
	{
		return getActualState().checkers;
	}





	/*! \brief return a reference to the actual state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline state& getActualState(void)const
	{
		//assert(stateIndex>=0);
		assert(stateIndex<STATE_INFO_LENGTH);
		return (state&) *actualState;/*stateInfo[stateIndex];*/
	}

	inline state& getState(unsigned int n)const
	{
		//assert(stateIndex>=0);
		assert(stateIndex<STATE_INFO_LENGTH);
		return (state&) stateInfo[n];
	}

private:

	/*! \brief insert a new state in memory
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void insertState(state & s)
	{
		assert(stateIndex+1<STATE_INFO_LENGTH);

		stateInfo[++stateIndex] = s;
		actualState = &stateInfo[stateIndex];
	}

	/*! \brief  remove the last state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void removeState()
	{
		//assert(stateIndex>=0);

		assert(stateIndex<STATE_INFO_LENGTH);
		actualState = &stateInfo[--stateIndex];
	}






public:

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
		Score s = pieceValue[ squares[m.bit.to] ][0] - (squares[m.bit.from]);
		if (m.bit.flags == Move::fpromotion)
		{
			s += (pieceValue[ whiteQueens +m.bit.promotion ] - pieceValue[whitePawns])[0];
		}
		else if(m.bit.flags == Move::fenpassant)
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
	inline unsigned int getGamePhase() const
	{
		const int opening = 570000;
		const int endgame = 150000;
		Score tot = getActualState().nonPawnMaterial[0]+getActualState().nonPawnMaterial[2];
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
		return squares[m.bit.to] !=empty || m.bit.flags == Move::fenpassant;
	}
	inline bool isCastleMove(const Move & m) const
	{
		return  m.bit.flags == Move::fcastle;
	}
	inline bool isCaptureMoveOrPromotion(const Move & m) const
	{
		return squares[m.bit.to] != empty || m.bit.flags == Move::fenpassant || m.bit.flags == Move::fpromotion;
	}
	inline bool isPassedPawnMove(const Move & m) const
	{
		if(isPawn(squares[m.bit.to]))
		{
			bool color = squares[m.bit.to] >= separationBitmap;
			bitMap theirPawns = color? bitBoard[whitePawns]:bitBoard[blackPawns];
			return !(theirPawns & PASSED_PAWN[color][m.bit.from]);
		}
		return false;
	}

private:

	U64 calcKey(void) const;
	U64 calcPawnKey(void) const;
	U64 calcMaterialKey(void) const;
	simdScore calcMaterialValue(void) const;
	void calcNonPawnMaterialValue(Score* s);
	bool checkPosConsistency(int nn);
	void clear();
	inline void calcCheckingSquares(void);
	bitMap getHiddenCheckers(tSquare kingSquare,eNextMove next) const;







	/*! \brief put a piece on the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void putPiece(const bitboardIndex piece,const tSquare s)
	{
		assert(s<squareNumber);
		assert(piece<lastBitboard);
		bitboardIndex color = piece > separationBitmap ? blackPieces : whitePieces;
		bitMap b=bitSet(s);

		assert(squares[s]==empty);

		squares[s] = piece;
		bitBoard[piece] |= b;
		bitBoard[occupiedSquares] |= b;
		bitBoard[color] |= b;
		unsigned int temp = index[s] = pieceCount[piece]++;
		assert(index[s]<maxNumberOfPieces);
		pieceList[piece][temp] = s;
	}

	/*! \brief move a piece on the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void movePiece(const bitboardIndex piece,const tSquare from,const tSquare to)
	{
		assert(from<squareNumber);
		assert(to<squareNumber);
		assert(piece<lastBitboard);
		// index[from] is not updated and becomes stale. This works as long
		// as index[] is accessed just by known occupied squares.
		assert(squares[from]!=empty);
		assert(squares[to]==empty);
		bitMap fromTo = bitSet(from) ^ bitSet(to);
		bitboardIndex color = piece > separationBitmap ? blackPieces : whitePieces;
		bitBoard[occupiedSquares] ^= fromTo;
		bitBoard[piece] ^= fromTo;
		bitBoard[color] ^= fromTo;
		squares[from] = empty;
		squares[to] = piece;
		index[to] = index[from];
		assert(index[s]<maxNumberOfPieces);
		pieceList[piece][index[to]] = to;


	}
	/*! \brief remove a piece from the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void removePiece(const bitboardIndex piece,const tSquare s)
	{

		assert(!isKing(piece));
		assert(s<squareNumber);
		assert(piece<lastBitboard);
		assert(squares[s]!=empty);

		// WARNING: This is not a reversible operation. If we remove a piece in
		// do_move() and then replace it in undo_move() we will put it at the end of
		// the list and not in its original place, it means index[] and pieceList[]
		// are not guaranteed to be invariant to a do_move() + undo_move() sequence.
		bitboardIndex color = piece  > separationBitmap ? blackPieces : whitePieces;
		bitMap b = bitSet(s);
		bitBoard[occupiedSquares] ^= b;
		bitBoard[piece] ^= b;
		bitBoard[color] ^= b;

		squares[s] = empty;

		const tSquare lastSquare = pieceList[piece][--pieceCount[piece]];
		unsigned int temp = index[lastSquare] = index[s];
		pieceList[piece][temp] = lastSquare;
		pieceList[piece][pieceCount[piece]] = squareNone;
	}

	template<color c> simdScore evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const;
	template<color c> simdScore evalPassedPawn(bitMap pp, bitMap * attackedSquares) const;
	template<Position::bitboardIndex piece>	simdScore evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap const blockedPawns, bitMap * const kingRing, unsigned int * const kingAttackersCount, unsigned int * const kingAttackersWeight, unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const;

	template<color c> Score evalShieldStorm(tSquare ksq) const;
	template<color c> simdScore evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const;

	std::unordered_map<U64, materialStruct> static materialKeyMap;

	const materialStruct  * getMaterialData();
	bool evalKBPvsK(Score& res);
	bool evalKQvsKP(Score& res);
	bool evalKRPvsKr(Score& res);
	bool evalKBNvsK(Score& res);
	bool evalKQvsK(Score& res);
	bool kingsDirectOpposition();
	bool evalKPvsK(Score& res);
	bool evalOppositeBishopEndgame(Score& res);
	bool evalKRvsKm(Score& res);
	bool evalKNNvsK(Score& res);






};

#endif /* POSITION_H_ */
