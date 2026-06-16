#pragma once

#include "setup.h"
#include "rook.h"
#include "bishop.h"
#include "knight.h"
#include "pawn.h"
#include "magics.h"


void makeMove(Board& board, Move move) 
{ 
    // 1. if capture, clear the to square on the enemy bitboard
    // 2. find which bitboard contains the piece on move.from
    // 3. clear the from square on that bitboard
    // 4. set the to square on that bitboard
    
    if (move.flags == CAPTURE) //1. If the move is a capture, clear the landing square
    { 
        if (board.blackPawns & (1ULL << move.to)){ 
        board.blackPawns &= ~(1ULL << move.to);
        } else if (board.blackKnights & (1ULL << move.to)) {
            board.blackKnights &= ~(1ULL << move.to);
        } else if (board.blackBishops & (1ULL << move.to)) { 
            board.blackBishops &= ~(1ULL << move.to);
        } else if (board.blackRooks & (1ULL << move.to)) { 
            board.blackRooks &= ~(1ULL << move.to);
        } else if (board.blackQueens & (1ULL << move.to)) { 
            board.blackQueens &= ~(1ULL << move.to);
        } else if (board.blackKing & (1ULL << move.to)) { 
            board.blackKing &= ~(1ULL << move.to);
        } else if (board.whitePawns & (1ULL << move.to)){ //Don't forget white pieces
            board.whitePawns &= ~(1ULL << move.to);
        } else if (board.whiteKnights & (1ULL << move.to)){ 
            board.whiteKnights &= ~(1ULL << move.to);
        } else if (board.whiteBishops & (1ULL << move.to)){ 
            board.whiteBishops &= ~(1ULL << move.to);
        } else if (board.whiteRooks & (1ULL << move.to)){ 
            board.whiteRooks &= ~(1ULL << move.to);
        } else if (board.whiteQueens & (1ULL << move.to)){ 
            board.whiteQueens &= ~(1ULL << move.to);
        } else if (board.whiteKing & (1ULL << move.to)){ 
            board.whiteKing &= ~(1ULL << move.to);
        }
    }

    if (board.whitePawns & (1ULL << move.from)) 
    { 
        board.whitePawns &= ~(1ULL << move.from);
        board.whitePawns |= 1ULL << move.to;
    } else if (board.whiteKnights & (1ULL << move.from))
    { 
        board.whiteKnights &= ~(1ULL << move.from);
        board.whiteKnights |= 1ULL << move.to;
    } else if (board.whiteBishops & (1ULL << move.from)) 
    { 
        board.whiteBishops &= ~(1ULL << move.from);
        board.whiteBishops |= 1ULL << move.to;
    } else if (board.whiteRooks & (1ULL << move.from)) 
    { 
        board.whiteRooks &= ~(1ULL << move.from);
        board.whiteRooks |= 1ULL << move.to;
    } else if (board.whiteQueens & (1ULL << move.from)) 
    { 
        board.whiteQueens  &= ~(1ULL << move.from);
        board.whiteQueens  |= 1ULL << move.to;
    } else if (board.whiteKing & (1ULL << move.from)) 
    { 
        board.whiteKing  &= ~(1ULL << move.from);
        board.whiteKing  |= 1ULL << move.to;
    } else if (board.blackPawns & (1ULL << move.from))
    { 
        board.blackPawns &= ~(1ULL << move.from);
        board.blackPawns |= 1ULL << move.to;
    } else if (board.blackKnights & (1ULL << move.from)) 
    { 
        board.blackKnights &= ~(1ULL << move.from);
        board.blackKnights |= 1ULL << move.to;
    } else if (board.blackBishops & (1ULL << move.from)) 
    { 
        board.blackBishops &= ~(1ULL << move.from);
        board.blackBishops |= 1ULL << move.to;
    } else if (board.blackRooks & (1ULL << move.from)) 
    { 
        board.blackRooks &= ~(1ULL << move.from);
        board.blackRooks |= 1ULL << move.to;
    } else if (board.blackQueens & (1ULL << move.from)) 
    { 
        board.blackQueens &= ~(1ULL << move.from);
        board.blackQueens |= 1ULL << move.to;
    } else if (board.blackKing & (1ULL << move.from)) 
    { 
        board.blackKing &= ~(1ULL << move.from);
        board.blackKing |= 1ULL << move.to;
    }

    //Promotion Handler
    if (move.flags == PROMOTION) { 
        if (board.turn == WHITE) { 
            board.whitePawns &= ~(1ULL << move.to);
            switch (move.promoted) { 
            case PROMOTE_QUEEN: board.whiteQueens |= 1ULL << move.to; break;
            case PROMOTE_ROOK: board.whiteRooks |= 1ULL << move.to; break;
            case PROMOTE_BISHOP: board.whiteBishops |= 1ULL << move.to; break;
            case PROMOTE_KNIGHT: board.whiteKnights |= 1ULL << move.to; break;
            } 
        } else if (board.turn == BLACK) { 
            board.blackPawns &= ~(1ULL << move.to);
            switch (move.promoted) { 
            case PROMOTE_QUEEN: board.blackQueens |= 1ULL << move.to; break;
            case PROMOTE_ROOK: board.blackRooks |= 1ULL << move.to; break;
            case PROMOTE_BISHOP: board.blackBishops |= 1ULL << move.to; break;
            case PROMOTE_KNIGHT: board.blackKnights |= 1ULL << move.to; break;
            }
        }
    }



    //En Passant Handler
    board.enPassantSquare = 0; //This always sets it to 0 for valid move purposes...
    if (move.to - move.from == 16) //...unless a double push occurs, setting the enpassant square
    { 
        board.enPassantSquare = 1ULL << (move.to - 8);
    } else if (move.to - move.from == -16)
    { 
        board.enPassantSquare = 1ULL << (move.to + 8);
    }

    if (move.flags == EN_PASSANT) { 
        if (board.turn == WHITE) { 
            board.blackPawns &= ~(1ULL<<(move.to - 8));
        } else { 
            board.whitePawns &= ~(1ULL<<(move.to + 8));
        }
    }

    //Castling Handler
    if (move.flags == CASTLING) { 
        if (board.turn == WHITE) { 
            if (move.to == 6) { //Kingside
                board.whiteRooks &= ~(1ULL << 7); // clear rook from h1
                board.whiteRooks |= 1ULL << 5;    // set rook on f1
            } else if (move.to == 2) { //Queenside
                board.whiteRooks &= ~(1ULL << 0);
                board.whiteRooks |= 1ULL << 3;
            }
        } else if (board.turn == BLACK) { 
            if (move.to == 62) { //Kingside
                board.blackRooks &= ~(1ULL << 63);
                board.blackRooks |= 1ULL << 61;
            } else if (move.to == 58) { //Queenside
                board.blackRooks &= ~(1ULL << 56);
                board.blackRooks |= 1ULL << 59;
            }
        }
    }

    if (move.from == 4) { //King move from home square
        board.whiteCastleKingside = false;
        board.whiteCastleQueenside = false;
    }
    if (move.from == 7) { //Rook move from home square kingside
        board.whiteCastleKingside = false;
    }
    if (move.from == 0) { //Rook move from home square queenside
        board.whiteCastleQueenside = false; 
    }
    if (move.from == 60) { 
        board.blackCastleKingside = false;
        board.blackCastleQueenside = false;
    }
    if (move.from == 63) { 
        board.blackCastleKingside = false;
    }
    if (move.from == 56) { 
        board.blackCastleQueenside = false;
    }
    
board.turn = (board.turn == WHITE) ? BLACK : WHITE; //switch turn after every makeMove()
}

