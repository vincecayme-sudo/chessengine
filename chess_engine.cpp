#include <iostream>
#include <cstdint> 
#include <sstream>
#include <fstream>

std::ofstream engineLog("engine_log.txt", std::ios::app);

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

void generatePawnMoves(const Board& board, std::vector<Move>& moves) { 
    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
    ull singlePush = (board.turn == WHITE) ?  (board.whitePawns << 8) & ~board.occupied()  : (board.blackPawns >> 8) & ~board.occupied();
    ull doublePush = (board.turn == WHITE) ?
    ((((0xFF00 & board.whitePawns) << 8) & ~board.occupied()) << 8) & ~board.occupied() :
    ((((0xFF000000000000 & board.blackPawns) >> 8) & ~board.occupied()) >> 8) & ~board.occupied();
    engineLog << "Black double push bb: " << doublePush << "\n";
    engineLog << "Black pawns: " << board.blackPawns << "\n";
    ull leftCapture = (board.turn == WHITE) ? (board.whitePawns << 7) & enemyPieces  & ~FILE_A : (board.blackPawns >> 7) & enemyPieces & ~FILE_H;
    ull rightCapture = (board.turn == WHITE) ? (board.whitePawns << 9) & enemyPieces &~FILE_H : (board.blackPawns >> 9) & enemyPieces & ~FILE_A;

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
        int from = (board.turn == WHITE) ? to - 15 : to + 15;
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
        ull targets = getQueenAttacks(sq, board.occupied()); //retrieve legal rook moves
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

int minimax(Board& board, int depth) {
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
            int score = minimax(board, depth - 1);// recurse here
            board = popBoard();
            // update best if score is better
            if (score > best) best = score;
        }
        return best;
    } else { // if black's turn, find minimum score
        int best = 99999; 
        for (auto& move : legalMoves) { 
            pushBoard(board);
            makeMove(board, move);
            int score = minimax(board, depth - 1);
            board = popBoard();
            if (score < best) best = score;
        }
        return best;
    }
    
}



std::string squareToString(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string({file, rank});
}

int stringToSquare(const std::string& s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

Move getBestMove(Board& board, int depth) {
    std::vector<Move> legalMoves;
    generateLegalMoves(board, legalMoves);

    engineLog << "Legal moves: ";
    for (auto& m : legalMoves) { 
        engineLog << squareToString((int)m.from) << squareToString((int)m.to) << " ";
    }
    engineLog << '\n' << std::flush;

    if (legalMoves.empty()) return Move{};


    Move bestMove = legalMoves[0];
    int bestScore = (board.turn == WHITE) ? -99999 : 99999; 
    // loop through moves, call minimax, track best
    for (auto& move : legalMoves) {
        
        pushBoard(board);
        makeMove(board, move);
        int score = minimax(board, depth - 1);
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

int main() {
    initKnightAttacks();
    initKingAttacks();
    initRookMagics();
    initBishopMagics();

    Board board;
    std::string line;

    
    while (std::getline(std::cin, line)) {
        engineLog << "IN: " << line << "\n" << std::flush;
        if (line == "uci") {
            std::cout << "id name Bean\n";
            std::cout << "id author Vince\n";
            std::cout << "uciok\n" << std::flush;

        } else if (line == "isready") {
            std::cout << "readyok\n" << std::flush;

        } else if (line == "ucinewgame") {
            board = Board();

        } else if (line.substr(0, 8) == "position") {
            board = Board();
            size_t movesPos = line.find("moves");
            if (movesPos != std::string::npos) {
                std::istringstream ss(line.substr(movesPos + 6));
                std::string moveStr;
                while (ss >> moveStr) {
                    Move m;
                    m.from = stringToSquare(moveStr.substr(0, 2));
                    m.to   = stringToSquare(moveStr.substr(2, 2));
                    ull fromBit = 1ULL << m.from;
                    ull toBit   = 1ULL << m.to;
                    ull enemyPieces = (board.turn == WHITE) ? board.blackPieces() : board.whitePieces();
                    if ((board.whiteKing & fromBit || board.blackKing & fromBit) && abs(m.to - m.from) == 2) {
                        m.flags = CASTLING;
                    } else if ((board.whitePawns & fromBit || board.blackPawns & fromBit) && toBit == board.enPassantSquare) {
                        m.flags = EN_PASSANT;
                    } else if ((board.whitePawns & fromBit || board.blackPawns & fromBit) && (toBit & RANK_8 || toBit & RANK_1)) {
                        m.flags = PROMOTION;
                        if (moveStr.length() == 5) {
                            switch (moveStr[4]) {
                                case 'q': m.promoted = PROMOTE_QUEEN;  break;
                                case 'r': m.promoted = PROMOTE_ROOK;   break;
                                case 'b': m.promoted = PROMOTE_BISHOP; break;
                                case 'n': m.promoted = PROMOTE_KNIGHT; break;
                            }
                        }
                    } else if (toBit & enemyPieces) {
                        m.flags = CAPTURE;
                    } else {
                        m.flags = QUIET;
                    }
                    makeMove(board, m);
                }
            }

        } else if (line.substr(0, 2) == "go") {
            Move best = getBestMove(board, 1    );
            std::string result = "bestmove " + squareToString(best.from) + squareToString(best.to);
            if (best.flags == PROMOTION) {
                switch (best.promoted) {
                    case PROMOTE_QUEEN:  result += 'q'; break;
                    case PROMOTE_ROOK:   result += 'r'; break;
                    case PROMOTE_BISHOP: result += 'b'; break;
                    case PROMOTE_KNIGHT: result += 'n'; break;
                }
            }
            std::cout << result << "\n" << std::flush;
            engineLog << "OUT: " << result << "\n" << std::flush;

            

        } else if (line == "quit") {
            return 0;
        }
        
    }
    return 0;
}