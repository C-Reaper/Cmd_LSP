# Project README

## Overview
- A simple C program that reads a file, compresses its contents using Zip library, decompresses it, and writes the decompressed data back to a new file.

## Features
- Reading and writing files
- Compressing and decompressing byte data using a hypothetical Zip library (`Zip_Byte_zip` and `Zip_Byte_open`)
- Basic error handling through printf statements

## Project Structure
### Prerequisites
- C/C++ Compiler (GCC)
- Make utility
- Standard development tools

## Build & Run
### Linux
1. Navigate to the project directory.
2. Build the project using:
   ```sh
   make -f Makefile.linux all
   ```
3. Execute the program with:
   ```sh
   make -f Makefile.linux exe
   ```

### Windows
1. Navigate to the project directory.
2. Build the project using:
   ```sh
   make -f Makefile.windows all
   ```
3. Execute the program with:
   ```sh
   make -f Makefile.windows exe
   ```

### Wine (Linux cross-compile for Windows)
1. Navigate to the project directory.
2. Build the project using:
   ```sh
   make -f Makefile.wine all
   ```
3. Execute the program within a Wine environment with:
   ```sh
   make -f Makefile.wine exe
   ```

### Web (Emscripten)
1. Navigate to the project directory.
2. Build the project using:
   ```sh
   make -f Makefile.web all
   ```
3. Execute the program using Emscripten's runtime with:
   ```sh
   wasmtime ./build/Main.wasm
   ```

---

### Project Organization:
```
Project/
├── build/              # .exe files produced by Main.c
├── src/
│   ├── Main.c          # Entry point
│   └── *.h             # Standalone Header-based C-files, without *.c files that implement it
├── Makefile.linux      # Linux Build configuration
├── Makefile.windows    # Windows Build configuration
├── Makefile.wine       # Wine Build configuration
├── Makefile.web        # Emscripten Build configuration
└── README.md           # This file
```