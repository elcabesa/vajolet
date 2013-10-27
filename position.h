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

#include <list>
#include <string>
#include "data.h"
#include "vectorclass/vectorclass.h"
#include "hashKeys.h"
#include "move.h"
#include "io.h"


//---------------------------------------------------
//	class
//---------------------------------------------------


class Position{
public:
	enum bitboardIndex{
		occupiedSquares=0,				//0		00000000
		whiteKing=1,					//1		00000001
		whiteQueens=2,					//2		00000010
		whiteRooks=3,					//3		00000011
		whiteBishops=4,					//4		00000100
		whiteKnights=5,					//5		00000101
		whitePawns=6,					//6		00000110
		whitePieces=7,					//7		00000111

		emptyBitmap=8,
		blackKing=9,					//9		00001001
		blackQueens=10,					//10	00001010
		blackRooks=11,					//11	00001011
		blackBishops=12,				//12	00001100
		blackKnights=13,				//13	00001101
		blackPawns=14,					//14	00001110
		blackPieces=15,					//15	00001111


		lastBitboard=16,

		empty=occupiedSquares

	};

	enum eNextMove{					// color turn. ( it's also used as offset to access bitmaps by index)
		whiteTurn,
		blackTurn=blackKing-whiteKing
	};

	enum eCastle{
		wCastleOO=1,
		wCastleOOO=2,
		bCastleOO=4,
		bCastleOOO=8,
	};					// castleRight



	struct state{
		U64 key,pawnKey,materialKey;
		Score nonPawnMaterial[4];
		eNextMove nextMove;
		eCastle castleRights;


		tSquare epSquare;
		int fiftyMoveCnt,pliesFromNull,ply;
		bitboardIndex capturedPiece;
		Score material[2];

	};



	void init(void);
	static void initScoreValues(void);
	void display(void) const;
	void displayFen(void) const;

	Position(){
	}

	inline char isPawn(bitboardIndex piece) const {
		return (piece&7)==6;
	}
	inline char isKing(bitboardIndex piece) const {
			return (piece&7)==1;
		}
	inline char isblack(bitboardIndex piece) const {
		return piece&8;
	}

inline void putPiece(bitboardIndex piece,tSquare s) {
	bitboardIndex color=piece>emptyBitmap? blackPieces:whitePieces;
	board[s] = piece;
	bitBoard[piece] |= bitSet(s);
	bitBoard[occupiedSquares] |= bitSet(s);
	bitBoard[color]|= bitSet(s);
	index[s] = pieceCount[piece]++;
	pieceList[piece][index[s]] = s;
	}

inline void movePiece(bitboardIndex piece, tSquare from, tSquare to) {

	// index[from] is not updated and becomes stale. This works as long
	// as index[] is accessed just by known occupied squares.
	bitMap fromTo = bitSet(from) ^ bitSet(to);
	bitboardIndex color=piece>emptyBitmap? blackPieces:whitePieces;
	bitBoard[occupiedSquares] ^= fromTo;
	bitBoard[piece] ^= fromTo;
	bitBoard[color] ^= fromTo;
	board[from] = empty;
	board[to] = piece;
	index[to] = index[from];
	pieceList[piece][index[to]] = to;


}

inline void removePiece( bitboardIndex piece, tSquare s) {

	// WARNING: This is not a reversible operation. If we remove a piece in
	// do_move() and then replace it in undo_move() we will put it at the end of
	// the list and not in its original place, it means index[] and pieceList[]
	// are not guaranteed to be invariant to a do_move() + undo_move() sequence.
	bitboardIndex color=piece>emptyBitmap? blackPieces:whitePieces;
	bitBoard[occupiedSquares]^= bitSet(s);
	bitBoard[piece] ^= bitSet(s);
	bitBoard[color] ^= bitSet(s);
	board[s] = empty;
	tSquare lastSquare = pieceList[piece][--pieceCount[piece]];
	index[lastSquare] = index[s];
	pieceList[piece][index[lastSquare]] = lastSquare;
	pieceList[piece][pieceCount[piece]] = squareNone;
}


inline void undoNullMove(void){
	stateInfo.pop_back();
}

	static int castleRightsMask[squareNumber];
	void doNullMove(void);
	void doMove(Move m);
	void undoMove(Move m);
	static void initCastlaRightsMask(void);
	void setupFromFen(const std::string& fenStr);
	Score eval(void) const;

private:

	U64 calcKey(void) const;
	U64 calcPawnKey(void) const;
	U64 calcMaterialKey(void) const;
	simdScore calcMaterialValue(void) const;
	void calcNonPawnMaterialValue(Score* s);
	bool checkPosConsistency(void);
	void clear();

	std::list<state> stateInfo;

	const char PIECE_NAMES_FEN[lastBitboard]={' ','K','Q','R','B','N','P',' ',' ','k','q','r','b','n','p',' '};

	static simdScore pieceValue[lastBitboard];

	bitboardIndex board[squareNumber];		// board square rapresentation to speed up, it contain pieces indexed by square
	bitMap bitBoard[lastBitboard];			// bitboards indexed by bitboardIndex enum
	unsigned int pieceCount[lastBitboard];	// number of pieces indexed by bitboardIndex enum
	tSquare pieceList[lastBitboard][16];	// lista di pezzi indicizzata per tipo di pezzo e numero ( puo contentere al massimo 16 pezzi di ogni tipo)
	unsigned int index[squareNumber];		// indice del pezzo all'interno della sua lista


};


#endif /* POSITION_H_ */
