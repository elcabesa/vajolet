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

#include "vajolet.h"
#include "position.h"
#include "data.h"
#include "bitops.h"
#include "io.h"
#include "hashKeys.h"
#include "move.h"



Score Position::pieceValue[lastBitboard];
int Position::castleRightsMask[squareNumber];

void Position::initScoreValues(void){
	for(int i=0;i<lastBitboard;i++){
		pieceValue[i]=0;
	}
	pieceValue[whitePawns]=10000;
	pieceValue[whiteKnights]=30000;
	pieceValue[whiteBishops]=32500;
	pieceValue[whiteRooks]=50000;
	pieceValue[whiteQueens]=90000;
	pieceValue[whiteKing]=300000;

	pieceValue[blackPawns]=-pieceValue[whitePawns];
	pieceValue[blackKnights]=-pieceValue[whiteKnights];
	pieceValue[blackBishops]=-pieceValue[whiteBishops];
	pieceValue[blackRooks]=-pieceValue[whiteRooks];
	pieceValue[blackQueens]=-pieceValue[whiteQueens];
	pieceValue[blackKing]=-pieceValue[whiteKing];

	/*for(int i=0;i<lastBitboard;i++){
		sync_cout<<(pieceValue[i]/100)[0]<<sync_endl;
	}*/


}
/*	\brief init a position with the starting position
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void Position::init(void){


	for (int i = 0; i < squareNumber; i++){
		board[i] = empty;
		index[i] = 0;
	};

	for (int i = 0; i < lastBitboard; i++){
		pieceCount[i]=0;
		bitBoard[i]=0;
		for (int n = 0; n < 16; n++){
			pieceList[i][n]=0;
		}
	}


	putPiece( whiteKing,E1);
	putPiece( whiteQueens,D1);
	putPiece( whiteRooks,A1);
	putPiece( whiteRooks,H1);
	putPiece( whiteKnights,B1);
	putPiece( whiteKnights,G1);
	putPiece( whiteBishops,C1);
	putPiece( whiteBishops,F1);
	putPiece( whitePawns,A2);
	putPiece( whitePawns,B2);
	putPiece( whitePawns,C2);
	putPiece( whitePawns,D2);
	putPiece( whitePawns,E2);
	putPiece( whitePawns,F2);
	putPiece( whitePawns,G2);
	putPiece( whitePawns,H2);

	putPiece( blackKing,E8);
	putPiece( blackQueens,D8);
	putPiece( blackRooks,A8);
	putPiece( blackRooks,H8);
	putPiece( blackKnights,B8);
	putPiece( blackKnights,G8);
	putPiece( blackBishops,C8);
	putPiece( blackBishops,F8);
	putPiece( blackPawns,A7);
	putPiece( blackPawns,B7);
	putPiece( blackPawns,C7);
	putPiece( blackPawns,D7);
	putPiece( blackPawns,E7);
	putPiece( blackPawns,F7);
	putPiece( blackPawns,G7);
	putPiece( blackPawns,H7);



	stateInfo.clear();
	state s;

	s.nextMove=whiteTurn;
	s.castleRights=(eCastle)(wCastleOO|wCastleOOO|bCastleOO|bCastleOOO);
	s.epSquare=squareNone;
	s.fiftyMoveCnt=0;
	s.pliesFromNull=0;
	s.ply=0;
	s.capturedPiece=empty;

	calcNonPawnMaterialValue(s.nonPawnMaterial);
	calcMaterialValue().store_partial(2,s.material);

	stateInfo.push_back(s);
	state & x=stateInfo.back();

	x.key=calcKey();
	x.pawnKey=calcPawnKey();
	x.materialKey=calcMaterialKey();



}




/*	\brief display a board for debug purpose
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
void Position::display()const {
	displayFen();
	int rank, file;
	sync_cout;
	{
		for (rank = 7; rank >= 0; rank--)
		{
			std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
			std::cout << rank+1 <<  " |";
			for (file = 0; file <= 7; file++)
			{
				std::cout << " " << PIECE_NAMES_FEN[board[BOARDINDEX[file][rank]]] << " |";
			}
			std::cout << std::endl;
		}
		std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
		std::cout << "    a   b   c   d   e   f   g   h" << std::endl << std::endl;

	}
	std::cout <<(stateInfo.back().nextMove?"BLACK TO MOVE":"WHITE TO MOVE") <<std::endl;
	std::cout <<"50 move counter "<<stateInfo.back().fiftyMoveCnt<<std::endl;
	std::cout <<"castleRights " <<stateInfo.back().castleRights<<std::endl;
	std::cout <<"epsquare ";

	if(stateInfo.back().epSquare!=squareNone){
		std::cout<<char('a'+FILES[stateInfo.back().epSquare]);
		std::cout<<char('1'+RANKS[stateInfo.back().epSquare]);
	}else{
		std::cout<<'-';
	}
	std::cout<<std::endl;
	std::cout<<"material "<<stateInfo.back().material[0]/10000.0<<std::endl;
	std::cout<<"white material "<<stateInfo.back().nonPawnMaterial[0]/10000.0<<std::endl;
	std::cout<<"black material "<<stateInfo.back().nonPawnMaterial[2]/10000.0<<std::endl;

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
	for (rank = 7; rank >= 0; rank--)
	{
		emptyFiles=0;
		for (file = 0; file <= 7; file++)
		{
			if(board[BOARDINDEX[file][rank]]!=0){
				if(emptyFiles!=0){
					std::cout<<emptyFiles;
				}
				emptyFiles=0;
				std::cout << PIECE_NAMES_FEN[board[BOARDINDEX[file][rank]]];
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
	if(stateInfo.back().nextMove){
		std::cout<<'b';
	}else{
		std::cout<<'w';
	}
	std::cout<<' ';

	if(stateInfo.back().castleRights&wCastleOO){
		std::cout<<"K";
	}
	if(stateInfo.back().castleRights&wCastleOOO){
		std::cout<<"Q";
	}
	if(stateInfo.back().castleRights&bCastleOO){
		std::cout<<"k";
	}
	if(stateInfo.back().castleRights&bCastleOOO){
		std::cout<<"q";
	}
	if(stateInfo.back().castleRights==0){
		std::cout<<"-";
	}
	std::cout<<' ';
	if(stateInfo.back().epSquare!=squareNone){
		std::cout<<char('a'+FILES[stateInfo.back().epSquare]);
		std::cout<<char('1'+RANKS[stateInfo.back().epSquare]);
	}else{
		std::cout<<'-';
	}
	std::cout<<' ';
	std::cout<<stateInfo.back().fiftyMoveCnt;
	std::cout<<" 1"<<sync_endl;
}


U64 Position::calcKey(void) const {
	U64 hash=0;
	for (int i = 0; i < squareNumber; i++)
	{
		if(board[i]!=empty){
			hash ^=HashKeys::keys[i][board[i]];
		}
	}

	if(stateInfo.back().nextMove==blackTurn){
		hash ^= HashKeys::side;
	}
	hash ^= HashKeys::castlingRight[stateInfo.back().castleRights];


	if(stateInfo.back().epSquare!= squareNone){
		hash ^= HashKeys::ep[stateInfo.back().epSquare];
	}

	return hash;
}

U64 Position::calcPawnKey(void) const {
	U64 hash=0;
	for (int i = 0; i < squareNumber; i++)
	{
		if(board[i]==whitePawns || board[i]==blackPawns){
			hash ^=HashKeys::keys[i][board[i]];
		}
	}



	return hash;
}

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


Score Position::calcMaterialValue(void) const{
	Score s=0;
	for (int i = 0; i < squareNumber; i++)
	{
		s+=pieceValue[board[i]];
	}
	return s;

}

void Position::calcNonPawnMaterialValue(unpackedScore* s){

	Score t[2];
	t[0]=0;
	t[1]=0;

	for (int i = 0; i < squareNumber; i++)
	{
		if(!isPawn(board[i]) && !isKing(board[i])){
			if(board[i]>emptyBitmap){
				t[1]-=pieceValue[board[i]];
			}
			else{
				t[0]+=pieceValue[board[i]];
			}
		}
	}
	t[0].store_partial(2,s);
	t[1].store_partial(2,&s[2]);

}

void Position::doNullMove(void){
	state n=stateInfo.back();
	stateInfo.push_back(n);
	state &x=stateInfo.back();
	if(x.epSquare!=squareNone){
		x.key^=HashKeys::ep[x.epSquare];
		x.epSquare=squareNone;
	}
	x.key^=HashKeys::side;
	x.fiftyMoveCnt++;
	x.pliesFromNull = 0;
	x.nextMove= (eNextMove)(blackTurn-x.nextMove);
	x.ply++;


}

void Position::doMove(Move m){
	state n=stateInfo.back();
	stateInfo.push_back(n);
	state &x=stateInfo.back();

	tSquare from =m.from;
	tSquare to =m.to;
	tSquare captureSquare =m.to;
	bitboardIndex piece= board[from];
	bitboardIndex capture = (m.flags ==Move::fenpassant ? (x.nextMove?blackPawns:whitePawns) :board[to]);

	// change side
	x.key^=HashKeys::side;
	x.ply++;

	// update counter
	x.fiftyMoveCnt++;
	x.pliesFromNull++;

	// reset ep square
	if(x.epSquare!=squareNone){
		x.key^=HashKeys::ep[x.epSquare];
		x.epSquare=squareNone;
	}

	// do castle additional instruction
	if(m.flags==Move::fcastle){
		bool kingSide=to > from;
		tSquare rFrom = kingSide? to+1: to-2;
		bitboardIndex rook = board[rFrom];
		tSquare rTo = kingSide? to-1: to+2;
		movePiece(rook,rFrom,rTo);
		x.key ^=HashKeys::keys[rFrom][rook];
		x.key ^=HashKeys::keys[rTo][rook];

	}

	// do capture
	if(capture){

		if(isPawn(capture)){
			if(m.flags==Move::fenpassant){
				captureSquare-=pawnPush(x.nextMove);
			}
			x.pawnKey ^= HashKeys::keys[captureSquare][capture];
		}
		else{
			// update non pawn material
			if(capture>emptyBitmap){
				// black piece captured
				x.nonPawnMaterial[2]+=pieceValue[capture][0];
				x.nonPawnMaterial[3]+=pieceValue[capture][1];
			}
			else{
				//white piece captured
				x.nonPawnMaterial[0]-=pieceValue[capture][0];
				x.nonPawnMaterial[1]-=pieceValue[capture][1];
			}
		}

		// remove piece
		removePiece(capture,captureSquare);

		// update material
		x.material[0]-=pieceValue[capture][0];
		x.material[1]-=pieceValue[capture][1];
		// update keys
		x.key ^= HashKeys::keys[captureSquare][capture];
		x.materialKey ^= HashKeys::keys[capture][pieceCount[capture]]; // ->after removing the piece

		// reset fifty move counter
		x.fiftyMoveCnt=0;
	}

	// update hashKey
	x.key^= HashKeys::keys[from][piece]^HashKeys::keys[to][piece];

	// Update castle rights if needed
	if (x.castleRights && (castleRightsMask[from] | castleRightsMask[to]))
	{
		int cr = castleRightsMask[from] | castleRightsMask[to];
		x.key ^= HashKeys::castlingRight[x.castleRights & cr];
		x.castleRights = (eCastle)(x.castleRights &(~cr));
	}

	movePiece(piece,from,to);

	if(isPawn(piece)){
		if(
				abs(from-to)==16
				// TODO AGGIUNGERE CODICE COMMENTATO SOTTO
				//&& PEZZO ATTACCATO DA PEDONE
		){
			x.epSquare=(from+to)>>1;
			x.key ^=HashKeys::ep[x.epSquare];
		}
		if(m.flags ==Move::fpromotion){
			bitboardIndex promotedPiece=(bitboardIndex)(piece+m.promotion);
			removePiece(piece,to);
			putPiece(promotedPiece,to);

			x.key ^= HashKeys::keys[to][piece]^ HashKeys::keys[to][promotedPiece];
			x.pawnKey ^= HashKeys::keys[from][piece];
			x.materialKey ^= HashKeys::keys[promotedPiece][pieceCount[promotedPiece]-1]^HashKeys::keys[piece][pieceCount[piece]];
		}
		x.pawnKey ^= HashKeys::keys[from][piece] ^ HashKeys::keys[to][piece];
		x.fiftyMoveCnt=0;
	}

	x.capturedPiece=capture;


	x.nextMove= (eNextMove)(blackTurn-x.nextMove);

	checkPosConsistency();


}


void Position::undoMove(Move m){

	state x=stateInfo.back();

	tSquare to = m.to;
	tSquare from = m.from;
	bitboardIndex piece= board[to];

	if(m.flags == Move::fpromotion){
		removePiece(piece,to);
		piece = (bitboardIndex)(piece-m.promotion);
		putPiece(piece,to);
	}

	if(m.flags== Move::fcastle){
		bool kingSide=to > from;
		tSquare rFrom = kingSide? to+1: to-2;
		tSquare rTo = kingSide? to-1: to+2;
		bitboardIndex rook = board[rTo];

		movePiece(rook,rTo,rFrom);
		movePiece(piece,to,from);

	}else{
		movePiece(piece,to,from);
	}

	if(x.capturedPiece){
		tSquare capSq=to;
		if(m.flags == Move::fenpassant){
			capSq-=pawnPush(x.nextMove);
		}
		putPiece(x.capturedPiece,capSq);
	}
	stateInfo.pop_back();

	checkPosConsistency();

}


void Position::initCastlaRightsMask(void){
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


bool Position::checkPosConsistency(void){
	state x =stateInfo.back();
	if(x.nextMove !=whiteTurn && x.nextMove !=blackTurn){
		sync_cout<<"nextMove error" <<sync_endl;
		return false;
	}

	// check board
	if(bitBoard[whitePieces] & bitBoard[blackPieces]){
		sync_cout<<"white piece & black piece intersected"<<sync_endl;
		return false;
	}
	if((bitBoard[whitePieces] | bitBoard[blackPieces]) !=bitBoard[occupiedSquares]){
		sync_cout<<"all piece problem"<<sync_endl;
		return false;
	}
	for(int i=0;i<squareNumber;i++){
		bitboardIndex id=board[i];

		if(id != empty && (bitBoard[id] & bitSet(i))==0){
			sync_cout<<"board inconsistency"<<sync_endl;
			return false;
		}
	}

	for (int i = whiteKing; i <= blackPawns; i++){
		for (int j = whiteKing; j <= blackPawns; j++){
			if(i!=j && i!= whitePieces && i!= emptyBitmap && j!= whitePieces && j!= emptyBitmap && (bitBoard[i] & bitBoard[j])){
				sync_cout<<"bitboard intersection"<<sync_endl;
				return false;
			}
		}
	}
	for (int i = whiteKing; i <= blackPawns; i++){
		if(i!= whitePieces && i!= emptyBitmap){
			if(pieceCount[i]!= bitCnt(bitBoard[i])){
				sync_cout<<"pieceCount Error"<<sync_endl;
				return false;
			}
		}
	}

	for (int i = empty; i < lastBitboard; i++){
		for (unsigned int n=0;n<pieceCount[i];n++){
			if((bitBoard[i] & bitSet(pieceList[i][n]))==0){
				sync_cout<<"pieceList Error"<<sync_endl;
				return false;
			}
		}
	}
	bitMap test=0;
	for (int i = whiteKing; i < whitePieces; i++){
		test |=bitBoard[i];
	}
	if(test!= bitBoard[whitePieces]){
		sync_cout<<"white piece error"<<sync_endl;
		return false;

	}
	test=0;
	for (int i = blackKing; i < blackPieces; i++){
		test |=bitBoard[i];
	}
	if(test!= bitBoard[blackPieces]){
		sync_cout<<"black piece error"<<sync_endl;
		return false;

	}
	if(x.key != calcKey()){
		sync_cout<<"hashKey error"<<sync_endl;
		return false;
	}
	if(x.pawnKey != calcPawnKey()){

		sync_cout<<"pawnKey error"<<sync_endl;
		return false;
	}
	if(x.materialKey != calcMaterialKey()){
		sync_cout<<"materialKey error"<<sync_endl;
		return false;
	}


	return true;
}
