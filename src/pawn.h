#pragma once

#include "setup.h"


void generatePawnMoves(const Board& board, std::vector<Move>& moves) { 
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull singlePush = (board.turn == WHITE) ?  (board.whitePawns << 8) & ~board.occupied()  : (board.blackPawns >> 8) & ~board.occupied();
    ull doublePush = 0;


    if (board.turn == WHITE) {
    ull firstStep = (board.whitePawns << 8) & ~board.occupied();
    ull secondStep = (firstStep << 8) & ~board.occupied() & RANK_4;
    doublePush = secondStep;
    } else {
    ull firstStep = (board.blackPawns >> 8) & ~board.occupied();
    ull secondStep = (firstStep >> 8) & ~board.occupied() & RANK_5;
    doublePush = secondStep;
    }
    
    ull rightCapture = (board.turn == WHITE) ?
    ((board.whitePawns & ~FILE_H) << 9) & enemyPieces :
    ((board.blackPawns & ~FILE_A) >> 9) & enemyPieces;

    ull leftCapture = (board.turn == WHITE) ?
    ((board.whitePawns & ~FILE_A) << 7) & enemyPieces :
    ((board.blackPawns & ~FILE_H) >> 7) & enemyPieces;
    while (singlePush) {
        int to = __builtin_ctzll(singlePush);
        int from = (board.turn == WHITE) ? to - 8 : to + 8;
        // what flags?
        uint8_t flag = ((1ULL << to) & RANK_8 || (1ULL << to) & RANK_1) ? PROMOTION : QUIET;
        // push the move
        singlePush &= singlePush - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }

    while (doublePush) { 
        int to = __builtin_ctzll(doublePush);
        int from = (board.turn == WHITE) ? to - 16 : to + 16;
        uint8_t flag = QUIET;
        doublePush &= doublePush - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }

    while (leftCapture) { 
        int to = __builtin_ctzll(leftCapture);
        int from = (board.turn == WHITE) ? to - 7 : to + 7;
        uint8_t flag = ((1ULL << to) & RANK_8 || (1ULL << to) & RANK_1) ? PROMOTION : CAPTURE;
        leftCapture &= leftCapture - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }

    while (rightCapture) { 
        int to = __builtin_ctzll(rightCapture);
        int from = (board.turn == WHITE) ? to - 9 : to + 9;
        uint8_t flag = ((1ULL << to) & RANK_8 || (1ULL << to) & RANK_1) ? PROMOTION : CAPTURE;
        rightCapture &= rightCapture - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }

    ull enPassantLeft = (board.turn == WHITE) ?
    (board.whitePawns << 7) & board.enPassantSquare :
    (board.blackPawns >> 9) & board.enPassantSquare;
    while (enPassantLeft) { 
        int to = __builtin_ctzll(enPassantLeft);
        int from = (board.turn == WHITE) ? (to - 7) : (to + 7);
        uint8_t flag = EN_PASSANT;
        enPassantLeft &= enPassantLeft - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }
    ull enPassantRight = (board.turn == WHITE) ?
    (board.whitePawns << 9) & board.enPassantSquare :
    (board.blackPawns >> 7) & board.enPassantSquare;
    while (enPassantRight) { 
        int to = __builtin_ctzll(enPassantRight);
        int from = (board.turn == WHITE) ? (to - 9) : (to + 9);
        uint8_t flag = EN_PASSANT;
        enPassantRight &= enPassantRight - 1;
        Move m;
        m.from = from;
        m.to = to;
        m.flags = flag;
        moves.push_back(m);
    }
}
