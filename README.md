# Minvalgen Chess Engine ♟️

**Minvalgen** is a high-performance C++ chess engine featuring a modern search engine, tapered evaluation, a 64MB transposition table, and standard UCI (Universal Chess Interface) protocol support. It plays natively in chess GUIs like Arena and CuteChess.

---

## Features

- **UCI Protocol Compliant**: Plug-and-play with standard chess GUIs.
- **Advanced Search Engine**:
  - Negamax Alpha-Beta with Principal Variation Search (PVS) & Iterative Deepening
  - Quiescence Search (QSearch) with Delta Pruning
  - Null Move Pruning (NMP) & Static Null Move Pruning
  - Late Move Reductions (LMR) & Check Extensions
  - Advanced Move Ordering (Transposition Table, MVV-LVA, Killers, History, Countermoves)
  - Full Draw Detection
- **Tapered Evaluation System**: Smooth Midgame/Endgame phase interpolation with Piece-Square Tables and positional bonuses.

---

## Building

Requires a C++17 compatible compiler (`g++`, `clang++`, or `MSVC`).

```bash
# Build using Makefile
make
```
This produces `Minvalgen.exe`.

---

## Usage

1. **GUI Integration**: Load `Minvalgen.exe` into any UCI-compliant GUI (e.g., Arena).
2. **Perft Testing**: Run `Minvalgen.exe` in your terminal and type `perft <depth>` to run performance tests.

---

## License

[MIT License](LICENSE)
