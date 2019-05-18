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
#include <algorithm>
#include <cassert>

#include "tbCommonData.h"
#include "tbtable.h"
#include "tbpairs.h"

bitboardIndex PairsData::_tbPieceConvert(uint8_t rawData) {
	
	static const bitboardIndex map[] = { 
		empty,
		whitePawns,
		whiteKnights,
		whiteBishops,
		whiteRooks,
		whiteQueens,
		whiteKing,
		empty,
		empty,
		blackPawns,
		blackKnights,
		blackBishops,
		blackRooks,
		blackQueens,
		blackKing,
		empty
	};
	return map[rawData];
}

void PairsData::setPiece(unsigned int idx, uint8_t rawData) {
	assert(idx < TBPIECES);
	_pieces[idx] = _tbPieceConvert(rawData);
}

bitboardIndex PairsData::getPiece(unsigned int idx) const {
	assert(idx < TBPIECES);
	return _pieces[idx];
}

// Group together pieces that will be encoded together. The general rule is that
// a group contains pieces of same type and color. The exception is the leading
// group that, in case of positions withouth pawns, can be formed by 3 different
// pieces (default) or by the king pair when there is not a unique piece apart
// from the kings. When there are pawns, pawns are always first in pieces[].
//
// As example KRKN -> KRK + N, KNNK -> KK + NN, KPPKP -> P + PP + K + K
//
// The actual grouping depends on the TB generator and can be inferred from the
// sequence of pieces in piece[] array.
void PairsData::setGroups(const TBTable& tbt, const int order[], const tFile f) {
	int n = 0, firstLen = tbt.hasPawns() ? 0 : tbt.hasUniquePieces() ? 3 : 2;
	
	for (unsigned int i = 0; i < TBPIECES + 1; ++i) {
		_groupLen[i] = 0;
		_groupIdx[i] = 0;
	}
	_groupLen[n] = 1;

	// Number of pieces per group is stored in groupLen[], for instance in KRKN
	// the encoder will default on '111', so groupLen[] will be (3, 1).
	for (unsigned int i = 1; i < tbt.getPieceCount(); ++i) {
		if (--firstLen > 0 || _pieces[i] == _pieces[i - 1]) {
			_groupLen[n]++;
		} else {
			_groupLen[++n] = 1;
		}
	}
	++n;


	// The sequence in pieces[] defines the groups, but not the order in which
	// they are encoded. If the pieces in a group g can be combined on the board
	// in N(g) different ways, then the position encoding will be of the form:
	//
	//           g1 * N(g2) * N(g3) + g2 * N(g3) + g3
	//
	// This ensures unique encoding for the whole position. The order of the
	// groups is a per-table parameter and could not follow the canonical leading
	// pawns/pieces -> remainig pawns -> remaining pieces. In particular the
	// first group is at order[0] position and the remaining pawns, when present,
	// are at order[1] position.
	bool pp = tbt.hasPawnOnBothSides(); // Pawns on both sides
	int next = pp ? 2 : 1;
	int freeSquares = 64 - _groupLen[0] - (pp ? _groupLen[1] : 0);
	uint64_t idx = 1;

	for (int k = 0; next < n || k == order[0] || k == order[1]; ++k) {
		if (k == order[0]) // Leading pawns or pieces
		{
			_groupIdx[0] = idx;
			idx *= tbt.hasPawns() ? TBCommonData::getLeadPawnsSize(_groupLen[0], f)
				  : tbt.hasUniquePieces() ? 31332 : 462;
		}
		else if (k == order[1]) // Remaining pawns
		{
			_groupIdx[1] = idx;
			idx *= TBCommonData::getBinomial(_groupLen[1], static_cast<tSquare>(48 - _groupLen[0]));
		}
		else // Remainig pieces
		{
			_groupIdx[next] = idx;
			idx *=  TBCommonData::getBinomial(_groupLen[next], static_cast<tSquare>(freeSquares));
			freeSquares -= _groupLen[next++];
		}
	}

	_groupIdx[n] = idx;
}

