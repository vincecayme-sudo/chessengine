#include "setup.h"
#include "rook.h"      // no dependencies beyond setup
#include "bishop.h"    // depends on rook
#include "pawn.h"      // depends on setup
#include "knight.h"    // depends on setup
#include "magics.h"    // depends on rook and bishop
#include "movehandler.h" // depends on all above
#include "minimax.h"   // depends on movehandler

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


int main() {
    initKnightAttacks();
    initKingAttacks();
    initRookMagics();
    initBishopMagics();

    Board board;
    std::string line;

    
    while (std::getline(std::cin, line)) {
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
            Move best = getBestMove(board, 2); //Set engine depth here
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

            

        } else if (line == "quit") {
            return 0;
        }
        
    }
    return 0;
}