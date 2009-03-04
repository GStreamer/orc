
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>

#ifdef HAVE_AMD64
int x86_64 = 1;
int x86_ptr_size = 8;
int x86_exec_ptr = X86_EDI;
#else
int x86_64 = 0;
int x86_ptr_size = 4;
int x86_exec_ptr = X86_EBP;
#endif


const char *
x86_get_regname(int i)
{
  static const char *x86_regs[] = {
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 16) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      printf("register %d\n", i);
      return "ERROR";
  }
}

int
x86_get_regnum(int i)
{
  return (i&0xf);
}

const char *
x86_get_regname_8(int i)
{
  static const char *x86_regs[] = { "al", "cl", "dl", "bl",
    "ah", "ch", "dh", "bh" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 8) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
x86_get_regname_16(int i)
{
  static const char *x86_regs[] = { "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 8) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
x86_get_regname_64(int i)
{
  static const char *x86_regs[] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 16) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
x86_get_regname_ptr(int i)
{
  if (x86_64) {
    return x86_get_regname_64 (i);
  } else {
    return x86_get_regname (i);
  }
}

const char *
x86_get_regname_mmx(int i)
{
  static const char *x86_regs[] = { "mm0", "mm1", "mm2", "mm3",
    "mm4", "mm5", "mm6", "mm7" };

  if (i>=X86_MM0 && i<X86_MM0 + 8) return x86_regs[i - X86_MM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
x86_get_regname_sse(int i)
{
  static const char *x86_regs[] = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };

  if (i>=X86_XMM0 && i<X86_XMM0 + 16) return x86_regs[i - X86_XMM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

void
x86_emit_push (OrcProgram *program, int size, int reg)
{

  if (size == 1) {
    program->error = 1;
  } else if (size == 2) {
    printf("  pushw %%%s\n", x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0x50 + x86_get_regnum(reg);
  } else {
    printf("  pushl %%%s\n", x86_get_regname(reg));
    *program->codeptr++ = 0x50 + x86_get_regnum(reg);
  }
}

void
x86_emit_pop (OrcProgram *program, int size, int reg)
{

  if (size == 1) {
    program->error = 1;
  } else if (size == 2) {
    printf("  popw %%%s\n", x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0x58 + x86_get_regnum(reg);
  } else {
    printf("  popl %%%s\n", x86_get_regname(reg));
    *program->codeptr++ = 0x58 + x86_get_regnum(reg);
  }
}

#define X86_MODRM(mod, rm, reg) ((((mod)&3)<<6)|(((rm)&7)<<0)|(((reg)&7)<<3))
#define X86_SIB(ss, ind, reg) ((((ss)&3)<<6)|(((ind)&7)<<3)|((reg)&7))

void
x86_emit_modrm_memoffset (OrcProgram *program, int reg1, int offset, int reg2)
{
  if (offset == 0 && reg2 != x86_exec_ptr) {
    if (reg2 == X86_ESP) {
      *program->codeptr++ = X86_MODRM(0, 4, reg1);
      *program->codeptr++ = X86_SIB(0, 4, reg2);
    } else {
      *program->codeptr++ = X86_MODRM(0, reg2, reg1);
    }
  } else if (offset >= -128 && offset < 128) {
    *program->codeptr++ = X86_MODRM(1, reg2, reg1);
    if (reg2 == X86_ESP) {
      *program->codeptr++ = X86_SIB(0, 4, reg2);
    }
    *program->codeptr++ = (offset & 0xff);
  } else {
    *program->codeptr++ = X86_MODRM(2, reg2, reg1);
    if (reg2 == X86_ESP) {
      *program->codeptr++ = X86_SIB(0, 4, reg2);
    }
    *program->codeptr++ = (offset & 0xff);
    *program->codeptr++ = ((offset>>8) & 0xff);
    *program->codeptr++ = ((offset>>16) & 0xff);
    *program->codeptr++ = ((offset>>24) & 0xff);
  }
}

void
x86_emit_modrm_reg (OrcProgram *program, int reg1, int reg2)
{
  *program->codeptr++ = X86_MODRM(3, reg1, reg2);
}

void
x86_emit_rex (OrcProgram *program, int size, int reg1, int reg2, int reg3)
{
  int rex = 0x40;

  if (x86_64) {
    if (size >= 8) rex |= 0x08;
    if (reg1 == 1 || (x86_get_regnum(reg1)>=8)) rex |= 0x4;
    if (reg2 == 1 || (x86_get_regnum(reg2)>=8)) rex |= 0x2;
    if (reg3 == 1 || (x86_get_regnum(reg3)>=8)) rex |= 0x1;

    if (rex != 0x40) *program->codeptr++ = rex;
    //*program->codeptr++ = rex;
  }
}

void
x86_emit_mov_memoffset_reg (OrcProgram *program, int size, int offset,
    int reg1, int reg2)
{

  switch (size) {
    case 1:
      printf("  movb %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_8(reg2));
      *program->codeptr++ = 0x8a;
      x86_emit_modrm_memoffset (program, reg2, offset, reg1);
      return;
    case 2:
      printf("  movw %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_16(reg2));
      *program->codeptr++ = 0x66;
      break;
    case 4:
      printf("  movl %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname(reg2));
      break;
    case 8:
      printf("  mov %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_64(reg2));
      break;
    default:
      printf("ERROR\n");
      break;
  }

  x86_emit_rex(program, size, reg2, 0, reg1);
  *program->codeptr++ = 0x8b;
  x86_emit_modrm_memoffset (program, reg2, offset, reg1);
}

void
x86_emit_mov_memoffset_mmx (OrcProgram *program, int size, int offset,
    int reg1, int reg2)
{
  if (size == 4) {
    printf("  movd %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
        x86_get_regname_mmx(reg2));
    x86_emit_rex(program, 0, reg2, 0, reg1);
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x6e;
  } else {
    printf("  movq %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
        x86_get_regname_mmx(reg2));
    x86_emit_rex(program, 0, reg2, 0, reg1);
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x6f;
  }
  x86_emit_modrm_memoffset (program, reg2, offset, reg1);
}

void
x86_emit_mov_memoffset_sse (OrcProgram *program, int size, int offset,
    int reg1, int reg2)
{
  switch (size) {
    case 4:
      printf("  movd %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg2, 0, reg1);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x6e;
      break;
    case 8:
      printf("  movq %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      *program->codeptr++ = 0xf3;
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x7e;
      break;
    case 16:
      printf("  movdqu %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      x86_emit_rex(program, 0, reg2, 0, reg1);
      *program->codeptr++ = 0xf3;
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x6f;
      break;
    default:
      printf("ERROR\n");
      break;
  }
  x86_emit_modrm_memoffset (program, reg2, offset, reg1);
}

void
x86_emit_mov_reg_memoffset (OrcProgram *program, int size, int reg1, int offset,
    int reg2)
{
  switch (size) {
    case 1:
      printf("  movb %%%s, %d(%%%s)\n", x86_get_regname_8(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x88;
      x86_emit_modrm_memoffset (program, reg1, offset, reg2);
      return;
    case 2:
      printf("  movw %%%s, %d(%%%s)\n", x86_get_regname_16(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      break;
    case 4:
      printf("  movl %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
          x86_get_regname_ptr(reg2));
      break;
    case 8:
      printf("  mov %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
          x86_get_regname_ptr(reg2));
      break;
    default:
      printf("ERROR\n");
      break;
  }

  x86_emit_rex(program, size, reg1, 0, reg2);
  *program->codeptr++ = 0x89;
  x86_emit_modrm_memoffset (program, reg1, offset, reg2);
}

void
x86_emit_mov_mmx_memoffset (OrcProgram *program, int size, int reg1, int offset,
    int reg2)
{
  x86_emit_rex(program, 0, reg1, 0, reg2);
  if (size == 4) {
    printf("  movd %%%s, %d(%%%s)\n", x86_get_regname_mmx(reg1), offset,
        x86_get_regname_ptr(reg2));
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x7e;
  } else if (size == 8) {
    printf("  movq %%%s, %d(%%%s)\n", x86_get_regname_mmx(reg1), offset,
        x86_get_regname_ptr(reg2));
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x7f;
  }

  x86_emit_modrm_memoffset (program, reg1, offset, reg2);
}

void
x86_emit_mov_sse_memoffset (OrcProgram *program, int size, int reg1, int offset,
    int reg2, int aligned)
{
  switch (size) {
    case 4:
      printf("  movd %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg1, 0, reg2);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x7e;
      break;
    case 8:
      printf("  movq %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg1, 0, reg2);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0xd6;
      break;
    case 16:
      if (aligned) {
        printf("  movdqa %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
            x86_get_regname_ptr(reg2));
        *program->codeptr++ = 0x66;
        *program->codeptr++ = 0x0f;
        *program->codeptr++ = 0x7f;
      } else {
        printf("  movdqu %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
            x86_get_regname_ptr(reg2));
        *program->codeptr++ = 0xf3;
        *program->codeptr++ = 0x0f;
        *program->codeptr++ = 0x7f;
      }
      break;
    default:
      printf("ERROR\n");
  }

  x86_emit_modrm_memoffset (program, reg1, offset, reg2);
}

void
x86_emit_mov_imm_reg (OrcProgram *program, int size, int value, int reg1)
{
  if (size == 2) {
    printf("  movw $%d, %%%s\n", value, x86_get_regname_16(reg1));
    x86_emit_rex(program, size, reg1, 0, 0);
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0xb8 + x86_get_regnum(reg1);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
  } else if (size == 4) {
    printf("  movl $%d, %%%s\n", value, x86_get_regname(reg1));
    x86_emit_rex(program, size, reg1, 0, 0);
    *program->codeptr++ = 0xb8 + x86_get_regnum(reg1);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    *program->codeptr++ = ((value>>16) & 0xff);
    *program->codeptr++ = ((value>>24) & 0xff);
  } else {
    /* FIXME */
  }

}

void x86_emit_mov_reg_reg (OrcProgram *program, int size, int reg1, int reg2)
{
  if (size == 2) {
    printf("  movw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  movl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    printf("  mov %%%s, %%%s\n", x86_get_regname_64(reg1),
        x86_get_regname_64(reg2));
  }

  x86_emit_rex(program, size, reg2, 0, reg1);
  *program->codeptr++ = 0x89;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void x86_emit_mov_sse_reg_reg (OrcProgram *program, int reg1, int reg2)
{
  printf("  movdqa %%%s, %%%s\n", x86_get_regname_sse(reg1),
        x86_get_regname_sse(reg2));

  *program->codeptr++ = 0x66;
  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6f;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_mmx_reg_reg (OrcProgram *program, int reg1, int reg2)
{
  printf("  movq %%%s, %%%s\n", x86_get_regname_mmx(reg1),
        x86_get_regname_mmx(reg2));

  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6f;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_reg_mmx (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  printf("  movd %%%s, %%%s\n", x86_get_regname(reg1),
      x86_get_regname_mmx(reg2));
  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6e;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_mmx_reg (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  printf("  movd %%%s, %%%s\n", x86_get_regname_mmx(reg1),
      x86_get_regname(reg2));
  x86_emit_rex(program, 0, reg2, 0, reg1);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x7e;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void x86_emit_mov_reg_sse (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  printf("  movd %%%s, %%%s\n", x86_get_regname(reg1),
      x86_get_regname_sse(reg2));
  *program->codeptr++ = 0x66;
  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6e;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_sse_reg (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  printf("  movd %%%s, %%%s\n", x86_get_regname_sse(reg1),
      x86_get_regname(reg2));
  *program->codeptr++ = 0x66;
  x86_emit_rex(program, 0, reg2, 0, reg1);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x7e;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void
x86_emit_test_reg_reg (OrcProgram *program, int size, int reg1, int reg2)
{
  if (size == 2) {
    printf("  testw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  testl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    printf("  test %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  }

  x86_emit_rex(program, size, reg2, 0, reg1);
  *program->codeptr++ = 0x85;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void
x86_emit_sar_imm_reg (OrcProgram *program, int size, int value, int reg)
{
  if (size == 2) {
    printf("  sarw $%d, %%%s\n", value, x86_get_regname_16(reg));
  } else if (size == 4) {
    printf("  sarl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    printf("  sar $%d, %%%s\n", value, x86_get_regname_64(reg));
  }

  x86_emit_rex(program, size, reg, 0, 0);
  if (value == 1) {
    *program->codeptr++ = 0xd1;
    x86_emit_modrm_reg (program, reg, 7);
  } else {
    *program->codeptr++ = 0xc1;
    x86_emit_modrm_reg (program, reg, 7);
    *program->codeptr++ = value;
  }
}

void
x86_emit_and_imm_memoffset (OrcProgram *program, int size, int value,
    int offset, int reg)
{
  if (size == 2) {
    printf("  andw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  andl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    printf("  and $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  if (value >= -128 && value < 128) {
    *program->codeptr++ = 0x83;
    /* FIXME */
    x86_emit_modrm_memoffset (program, 0, offset, reg);
    *program->codeptr++ = (value & 0xff);
  } else {
    *program->codeptr++ = 0x81;
    /* FIXME */
    x86_emit_modrm_memoffset (program, 0, offset, reg);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    if (size == 4) {
      *program->codeptr++ = ((value>>16) & 0xff);
      *program->codeptr++ = ((value>>24) & 0xff);
    }
  }
}

void
x86_emit_and_imm_reg (OrcProgram *program, int size, int value, int reg)
{
  if (size == 2) {
    printf("  andw $%d, %%%s\n", value, x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  andl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    printf("  and $%d, %%%s\n", value, x86_get_regname_64(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  if (value >= -128 && value < 128) {
    *program->codeptr++ = 0x83;
    x86_emit_modrm_reg (program, reg, 4);
    *program->codeptr++ = (value & 0xff);
  } else {
    *program->codeptr++ = 0x81;
    x86_emit_modrm_reg (program, reg, 4);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    if (size == 4) {
      *program->codeptr++ = ((value>>16) & 0xff);
      *program->codeptr++ = ((value>>24) & 0xff);
    }
  }
}

void
x86_emit_add_imm_memoffset (OrcProgram *program, int size, int value,
    int offset, int reg)
{
  if (size == 2) {
    printf("  addw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  addl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    printf("  add $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  if (value >= -128 && value < 128) {
    *program->codeptr++ = 0x83;
    x86_emit_modrm_memoffset (program, 0, offset, reg);
    *program->codeptr++ = (value & 0xff);
  } else {
    *program->codeptr++ = 0x81;
    x86_emit_modrm_memoffset (program, 0, offset, reg);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    if (size == 4) {
      *program->codeptr++ = ((value>>16) & 0xff);
      *program->codeptr++ = ((value>>24) & 0xff);
    }
  }
}

void
x86_emit_add_imm_reg (OrcProgram *program, int size, int value, int reg)
{
  if (size == 2) {
    printf("  addw $%d, %%%s\n", value, x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  addl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    printf("  add $%d, %%%s\n", value, x86_get_regname_64(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  if (value >= -128 && value < 128) {
    *program->codeptr++ = 0x83;
    x86_emit_modrm_reg (program, reg, 0);
    *program->codeptr++ = (value & 0xff);
  } else {
    *program->codeptr++ = 0x81;
    x86_emit_modrm_reg (program, reg, 0);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    if (size == 4) {
      *program->codeptr++ = ((value>>16) & 0xff);
      *program->codeptr++ = ((value>>24) & 0xff);
    }
  }
}

void
x86_emit_sub_reg_reg (OrcProgram *program, int size, int reg1, int reg2)
{
  if (size == 2) {
    printf("  subw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  subl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    printf("  sub %%%s, %%%s\n", x86_get_regname_64(reg1),
        x86_get_regname_64(reg2));
  }

  x86_emit_rex(program, size, reg2, 0, reg1);
  *program->codeptr++ = 0x29;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void
x86_emit_sub_memoffset_reg (OrcProgram *program, int size,
    int offset, int reg, int destreg)
{
  if (size == 2) {
    printf("  subw %d(%%%s), %%%s\n", offset,
        x86_get_regname_ptr(reg),
        x86_get_regname_16(destreg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  subl %d(%%%s), %%%s\n", offset,
        x86_get_regname_ptr(reg),
        x86_get_regname(destreg));
  } else {
    printf("  sub %d(%%%s), %%%s\n", offset,
        x86_get_regname_ptr(reg),
        x86_get_regname_64(destreg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  *program->codeptr++ = 0x2b;
  x86_emit_modrm_memoffset (program, destreg, offset, reg);
}

void
x86_emit_cmp_reg_memoffset (OrcProgram *program, int size, int reg1,
    int offset, int reg)
{
  if (size == 2) {
    printf("  cmpw %%%s, %d(%%%s)\n", x86_get_regname_16(reg1), offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  cmpl %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
        x86_get_regname_ptr(reg));
  } else {
    printf("  cmp %%%s, %d(%%%s)\n", x86_get_regname_64(reg1), offset,
        x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  *program->codeptr++ = 0x39;
  x86_emit_modrm_memoffset (program, reg1, offset, reg);
}

void
x86_emit_cmp_imm_memoffset (OrcProgram *program, int size, int value,
    int offset, int reg)
{
  if (size == 2) {
    printf("  cmpw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  cmpl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    printf("  cmp $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  if (value >= -128 && value < 128) {
    *program->codeptr++ = 0x83;
    x86_emit_modrm_memoffset (program, 7, offset, reg);
    *program->codeptr++ = (value & 0xff);
  } else {
    *program->codeptr++ = 0x81;
    x86_emit_modrm_memoffset (program, 7, offset, reg);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
    if (size == 4) {
      *program->codeptr++ = ((value>>16) & 0xff);
      *program->codeptr++ = ((value>>24) & 0xff);
    }
  }
}

void
x86_emit_dec_memoffset (OrcProgram *program, int size,
    int offset, int reg)
{
  if (size == 2) {
    printf("  decw %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    printf("  decl %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
  } else {
    printf("  dec %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  *program->codeptr++ = 0xff;
  x86_emit_modrm_memoffset (program, 1, offset, reg);
}

void x86_emit_ret (OrcProgram *program)
{
  if (x86_64) {
    printf("  retq\n");
  } else {
    printf("  ret\n");
  }
  *program->codeptr++ = 0xc3;
}

void x86_emit_emms (OrcProgram *program)
{
  printf("  emms\n");
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x77;
}

void
x86_add_fixup (OrcProgram *program, unsigned char *ptr, int label, int type)
{
  program->fixups[program->n_fixups].ptr = ptr;
  program->fixups[program->n_fixups].label = label;
  program->fixups[program->n_fixups].type = type;
  program->n_fixups++;
}

void
x86_add_label (OrcProgram *program, unsigned char *ptr, int label)
{
  program->labels[label] = ptr;
}

void x86_emit_jmp (OrcProgram *program, int label)
{
  printf("  jmp .L%d\n", label);

  *program->codeptr++ = 0xeb;
  x86_add_fixup (program, program->codeptr, label, 0);
  *program->codeptr++ = 0xff;
}

void x86_emit_jle (OrcProgram *program, int label)
{
  printf("  jle .L%d\n", label);

  *program->codeptr++ = 0x7e;
  x86_add_fixup (program, program->codeptr, label, 0);
  *program->codeptr++ = 0xff;
}

void x86_emit_je (OrcProgram *program, int label)
{
  printf("  je .L%d\n", label);

  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x84;
  x86_add_fixup (program, program->codeptr, label, 1);
  *program->codeptr++ = 0xfc;
  *program->codeptr++ = 0xff;
  *program->codeptr++ = 0xff;
  *program->codeptr++ = 0xff;
}

void x86_emit_jne (OrcProgram *program, int label)
{
  printf("  jne .L%d\n", label);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x85;
  x86_add_fixup (program, program->codeptr, label, 1);
  *program->codeptr++ = 0xfc;
  *program->codeptr++ = 0xff;
  *program->codeptr++ = 0xff;
  *program->codeptr++ = 0xff;
}

void x86_emit_label (OrcProgram *program, int label)
{
  printf(".L%d:\n", label);

  x86_add_label (program, program->codeptr, label);
}

void
x86_emit_align (OrcProgram *program)
{
  int diff;
  int align_shift = 4;

  diff = (program->code - program->codeptr)&((1<<align_shift) - 1);
  while (diff) {
    printf("  nop\n");
    *program->codeptr++ = 0x90;
    diff--;
  }
}

void
x86_test (OrcProgram *program)
{
  int size;
  int i;
  int j;
  int reg;

  for(size=2;size<=4;size+=2) {
    for(i=0;i<8;i++){
      reg = ORC_GP_REG_BASE + i;
      x86_emit_push (program, size, reg);
      x86_emit_pop (program, size, reg);
      x86_emit_mov_imm_reg (program, size, 0, reg);
      x86_emit_mov_imm_reg (program, size, 1, reg);
      x86_emit_mov_imm_reg (program, size, 256, reg);
      x86_emit_dec_memoffset (program, size, 0, reg);
      x86_emit_dec_memoffset (program, size, 1, reg);
      x86_emit_dec_memoffset (program, size, 256, reg);
      x86_emit_add_imm_memoffset (program, size, 1, 0, reg);
      x86_emit_add_imm_memoffset (program, size, 1, 1, reg);
      x86_emit_add_imm_memoffset (program, size, 1, 256, reg);
      x86_emit_add_imm_memoffset (program, size, 256, 0, reg);
      x86_emit_add_imm_memoffset (program, size, 256, 1, reg);
      x86_emit_add_imm_memoffset (program, size, 256, 256, reg);
      for(j=0;j<8;j++){
        int reg2 = ORC_GP_REG_BASE + j;
        x86_emit_mov_reg_reg (program, size, reg, reg2);
        x86_emit_mov_memoffset_reg (program, size, 0, reg, reg2);
        x86_emit_mov_memoffset_reg (program, size, 1, reg, reg2);
        x86_emit_mov_memoffset_reg (program, size, 256, reg, reg2);
        x86_emit_mov_reg_memoffset (program, size, reg, 0, reg2);
        x86_emit_mov_reg_memoffset (program, size, reg, 1, reg2);
        x86_emit_mov_reg_memoffset (program, size, reg, 256, reg2);
      }
    }
  }

}

