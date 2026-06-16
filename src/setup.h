#pragma once

#include <iostream>
#include <cstdint> 
#include <sstream>
#include <fstream>
#include <vector>

using ull = uint64_t; //originally typedef, but using is the modern alternative

enum Color {WHITE, BLACK };

static const uint8_t QUIET      = 0;
static const uint8_t CAPTURE    = 1;
static const uint8_t PROMOTION  = 2;
static const uint8_t EN_PASSANT = 3;
static const uint8_t CASTLING   = 4;

static const uint8_t PROMOTE_QUEEN  = 0;
static const uint8_t PROMOTE_ROOK   = 1;
static const uint8_t PROMOTE_BISHOP = 2;
static const uint8_t PROMOTE_KNIGHT = 3;

static const ull FILE_A = 0x0101010101010101; //Masks for outer bounds
static const ull FILE_H = 0x8080808080808080;
static const ull FILE_AB = 0x0303030303030303;
static const ull FILE_GH = 0xC0C0C0C0C0C0C0C0;
static const ull RANK_1 = 0xFF;
static const ull RANK_8 = 0xFF00000000000000;
static const ull RANK_5 = 0xFF00000000;
static const ull RANK_4 = 0xFF000000;

ull KNIGHT_ATTACKS[64];

struct Board { 
    ull whitePawns = 0xFF00;
    ull whiteKnights = 0x42;
    ull whiteBishops = 0x24;
    ull whiteRooks = 0x81;
    ull whiteQueens = 0x8;
    ull whiteKing = 0x10;

    ull blackPawns = 0xFF000000000000;
    ull blackKnights = 0x4200000000000000;
    ull blackBishops = 0x2400000000000000;
    ull blackRooks = 0x8100000000000000;
    ull blackQueens = 0x0800000000000000;
    ull blackKing = 0x1000000000000000;

    ull enPassantSquare = 0;

    Color turn = WHITE;

    bool whiteCastleKingside = true, whiteCastleQueenside = true;
    bool blackCastleKingside = true, blackCastleQueenside = true;


    ull whitePieces() const { return whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing; }
    

    ull blackPieces() const { return blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing; }
    

    ull occupied() const { return whitePieces() | blackPieces(); }
    
};

struct Move { 
    uint8_t from;
    uint8_t to;
    uint8_t flags; // encodes move type: capture, promotion, en passant, castling
    uint8_t promoted; // which piece was chosen if promotion



};