const uint8_t* PairsData::setSizes(const uint8_t* data) {
	_flags = *data++;

	if (_flags & SingleValueFlag) {
		_blocksNum = _blockLengthSize = 0;
		_span = _sparseIndexSize = 0; // Broken MSVC zero-init
		_minSymLen = *data++; // Here we store the single value
		return data;
	}

	// groupLen[] is a zero-terminated list of group lengths, the last groupIdx[]
	// element stores the biggest index that is the tb size.
	uint64_t tbSize = _groupIdx[std::distance(_groupLen.begin(), std::find(_groupLen.begin(), _groupLen.end(), 0))];
	_sizeofBlock = 1ULL << (*data++);
	_span = 1ULL << (*data++);
	_sparseIndexSize = (tbSize + _span - 1) / _span; // Round up
	auto padding = *data++;
	_blocksNum = *((uint32_t*)data);
	data += sizeof(uint32_t);
	_blockLengthSize = _blocksNum + padding; // Padded to ensure SparseIndex[]
												 // does not point out of range.
	_maxSymLen = *data++;
	_minSymLen = *data++;
	_lowestSym = (Sym*)data;
	_base64.resize(_maxSymLen - _minSymLen + 1);

	// The canonical code is ordered such that longer symbols (in terms of
	// the number of bits of their Huffman code) have lower numeric value,
	// so that d->lowestSym[i] >= d->lowestSym[i+1] (when read as LittleEndian).
	// Starting from this we compute a base64[] table indexed by symbol length
	// and containing 64 bit values so that d->base64[i] >= d->base64[i+1].
	// See http://www.eecs.harvard.edu/~michaelm/E210/huffman.pdf
	for (int i = _base64.size() - 2; i >= 0; --i) {
		_base64[i] = (_base64[i + 1] + *((Sym*)(&_lowestSym[i]))
										 - *((Sym*)(&_lowestSym[i + 1]))) / 2;

		assert(_base64[i] * 2 >= _base64[i+1]);
	}

	// Now left-shift by an amount so that d->base64[i] gets shifted 1 bit more
	// than d->base64[i+1] and given the above assert condition, we ensure that
	// d->base64[i] >= d->base64[i+1]. Moreover for any symbol s64 of length i
	// and right-padded to 64 bits holds d->base64[i-1] >= s64 >= d->base64[i].
	for (size_t i = 0; i < _base64.size(); ++i)
		_base64[i] <<= 64 - i - _minSymLen; // Right-padding to 64 bits

	data += _base64.size() * sizeof(Sym);
	
	_symlen.resize(*((uint16_t*)(data)));
	data += sizeof(uint16_t);
	
	_btree = (LR*)data;

	// The compression scheme used is "Recursive Pairing", that replaces the most
	// frequent adjacent pair of symbols in the source message by a new symbol,
	// reevaluating the frequencies of all of the symbol pairs with respect to
	// the extended alphabet, and then repeating the process.
	// See http://www.larsson.dogma.net/dcc99.pdf
	std::vector<bool> visited(_symlen.size());

	for (Sym sym = 0; sym < _symlen.size(); ++sym)
		if (!visited[sym])
			_symlen[sym] = _setSymlen(sym, visited);

	return data + _symlen.size() * sizeof(LR) + (_symlen.size() & 1);
}


// In Recursive Pairing each symbol represents a pair of childern symbols. So
// read d->btree[] symbols data and expand each one in his left and right child
// symbol until reaching the leafs that represent the symbol value.
uint8_t PairsData::_setSymlen(const Sym s, std::vector<bool>& visited) {
	visited[s] = true; // We can set it now because tree is acyclic
	Sym sr = _btree[s].getRight();

	if (sr == 0xFFF) {
		return 0;
	}

	Sym sl = _btree[s].getLeft();

	if (!visited[sl]) {
		_symlen[sl] = _setSymlen(sl, visited);
	}

	if (!visited[sr]) {
		_symlen[sr] = _setSymlen(sr, visited);
	}

	return _symlen[sl] + _symlen[sr] + 1;
}

int PairsData::getGroupLen(unsigned int group) const {
	assert(group <TBPIECES+1);
	return _groupLen[group];
}

uint64_t PairsData::getGroupIdx(unsigned int group) const {
	assert(group <TBPIECES+1);
	return _groupIdx[group];
}

uint8_t PairsData::getFlags() const {return _flags;}
uint8_t PairsData::getMaxSymLen() const {return _maxSymLen;}
uint8_t PairsData::getMinSymLen() const {return _minSymLen;}
uint32_t PairsData::getBlocksNum() const {return _blocksNum;}
size_t PairsData::getSizeofBlock() const {return _sizeofBlock;}
size_t PairsData::getSpan() const {return _span;}
size_t PairsData::getSparseIndexSize() const {return _sparseIndexSize;}
uint32_t PairsData::getBlockLengthSize() const {return _blockLengthSize;}
Sym* PairsData::getLowestSym() const {return _lowestSym;}

uint64_t PairsData::getBase64(unsigned int idx) const {
	assert(idx < _base64.size());
	return _base64[idx];
}

uint8_t PairsData::getSymLen(unsigned int idx) const {
	assert(idx < _symlen.size());
	return _symlen[idx];
}


const uint8_t* PairsData::setDtzMap(const uint8_t* map, const uint8_t* data) {

	if (_flags & MappedFlag) {
		if (_flags & WideFlag) {
			data += (uintptr_t)data & 1;  // Word alignment, we may have a mixed table
			for (int i = 0; i < 4; ++i) { // Sequence like 3,x,x,x,1,x,0,2,x,x
				_map_idx[i] = (uint16_t)((uint16_t *)data - (uint16_t *)map + 1);
				data += 2 * (*((uint16_t*)data)) + 2;
			}
		}
		else {
			for (int i = 0; i < 4; ++i) {
				_map_idx[i] = (uint16_t)(data - map + 1);
				data += (*data) + 1;
			}
		}
	}
	return data;
}
