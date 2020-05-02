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
#ifndef BITBOARDINDEX_H_
#define BITBOARDINDEX_H_

/*! \brief define the index of the bitboards
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
enum bitboardIndex
{
	occupiedSquares,			//0		00000000
	whiteKing,					//1		00000001
	whiteQueens,				//2		00000010
	whiteRooks,					//3		00000011
	whiteBishops,				//4		00000100
	whiteKnights,				//5		00000101
	whitePawns,					//6		00000110
	whitePieces,				//7		00000111

	separationBitmap,			//8
	blackKing,					//9		00001001
	blackQueens,				//10	00001010
	blackRooks,					//11	00001011
	blackBishops,				//12	00001100
	blackKnights,				//13	00001101
	blackPawns,					//14	00001110
	blackPieces,				//15	00001111

	lastBitboard,

	King = whiteKing,
	Queens,
	Rooks,
	Bishops,
	Knights,
	Pawns,
	Pieces,
	whites = occupiedSquares,
	blacks = separationBitmap,

	empty = occupiedSquares

};

inline bitboardIndex operator++(bitboardIndex& d, int) { bitboardIndex r = d; d = static_cast<bitboardIndex>(static_cast<int>(d) + 1); return r; }
inline bitboardIndex& operator++(bitboardIndex& d) { d = static_cast<bitboardIndex>(static_cast<int>(d) + 1); return d; }

inline bitboardIndex operator--(bitboardIndex& d, int) { bitboardIndex r = d; d = static_cast<bitboardIndex>(static_cast<int>(d) - 1); return r; }
inline bitboardIndex& operator--(bitboardIndex& d) { d = static_cast<bitboardIndex>(static_cast<int>(d) - 1); return d; }

enum eNextMove	// color turn. ( it's also used as offset to access bitmaps by index)
{
	whiteTurn = 0,
	blackTurn = blackKing - whiteKing
};

/*! \brief tell if the piece is a pawn
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
inline static bool isPawn(const bitboardIndex piece)
{
	return (piece & 7) == Pawns;
}
/*! \brief tell if the piece is a king
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
inline static bool isKing(const bitboardIndex piece)
{
	return (piece & 7) == King;
}
/*! \brief tell if the piece is a queen
	\author Marco Belli
	\version 1.0
	\date 04/11/2013
*/
inline static bool isQueen(const bitboardIndex piece)
{
	return (piece & 7) == Queens;
}
/*! \brief tell if the piece is a rook
	\author Marco Belli
	\version 1.0
	\date 04/11/2013
*/
inline static bool isRook(const bitboardIndex piece)
{
	return (piece & 7) == Rooks;
}
/*! \brief tell if the piece is a bishop
	\author Marco Belli
	\version 1.0
	\date 04/11/2013
*/
inline static bool isBishop(const bitboardIndex piece)
{
	return (piece & 7) == Bishops;
}

/*! \brief tell if the piece is a bishop
	\author Marco Belli
	\version 1.0
	\date 04/11/2013
*/
inline static bool isKnight(const bitboardIndex piece)
{
	return (piece & 7) == Knights;
}
/*! \brief tell the color of a piece
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
inline static bool isblack(const bitboardIndex piece)
{
	return bool(piece & 8);
}

	/*! \brief tell the color of a piece
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
inline static bool isValidPiece(const bitboardIndex piece)
{
	return piece < lastBitboard && piece != occupiedSquares && piece != whitePieces && piece != separationBitmap && piece != blackPieces;
}

inline bitboardIndex getPieceOfPlayer( const bitboardIndex piece, const eNextMove nM)
{
	return static_cast<bitboardIndex>(static_cast<int>(piece) + static_cast<int>(nM));
}

inline bitboardIndex getPieceOfOpponent( const bitboardIndex piece, const eNextMove nM)
{
	return (bitboardIndex)( separationBitmap + piece - nM );
}

inline bitboardIndex getPieceType( const bitboardIndex piece )
{
	return bitboardIndex(piece % separationBitmap);
}

inline bool isBlackPiece( const bitboardIndex piece )
{
	return piece >= separationBitmap;
}


#endif /* BITBOARDINDEX_H_ */