bool inCheck(const Board& board, Color side) {
    ull kings = (side == WHITE) ? board.whiteKing : board.blackKing;
    int kingSq = __builtin_ctzll(kings);// find the king's square

    ull pawnAttacks = (side == WHITE) ?
    ((1ULL << kingSq) << 7 & ~FILE_H) | 
    ((1ULL << kingSq) << 9 & ~FILE_A)
    :
    ((1ULL << kingSq) >> 7 & ~FILE_A) | 
    ((1ULL << kingSq) >> 9 & ~FILE_H);


    ull enemyBishopsAndQueens = (side == WHITE) ? 
    board.blackBishops | board.blackQueens : 
    board.whiteBishops | board.whiteQueens;

    ull enemyRooksAndQueens = (side == WHITE) ? 
    board.blackRooks | board.blackQueens : 
    board.whiteRooks | board.whiteQueens;

    ull enemyKnights = (side == WHITE) ? 
    board.blackKnights : board.whiteKnights;

    ull enemyPawns = (side == WHITE) ? 
    board.blackPawns : board.whitePawns;

    //Check if white king is attacked
    return (getRookAttacks(kingSq, board.occupied()) & enemyRooksAndQueens) ||
       (getBishopAttacks(kingSq, board.occupied()) & enemyBishopsAndQueens) ||
       (KNIGHT_ATTACKS[kingSq] & enemyKnights) ||
       (pawnAttacks & enemyPawns);
}

