#pragma once

#include "setup.h"


ull computeBishopAttacks(int sq, ull occupied) {
    ull attacks = 0;
    
    // four diagonal directions: {rank change, file change}
    // {1,1} = up-right, {1,-1} = up-left
    // {-1,1} = down-right, {-1,-1} = down-left
    int directions[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    
    for (auto& dir : directions) {
        int dr = dir[0]; // rank direction: +1 = up, -1 = down
        int df = dir[1]; // file direction: +1 = right, -1 = left
        
        // start one step away from the bishop's square
        int rank = sq / 8 + dr; // sq / 8 extracts the rank (0-7)
        int file = sq % 8 + df; // sq % 8 extracts the file (0-7)
        
        // continue along the ray while still on the board
        while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
            int current = rank * 8 + file; // convert rank/file back to square index
            attacks |= 1ULL << current;    // add this square to attack set
            
            // if a piece is on this square, ray is blocked - stop here
            // the blocker square is included (can be captured) but no further
            if (occupied & (1ULL << current)) break;
            
            // move one more step in the same direction
            rank += dr;
            file += df;
        }
    }
    return attacks;
}

ull computeBishopMask(int sq) { 
    ull edges = RANK_1 | RANK_8 | FILE_A | FILE_H;
    ull mask = computeBishopAttacks(sq, 0) & ~edges;
    return mask;
}

ull getBishopAttacks(int sq, ull occupied) {
    /*ull blockers = occupied & bishopMagics[sq].mask;
    int index = (blockers * bishopMagics[sq].magic) >> bishopMagics[sq].shift;
    return bishopMagics[sq].attacks[index]; */
    return computeBishopAttacks(sq, occupied);
}

void generateBishopMoves(const Board& board, std::vector<Move>& moves) { 
    ull bishops = (board.turn == WHITE) ? board.whiteBishops : board.blackBishops;
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull ownPieces = (board.turn == WHITE) ? board.whitePieces()  :  board.blackPieces();
    while (bishops) { 
        int sq = __builtin_ctzll(bishops); //gets position of the first rook from 0
        bishops &= bishops - 1; //isolate rook
        ull targets = getBishopAttacks(sq, board.occupied()) & ~ownPieces; //retrieve legal rook moves
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

