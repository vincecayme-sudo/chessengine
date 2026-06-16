#pragma once

#include "setup.h"
#include "movehandler.h"


int evaluate(const Board& board) {
    // count white material
    int whiteMaterial = 
    __builtin_popcountll(board.whitePawns)   * 100 +
    __builtin_popcountll(board.whiteKnights) * 320 +
    __builtin_popcountll(board.whiteBishops) * 330 + 
    __builtin_popcountll(board.whiteRooks)   * 500 + 
    __builtin_popcountll(board.whiteQueens)  * 900;


    // count black material
    int blackMaterial = 
    __builtin_popcountll(board.blackPawns)   * 100 +
    __builtin_popcountll(board.blackKnights) * 320 +
    __builtin_popcountll(board.blackBishops) * 330 + 
    __builtin_popcountll(board.blackRooks)   * 500 + 
    __builtin_popcountll(board.blackQueens)  * 900;
    // return difference
    return whiteMaterial - blackMaterial;
}

int minimax(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) {return evaluate(board);}
    // generate legal moves
    std::vector<Move>legalMoves;
    generateLegalMoves(board, legalMoves);
    if (legalMoves.empty()) {
    if (inCheck(board, board.turn)) return (board.turn == WHITE) ? -99999 : 99999;
    return 0; // stalemate
    }
    // if white's turn, find maximum score
    if (board.turn == WHITE) {
        int best = -99999;
        for (auto& move : legalMoves) {
            pushBoard(board);
            makeMove(board, move);
            int score = minimax(board, depth - 1, alpha, beta);// recurse here
            board = popBoard();
            // update best if score is better
            if (score > best) best = score;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else { // if black's turn, find minimum score
        int best = 99999; 
        for (auto& move : legalMoves) { 
            pushBoard(board);
            makeMove(board, move);
            int score = minimax(board, depth - 1, alpha, beta);
            board = popBoard();
            if (score < best) best = score;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
    
}

Move getBestMove(Board& board, int depth) {
    std::vector<Move> legalMoves;
    generateLegalMoves(board, legalMoves);

    if (legalMoves.empty()) return Move{};


    Move bestMove = legalMoves[0];
    int bestScore = (board.turn == WHITE) ? -99999 : 99999; 
    // loop through moves, call minimax, track best
    for (auto& move : legalMoves) {
        
        pushBoard(board);
        makeMove(board, move);
        int score = minimax(board, depth - 1, -99999, 99999);
        board = popBoard();
        if (board.turn == WHITE) { 
            if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            }
        } else { 
            if (score < bestScore) {
            bestScore = score;
            bestMove = move;
            }
        }
    }
    
    return bestMove;
}