Board history[256]; // stack of board states
int historyIndex = 0;

void pushBoard(const Board& board) {
    history[historyIndex++] = board;
}

Board popBoard() {
    return history[--historyIndex];
}

ull getQueenAttacks(int sq, ull occupied) {
    
    return getRookAttacks(sq, occupied) | getBishopAttacks(sq, occupied);
}

void generateQueenMoves(const Board& board, std::vector<Move>& moves) { 
    ull queens = (board.turn == WHITE) ? board.whiteQueens : board.blackQueens;
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull ownPieces = (board.turn == WHITE) ? board.whitePieces()  :  board.blackPieces();
    while (queens) { 
        int sq = __builtin_ctzll(queens); //gets position of the first rook from 0
        queens &= queens - 1; //isolate rook
        ull targets = getQueenAttacks(sq, board.occupied()) & ~ownPieces; //retrieve legal rook moves
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

ull KING_ATTACKS[64];

void initKingAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        ull k = 1ULL << sq;
        KING_ATTACKS[sq] =  // 8 directions OR'd together
        k << 1 & ~FILE_H| 
        k >> 1 & ~FILE_A| 
        k << 8 | k >> 8 |   //Straight Moves

        k << 7 & ~FILE_A| 
        k >> 7 & ~FILE_H| 
        k << 9 & ~FILE_H| 
        k >> 9 & ~FILE_A;   //Diagonal Moves
    }
}

void generateKingMoves(const Board& board, std::vector<Move>& moves) { 
    ull kings = (board.turn == WHITE) ? board.whiteKing : board.blackKing;
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull ownPieces = (board.turn == WHITE) ? board.whitePieces() : board.blackPieces();
    while (kings) { 
        int sq = __builtin_ctzll(kings); //gets position of the king
        kings &= kings - 1; //isolate rook
        ull targets = KING_ATTACKS[sq] & ~ownPieces; //retrieve legal king moves
        while (targets) { 
            Move m; //create object m of Move struct
            m.from = sq;
            m.to = __builtin_ctzll(targets); //gets position of first target square from king square
            m.flags = (enemyPieces & (1ULL << m.to)) ? CAPTURE : QUIET; //mark move as a capture move or quiet move
            moves.push_back(m); //add to possible moves
            targets &= targets - 1;
        }
    }
}



void generateLegalMoves(Board& board, std::vector<Move>& moves) {
    std::vector<Move> pseudoLegal;
    Color sideToCheck = board.turn;
    generatePawnMoves(board, pseudoLegal);
    generateKnightMoves(board, pseudoLegal);
    generateBishopMoves(board, pseudoLegal);
    generateRookMoves(board, pseudoLegal);
    generateQueenMoves(board, pseudoLegal);
    generateKingMoves(board, pseudoLegal);
    for (auto& i : pseudoLegal) {
        pushBoard(board);
        makeMove(board, i);
        if (!inCheck(board, sideToCheck)) {
            moves.push_back(i);
        }
        board = popBoard();
    }
}