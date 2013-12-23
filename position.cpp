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

#include <sstream>
#include <string>
#include "vajolet.h"
#include "position.h"
#include "data.h"
#include "bitops.h"
#include "io.h"
#include "hashKeys.h"
#include "move.h"
#include "movegen.h"
#include "transposition.h"



simdScore Position::pieceValue[lastBitboard];
simdScore Position::pstValue[lastBitboard][squareNumber];
simdScore Position::nonPawnValue[lastBitboard][squareNumber];
int Position::castleRightsMask[squareNumber];

/*! \brief setup a position from a fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::setupFromFen(const std::string& fenStr){
	char col,row,token;
	tSquare sq = A8;
	std::istringstream ss(fenStr);

	clear();
	ss >> std::noskipws;

	while ((ss >> token) && !std::isspace(token))
	{
		if (isdigit(token))
			sq += tSquare(token - '0'); // Advance the given number of files
		else if (token == '/')
			sq -= tSquare(16);
		else
		{
			switch (token)
			{
			case 'P':
				putPiece(whitePawns,sq);
				break;
			case 'N':
				putPiece(whiteKnights,sq);
				break;
			case 'B':
				putPiece(whiteBishops,sq);
				break;
			case 'R':
				putPiece(whiteRooks,sq);
				break;
			case 'Q':
				putPiece(whiteQueens,sq);
				break;
			case 'K':
				putPiece(whiteKing,sq);
				break;
			case 'p':
				putPiece(blackPawns,sq);
				break;
			case 'n':
				putPiece(blackKnights,sq);
				break;
			case 'b':
				putPiece(blackBishops,sq);
				break;
			case 'r':
				putPiece(blackRooks,sq);
				break;
			case 'q':
				putPiece(blackQueens,sq);
				break;
			case 'k':
				putPiece(blackKing,sq);
				break;
			}
			sq++;
		}
	}
	state s;

	insertState(s);
	state &x= getActualState();


	ss >> token;
	x.nextMove = (token == 'w' ? whiteTurn : blackTurn);
	x.Us=&bitBoard[x.nextMove];
	x.Them=&bitBoard[(blackTurn-x.nextMove)];
	ss >> token;

	x.castleRights=(eCastle)0;
	while ((ss >> token) && !isspace(token))
	{
		switch(token){
		case 'K':
			x.castleRights =(eCastle)(x.castleRights|wCastleOO);
			break;
		case 'Q':
			x.castleRights =(eCastle)(x.castleRights|wCastleOOO);
			break;
		case 'k':
			x.castleRights =(eCastle)(x.castleRights|bCastleOO);
			break;
		case 'q':
			x.castleRights =(eCastle)(x.castleRights|bCastleOOO);
			break;
		}
	}

	x.epSquare=squareNone;
	if (((ss >> col) && (col >= 'a' && col <= 'h'))
		&& ((ss >> row) && (row == '3' || row == '6')))
	{
		x.epSquare =(tSquare) (((int) col - 'a') + 8 * (row - '1')) ;
		if (!(getAttackersTo(x.epSquare) & bitBoard[whitePawns+x.nextMove]))
			x.epSquare = squareNone;
	}



	ss >> std::skipws >> x.fiftyMoveCnt;
	if(ss.eof()){
		x.ply=0;
		x.fiftyMoveCnt=0;

	}else{
		ss>> x.ply;
		x.ply = std::max(2 * (x.ply - 1), 0) + int(x.nextMove == blackTurn);
	}

	x.pliesFromNull=0;
	x.skipNullMove=true;
	x.excludedMove=0;
	x.currentMove=0;
	x.capturedPiece=empty;



	calcNonPawnMaterialValue(x.nonPawnMaterial);
	calcMaterialValue().store_partial(2,x.material);

	x.key=calcKey();
	x.pawnKey=calcPawnKey();
	x.materialKey=calcMaterialKey();

	calcCheckingSquares();

	x.hiddenCheckersCandidate=getHiddenCheckers(pieceList[(bitboardIndex)(blackKing-x.nextMove)][0],x.nextMove);
	x.pinnedPieces=getHiddenCheckers(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0],eNextMove(blackTurn-x.nextMove));
	x.checkers= getAttackersTo(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0]) & bitBoard[blackPieces-x.nextMove];



	checkPosConsistency(1);
}


/*! \brief init the score value in the static const
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initScoreValues(void){
	for(auto &val :pieceValue){
		val=0;
	}
	pieceValue[whitePawns]=10000;
	pieceValue[whiteKnights]=30000;
	pieceValue[whiteBishops]=32500;
	pieceValue[whiteRooks]=50000;
	pieceValue[whiteQueens]=90000;
	pieceValue[whiteKing]=3000000;

	pieceValue[blackPawns]=-pieceValue[whitePawns];
	pieceValue[blackKnights]=-pieceValue[whiteKnights];
	pieceValue[blackBishops]=-pieceValue[whiteBishops];
	pieceValue[blackRooks]=-pieceValue[whiteRooks];
	pieceValue[blackQueens]=-pieceValue[whiteQueens];
	pieceValue[blackKing]=-pieceValue[whiteKing];


	for(int piece=0;piece<lastBitboard;piece++){
		for(tSquare s=(tSquare)0;s<squareNumber;s++){
			assert(piece<lastBitboard);
			assert(s<squareNumber);
			nonPawnValue[piece][s]=0;
			pstValue[piece][s]=0;
			int rank=RANKS[s];
			int file=FILES[s];
			if(piece >occupiedSquares && piece <whitePieces ){
				pstValue[piece][s]=-10*(rank-3.5)*(rank-3.5)*(file-3.5)*(file-3.5);
				/*if(isPawn((bitboardIndex)piece)){
					pstValue[piece][s].insert(1,100*(rank-3.5));
				}*/

				if(!isKing((bitboardIndex)piece)){
					pstValue[piece][s]+=pieceValue[piece];
				}
				else{
					pstValue[piece][s].insert(0,-pstValue[piece][s][0]);
				}

				if(!isPawn((bitboardIndex)piece)){
					nonPawnValue[piece][s].insert(0,pstValue[piece][s][0]);
					nonPawnValue[piece][s].insert(1,pstValue[piece][s][1]);
				}

			}
			else if(piece >separationBitmap && piece <blackPieces ){
				pstValue[piece][s]=10*(rank-3.5)*(rank-3.5)*(file-3.5)*(file-3.5);
				/*if(isPawn((bitboardIndex)piece)){
					pstValue[piece][s].insert(1,-100*(rank-3.5));
				}*/

				if(!isKing((bitboardIndex)piece)){
					pstValue[piece][s]+=pieceValue[piece];
				}
				else{
					pstValue[piece][s].insert(0,-pstValue[piece][s][0]);
				}
				if(!isPawn((bitboardIndex)piece)){

					nonPawnValue[piece][s].insert(2,-pstValue[piece][s][0]);
					nonPawnValue[piece][s].insert(3,-pstValue[piece][s][1]);
				}

			}
			else{
				pstValue[piece][s]=0;
			}
		}
	}

}
/*! \brief clear a position and his history
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::clear() {
	for (tSquare i = square0; i < squareNumber; i++) {
		squares[i] = empty;
		index[i] = 0;
	};
	for (int i = 0; i < lastBitboard; i++) {
		pieceCount[i] = 0;
		bitBoard[i] = 0;
		for (int n = 0; n < 16; n++) {
			pieceList[i][n] = square0;
		}
	}
	stateIndex=0;
	actualState=nullptr;
	//stateInfo.clear();
}


/*	\brief display a board for debug purpose
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
void Position::display()const {
	displayFen();
	int rank, file;
	state& st =getActualState();
	sync_cout;
	{
		for (rank = 7; rank >= 0; rank--)
		{
			std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
			std::cout << rank+1 <<  " |";
			for (file = 0; file <= 7; file++)
			{
				std::cout << " " << PIECE_NAMES_FEN[squares[BOARDINDEX[file][rank]]] << " |";
			}
			std::cout << std::endl;
		}
		std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
		std::cout << "    a   b   c   d   e   f   g   h" << std::endl << std::endl;

	}
	std::cout <<(st.nextMove?"BLACK TO MOVE":"WHITE TO MOVE") <<std::endl;
	std::cout <<"50 move counter "<<st.fiftyMoveCnt<<std::endl;
	std::cout <<"castleRights ";
	if(st.castleRights&wCastleOO) std::cout<<"K";
	if(st.castleRights&wCastleOOO) std::cout<<"Q";
	if(st.castleRights&bCastleOO) std::cout<<"k";
	if(st.castleRights&bCastleOOO) std::cout<<"q";
	if(st.castleRights==0) std::cout<<"-";
	std::cout<<std::endl;
	std::cout <<"epsquare ";

	if(st.epSquare!=squareNone){
		std::cout<<char('a'+FILES[st.epSquare]);
		std::cout<<char('1'+RANKS[st.epSquare]);
	}else{
		std::cout<<'-';
	}
	std::cout<<std::endl;
	std::cout<<"material "<<st.material[0]/10000.0<<std::endl;
	std::cout<<"white material "<<st.nonPawnMaterial[0]/10000.0<<std::endl;
	std::cout<<"black material "<<st.nonPawnMaterial[2]/10000.0<<std::endl;

	std::cout<<sync_endl;

}


/*	\brief display the fen string of the position
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
void Position::displayFen() const {

	sync_cout;

	int file,rank;
	int emptyFiles=0;
	state& st =getActualState();
	for (rank = 7; rank >= 0; rank--)
	{
		emptyFiles=0;
		for (file = 0; file <= 7; file++)
		{
			if(squares[BOARDINDEX[file][rank]]!=0){
				if(emptyFiles!=0){
					std::cout<<emptyFiles;
				}
				emptyFiles=0;
				std::cout << PIECE_NAMES_FEN[squares[BOARDINDEX[file][rank]]];
			}else{
				emptyFiles++;
			}
		}
		if(emptyFiles!=0){
			std::cout<<emptyFiles;
		}
		if(rank!=0){
			std::cout<<'/';
		}
	}
	std::cout<<' ';
	if(st.nextMove){
		std::cout<<'b';
	}else{
		std::cout<<'w';
	}
	std::cout<<' ';

	if(st.castleRights&wCastleOO){
		std::cout<<"K";
	}
	if(st.castleRights&wCastleOOO){
		std::cout<<"Q";
	}
	if(st.castleRights&bCastleOO){
		std::cout<<"k";
	}
	if(st.castleRights&bCastleOOO){
		std::cout<<"q";
	}
	if(st.castleRights==0){
		std::cout<<"-";
	}
	std::cout<<' ';
	if(st.epSquare!=squareNone){
		std::cout<<char('a'+FILES[st.epSquare]);
		std::cout<<char('1'+RANKS[st.epSquare]);
	}else{
		std::cout<<'-';
	}
	std::cout<<' ';
	std::cout<<st.fiftyMoveCnt;
	std::cout<<" "<<1 + (st.ply - int(st.nextMove == blackTurn)) / 2<<sync_endl;
}


/*! \brief calc the hash key of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
U64 Position::calcKey(void) const {
	U64 hash=0;
	state& st =getActualState();

	for (int i = 0; i < squareNumber; i++)
	{
		if(squares[i]!=empty){
			hash ^=HashKeys::keys[i][squares[i]];
		}
	}

	if(st.nextMove==blackTurn){
		hash ^= HashKeys::side;
	}
	hash ^= HashKeys::castlingRight[st.castleRights];


	if(st.epSquare!= squareNone){
		hash ^= HashKeys::ep[st.epSquare];
	}

	return hash;
}

/*! \brief calc the hash key of the pawn formation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
U64 Position::calcPawnKey(void) const {
	U64 hash=0;
	for (int i = 0; i < squareNumber; i++)
	{
		if(squares[i]==whitePawns || squares[i]==blackPawns){
			hash ^=HashKeys::keys[i][squares[i]];
		}
	}

	return hash;
}

/*! \brief calc the hash key of the material signature
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
U64 Position::calcMaterialKey(void) const {
	U64 hash=0;
	for (int i = 0; i < lastBitboard; i++)
	{
		for (unsigned int cnt = 0; cnt < pieceCount[i]; cnt++){
			hash ^= HashKeys::keys[i][cnt];
		}
	}

	return hash;
}

/*! \brief calc the material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
simdScore Position::calcMaterialValue(void) const{
	simdScore score=0;
	for (tSquare s=(tSquare)0;s<squareNumber;s++){
		bitboardIndex val=squares[s];
		score+=pstValue[val][s];
	}
	return score;

}
/*! \brief calc the non pawn material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::calcNonPawnMaterialValue(Score* s){

	simdScore t[2];
	t[0]=0;
	t[1]=0;

	for (tSquare n=(tSquare)0;n<squareNumber;n++){
		bitboardIndex val=squares[n];
		if(!isPawn(val)){
			if(val>separationBitmap){
				t[1]-=pstValue[val][n];
			}
			else{
				t[0]+=pstValue[val][n];
			}
		}
	}
	t[0].store_partial(2,s);
	t[1].store_partial(2,&s[2]);

}
/*! \brief do a null move
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::doNullMove(void){
	state n=getActualState();
	insertState(n);
	state &x=getActualState();
	x.currentMove=0;
	if(x.epSquare!=squareNone){
		assert(x.epSquare<squareNumber);
		x.key^=HashKeys::ep[x.epSquare];
		x.epSquare=squareNone;
	}
	x.key^=HashKeys::side;

#ifndef DISABLE_PREFETCH
	__builtin_prefetch (TT.findCluster(x.key));
#endif
	x.fiftyMoveCnt++;
	x.pliesFromNull = 0;
	x.skipNullMove=true;
	x.nextMove= (eNextMove)(blackTurn-x.nextMove);
	x.Us=&bitBoard[x.nextMove];
	x.Them=&bitBoard[(blackTurn-x.nextMove)];
	x.ply++;
	x.capturedPiece=empty;
	x.excludedMove=0;


	calcCheckingSquares();
	assert(pieceList[(bitboardIndex)(blackKing-x.nextMove)][0]!=squareNone);
	assert(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0]!=squareNone);
	x.hiddenCheckersCandidate=getHiddenCheckers(pieceList[(bitboardIndex)(blackKing-x.nextMove)][0],x.nextMove);
	x.pinnedPieces=getHiddenCheckers(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0],eNextMove(blackTurn-x.nextMove));

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}
/*! \brief do a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::doMove(Move & m){
	assert(m.packed);
	state n=getActualState();
	bool moveIsCheck=moveGivesCheck(m);
	insertState(n);
	state &x=getActualState();
	x.currentMove=m;




	tSquare from =(tSquare)m.from;
	tSquare to =(tSquare)m.to;
	tSquare captureSquare =(tSquare)m.to;
	bitboardIndex piece= squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);

	bitboardIndex capture = (m.flags ==Move::fenpassant ? (x.nextMove?whitePawns:blackPawns) :squares[to]);
	assert(capture!=separationBitmap);
	assert(capture!=whitePieces);
	assert(capture!=blackPieces);

	simdScore mv;
	mv.load_partial(2,x.material);
	simdScore npm;
	npm.load(x.nonPawnMaterial);

	// change side
	x.key^=HashKeys::side;
	x.ply++;

	// update counter
	x.fiftyMoveCnt++;
	x.pliesFromNull++;
	x.skipNullMove=false;
	x.excludedMove=0;

	// reset ep square
	if(x.epSquare!=squareNone){
		assert(x.epSquare<squareNumber);
		x.key^=HashKeys::ep[x.epSquare];
		x.epSquare=squareNone;
	}

	// do castle additional instruction
	if(m.flags==Move::fcastle){
		bool kingSide=to > from;
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		assert(rFrom<squareNumber);
		bitboardIndex rook = squares[rFrom];
		assert(rook<lastBitboard);
		assert(isRook(rook));
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rTo<squareNumber);
		movePiece(rook,rFrom,rTo);
		mv+=pstValue[rook][rTo]-pstValue[rook][rFrom];

		npm+=nonPawnValue[rook][rTo]-nonPawnValue[rook][rFrom];

		x.key ^=HashKeys::keys[rFrom][rook];
		x.key ^=HashKeys::keys[rTo][rook];

	}

	// do capture
	if(capture){

		if(isPawn(capture)){

			if(m.flags==Move::fenpassant){
				captureSquare-=pawnPush(x.nextMove);
			}
			assert(captureSquare<squareNumber);
			x.pawnKey ^= HashKeys::keys[captureSquare][capture];
		}
		npm-=nonPawnValue[capture][captureSquare];


		// remove piece
		removePiece(capture,captureSquare);
		// update material
		mv-=pstValue[capture][captureSquare];

		// update keys
		x.key ^= HashKeys::keys[captureSquare][capture];
		assert(pieceCount[capture]<30);
		x.materialKey ^= HashKeys::keys[capture][pieceCount[capture]]; // ->after removing the piece

		// reset fifty move counter
		x.fiftyMoveCnt=0;
	}

	// update hashKey
	x.key^= HashKeys::keys[from][piece]^HashKeys::keys[to][piece];
	movePiece(piece,from,to);

	mv+=pstValue[piece][to]-pstValue[piece][from];
	npm+=nonPawnValue[piece][to]-nonPawnValue[piece][from];
	// update non pawn material




	// Update castle rights if needed
	if (x.castleRights && (castleRightsMask[from] | castleRightsMask[to]))
	{
		int cr = castleRightsMask[from] | castleRightsMask[to];
		assert((x.castleRights & cr)<16);
		x.key ^= HashKeys::castlingRight[x.castleRights & cr];
		x.castleRights = (eCastle)(x.castleRights &(~cr));
	}



	if(isPawn(piece)){


		if(
				abs(from-to)==16
				&& (getAttackersTo((tSquare)((from+to)>>1))  & x.Them[Pawns])
		){
			x.epSquare=(tSquare)((from+to)>>1);
			assert(x.epSquare<squareNumber);
			x.key ^=HashKeys::ep[x.epSquare];
		}
		if(m.flags ==Move::fpromotion){
			bitboardIndex promotedPiece=(bitboardIndex)(whiteQueens+x.nextMove+m.promotion);
			assert(promotedPiece<lastBitboard);
			removePiece(piece,to);
			putPiece(promotedPiece,to);

			mv+=pstValue[promotedPiece][to]-pstValue[piece][to];
			npm+=nonPawnValue[promotedPiece][to];


			x.key ^= HashKeys::keys[to][piece]^ HashKeys::keys[to][promotedPiece];
			x.pawnKey ^= HashKeys::keys[to][piece];
			x.materialKey ^= HashKeys::keys[promotedPiece][pieceCount[promotedPiece]-1]^HashKeys::keys[piece][pieceCount[piece]];
		}
		x.pawnKey ^= HashKeys::keys[from][piece] ^ HashKeys::keys[to][piece];
		x.fiftyMoveCnt=0;
	}
#ifndef DISABLE_PREFETCH
	__builtin_prefetch (TT.findCluster(x.key));
#endif


	mv.store_partial(2,x.material);
	npm.store(x.nonPawnMaterial);

	x.capturedPiece=capture;


	x.nextMove= (eNextMove)(blackTurn-x.nextMove);
	x.Us=&bitBoard[x.nextMove];
	x.Them=&bitBoard[(blackTurn-x.nextMove)];
	assert(x.Us);
	assert(x.Them);

	x.checkers=0;
	if(moveIsCheck){

		if(m.flags !=Move::fnone){
			assert(pieceList[whiteKing+x.nextMove][0]<squareNumber);
			x.checkers |= getAttackersTo(pieceList[whiteKing+x.nextMove][0]) & x.Them[Pieces];
		}
		else{
			if(n.checkingSquares[piece] & bitSet(to)){
				x.checkers |=bitSet(to);
			}
			if(n.hiddenCheckersCandidate && (n.hiddenCheckersCandidate & bitSet(from))){
				if(!isRook(piece)){
					assert(pieceList[whiteKing+x.nextMove][0]<squareNumber);
					x.checkers|=Movegen::attackFromRook(pieceList[whiteKing+x.nextMove][0],bitBoard[occupiedSquares]) & (x.Them[Queens] |x.Them[Rooks]);
				}
				if(!isBishop(piece)){
					assert(pieceList[whiteKing+x.nextMove][0]<squareNumber);
					x.checkers|=Movegen::attackFromBishop(pieceList[whiteKing+x.nextMove][0],bitBoard[occupiedSquares]) & (x.Them[Queens] |x.Them[Bishops]);
				}
			}
		}
	}

	calcCheckingSquares();
	assert(pieceList[(bitboardIndex)(blackKing-x.nextMove)][0]<squareNumber);
	x.hiddenCheckersCandidate=getHiddenCheckers(pieceList[(bitboardIndex)(blackKing-x.nextMove)][0],x.nextMove);
	assert(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0]<squareNumber);
	x.pinnedPieces=getHiddenCheckers(pieceList[(bitboardIndex)(whiteKing+x.nextMove)][0],eNextMove(blackTurn-x.nextMove));

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}

/*! \brief undo a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::undoMove(Move & m){

	assert(m.packed);
	state x=getActualState();

	tSquare to = (tSquare)m.to;
	tSquare from = (tSquare)m.from;
	bitboardIndex piece= squares[to];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);

	if(m.flags == Move::fpromotion){
		removePiece(piece,to);
		piece = (bitboardIndex)(piece>separationBitmap?blackPawns:whitePawns);
		putPiece(piece,to);
	}

	if(m.flags== Move::fcastle){
		bool kingSide=to > from;
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);
		bitboardIndex rook = squares[rTo];
		assert(rook<lastBitboard);
		assert(isRook(rook));


		movePiece(rook,rTo,rFrom);
		movePiece(piece,to,from);

	}else{
		movePiece(piece,to,from);
	}

	assert(x.capturedPiece<lastBitboard);
	if(x.capturedPiece){
		tSquare capSq=to;
		if(m.flags == Move::fenpassant){
			capSq+=pawnPush(x.nextMove);
		}
		assert(capSq<squareNumber);
		putPiece(x.capturedPiece,capSq);
	}
	removeState();

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(0);
#endif

}

/*! \brief init the helper castle mash
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initCastleRightsMask(void){
	for(int i=0;i<squareNumber;i++){
		castleRightsMask[i]=0;
	}
	castleRightsMask[E1]=wCastleOO | wCastleOOO;
	castleRightsMask[A1]=wCastleOOO;
	castleRightsMask[H1]=wCastleOO;
	castleRightsMask[E8]=bCastleOO | bCastleOOO;
	castleRightsMask[A8]=bCastleOOO;
	castleRightsMask[H8]=bCastleOO;
}


/*! \brief do a sanity check on the board
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
bool Position::checkPosConsistency(int nn){
	state &x =getActualState();
	if(x.nextMove !=whiteTurn && x.nextMove !=blackTurn){
		sync_cout<<"nextMove error" <<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}

	// check board
	if(bitBoard[whitePieces] & bitBoard[blackPieces]){
		sync_cout<<"white piece & black piece intersected"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if((bitBoard[whitePieces] | bitBoard[blackPieces]) !=bitBoard[occupiedSquares]){
		display();
		displayBitmap(bitBoard[whitePieces]);
		displayBitmap(bitBoard[blackPieces]);
		displayBitmap(bitBoard[occupiedSquares]);

		sync_cout<<"all piece problem"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	for(tSquare i=square0;i<squareNumber;i++){
		bitboardIndex id=squares[i];

		if(id != empty && (bitBoard[id] & bitSet(i))==0){
			sync_cout<<"board inconsistency"<<sync_endl;
			sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
			while(1){}
			return false;
		}
	}

	for (int i = whiteKing; i <= blackPawns; i++){
		for (int j = whiteKing; j <= blackPawns; j++){
			if(i!=j && i!= whitePieces && i!= separationBitmap && j!= whitePieces && j!= separationBitmap && (bitBoard[i] & bitBoard[j])){
				sync_cout<<"bitboard intersection"<<sync_endl;
				sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
				while(1){}
				return false;
			}
		}
	}
	for (int i = whiteKing; i <= blackPawns; i++){
		if(i!= whitePieces && i!= separationBitmap){
			if(pieceCount[i]!= bitCnt(bitBoard[i])){
				sync_cout<<"pieceCount Error"<<sync_endl;
				sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
				while(1){}
				return false;
			}
		}
	}

	for (int i = empty; i < lastBitboard; i++){
		for (unsigned int n=0;n<pieceCount[i];n++){
			if((bitBoard[i] & bitSet(pieceList[i][n]))==0){
				sync_cout<<"pieceList Error"<<sync_endl;
				sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
				while(1){}
				return false;
			}
		}
	}
	bitMap test=0;
	for (int i = whiteKing; i < whitePieces; i++){
		test |=bitBoard[i];
	}
	if(test!= bitBoard[whitePieces]){
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		sync_cout<<"white piece error"<<sync_endl;
		while(1){}
		return false;

	}
	test=0;
	for (int i = blackKing; i < blackPieces; i++){
		test |=bitBoard[i];
	}
	if(test!= bitBoard[blackPieces]){
		sync_cout<<"black piece error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;

	}
	if(x.key != calcKey()){
		display();
		sync_cout<<"hashKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if(x.pawnKey != calcPawnKey()){


		display();
		sync_cout<<"pawnKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if(x.materialKey != calcMaterialKey()){
		sync_cout<<"materialKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}

	simdScore sc=calcMaterialValue();
	if((sc[0]!=x.material[0]) || (sc[1]!=x.material[1])){
		display();
		sync_cout<<sc[0]<<":"<<x.material[0]<<sync_endl;
		sync_cout<<sc[1]<<":"<<x.material[1]<<sync_endl;
		sync_cout<<"material error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	Score score[4];
	calcNonPawnMaterialValue(score);
	if(score[0]!= x.nonPawnMaterial[0] ||
		score[1]!= x.nonPawnMaterial[1] ||
		score[2]!= x.nonPawnMaterial[2] ||
		score[3]!= x.nonPawnMaterial[3]
	){
		display();
		sync_cout<<score[0]<<":"<<x.nonPawnMaterial[0]<<sync_endl;
		sync_cout<<score[1]<<":"<<x.nonPawnMaterial[1]<<sync_endl;
		sync_cout<<score[2]<<":"<<x.nonPawnMaterial[2]<<sync_endl;
		sync_cout<<score[3]<<":"<<x.nonPawnMaterial[3]<<sync_endl;
		sync_cout<<"non pawn material error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}





	return true;
}


/*! \brief calculate the perft result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Position::perft(unsigned int depth){

#define FAST_PERFT
#ifndef FAST_PERFT
	if (depth == 0) {
		return 1;
	}
#endif

	unsigned long long tot = 0;
	Move m;
	m=0;
	Movegen mg(*this,m);
#ifdef FAST_PERFT
	if(depth==1){
		mg.generateMoves<Movegen::allMg>();
		return mg.getGeneratedMoveNumber();
	}
#endif

	while ((m=mg.getNextMove()).packed) {
		doMove(m);
		tot += perft(depth - 1);
		undoMove(m);
	}
	return tot;

}

/*! \brief calculate the divide result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Position::divide(unsigned int depth){

	Move m;
	m=0;
	Movegen mg(*this,m);
	unsigned long long tot = 0;
	unsigned int mn=0;
	while ((m=mg.getNextMove()).packed) {
		mn++;
		doMove(m);
		unsigned long long n= perft(depth - 1);
		tot+=n;
		undoMove(m);
		sync_cout<<mn<<") "<<displayMove(m)<<": "<<n<<sync_endl;

	}
	return tot;

}


/*! \brief calculate the checking squares given the king position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline void Position::calcCheckingSquares(void){
	state & s=getActualState();
	bitboardIndex opponentKing=(bitboardIndex)(blackKing-s.nextMove);
	assert(opponentKing<lastBitboard);
	bitboardIndex attackingPieces=(bitboardIndex)s.nextMove;
	assert(attackingPieces<lastBitboard);


	tSquare kingSquare=pieceList[opponentKing][0];

	bitMap occupancy=bitBoard[occupiedSquares];

	s.checkingSquares[whiteKing+attackingPieces]=0;
	assert(kingSquare<squareNumber);
	assert(whitePawns+attackingPieces<lastBitboard);
	s.checkingSquares[whiteRooks+attackingPieces]=Movegen::attackFromRook(kingSquare,occupancy);
	s.checkingSquares[whiteBishops+attackingPieces]=Movegen::attackFromBishop(kingSquare,occupancy);
	s.checkingSquares[whiteQueens+attackingPieces]=s.checkingSquares[whiteRooks+attackingPieces]|s.checkingSquares[whiteBishops+attackingPieces];
	s.checkingSquares[whiteKnights+attackingPieces]=Movegen::attackFromKnight(kingSquare);

	if(attackingPieces){
		s.checkingSquares[whitePawns+attackingPieces]=Movegen::attackFromPawn(kingSquare,0);
	}else{
		s.checkingSquares[whitePawns+attackingPieces]=Movegen::attackFromPawn(kingSquare,1);
	}

	assert(blackPawns-attackingPieces>=0);
	s.checkingSquares[blackKing-attackingPieces]=0;
	s.checkingSquares[blackRooks-attackingPieces]=0;
	s.checkingSquares[blackBishops-attackingPieces]=0;
	s.checkingSquares[blackQueens-attackingPieces]=0;
	s.checkingSquares[blackKnights-attackingPieces]=0;
	s.checkingSquares[blackPawns-attackingPieces]=0;

}

/*! \brief get the hidden checkers/pinners of a position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bitMap Position::getHiddenCheckers(tSquare kingSquare,eNextMove next){
	assert(kingSquare<squareNumber);
	assert(whitePawns+next<lastBitboard);
	bitMap result=0;
	bitMap pinners= Movegen::getBishopPseudoAttack(kingSquare) &(bitBoard[(bitboardIndex)(whiteBishops+next)]| bitBoard[(bitboardIndex)(whiteQueens+next)]);
	pinners |= Movegen::getRookPseudoAttack(kingSquare) &(bitBoard[(bitboardIndex)(whiteRooks+next)]| bitBoard[(bitboardIndex)(whiteQueens+next)]);

	while(pinners){
		bitMap b = SQUARES_BETWEEN[kingSquare][firstOne(pinners)] & bitBoard[occupiedSquares];
		if (!moreThanOneBit(b)){
			result |= b & bitBoard[(bitboardIndex)(whitePieces+getActualState().nextMove)];
		}
		pinners&= pinners-1;
	}
	return result;

}

/*! \brief get all the attackers/defender of a given square with a given occupancy
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bitMap Position::getAttackersTo(tSquare to, bitMap occupancy) const {
	assert(to<squareNumber);
	bitMap res=(Movegen::attackFromPawn(to,1) & bitBoard[whitePawns])
			|(Movegen::attackFromPawn(to,0) & bitBoard[blackPawns])
			|(Movegen::attackFromKnight(to) & (bitBoard[blackKnights]|bitBoard[whiteKnights]))
			|(Movegen::attackFromKing(to) & (bitBoard[blackKing]|bitBoard[whiteKing]));
	bitMap mask=(bitBoard[blackBishops]|bitBoard[whiteBishops]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	if(Movegen::getBishopPseudoAttack(to) &mask){
		res |=Movegen::attackFromBishop(to,occupancy) & mask;
	}
	mask= (bitBoard[blackRooks]|bitBoard[whiteRooks]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	if(Movegen::getRookPseudoAttack(to) &mask){
		res |=Movegen::attackFromRook(to,occupancy) & mask;
	}
	return	res;
}


/*! \brief tell us if a move gives check before doing the move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bool Position::moveGivesCheck(Move& m)const {
	assert(m.packed);
	tSquare from = (tSquare)m.from;
	tSquare to = (tSquare)m.to;
	bitboardIndex piece = squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);
	state & s=getActualState();

	// Direct check ?
	if (s.checkingSquares[piece] & bitSet(to)){
		return true;
	}

	// Discovery check ?
	if(s.hiddenCheckersCandidate && (s.hiddenCheckersCandidate & bitSet(from)))
	{
		// For pawn and king moves we need to verify also direction
		assert(pieceList[blackKing-s.nextMove][0]<squareNumber);
		if ( (!isPawn(piece)&& !isKing(piece)) || !squaresAligned(from, to, pieceList[blackKing-s.nextMove][0]))
			return true;
	}
	if(m.flags == Move::fnone){
		return false;
	}

	tSquare kingSquare =pieceList[blackKing-s.nextMove][0];
	assert(kingSquare<squareNumber);

	switch(m.flags){
	case Move::fpromotion:{
		bitMap occ= bitBoard[occupiedSquares] ^ bitSet(from);
		assert((bitboardIndex)(whiteQueens+s.nextMove+m.promotion)<lastBitboard);
		return Movegen::attackFrom((bitboardIndex)(whiteQueens+s.nextMove+m.promotion), to, occ) & bitSet(kingSquare);

	}
		break;
	case Move::fcastle:
	{
		tSquare kFrom = from;
		tSquare kTo = to;
		tSquare rFrom=to>from?to+est:to+ovest+ovest;
		tSquare rTo=to>from?to+ovest:to+est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);

		bitMap occ = (bitBoard[occupiedSquares] ^ bitSet(kFrom) ^ bitSet(rFrom)) | bitSet(rTo) | bitSet(kTo);
		return   (Movegen::getRookPseudoAttack(rTo) & bitSet(kingSquare))
			     && (Movegen::attackFromRook(rTo,occ) & bitSet(kingSquare));
	}
		break;
	case Move::fenpassant:
	{
		bitMap captureSquare= FILEMASK[m.to] & RANKMASK[m.from];
		bitMap occ= bitBoard[occupiedSquares]^bitSet((tSquare)m.from)^bitSet((tSquare)m.to)^captureSquare;
		return
				(Movegen::attackFromRook(kingSquare, occ) & (s.Us[Queens] |s.Us[Rooks]))
			   | (Movegen::attackFromBishop(kingSquare, occ) & (s.Us[Queens] |s.Us[Bishops]));

	}
		break;
	default:
		return false;
	}
	return false;
}


bool Position::isDraw() const {

	// Draw by material?

	if (   !bitBoard[whitePawns] && !bitBoard[blackPawns] && ((getActualState().nonPawnMaterial[0]+getActualState().nonPawnMaterial[2])<= pieceValue[whiteBishops][0]))
		return true;

	// Draw by the 50 moves rule?
	if (getActualState().fiftyMoveCnt>  99){
		if(!getActualState().checkers){
			return true;
		}
		Move m;
		m=0;
		Movegen mg(*this,m);
		mg.generateMoves<Movegen::genType::allMg>();
		if(mg.getGeneratedMoveNumber()){
			return true;
		}
	}

	// Draw by repetition?
	for(int i = 4, e = std::min(getActualState().fiftyMoveCnt, getActualState().pliesFromNull);	i<=e;i+=2){
		unsigned int stateIndexPointer=stateIndex-1-i;
		assert(stateIndex>=i+1);
		const state* stp=&stateInfo[stateIndexPointer];
		if(stp->key==getActualState().key){
			return true;
		}
	}
	return false;
}
