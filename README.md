# MyDB

A small C++ database built step by step to explore parsing, paging, persistence,
cursors, and B-trees.

## Project structure

- `src/main.cpp` runs the REPL loop.
- `src/repl.cpp` handles meta commands and statement parsing.
- `src/database.cpp` handles execution, rows, pages, persistence, and cursors.
- `src/btree.cpp` contains the B-tree node, search, and insertion logic.
- `src/mydb.hpp` contains shared types, constants, and function declarations.

## Build

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp src/repl.cpp src/database.cpp src/btree.cpp -o mydb.exe
```

## Run

```powershell
./mydb.exe
```
