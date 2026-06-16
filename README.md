# chessengine
This is my first pet project meant to develop my skills as a C++ programmer.

# Bean Chess Engine

Bean is a simple chess engine written from scratch in C++.

It uses bitboards for board representation, generates legal chess moves, searches positions with minimax, and communicates with chess GUIs through the UCI protocol. It currently runs with CuteChess and supports normal moves, captures, promotion, en passant, and castling.

## Features

- Bitboard-based board representation
- Legal move generation for all pieces
- Check, checkmate, and stalemate detection
- Basic material evaluation
- Minimax search with alpha-beta pruning
- UCI support for chess GUI compatibility

## Current Goal

The next major improvement is replacing plain minimax with alpha-beta pruning, allowing deeper search at better speed.

## Build

Compile `engine.cpp` with a C++ compiler:

```bash
g++ src/engine.cpp -o bean