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

The next major improvement is adding basic move ordering. Currently if the engine checks 'bad' moves first, there is less pruning. So I need to add a way to make it look for "better" moves first which looks something like looking for forcing moves first, or captures, etc.

## Build

Compile `engine.cpp` with a C++ compiler:

```bash
g++ src/engine.cpp -o bean
