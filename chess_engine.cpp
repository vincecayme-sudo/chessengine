#include <iostream>
#include <cstdint> 

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
        board.enPassantSquare = 1ULL << (move.to >> 8);
    } else if (move.to - move.from == -16)
    { 
        board.enPassantSquare = 1ULL << (move.to << 8);
    }

    if (move.flags == EN_PASSANT) { 
        if (board.turn == WHITE) { 
            board.blackPawns &= ~(1ULL<<(move.to>>8));
        } else { 
            board.whitePawns &= ~(1ULL<<(move.to<<8));
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

struct MagicEntry { 
	ull mask;
	ull magic;
	int shift;
  //point to precomputed table of attack bbs for the corresponding square
	ull* attacks; 

    
};

MagicEntry rookMagics[64];
MagicEntry bishopMagics[64];

ull computeRookAttacks(int sq, ull occupied) { 
    int current = sq + 1; 
    ull attacks = 0;
    while (current < 64 && ((1ULL << current) & ~FILE_H)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current++;
    }
    current = sq - 1; //declare current as int to prevent underflow (sq(as 0) - 1)
    while (current < 64 && current >=0 && ((1ULL << current) & ~FILE_A)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current--;
    }
    current = sq + 8;
    while (current < 64) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current += 8;
    }
    current = sq - 8;
    while (current < 64 && current >= 0) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current -= 8;
    }
    return attacks;
}

ull computeBishopAttacks(int sq, ull occupied) { 
    int current = sq + 7; //diagonal left up
    ull attacks = 0;
    while (current < 64 && current >=0 && ((1ULL << current) & ~FILE_A)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current += 7;
    }

    current = sq + 9; //diagonal right up
    while (current < 64 && current >= 0 && ((1ULL << current) & ~FILE_H)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current += 9;
    }

    current = sq - 7; //diagonal right down
    while (current < 64 && current >= 0 && ((1ULL << current) & ~FILE_H)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current -= 7;
    }
    current = sq - 9; //diagonal left down
    while (current < 64 && current >= 0 && ((1ULL <<current) & ~FILE_A)) { 
        attacks |= 1ULL << current;
        if (occupied & (1ULL << current)) { 
            break;
        }
        current -= 9;
    }
    return attacks;
}

ull computeRookMask(int sq) { 
    ull edges = RANK_1 | RANK_8 | FILE_A | FILE_H;
    ull mask = computeRookAttacks(sq, 0) & ~edges;
    return mask;
}

ull computeBishopMask(int sq) { 
    ull edges = RANK_1 | RANK_8 | FILE_A | FILE_H;
    ull mask = computeBishopAttacks(sq, 0) & ~edges;
    return mask;
}

static ull rookAttackTable[64 * 4096]; // worse case size
static int offset = 0;

void initRookMagics() { 
    for (int sq = 0; sq < 64; sq++) { 
        ull mask = computeRookMask(sq);
        int popcount = __builtin_popcountll(mask);
        int shift = 64 - popcount;
        int size = 1 << popcount;

        rookMagics[sq].mask = mask;
        rookMagics[sq].shift = shift;
        rookMagics[sq].attacks = rookAttackTable + offset;
        offset += size;

        ull subset = 0;
        do { 
            subset = (subset - mask) & mask;
            int index = (subset * rookMagics[sq].magic) >> shift;
            rookAttackTable[subset] = subset;
        } while (subset !=0);

        
    }
}