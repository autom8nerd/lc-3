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
uint16_t sign_extend(uint16_t, int);
void update_flags(uint16_t);
void add(uint16_t instr);
uint16_t mem_read(uint16_t);
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
  if (argc < 2) {
    printf("lc2 [image-file1] ...\n");
    exit(2);
  }
  for (int j = 0; j < argc; j++) {
    if (!read_image(argv[j])) {
      printf("failed to load image: %s \n", argv[j]);
      exit(1);
    }
  }

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
      add(instr);
      break;
    case OP_AND: {
      {
        uint16_t r0 = (instr >> 9) & 0x7;
        uint16_t r1 = (instr >> 6) & 0x7;
        uint16_t imm_flag = (instr >> 5) & 0x1;

        if (imm_flag) {
          uint16_t imm5 = sign_extend(instr & 0x1F, 5);
          reg[r0] = reg[r1] & imm5;
        } else {
          uint16_t r2 = instr & 0x7;
          reg[r0] = reg[r1] & reg[r2];
        }
        update_flags(r0);
      }
    } break;
    case OP_NOT: {
      {
        uint16_t r0 = (instr >> 9) & 0x7;
        uint16_t r1 = (instr >> 6) & 0x7;

        reg[r0] = ~reg[r1];
        update_flags(r0);
      }
    } break;
    case OP_BR: {
      {
        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
        uint16_t cond_flag = (instr >> 9) & 0x7;
        if (cond_flag & reg[R_COND]) {
          reg[R_PC] += pc_offset;
        }
      }
    } break;
    case OP_JMP: {
      uint16_t r1 = (instr >> 6) & 0x7;
      reg[R_PC] = reg[r1];
    } break;
    case OP_JSR: {
      {
        uint16_t long_flag = (instr >> 11) & 1;
        reg[R_R7] = reg[R_PC];
        if (long_flag) {
          uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
          reg[R_PC] += long_pc_offset; /* JSR */
        } else {
          uint16_t r1 = (instr >> 6) & 0x7;
          reg[R_PC] = reg[r1]; /* JSRR */
        }
      }
    } break;
    case OP_LD: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
      reg[r0] = mem_read(reg[R_PC] + pc_offset);
      update_flags(r0);
    } break;
    case OP_LDI: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t offset = sign_extend(instr & 0x3F, 6);
      reg[r0] = mem_read(reg[r1] + offset);
      update_flags(r0);
    } break;
    case OP_LDR: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t offset = sign_extend(instr & 0x3F, 6);
      reg[r0] = mem_read(reg[r1] + offset);
      update_flags(r0);
    } break;
    case OP_LEA: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
      reg[r0] = reg[R_PC] + pc_offset;
      update_flags(r0);
    } break;
    case OP_ST: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
      mem_write(reg[R_PC] + pc_offset, reg[r0]);
    } break;
    case OP_STI: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
      mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
    } break;
    case OP_STR: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t offset = sign_extend(instr & 0x3F, 6);
      mem_write(reg[r1] + offset, reg[r0]);
    } break;
    case OP_TRAP:
      @{ TRAP } break;
    case OP_RES:
    case OP_RTI:
    default:
      @{ BAD OPCODE } break;
    }
  }
}
void update_flags(uint16_t r) {
  if (reg[r] == 0) {
    reg[R_COND] = FL_ZRO;
  } else if (reg[r] >> 15) {
    reg[R_COND] = FL_NEG;
  } else {
    reg[R_COND] = FL_POS;
  }
}
uint16_t sign_extend(uint16_t x, int bit_count) {
  if ((x >> (bit_count - 1) & 1)) {
    x |= (0xFFFF << bit_count);
  }
  return x;
}

void add(uint16_t instr) {
  uint16_t destination_r = (instr >> 9) & 0x7;
  uint16_t source1_r = (instr >> 6) & 0x7;
  uint16_t flag = (instr >> 5) & 0x1;
  if (flag != 1) {
    uint16_t source2_r = instr & 0x7;
    reg[destination_r] = reg[source2_r] + reg[source1_r];
  } else {
    uint16_t imm_value = sign_extend(instr & 0x1F, 5);
    reg[destination_r] = reg[source1_r] + imm_value;
  }
  update_flags(destination_r);
}
