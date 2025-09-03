# Memory Management with Paging and Swap

## Overview

This project is a simulation of an **Operating System’s Memory Management Unit** implementing **paging** with both **main memory** and **swap memory**, along with **page replacement algorithms** (LRU and FIFO).

The system loads executable files, executes them instruction by instruction, manages logical–to–physical address translation, and swaps pages in and out of main memory using the selected replacement algorithm.

---

## Features

* **Two Page Replacement Policies**:

  * **LRU** (Least Recently Used)
  * **FIFO** (First-In First-Out)
* **Process Management**:

  * Load multiple executables into main or swap memory
  * Assign unique process IDs (PID)
  * Kill processes and reclaim memory
* **Address Translation**:

  * Logical → Physical mapping with page tables
  * Invalid address detection and error reporting
* **Execution Engine**:

  * Supports commands: `add`, `sub`, `print`, and `load`
  * On-demand page swapping during execution
* **System Commands**:

  * `load`, `run`, `kill`, `listpr`, `pte`, `pteall`, `print`, `exit`
* **Logging**:

  * Page table information output to files with timestamps

---

## Build Instructions

```bash
# Clean old builds
make clean

# Build LRU version
make lru

# Build FIFO version
make fifo
```

---

## Usage

```bash
./lru  -M <main_memory_KB> -V <swap_memory_KB> -P <page_size_bytes> -i <infile> -o <outfile>
./fifo -M <main_memory_KB> -V <swap_memory_KB> -P <page_size_bytes> -i <infile> -o <outfile>
```

Example:

```bash
./lru -M 32 -V 32 -P 512 -i infile -o outfile
```

---

## Input Files

### Executable File Format

1. **Size in KB** (used for paging calculation)
2. **List of commands** (one per line):

   * `load a, (y)`
   * `add (x), (y), (z)`
   * `sub (x), (y), (z)`
   * `print (x)`

### Infile Format

Example:

```
load file1 file2 file3
run 1
pteall outfile1
pte 2 outfile1
load file4
listpr
run 2
kill 1
exit
```

---

## Output

* All command results and error messages are printed to the console.
* Page table dumps are written/appended to specified output files.

---

## Known Limitations

* No concurrency — execution is strictly sequential.
* PID values are never reused after process termination.
* Swap space simulation is in-memory only (not backed by disk).
* No persistent data after program exit.
* Address bounds checking is limited to the process’s declared size.

---

## Possible Extensions

* Implement **Clock** replacement algorithm for performance comparison.
* Add **process scheduling** simulation to combine with paging.
* Integrate a **disk-backed swap simulation** for realism.

---
