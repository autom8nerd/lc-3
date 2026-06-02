# LC-3 Virtual Machine

A simple lc-3 vm made by following the [blog](https://www.jmeiners.com/lc3-vm/) by Justin Meiners and Ryan Pendleton.

## Getting Started

### Dependencies

Electricity and an OS installed (preferably Linux idk about Windows)

## 🧠 How It Works

![LC-3 Instruction Format](/assets/gh.png)

This project implements the CPU execution loop (Fetch, Decode, Execute). It includes:
* **64KB Memory Array:** Simulating the 16-bit address space of the LC-3.
* **10 CPU Registers:** 8 general-purpose registers (R0-R7), a Program Counter (PC), and a Condition Flags register (COND).
* **Complete Instruction Set:** Implements all core LC-3 opcodes (ADD, AND, JMP, BR, LD, LDI, LDR, ST, STI, STR, etc.).
* **Memory-Mapped I/O:** Simulates keyboard status and data registers for handling asynchronous terminal input.
* **Trap Routines:** Simulates basic OS-level system calls for console I/O and program halting.

## ⚙️ A Note on the Assembler

This repository contains the **Virtual Machine** (the processor), not an assembler. 

Because of this, the VM does not read human-readable assembly text (e.g., `ADD R0, R0, 1`). Instead, it executes raw 16-bit binary instructions directly from memory. To run programs on this VM, you must pass pre-assembled `.obj` files containing the compiled machine code, which the VM will load into its virtual memory array starting at the origin address (usually `0x3000`).

## 🚀 Building and Running

Compile the VM using any standard C compiler (like GCC):

```bash
gcc -O3 lc3.c -o lc3-vm
```

Run the VM by passing an assembled LC-3 object file as an argument:

```bash
./lc3-vm path/to/program.obj
```

## Authors
Me, i guess?

## Acknowledgments
I followed them, so credit goes to them (i copied the whole thing but ok T-T)

[Ryan Pendleton](https://github.com/rpendleton)

[Justin Meiners](https://github.com/justinmeiners)
