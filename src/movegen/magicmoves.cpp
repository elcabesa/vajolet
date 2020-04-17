/**
 *magicmoves.h
 *
 *Source file for magic move bitboard generation.
 *
 *See header file for instructions on usage.
 *
 *The magic keys are not optimal for all squares but they are very close
 *to optimal.
 *
 *Copyright (C) 2007 Pradyumna Kannan.
 *
 *This code is provided 'as-is', without any express or implied warranty.
 *In no event will the authors be held liable for any damages arising from
 *the use of this code. Permission is granted to anyone to use this
 *code for any purpose, including commercial applications, and to alter
 *it and redistribute it freely, subject to the following restrictions:
 *
 *1. The origin of this code must not be misrepresented; you must not
 *claim that you wrote the original code. If you use this code in a
 *product, an acknowledgment in the product documentation would be
 *appreciated but is not required.
 *
 *2. Altered source versions must be plainly marked as such, and must not be
 *misrepresented as being the original code.
 *
 *3. This notice may not be removed or altered from any source distribution.
 */

#include "magicmoves.h"

//For rooks

#define C64(c) c##ULL

uint64_t magicmovesrdb[102400];

const Magic rookMagic[64] {
	{C64(0x0080001020400080), C64(0x000101010101017E), magicmovesrdb+86016, 52},
	{C64(0x0040001000200040), C64(0x000202020202027C), magicmovesrdb+73728, 53},
	{C64(0x0080081000200080), C64(0x000404040404047A), magicmovesrdb+36864, 53},
	{C64(0x0080040800100080), C64(0x0008080808080876), magicmovesrdb+43008, 53},
	{C64(0x0080020400080080), C64(0x001010101010106E), magicmovesrdb+47104, 53},
	{C64(0x0080010200040080), C64(0x002020202020205E), magicmovesrdb+51200, 53},
	{C64(0x0080008001000200), C64(0x004040404040403E), magicmovesrdb+77824, 53},
	{C64(0x0080002040800100), C64(0x008080808080807E), magicmovesrdb+94208, 52},
	{C64(0x0000800020400080), C64(0x0001010101017E00), magicmovesrdb+69632, 53},
	{C64(0x0000400020005000), C64(0x0002020202027C00), magicmovesrdb+32768, 54},
	{C64(0x0000801000200080), C64(0x0004040404047A00), magicmovesrdb+38912, 54},
	{C64(0x0000800800100080), C64(0x0008080808087600), magicmovesrdb+10240, 54},
	{C64(0x0000800400080080), C64(0x0010101010106E00), magicmovesrdb+14336, 54},
	{C64(0x0000800200040080), C64(0x0020202020205E00), magicmovesrdb+53248, 54},
	{C64(0x0000800100020080), C64(0x0040404040403E00), magicmovesrdb+57344, 54},
	{C64(0x0000800040800100), C64(0x0080808080807E00), magicmovesrdb+81920, 53},
	{C64(0x0000208000400080), C64(0x00010101017E0100), magicmovesrdb+24576, 53},
	{C64(0x0000404000201000), C64(0x00020202027C0200), magicmovesrdb+33792, 54},
	{C64(0x0000808010002000), C64(0x00040404047A0400), magicmovesrdb+6144,  54},
	{C64(0x0000808008001000), C64(0x0008080808760800), magicmovesrdb+11264, 54},
	{C64(0x0000808004000800), C64(0x00101010106E1000), magicmovesrdb+15360, 54},
	{C64(0x0000808002000400), C64(0x00202020205E2000), magicmovesrdb+18432, 54},
	{C64(0x0000010100020004), C64(0x00404040403E4000), magicmovesrdb+58368, 54},
	{C64(0x0000020000408104), C64(0x00808080807E8000), magicmovesrdb+61440, 53},
	{C64(0x0000208080004000), C64(0x000101017E010100), magicmovesrdb+26624, 53},
	{C64(0x0000200040005000), C64(0x000202027C020200), magicmovesrdb+4096,  54},
	{C64(0x0000100080200080), C64(0x000404047A040400), magicmovesrdb+7168,  54},
	{C64(0x0000080080100080), C64(0x0008080876080800), magicmovesrdb+0,     54},
	{C64(0x0000040080080080), C64(0x001010106E101000), magicmovesrdb+2048,  54},
	{C64(0x0000020080040080), C64(0x002020205E202000), magicmovesrdb+19456, 54},
	{C64(0x0000010080800200), C64(0x004040403E404000), magicmovesrdb+22528, 54},
	{C64(0x0000800080004100), C64(0x008080807E808000), magicmovesrdb+63488, 53},
	{C64(0x0000204000800080), C64(0x0001017E01010100), magicmovesrdb+28672, 53},
	{C64(0x0000200040401000), C64(0x0002027C02020200), magicmovesrdb+5120,  54},
	{C64(0x0000100080802000), C64(0x0004047A04040400), magicmovesrdb+8192,  54},
	{C64(0x0000080080801000), C64(0x0008087608080800), magicmovesrdb+1024,  54},
	{C64(0x0000040080800800), C64(0x0010106E10101000), magicmovesrdb+3072,  54},
	{C64(0x0000020080800400), C64(0x0020205E20202000), magicmovesrdb+20480, 54},
	{C64(0x0000020001010004), C64(0x0040403E40404000), magicmovesrdb+23552, 54},
	{C64(0x0000800040800100), C64(0x0080807E80808000), magicmovesrdb+65536, 53},
	{C64(0x0000204000808000), C64(0x00017E0101010100), magicmovesrdb+30720, 53},
	{C64(0x0000200040008080), C64(0x00027C0202020200), magicmovesrdb+34816, 54},
	{C64(0x0000100020008080), C64(0x00047A0404040400), magicmovesrdb+9216,  54},
	{C64(0x0000080010008080), C64(0x0008760808080800), magicmovesrdb+12288, 54},
	{C64(0x0000040008008080), C64(0x00106E1010101000), magicmovesrdb+16384, 54},
	{C64(0x0000020004008080), C64(0x00205E2020202000), magicmovesrdb+21504, 54},
	{C64(0x0000010002008080), C64(0x00403E4040404000), magicmovesrdb+59392, 54},
	{C64(0x0000004081020004), C64(0x00807E8080808000), magicmovesrdb+67584, 53},
	{C64(0x0000204000800080), C64(0x007E010101010100), magicmovesrdb+71680, 53},
	{C64(0x0000200040008080), C64(0x007C020202020200), magicmovesrdb+35840, 54},
	{C64(0x0000100020008080), C64(0x007A040404040400), magicmovesrdb+39936, 54},
	{C64(0x0000080010008080), C64(0x0076080808080800), magicmovesrdb+13312, 54},
	{C64(0x0000040008008080), C64(0x006E101010101000), magicmovesrdb+17408, 54},
	{C64(0x0000020004008080), C64(0x005E202020202000), magicmovesrdb+54272, 54},
	{C64(0x0000800100020080), C64(0x003E404040404000), magicmovesrdb+60416, 54},
	{C64(0x0000800041000080), C64(0x007E808080808000), magicmovesrdb+83968, 53},
	{C64(0x00FFFCDDFCED714A), C64(0x7E01010101010100), magicmovesrdb+90112, 53},
	{C64(0x007FFCDDFCED714A), C64(0x7C02020202020200), magicmovesrdb+75776, 54},
	{C64(0x003FFFCDFFD88096), C64(0x7A04040404040400), magicmovesrdb+40960, 54},
	{C64(0x0000040810002101), C64(0x7608080808080800), magicmovesrdb+45056, 53},
	{C64(0x0001000204080011), C64(0x6E10101010101000), magicmovesrdb+49152, 53},
	{C64(0x0001000204000801), C64(0x5E20202020202000), magicmovesrdb+55296, 53},
	{C64(0x0001000082000401), C64(0x3E40404040404000), magicmovesrdb+79872, 53},
	{C64(0x0001FFFAABFAD1A2), C64(0x7E80808080808000), magicmovesrdb+98304, 53}
};

