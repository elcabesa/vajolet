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

#include <cstdint>
#include <chrono>
#include <random>


#include "bitops.h"
#include "book.h"
#include "movepicker.h"
#include "position.h"
#include "vajolet.h"


// polyglot_key() returns the PolyGlot hash key of the given position
uint64_t PolyglotBook::polyglotKey(const Position& pos) const
{
	uint64_t k = 0;
	bitMap b = pos.getOccupationBitmap();

	while( b )
	{
		tSquare s = iterateBit(b);
		bitboardIndex p = pos.getPieceAt(s);

		// PolyGlot pieces are: BP = 0, WP = 1, BN = 2, ... BK = 10, WK = 11
		k ^= PG.Zobrist.psq[pieceMapping[p]][s];
	}

	b = pos.getCastleRights();

	while(b)
	{
		k ^= PG.Zobrist.castle[iterateBit(b)];
	}

	if( pos.hasEpSquare() )
	{
		k ^= PG.Zobrist.enpassant[getFileOf(pos.getEpSquare())];
	}

	if( pos.isWhiteTurn() )
	{
		k ^= PG.Zobrist.turn;
	}

	return k;
}



PolyglotBook::PolyglotBook() {
}

PolyglotBook::~PolyglotBook() { if (is_open()) close(); }

/// operator>>() reads sizeof(T) chars from the file's binary byte stream and
/// converts them in a number of type T. A Polyglot book stores numbers in
/// big-endian format.

template<typename T> PolyglotBook& PolyglotBook::operator>>(T& n)
{
	n = 0;
	for (size_t i = 0; i < sizeof(T); i++)
		n = T((n << 8) + std::ifstream::get());

	return *this;
}

template<> PolyglotBook& PolyglotBook::operator>>(Entry& e) {
	return *this >> e.key >> e.move >> e.count >> e.learn;
}

/// open() tries to open a book file with the given name after closing any
/// exsisting one.

bool PolyglotBook::open(const std::string& fName) {

	if (is_open()) // Cannot close an already closed file
		close();

	std::ifstream::open(fName, std::ifstream::in | std::ifstream::binary);
	std::ifstream::clear(); // Reset any error flag to allow retry ifstream::open()
	return is_open();
}

/// find_first() takes a book key as input, and does a binary search through
/// the book file for the given key. Returns the index of the leftmost book
/// entry with the same key as the input.

size_t PolyglotBook::find_first(uint64_t key)
{

	seekg(0, std::ios::end); // Move pointer to end, so tellg() gets file's size

	size_t low = 0, mid, high = (size_t)tellg() / sizeof(Entry) - 1;
	Entry e;

	assert(low <= high);

	while (low < high && good())
	{
		mid = (low + high) / 2;

		assert(mid >= low && mid < high);

		seekg(mid * sizeof(Entry), ios_base::beg);
		*this >> e;

		if (key <= e.key)
			high = mid;
		else
			low = mid + 1;
	}
	assert(low == high);

	return low;
}


/// probe() tries to find a book move for the given position. If no move is
/// found returns MOVE_NONE. If pickBest is true returns always the highest
/// rated move, otherwise randomly chooses one, based on the move score.

Move PolyglotBook::probe(const Position& pos, bool pickBest)
{

	if (!open("book.bin"))
		return Move::NOMOVE;

	Move m(Move::NOMOVE);
	Entry e;
	uint16_t best = 0;
	unsigned sum = 0;
	uint64_t key = polyglotKey(pos);

	auto idx = find_first(key);
	seekg(idx * sizeof(Entry), ios_base::beg);
	
	std::vector<Entry> moves;

	// search best move and collect statistics
	while (*this >> e, e.key == key && good())
	{
		//std::cout<<"----------"<<std::endl;
		//std::cout<<UciManager::getInstance().displayUci(Move(e.move))<<std::endl;
		moves.push_back(e);
		if( e.count > best )
		{
			best = e.count;
			m = e.move;
		}
		sum += e.count;
	}
	
	if (!pickBest) {
		
		//get a random number
		
		std::mt19937_64 rnd;
		std::uniform_int_distribution<unsigned int> uint_dist(0, sum);
		// use current time (in seconds) as random seed:
		rnd.seed(std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count());
		unsigned int r = uint_dist(rnd);
		
		unsigned total = 0;
		for(auto& e: moves)
		{
			total += e.count;
			
			/// Choose book move according to its score. If a move has a very
			// high score it has higher probability to be choosen than a move
			// with lower score. Note that first entry is always chosen.
			if (total > r) {
				m = e.move;
				break;
			}
		}
	}

	if ( m == Move::NOMOVE )
	{
		return Move::NOMOVE;
	}
	// A PolyGlot book move is encoded as follows:
	//
	// bit  0- 5: destination square (from 0 to 63)
	// bit  6-11: origin square (from 0 to 63)
	// bit 12-14: promotion piece (from KNIGHT == 1 to QUEEN == 4)
	//
	// Castling moves follow "king captures rook" representation. So in case book
	// move is a promotion we have to convert to our representation, in all the
	// other cases we can directly compare with a Move after having masked out
	// the special Move's flags (bit 14-15) that are not supported by PolyGlot.

	// scambio from e to

	Move tempMove(m);

	m.setTo( tempMove.getFrom() );
	m.setFrom( tempMove.getTo() );

	int pt = (tempMove.getPacked() >> 12) & 7;
	if (pt)
	{
		m.setFlag( Move::fpromotion );
		m.setPromotion( (Move::epromotion)(3-pt) );
	}

	Move mm;
	MovePicker mp(pos);
	while( ( mm = mp.getNextMove() ) )
	{
		if (mm.isPromotionMove()) {
			if (m.getFrom() == mm.getFrom() && m.getTo() == mm.getTo()) {
				if (m.getPromotionType() == mm.getPromotionType()) {
					return mm;
				}
			}	
		} else if (mm.isCastleMove()) {
			if (mm.isKingSideCastle()) {
				if (m.getFrom() == mm.getFrom()) {
					if (getFileOf(m.getTo()) == FILEH) {
						if (getRankOf(m.getTo()) == getRankOf(m.getFrom())) {
							return mm;
						}
					}
				}
			} else {
				if (m.getFrom() == mm.getFrom()) {
					if (getFileOf(m.getTo()) == FILEA) {
						if (getRankOf(m.getTo()) == getRankOf(m.getFrom())) {
							return mm;
						}
					}
				}
			}
			//todo mm has king from-to format, m has king->rook format
		} else if (mm.isEnPassantMove()) {
			if (m.getFrom() == mm.getFrom() && m.getTo() == mm.getTo()) {
				return mm;
			}
		} else {
			if (m.getFrom() == mm.getFrom() && m.getTo() == mm.getTo()) {
				return mm;
			}
		}
	}

	return Move::NOMOVE;
}




