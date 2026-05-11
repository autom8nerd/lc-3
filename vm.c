#include <signal.h>
#include <stdint.h>
#include <stdio.h>
/* unix only */
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// defining memory locations
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];

// registers; lc-3 has total of 10 registers. 8 GPCs, 1 PC, 1 conditional flag
enum {
  R_R0 = 0,
  R_R1,
  R_R2,
  R_R3,

  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC,
  R_COND,
  R_COUNT
};

// register storage
uint16_t reg[R_COUNT];

// total 16 opcodes in lc-3. the left most 4 bits store 4 bit opcode, and the
// rest 12 bits contain the parameters gotta keep the enum below in order to
// preserve the number assigned to the opcode
enum {
  OP_BR = 0, /* branch */
  OP_ADD,    /* add  */
  OP_LD,     /* load */
  OP_ST,     /* store */
  OP_JSR,    /* jump register */
  OP_AND,    /* bitwise and */
  OP_LDR,    /* load register */
  OP_STR,    /* store register */
  OP_RTI,    /* unused */
  OP_NOT,    /* bitwise not */
  OP_LDI,    /* load indirect */
  OP_STI,    /* store indirect */
  OP_JMP,    /* jump */
  OP_RES,    /* reserved (unused) */
  OP_LEA,    /* load effective address */
  OP_TRAP    /* execute trap */
};

enum {
  FL_POS =
      1 << 0, // P you set the respective bit with 1 << 0. in this, it is 001
  FL_ZRO = 1 << 1, // Z 010
  FL_NEG = 1 << 2  // N 100
};

int main(int argc, const char *argv[]) {

  reg[R_COND] = FL_ZRO;
  // pc start address at 0x4600
  enum { PC_START = 0x4600 };
  reg[R_PC] = PC_START;

  int going = 1;
  while (going) {
    uint16_t instr = mem_read(reg[R_PC]++);
    uint16_t op =
        instr >>
        12; // op contains the opcode; the upper 4 bits out of the 16 bits
    switch (op) {
    case OP_ADD:
      @{ ADD } break;
    case OP_AND:
      @{ AND } break;
    case OP_NOT:
      @{ NOT } break;
    case OP_BR:
      @{ BR } break;
    case OP_JMP:
      @{ JMP } break;
    case OP_JSR:
      @{ JSR } break;
    case OP_LD:
      @{ LD } break;
    case OP_LDI:
      @{ LDI } break;
    case OP_LDR:
      @{ LDR } break;
    case OP_LEA:
      @{ LEA } break;
    case OP_ST:
      @{ ST } break;
    case OP_STI:
      @{ STI } break;
    case OP_STR:
      @{ STR } break;
    case OP_TRAP:
      @{ TRAP } break;
    case OP_RES:
    case OP_RTI:
    default:
      @{ BAD OPCODE } break;
    }
  }
}
}
