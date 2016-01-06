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

#include <exception>
#include <vector>
#include <list>
#include <string>
#include "vajolet.h"
#include "data.h"
#include "vectorclass/vectorclass.h"
#include "hashKeys.h"
#include "move.h"
#include "io.h"
#include "tables.h"





//---------------------------------------------------
//	class
//---------------------------------------------------


class Position{
	public:
	//static unsigned long testPointCounter;
	Position(const Position& other)
	    : stateIndex(other.stateIndex) // calls the copy constructor of the age

	{
		for(int i=0;i<STATE_INFO_LENGTH;i++){
			stateInfo[i]=other.stateInfo[i];
		}
		for(int i=0;i<squareNumber;i++){
			squares[i]=other.squares[i];
			index[i]=other.index[i];
		}
		for(int i=0;i<lastBitboard;i++){
			bitBoard[i]=other.bitBoard[i];
			pieceCount[i]=other.pieceCount[i];
			for(int n=0;n<64;n++){
				pieceList[i][n] =other.pieceList[i][n];
			}
		}


		Us=&bitBoard[getActualState().nextMove];
		Them=&bitBoard[(blackTurn-getActualState().nextMove)];
	};

	Position& operator=(const Position& other)

	{
		if (this == &other)
			return *this;


		stateIndex = other.stateIndex;
		for(int i=0;i<STATE_INFO_LENGTH;i++){
			stateInfo[i]=other.stateInfo[i];
		}
		for(int i=0;i<squareNumber;i++){
			squares[i]=other.squares[i];
			index[i]=other.index[i];
		}
		for(int i=0;i<lastBitboard;i++){
			bitBoard[i]=other.bitBoard[i];
			pieceCount[i]=other.pieceCount[i];
			for(int n=0;n<64;n++){
				pieceList[i][n] =other.pieceList[i][n];
			}
		}




		Us=&bitBoard[getActualState().nextMove];
		Them=&bitBoard[(blackTurn-getActualState().nextMove)];

		return *this;
	};




	/*! \brief define the index of the bitboards
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	enum bitboardIndex{
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



	enum eNextMove{					// color turn. ( it's also used as offset to access bitmaps by index)
		whiteTurn,
		blackTurn=blackKing-whiteKing
	};

	enum eCastle{
		wCastleOO=1,
		wCastleOOO=2,
		bCastleOO=4,
		bCastleOOO=8,
	};					// castleRights


	/*! \brief define the state of the board
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	struct state{
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
		//bitMap *Us,*Them;	/*!< pointer to our & their pieces bitboard*/
		//Move killers[2];	/*!< killer move at ply x*/
		bool skipNullMove;
		Move excludedMove;
		Move currentMove;