uint64_t magicmovesbdb[5248];
const Magic bishopMagic[64] {
	{C64(0x0002020202020200), C64(0x0040201008040200), magicmovesbdb+4992, 58},
	{C64(0x0002020202020000), C64(0x0000402010080400), magicmovesbdb+2624, 59},
	{C64(0x0004010202000000), C64(0x0000004020100A00), magicmovesbdb+256,  59},
	{C64(0x0004040080000000), C64(0x0000000040221400), magicmovesbdb+896,  59},
	{C64(0x0001104000000000), C64(0x0000000002442800), magicmovesbdb+1280, 59},
	{C64(0x0000821040000000), C64(0x0000000204085000), magicmovesbdb+1664, 59},
	{C64(0x0000410410400000), C64(0x0000020408102000), magicmovesbdb+4800, 59},
	{C64(0x0000104104104000), C64(0x0002040810204000), magicmovesbdb+5120, 58},
	{C64(0x0000040404040400), C64(0x0020100804020000), magicmovesbdb+2560, 59},
	{C64(0x0000020202020200), C64(0x0040201008040000), magicmovesbdb+2656, 59},
	{C64(0x0000040102020000), C64(0x00004020100A0000), magicmovesbdb+288,  59},
	{C64(0x0000040400800000), C64(0x0000004022140000), magicmovesbdb+928,  59},
	{C64(0x0000011040000000), C64(0x0000000244280000), magicmovesbdb+1312, 59},
	{C64(0x0000008210400000), C64(0x0000020408500000), magicmovesbdb+1696, 59},
	{C64(0x0000004104104000), C64(0x0002040810200000), magicmovesbdb+4832, 59},
	{C64(0x0000002082082000), C64(0x0004081020400000), magicmovesbdb+4928, 59},
	{C64(0x0004000808080800), C64(0x0010080402000200), magicmovesbdb+0,    59},
	{C64(0x0002000404040400), C64(0x0020100804000400), magicmovesbdb+128,  59},
	{C64(0x0001000202020200), C64(0x004020100A000A00), magicmovesbdb+320,  57},
	{C64(0x0000800802004000), C64(0x0000402214001400), magicmovesbdb+960,  57},
	{C64(0x0000800400A00000), C64(0x0000024428002800), magicmovesbdb+1344, 57},
	{C64(0x0000200100884000), C64(0x0002040850005000), magicmovesbdb+1728, 57},
	{C64(0x0000400082082000), C64(0x0004081020002000), magicmovesbdb+2304, 59},
	{C64(0x0000200041041000), C64(0x0008102040004000), magicmovesbdb+2432, 59},
	{C64(0x0002080010101000), C64(0x0008040200020400), magicmovesbdb+32,   59},
	{C64(0x0001040008080800), C64(0x0010080400040800), magicmovesbdb+160,  59},
	{C64(0x0000208004010400), C64(0x0020100A000A1000), magicmovesbdb+448,  57},
	{C64(0x0000404004010200), C64(0x0040221400142200), magicmovesbdb+2752, 55},
	{C64(0x0000840000802000), C64(0x0002442800284400), magicmovesbdb+3776, 55},
	{C64(0x0000404002011000), C64(0x0004085000500800), magicmovesbdb+1856, 57},
	{C64(0x0000808001041000), C64(0x0008102000201000), magicmovesbdb+2336, 59},
	{C64(0x0000404000820800), C64(0x0010204000402000), magicmovesbdb+2464, 59},
	{C64(0x0001041000202000), C64(0x0004020002040800), magicmovesbdb+64,   59},
	{C64(0x0000820800101000), C64(0x0008040004081000), magicmovesbdb+192,  59},
	{C64(0x0000104400080800), C64(0x00100A000A102000), magicmovesbdb+576,  57},
	{C64(0x0000020080080080), C64(0x0022140014224000), magicmovesbdb+3264, 55},
	{C64(0x0000404040040100), C64(0x0044280028440200), magicmovesbdb+4288, 55},
	{C64(0x0000808100020100), C64(0x0008500050080400), magicmovesbdb+1984, 57},
	{C64(0x0001010100020800), C64(0x0010200020100800), magicmovesbdb+2368, 59},
	{C64(0x0000808080010400), C64(0x0020400040201000), magicmovesbdb+2496, 59},
	{C64(0x0000820820004000), C64(0x0002000204081000), magicmovesbdb+96,   59},
	{C64(0x0000410410002000), C64(0x0004000408102000), magicmovesbdb+224,  59},
	{C64(0x0000082088001000), C64(0x000A000A10204000), magicmovesbdb+704,  57},
	{C64(0x0000002011000800), C64(0x0014001422400000), magicmovesbdb+1088, 57},
	{C64(0x0000080100400400), C64(0x0028002844020000), magicmovesbdb+1472, 57},
	{C64(0x0001010101000200), C64(0x0050005008040200), magicmovesbdb+2112, 57},
	{C64(0x0002020202000400), C64(0x0020002010080400), magicmovesbdb+2400, 59},
	{C64(0x0001010101000200), C64(0x0040004020100800), magicmovesbdb+2528, 59},
	{C64(0x0000410410400000), C64(0x0000020408102000), magicmovesbdb+2592, 59},
	{C64(0x0000208208200000), C64(0x0000040810204000), magicmovesbdb+2688, 59},
	{C64(0x0000002084100000), C64(0x00000A1020400000), magicmovesbdb+832,  59},
	{C64(0x0000000020880000), C64(0x0000142240000000), magicmovesbdb+1216, 59},
	{C64(0x0000001002020000), C64(0x0000284402000000), magicmovesbdb+1600, 59},
	{C64(0x0000040408020000), C64(0x0000500804020000), magicmovesbdb+2240, 59},
	{C64(0x0004040404040000), C64(0x0000201008040200), magicmovesbdb+4864, 59},
	{C64(0x0002020202020000), C64(0x0000402010080400), magicmovesbdb+4960, 59},
	{C64(0x0000104104104000), C64(0x0002040810204000), magicmovesbdb+5056, 58},
	{C64(0x0000002082082000), C64(0x0004081020400000), magicmovesbdb+2720, 59},
	{C64(0x0000000020841000), C64(0x000A102040000000), magicmovesbdb+864,  59},
	{C64(0x0000000000208800), C64(0x0014224000000000), magicmovesbdb+1248, 59},
	{C64(0x0000000010020200), C64(0x0028440200000000), magicmovesbdb+1632, 59},
	{C64(0x0000000404080200), C64(0x0050080402000000), magicmovesbdb+2272, 59},
	{C64(0x0000040404040400), C64(0x0020100804020000), magicmovesbdb+4896, 59},
	{C64(0x0002020202020200), C64(0x0040201008040200), magicmovesbdb+5184, 58}
};


