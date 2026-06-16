#pragma once

#include "setup.h"

ull computeRookAttacks(int sq, ull occupied) {
    ull attacks = 0;
    int r, f;
    int directions[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    
    for (auto& dir : directions) {
        int dr = dir[0], df = dir[1];
        int rank = sq / 8 + dr;
        int file = sq % 8 + df;
        while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
            int current = rank * 8 + file;
            attacks |= 1ULL << current;
            if (occupied & (1ULL << current)) break;
            rank += dr;
            file += df;
        }
    }
    return attacks;
}




ull getRookAttacks(int sq, ull occupied) {
    /*ull blockers = occupied & rookMagics[sq].mask;
    int index = (blockers * rookMagics[sq].magic) >> rookMagics[sq].shift; //magic bitboards dont seem to work correctly
    return rookMagics[sq].attacks[index];*/
    return computeRookAttacks(sq, occupied);
}


void generateRookMoves(const Board& board, std::vector<Move>& moves) { 
    ull rooks = (board.turn == WHITE) ? board.whiteRooks : board.blackRooks;
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull ownPieces = (board.turn == WHITE) ? board.whitePieces() : board.blackPieces();

    while (rooks) { 
        int sq = __builtin_ctzll(rooks); //gets position of the first rook from 0
        rooks &= rooks - 1; //isolate rook
        ull targets = getRookAttacks(sq, board.occupied()) & ~ownPieces; //retrieve legal rook moves
        while (targets) { 
            Move m; //create object m of Move struct
            m.from = sq;
            m.to = __builtin_ctzll(targets); //gets position of first target square from rook square
            m.flags = (enemyPieces & (1ULL << m.to)) ? CAPTURE : QUIET; //mark move as a capture move or quiet move
            moves.push_back(m); //add to possible moves
            targets &= targets - 1;
        }
    }
}


