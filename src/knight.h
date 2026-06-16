#pragma once

#include "setup.h"


void initKnightAttacks() { //finds legal knight moves
    for (int sq = 0; sq < 64; sq++) {
        ull n = 1ULL << sq;
        KNIGHT_ATTACKS[sq] = //generate legal moves AND not on corresponding mask
        (n << 10) & ~FILE_GH |
        (n << 6)  & ~FILE_AB |
        (n << 17) & ~FILE_H  |
        (n << 15) & ~FILE_A  |
        (n >> 10) & ~FILE_GH |
        (n >> 6)  & ~FILE_AB |
        (n >> 17) & ~FILE_H  |
        (n >> 15) & ~FILE_A;
    }
}

ull knightMoves(int sq, const Board& board) { //runtime move generation function
    //returns a bitboard of all possible knight moves where there is no ally piece
    ull ownPieces = (board.turn == WHITE) ? board.whitePieces() : board.blackPieces();
    return KNIGHT_ATTACKS[sq] & ~ownPieces;
}

void generateKnightMoves(const Board& board, std::vector<Move>& moves) { 
    ull knights = (board.turn == WHITE) ? board.whiteKnights : board.blackKnights;
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    while (knights) { 
        int sq = __builtin_ctzll(knights); //gets position of the first knight from 0
        knights &= knights - 1; //isolate knight
        ull targets = knightMoves(sq, board); //retrieve legal knight moves
        while (targets) { 
            Move m; //create object m of Move struct
            m.from = sq;
            m.to = __builtin_ctzll(targets); //gets position of first target square from knight square
            m.flags = (enemyPieces & (1ULL << m.to)) ? CAPTURE : QUIET; //mark move as a capture move or quiet move
            moves.push_back(m); //add to possible moves
            targets &= targets - 1;
        }
    }
}