uint64_t initmagicmoves_occ(const int* squares, const int numSquares, const uint64_t linocc)
{
	int i;
	uint64_t ret=0;
	for(i=0;i<numSquares;i++)
		if(linocc&(((uint64_t)(1))<<i)) ret|=(((uint64_t)(1))<<squares[i]);
	return ret;
}

uint64_t initmagicmoves_Rmoves(const int square, const uint64_t occ)
{
	uint64_t ret=0;
	uint64_t bit;
	uint64_t rowbits=(((uint64_t)0xFF)<<(8*(square/8)));
	
	bit=(((uint64_t)(1))<<square);
	do
	{
		bit<<=8;
		ret|=bit;
	}while(bit && !(bit&occ));
	bit=(((uint64_t)(1))<<square);
	do
	{
		bit>>=8;
		ret|=bit;
	}while(bit && !(bit&occ));
	bit=(((uint64_t)(1))<<square);
	do
	{
		bit<<=1;
		if(bit&rowbits) ret|=bit;
		else break;
	}while(!(bit&occ));
	bit=(((uint64_t)(1))<<square);
	do
	{
		bit>>=1;
		if(bit&rowbits) ret|=bit;
		else break;
	}while(!(bit&occ));
	return ret;
}

uint64_t initmagicmoves_Bmoves(const int square, const uint64_t occ)
{
	uint64_t ret=0;
	uint64_t bit;
	uint64_t bit2;
	uint64_t rowbits=(((uint64_t)0xFF)<<(8*(square/8)));
	
	bit=(((uint64_t)(1))<<square);
	bit2=bit;
	do
	{
		bit<<=8-1;
		bit2>>=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&occ));
	bit=(((uint64_t)(1))<<square);
	bit2=bit;
	do
	{
		bit<<=8+1;
		bit2<<=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&occ));
	bit=(((uint64_t)(1))<<square);
	bit2=bit;
	do
	{
		bit>>=8-1;
		bit2<<=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&occ));
	bit=(((uint64_t)(1))<<square);
	bit2=bit;
	do
	{
		bit>>=8+1;
		bit2>>=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&occ));
	return ret;
}