		/*state(){}
		state(const state& other){
			key=other.key;
			pawnKey=other.pawnKey;
			materialKey=other.materialKey;
			nonPawnMaterial[0]=other.nonPawnMaterial[0];
			nonPawnMaterial[1]=other.nonPawnMaterial[1];
			nonPawnMaterial[2]=other.nonPawnMaterial[2];
			nonPawnMaterial[3]=other.nonPawnMaterial[3];
			nextMove=other.nextMove;
			castleRights=other.castleRights;
			epSquare=other.epSquare;
			fiftyMoveCnt=other.fiftyMoveCnt;
			pliesFromNull=other.fiftyMoveCnt;
			ply=other.ply;
			capturedPiece=other.capturedPiece;
			material[0]=other.material[0];
			material[1]=other.material[1];

			for(int i=0;i<lastBitboard;i++){
				checkingSquares[i]=other.checkingSquares[i];
			}
			hiddenCheckersCandidate=other.hiddenCheckersCandidate;
			pinnedPieces=other.pinnedPieces;
			checkers=other.checkers;
			killers[0]=other.killers[0];
			killers[1]=other.killers[1];
			skipNullMove=other.skipNullMove;
			excludedMove=other.excludedMove;
			currentMove=other.currentMove;
		}*/

	};

	/*! \brief helper mask used to speedup castle right management
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	static int castleRightsMask[squareNumber];
public:
	unsigned int getStateIndex(void){ return stateIndex;}
	/*! \brief array of char to create the fen string
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	static const char PIECE_NAMES_FEN[lastBitboard];
private:

	unsigned int stateIndex;


	/*! \brief piece values used to calculate scores
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
public:
	static simdScore pieceValue[lastBitboard];
	static simdScore pstValue[lastBitboard][squareNumber];
	static simdScore nonPawnValue[lastBitboard];
public:


	static void initScoreValues(void);
	static void initPstValues(void);
	void display(void) const;
	std::string displayFen(void) const;
	std::string getSymmetricFen() const;
	void doNullMove(void);
	void doMove(const Move &m);
	void undoMove(const Move &m);
	static void initCastleRightsMask(void);
	void setupFromFen(const std::string& fenStr);
	template<bool trace>Score eval(void);
	unsigned long long perft(unsigned int depth);
	unsigned long long divide(unsigned int depth);
	bool moveGivesCheck(const Move& m)const ;
	bool moveGivesDoubleCheck(const Move& m)const;
	bool moveGivesSafeDoubleCheck(const Move& m)const;
	Score see(const Move& m) const;
	Score seeSign(const Move& m) const;
	bool isDraw(bool isPVline) const;

	/*! \brief constructor
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	Position()
	{
		stateIndex=0;
	}

	/*! \brief tell if the piece is a pawn
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static char isPawn(bitboardIndex piece) {
		return (piece&7)==6;
	}
	/*! \brief tell if the piece is a king
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static char isKing(bitboardIndex piece){
		return (piece&7)==1;
	}
	/*! \brief tell if the piece is a queen
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static char isQueen(bitboardIndex piece){
		return (piece&7)==2;
	}
	/*! \brief tell if the piece is a rook
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static char isRook(bitboardIndex piece){
		return (piece&7)==3;
	}
	/*! \brief tell if the piece is a bishop
		\author Marco Belli
		\version 1.0
		\date 04/11/2013
	*/
	inline static char isBishop(bitboardIndex piece){
		return (piece&7)==4;
	}
	/*! \brief tell the color of a piece
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline static char isblack(bitboardIndex piece){
		return piece&8;
	}


	U64 getKey(void){
		return getActualState().key;
	}

	U64 getExclusionKey(void){
		return getActualState().key^HashKeys::exclusion;
	}

	/*! \brief undo a null move
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline void undoNullMove(void){
		removeState();
		std::swap(Us,Them);

#ifdef ENABLE_CHECK_CONSISTENCY
		checkPosConsistency(0);
#endif
	}
	/*! \brief return a reference to the actual state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline state& getActualState(void)const {
		//assert(stateIndex>=0);
		assert(stateIndex<STATE_INFO_LENGTH);
		return (state&) stateInfo[stateIndex];
	}

	inline state& getState(unsigned int n)const {
			//assert(stateIndex>=0);
			assert(stateIndex<STATE_INFO_LENGTH);
			return (state&) stateInfo[n];
		}

	/*! \brief insert a new state in memory
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void insertState(state & s){


		stateIndex++;
		assert(stateIndex<STATE_INFO_LENGTH);
		//Move killer0;
		//Move killer1;
		//killer0=stateInfo[stateIndex].killers[0];
		//killer1=stateInfo[stateIndex].killers[1];
		stateInfo[stateIndex]=s;
		//stateInfo[stateIndex].killers[0]=killer0;
		//stateInfo[stateIndex].killers[1]=killer1;

	}

	/*! \brief  remove the last state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void removeState(){
		//assert(stateIndex>=0);

		stateIndex--;
		assert(stateIndex<STATE_INFO_LENGTH);
	}


	/*! \brief return the uci string for a given move
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	static std::string displayUci(const Move & m){


		std::string s;

		if(m.packed==0){
			s="0000";
			return s;
		}

		//from
		s+=char('a'+FILES[m.bit.from]);
		s+=char('1'+RANKS[m.bit.from]);


		//to
		s+=char('a'+FILES[m.bit.to]);
		s+=char('1'+RANKS[m.bit.to]);
		//promotion
		if(m.bit.flags == Move::fpromotion){
			s += Position::PIECE_NAMES_FEN[m.bit.promotion+Position::blackQueens];
		}
		return s;

	}

	/*! \brief return the uci string for a given move
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	std::string displayMove(Move & m)const {

		std::string s;
		//state st=getActualState();

		bool capture = (bitSet((tSquare)m.bit.to) & Them[Pieces]);
		if(!isPawn(squares[m.bit.from])){
			s+=PIECE_NAMES_FEN[squares[m.bit.from]%Position::separationBitmap];
		}
		if(capture && isPawn(squares[m.bit.from])){
			s+=char('a'+FILES[m.bit.from]);
		}
		if(capture){
			s+="x";
		}




		//to
		s+=char('a'+FILES[m.bit.to]);
		s+=char('1'+RANKS[m.bit.to]);
		if(moveGivesCheck(m)){
			s+="+";
		}
		//promotion
		if(m.bit.flags == Move::fpromotion){
			s +="=";
			s += Position::PIECE_NAMES_FEN[m.bit.promotion+Position::whiteQueens];
		}
		return s;

	}


	/*! \brief return a bitmap with all the attacker/defender of a given square
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline bitMap getAttackersTo(const tSquare to) const {
		return getAttackersTo(to, bitBoard[occupiedSquares]);
	}

	bitMap getAttackersTo(const tSquare to, const bitMap occupancy) const;


	/*! \brief return the mvvlva score
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline Score getMvvLvaScore(const Move & m) const {
		Score s=pieceValue[squares[m.bit.to]][0]-(squares[m.bit.from]);
		if (m.bit.flags == Move::fpromotion){
			s += (pieceValue[whiteQueens +m.bit.promotion] - pieceValue[whitePawns])[0];
		}else if(m.bit.flags == Move::fenpassant){
			s +=pieceValue[whitePawns][0];
		}
		return s;
	}

	/*! \brief return the gamephase for interpolation
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline unsigned int getGamePhase() const{
		const int opening = 570000;
		const int endgame = 150000;
		Score tot=getActualState().nonPawnMaterial[0]+getActualState().nonPawnMaterial[2];
		if(tot>opening){ //opening
			return 0;

		}
		if(tot<endgame){	//endgame
			return 65536;

		}
		return (unsigned int)((float)(opening-tot)*(65536.0f/(float)(opening-endgame)));
	}



	inline bool isCaptureMove(const Move & m) const {
		return squares[m.bit.to]!=empty || m.bit.flags==Move::fenpassant;
	}
	inline bool isCastleMove(const Move & m) const {
		return  m.bit.flags==Move::fcastle;
	}
	inline bool isCaptureMoveOrPromotion(const Move & m) const {
		return squares[m.bit.to]!=empty || m.bit.flags==Move::fenpassant || m.bit.flags == Move::fpromotion;
	}
	inline bool isPassedPawnMove(const Move & m) const {
		if(isPawn(squares[m.bit.to])){
			bool color=squares[m.bit.to]>=separationBitmap;
			bitMap theirPawns=color? bitBoard[whitePawns]:bitBoard[blackPawns];
			return !(theirPawns & PASSED_PAWN[color][m.bit.from]);
		}
		return false;
	}
	void cleanStateInfo(){
		for( auto& s:stateInfo){
			//s.killers[0].packed=0;
			//s.killers[1].packed=0;
			s.skipNullMove=true;
			s.excludedMove=0;
			//s.currentMove=0;
		}
	}

	const Move&  getKillers(const int n) const { return killers[stateIndex][n];}

	void saveKillers(Move& m)
	{
		if(killers[stateIndex][0] != m)
		{
			killers[stateIndex][1] = killers[stateIndex][0];
			killers[stateIndex][0] = m;
		}

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
	bitMap getHiddenCheckers(tSquare kingSquare,eNextMove next);

	pawnTable pawnHashTable;



	/*! \brief list of the past states, this is the history of the position
	  	the last element is the actual state
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	state stateInfo[STATE_INFO_LENGTH];

	Move killers[STATE_INFO_LENGTH][2];

	/*! \brief put a piece on the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void putPiece(bitboardIndex piece,tSquare s) {
		assert(s<squareNumber);
		assert(piece<lastBitboard);
		bitboardIndex color=piece>separationBitmap? blackPieces:whitePieces;
		bitMap b=bitSet(s);

		assert(squares[s]==empty);
		squares[s] = piece;
		bitBoard[piece] |= b;
		bitBoard[occupiedSquares] |= b;
		bitBoard[color]|= b;
		index[s] = pieceCount[piece]++;
		pieceList[piece][index[s]] = s;
	}

	/*! \brief move a piece on the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void movePiece(bitboardIndex piece, tSquare from, tSquare to) {
		assert(from<squareNumber);
		assert(to<squareNumber);
		assert(piece<lastBitboard);
		// index[from] is not updated and becomes stale. This works as long
		// as index[] is accessed just by known occupied squares.
		assert(squares[from]!=empty);
		assert(squares[to]==empty);
		bitMap fromTo = bitSet(from) ^ bitSet(to);
		bitboardIndex color=piece>separationBitmap? blackPieces:whitePieces;
		bitBoard[occupiedSquares] ^= fromTo;
		bitBoard[piece] ^= fromTo;
		bitBoard[color] ^= fromTo;
		squares[from] = empty;
		squares[to] = piece;
		index[to] = index[from];
		pieceList[piece][index[to]] = to;


	}
	/*! \brief remove a piece from the board
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	inline void removePiece( bitboardIndex piece, tSquare s) {

		assert(!isKing(piece));
		assert(s<squareNumber);
		assert(piece<lastBitboard);
		assert(squares[s]!=empty);

		// WARNING: This is not a reversible operation. If we remove a piece in
		// do_move() and then replace it in undo_move() we will put it at the end of
		// the list and not in its original place, it means index[] and pieceList[]
		// are not guaranteed to be invariant to a do_move() + undo_move() sequence.
		bitboardIndex color=piece>separationBitmap? blackPieces:whitePieces;
		bitMap b=bitSet(s);
		bitBoard[occupiedSquares]^= b;
		bitBoard[piece] ^= b;
		bitBoard[color] ^= b;
		squares[s] = empty;
		tSquare lastSquare = pieceList[piece][--pieceCount[piece]];
		index[lastSquare] = index[s];
		pieceList[piece][index[lastSquare]] = lastSquare;
		pieceList[piece][pieceCount[piece]] = squareNone;
	}



public:
	bool isMoveLegal(const Move &m) const;

	/*! \brief board rapresentation
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	bitboardIndex squares[squareNumber];		// board square rapresentation to speed up, it contain pieces indexed by square
	bitMap bitBoard[lastBitboard];			// bitboards indexed by bitboardIndex enum
	unsigned int pieceCount[lastBitboard];	// number of pieces indexed by bitboardIndex enum
	tSquare pieceList[lastBitboard][64];	// lista di pezzi indicizzata per tipo di pezzo e numero ( puo contentere al massimo 64 pezzi di ogni tipo)
	unsigned int index[squareNumber];		// indice del pezzo all'interno della sua lista
	bitMap *Us,*Them;	/*!< pointer to our & their pieces bitboard*/

};

extern simdScore initialPieceValue[Position::lastBitboard];
extern simdScore PawnD3;
extern simdScore PawnD4;
extern simdScore PawnD5;
extern simdScore PawnE3;
extern simdScore PawnE4;
extern simdScore PawnE5;
extern simdScore PawnCentering;
extern simdScore PawnRankBonus;
extern simdScore KnightPST;
extern simdScore BishopPST;
extern simdScore RookPST;
extern simdScore QueenPST;
extern simdScore KingPST;

extern simdScore BishopBackRankOpening;
extern simdScore KnightBackRankOpening;
extern simdScore RookBackRankOpening;
extern simdScore QueenBackRankOpening;
extern simdScore BishopOnBigDiagonals;

#endif /* POSITION_H_ */
