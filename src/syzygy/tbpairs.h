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

#ifndef TBPAIRS_H
#define TBPAIRS_H

#include <vector>
#include <array>

#include "bitBoardIndex.h"
#include "tSquare.h"
#include "LR.h"
#include "sparseEntry.h"
#include "tbtypes.h"

class TBTable;

// struct PairsData contains low level indexing information to access TB data.
// There are 8, 4 or 2 PairsData records for each TBTable, according to type of
// table and if positions have pawns or not. It is populated at first access.
class PairsData {
private:
	uint8_t _flags;                 // Table flags, see enum TBFlag
	uint8_t _maxSymLen;             // Maximum length in bits of the Huffman symbols
	uint8_t _minSymLen;             // Minimum length in bits of the Huffman symbols
	uint32_t _blocksNum;            // Number of blocks in the TB file
	size_t _sizeofBlock;            // Block size in bytes
	size_t _span;                   // About every span values there is a SparseIndex[] entry
	Sym* _lowestSym;                // lowestSym[l] is the symbol of length l with the lowest value
	LR* _btree;                     // btree[sym] stores the left and right symbols that expand sym
	uint16_t* _blockLength;         // Number of stored positions (minus one) for each block: 1..65536
	uint32_t _blockLengthSize;      // Size of blockLength[] table: padded so it's bigger than blocksNum
	SparseEntry* _sparseIndex;      // Partial indices into blockLength[]
	size_t _sparseIndexSize;        // Size of SparseIndex[] table
	const uint8_t* _data;                 // Start of Huffman compressed data
	std::vector<uint64_t> _base64;  // base64[l - min_sym_len] is the 64bit-padded lowest symbol of length l
	std::vector<uint8_t> _symlen;   // Number of values (-1) represented by a given Huffman symbol: 1..256
	std::array<bitboardIndex, TBPIECES> _pieces;// Position pieces: the order of pieces defines the groups
	std::array<uint64_t, TBPIECES+1> _groupIdx; // Start index used for the encoding of the group's pieces
	std::array<int, TBPIECES+1> _groupLen;      // Number of pieces in a given group: KRKN -> (3, 1)
	std::array<uint16_t, 4> _map_idx;           // WDLWin, WDLLoss, WDLCursedWin, WDLBlessedLoss (used in DTZ)
	
	static bitboardIndex _tbPieceConvert(uint8_t rawData);
	
	// Each table has a set of flags: all of them refer to DTZ tables, the last one to WDL tables
	static const unsigned int SingleValueFlag = 128;
	static const unsigned int WideFlag = 16;
	static const unsigned int  LossPliesFlag = 8;
	static const unsigned int WinPliesFlag = 4;
	static const unsigned int MappedFlag = 2;
	static const unsigned int STMFlag = 1;
	
	
	uint8_t _setSymlen(const Sym s, std::vector<bool>& visited);
	

public:
	PairsData() {}
	~PairsData() {}
	PairsData(const PairsData& other) = delete;
	PairsData(PairsData&& other) noexcept = delete;// move constructor
	PairsData& operator=(const PairsData& other) =delete; // copy assignment
	PairsData& operator=(PairsData&& other) noexcept = delete; // move assignment
	
	void setPiece(unsigned int idx, uint8_t rawData);
	bitboardIndex getPiece(unsigned int idx) const;
	void setGroups(const TBTable& tbt, const int order[], const tFile f);
	const uint8_t* setSizes(const uint8_t* data);
	const uint8_t* setDtzMap(const uint8_t* map, const uint8_t* data);
	const uint8_t* setSparseIndex(const uint8_t* data);
	const uint8_t* setBlockLength(const uint8_t* data);
	const uint8_t* setData(const uint8_t* data);
	int getGroupLen(unsigned int group) const;
	uint64_t getGroupIdx(unsigned int group) const;
	uint8_t getFlags() const;
	uint8_t getMaxSymLen() const;
	uint8_t getMinSymLen() const;
	uint32_t getBlocksNum() const;
	size_t getSizeofBlock() const;
	size_t getSpan() const;
	size_t getSparseIndexSize() const;
	uint32_t getBlockLengthSize() const;
	Sym* getLowestSym() const;
	uint64_t getBase64(unsigned int idx) const;
	uint8_t getSymLen(unsigned int idx) const;

};


#endif