void initmagicmoves(void)
{
	int i;

	//for bitscans :
	//initmagicmoves_bitpos64_database[(x*C64(0x07EDD5E59A4E28C2))>>58]
	int initmagicmoves_bitpos64_database[64] =
	{
	63,  0, 58,  1, 59, 47, 53,  2,
	60, 39, 48, 27, 54, 33, 42,  3,
	61, 51, 37, 40, 49, 18, 28, 20,
	55, 30, 34, 11, 43, 14, 22,  4,
	62, 57, 46, 52, 38, 26, 32, 41,
	50, 36, 17, 19, 29, 10, 13, 21,
	56, 45, 25, 31, 35, 16,  9, 12,
	44, 24, 15,  8, 23,  7,  6,  5
	};


	// todo do it with bitmap & t_squares & functions

	for(i=0;i<64;i++)
	{
		int squares[64];
		int numsquares=0;
		uint64_t temp=bishopMagic[i].mask;
		while(temp)
		{
			// cppcheck-suppress oppositeExpression
			uint64_t bit=temp&-temp;
			squares[numsquares++]=initmagicmoves_bitpos64_database[(bit*C64(0x07EDD5E59A4E28C2))>>58];
			temp^=bit;
		}
		for(temp=0;temp<(((uint64_t)(1))<<numsquares);temp++)
		{
			uint64_t tempocc=initmagicmoves_occ(squares,numsquares,temp);
			*(bishopMagic[i].move_pointer(tempocc)) = initmagicmoves_Bmoves(i,tempocc);
		}
	}
	for(i=0;i<64;i++)
	{
		int squares[64];
		int numsquares=0;
		uint64_t temp=rookMagic[i].mask;
		while(temp)
		{
			// cppcheck-suppress oppositeExpression
			uint64_t bit=temp&-temp;
			squares[numsquares++]=initmagicmoves_bitpos64_database[(bit*C64(0x07EDD5E59A4E28C2))>>58];
			temp^=bit;
		}
		for(temp=0;temp<(((uint64_t)(1))<<numsquares);temp++)
		{
			uint64_t tempocc=initmagicmoves_occ(squares,numsquares,temp);
			*(rookMagic[i].move_pointer(tempocc)) = initmagicmoves_Rmoves(i,tempocc);
		}
	}
}
