#pragma once

#include "setup.h"
#include "rook.h"
#include "bishop.h"

struct MagicEntry { 
	ull mask;
	ull magic;
	int shift;
  //point to precomputed table of attack bbs for the corresponding square
	ull* attacks; 

    
};

MagicEntry rookMagics[64];
MagicEntry bishopMagics[64];

static ull bishopAttackTable[64 * 512]; //worst case size
static int bishopOffset = 0;

static const ull BISHOP_MAGICS[64] = { 
    0x4004084440408101ULL, 0x2014209100140001ULL, 0x1020282023001101ULL, 0x11010050840202ULL,
    0x84144104100310ULL, 0x21800c120011ULL, 0x100441011000ULL, 0x810410010042ULL,
    0x4040084844081001ULL, 0x100084100110001ULL, 0x40101201011ULL, 0x4080110010100ULL,
    0x10800040108ULL, 0x102401004ULL, 0x1081102401020ULL, 0x10240810ULL,
    0x4008108844101ULL, 0x120800840840001ULL, 0x80801040ULL, 0x404020ULL,
    0x1004080100ULL, 0x8042ULL, 0x810ULL, 0x101010ULL,
    0x84008080080ULL, 0x801010000ULL, 0x20840ULL, 0x101001ULL,
    0x10408102ULL, 0x10080000ULL, 0x40040ULL, 0x100400ULL,
    0x8001ULL, 0x42008ULL, 0x20ULL, 0x1010ULL,
    0x8201010ULL, 0x102000ULL, 0x10ULL, 0x40ULL,
    0x8101ULL, 0x10ULL, 0x2ULL, 0x10110ULL,
    0x101ULL, 0x4ULL, 0x2010ULL, 0x1002ULL,
    0x2ULL, 0x20ULL, 0x10ULL, 0x1ULL,
    0x1ULL, 0x1ULL, 0x2ULL, 0x1ULL,
    0x1ULL, 0x1ULL, 0x1ULL, 0x1ULL,
    0x1ULL, 0x1ULL, 0x1ULL, 0x1ULL
};

void initBishopMagics() { 
    for (int sq = 0; sq < 64; sq++) { 
        ull mask = computeBishopMask(sq);
        int popcount = __builtin_popcountll(mask);
        int shift = 64 - popcount;
        int size = 1 << popcount;

        bishopMagics[sq].magic = BISHOP_MAGICS[sq];
        bishopMagics[sq].mask = mask;
        bishopMagics[sq].shift = shift;
        bishopMagics[sq].attacks = bishopAttackTable + bishopOffset;
        bishopOffset += size;

        ull subset = 0;
        do { 
            subset = (subset - mask) & mask;
            int index = (subset * bishopMagics[sq].magic) >> shift;
            bishopMagics[sq].attacks[index] = computeBishopAttacks(sq, subset);
        } while (subset !=0);
        
    
    }
}

ull computeRookMask(int sq) { 
    ull edges = RANK_1 | RANK_8 | FILE_A | FILE_H;
    ull mask = computeRookAttacks(sq, 0) & ~edges;
    return mask;
}


static ull rookAttackTable[64 * 4096]; // worst case size
static int rookOffset = 0;

static const ull ROOK_MAGICS[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
    0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
    0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
    0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL
};

void initRookMagics() { 
    for (int sq = 0; sq < 64; sq++) { 
        ull mask = computeRookMask(sq);
        int popcount = __builtin_popcountll(mask);
        int shift = 64 - popcount;
        int size = 1 << popcount;

        rookMagics[sq].magic = ROOK_MAGICS[sq];
        rookMagics[sq].mask = mask;
        rookMagics[sq].shift = shift;
        rookMagics[sq].attacks = rookAttackTable + rookOffset;
        rookOffset += size;

        ull subset = 0;
        do { 
            subset = (subset - mask) & mask;
            int index = (subset * rookMagics[sq].magic) >> shift;
            rookMagics[sq].attacks[index] = computeRookAttacks(sq, subset);
        } while (subset !=0);

        
    }
}