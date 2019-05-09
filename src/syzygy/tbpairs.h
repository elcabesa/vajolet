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

#include "bitboardIndex.h"
#include "LR.h"
#include "sparseEntry.h"
#include "tbtypes.h"

// struct PairsData contains low level indexing information to access TB data.
// There are 8, 4 or 2 PairsData records for each TBTable, according to type of
// table and if positions have pawns or not. It is populated at first access.
class PairsData {
    uint8_t flags;                 // Table flags, see enum TBFlag
    uint8_t maxSymLen;             // Maximum length in bits of the Huffman symbols
    uint8_t minSymLen;             // Minimum length in bits of the Huffman symbols
    uint32_t blocksNum;            // Number of blocks in the TB file
    size_t sizeofBlock;            // Block size in bytes
    size_t span;                   // About every span values there is a SparseIndex[] entry
    Sym* lowestSym;                // lowestSym[l] is the symbol of length l with the lowest value
    LR* btree;                     // btree[sym] stores the left and right symbols that expand sym
    uint16_t* blockLength;         // Number of stored positions (minus one) for each block: 1..65536
    uint32_t blockLengthSize;      // Size of blockLength[] table: padded so it's bigger than blocksNum
    SparseEntry* sparseIndex;      // Partial indices into blockLength[]
    size_t sparseIndexSize;        // Size of SparseIndex[] table
    uint8_t* data;                 // Start of Huffman compressed data
    std::vector<uint64_t> base64;  // base64[l - min_sym_len] is the 64bit-padded lowest symbol of length l
    std::vector<uint8_t> symlen;   // Number of values (-1) represented by a given Huffman symbol: 1..256
		// todo pay attenction that vajolet piece encoding is different from stockfish one
    std::array<bitboardIndex, TBPIECES> pieces;// Position pieces: the order of pieces defines the groups
    std::array<uint64_t, TBPIECES+1> groupIdx; // Start index used for the encoding of the group's pieces
    std::array<int, TBPIECES+1> groupLen;      // Number of pieces in a given group: KRKN -> (3, 1)
    std::array<uint16_t, 4> map_idx;           // WDLWin, WDLLoss, WDLCursedWin, WDLBlessedLoss (used in DTZ)
};

#endif