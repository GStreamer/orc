
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>
#include <orc/orcdebug.h>
#include <orc/orcutils.h>

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
    ORC_ASM_CODE(program,"  pushw %%%s\n", x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0x50 + x86_get_regnum(reg);
  } else {
    ORC_ASM_CODE(program,"  pushl %%%s\n", x86_get_regname(reg));
    *program->codeptr++ = 0x50 + x86_get_regnum(reg);
  }
}

void
x86_emit_pop (OrcProgram *program, int size, int reg)
{

  if (size == 1) {
    program->error = 1;
  } else if (size == 2) {
    ORC_ASM_CODE(program,"  popw %%%s\n", x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0x58 + x86_get_regnum(reg);
  } else {
    ORC_ASM_CODE(program,"  popl %%%s\n", x86_get_regname(reg));
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
  }
}

void
x86_emit_mov_memoffset_reg (OrcProgram *program, int size, int offset,
    int reg1, int reg2)
{

  switch (size) {
    case 1:
      ORC_ASM_CODE(program,"  movb %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_8(reg2));
      *program->codeptr++ = 0x8a;
      x86_emit_modrm_memoffset (program, reg2, offset, reg1);
      return;
    case 2:
      ORC_ASM_CODE(program,"  movw %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_16(reg2));
      *program->codeptr++ = 0x66;
      break;
    case 4:
      ORC_ASM_CODE(program,"  movl %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname(reg2));
      break;
    case 8:
      ORC_ASM_CODE(program,"  mov %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_64(reg2));
      break;
    default:
      ORC_PROGRAM_ERROR(program, "bad size");
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
    ORC_ASM_CODE(program,"  movd %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
        x86_get_regname_mmx(reg2));
    x86_emit_rex(program, 0, reg2, 0, reg1);
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x6e;
  } else {
    ORC_ASM_CODE(program,"  movq %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
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
      ORC_ASM_CODE(program,"  movd %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg2, 0, reg1);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(program,"  movq %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      *program->codeptr++ = 0xf3;
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x7e;
      break;
    case 16:
      ORC_ASM_CODE(program,"  movdqu %d(%%%s), %%%s\n", offset, x86_get_regname_ptr(reg1),
          x86_get_regname_sse(reg2));
      *program->codeptr++ = 0xf3;
      x86_emit_rex(program, 0, reg2, 0, reg1);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x6f;
      break;
    default:
      ORC_PROGRAM_ERROR(program, "bad size");
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
      ORC_ASM_CODE(program,"  movb %%%s, %d(%%%s)\n", x86_get_regname_8(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x88;
      x86_emit_modrm_memoffset (program, reg1, offset, reg2);
      return;
    case 2:
      ORC_ASM_CODE(program,"  movw %%%s, %d(%%%s)\n", x86_get_regname_16(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      break;
    case 4:
      ORC_ASM_CODE(program,"  movl %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
          x86_get_regname_ptr(reg2));
      break;
    case 8:
      ORC_ASM_CODE(program,"  mov %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
          x86_get_regname_ptr(reg2));
      break;
    default:
      ORC_PROGRAM_ERROR(program, "bad size");
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
    ORC_ASM_CODE(program,"  movd %%%s, %d(%%%s)\n", x86_get_regname_mmx(reg1), offset,
        x86_get_regname_ptr(reg2));
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x7e;
  } else if (size == 8) {
    ORC_ASM_CODE(program,"  movq %%%s, %d(%%%s)\n", x86_get_regname_mmx(reg1), offset,
        x86_get_regname_ptr(reg2));
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x7f;
  }

  x86_emit_modrm_memoffset (program, reg1, offset, reg2);
}

void
x86_emit_mov_sse_memoffset (OrcProgram *program, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(program,"  movd %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg1, 0, reg2);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0x7e;
      break;
    case 8:
      ORC_ASM_CODE(program,"  movq %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
          x86_get_regname_ptr(reg2));
      *program->codeptr++ = 0x66;
      x86_emit_rex(program, 0, reg1, 0, reg2);
      *program->codeptr++ = 0x0f;
      *program->codeptr++ = 0xd6;
      break;
    case 16:
      if (aligned) {
        if (uncached) {
          ORC_ASM_CODE(program,"  movntdq %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
              x86_get_regname_ptr(reg2));
          *program->codeptr++ = 0x66;
          *program->codeptr++ = 0x0f;
          *program->codeptr++ = 0xe7;
        } else {
          ORC_ASM_CODE(program,"  movdqa %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
              x86_get_regname_ptr(reg2));
          *program->codeptr++ = 0x66;
          *program->codeptr++ = 0x0f;
          *program->codeptr++ = 0x7f;
        }
      } else {
        ORC_ASM_CODE(program,"  movdqu %%%s, %d(%%%s)\n", x86_get_regname_sse(reg1), offset,
            x86_get_regname_ptr(reg2));
        *program->codeptr++ = 0xf3;
        *program->codeptr++ = 0x0f;
        *program->codeptr++ = 0x7f;
      }
      break;
    default:
      ORC_PROGRAM_ERROR(program, "bad size");
      break;
  }

  x86_emit_modrm_memoffset (program, reg1, offset, reg2);
}

void
x86_emit_mov_imm_reg (OrcProgram *program, int size, int value, int reg1)
{
  if (size == 2) {
    ORC_ASM_CODE(program,"  movw $%d, %%%s\n", value, x86_get_regname_16(reg1));
    x86_emit_rex(program, size, reg1, 0, 0);
    *program->codeptr++ = 0x66;
    *program->codeptr++ = 0xb8 + x86_get_regnum(reg1);
    *program->codeptr++ = (value & 0xff);
    *program->codeptr++ = ((value>>8) & 0xff);
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  movl $%d, %%%s\n", value, x86_get_regname(reg1));
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
    ORC_ASM_CODE(program,"  movw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  movl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    ORC_ASM_CODE(program,"  mov %%%s, %%%s\n", x86_get_regname_64(reg1),
        x86_get_regname_64(reg2));
  }

  x86_emit_rex(program, size, reg2, 0, reg1);
  *program->codeptr++ = 0x89;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void x86_emit_mov_sse_reg_reg (OrcProgram *program, int reg1, int reg2)
{
  ORC_ASM_CODE(program,"  movdqa %%%s, %%%s\n", x86_get_regname_sse(reg1),
        x86_get_regname_sse(reg2));

  *program->codeptr++ = 0x66;
  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6f;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_mmx_reg_reg (OrcProgram *program, int reg1, int reg2)
{
  ORC_ASM_CODE(program,"  movq %%%s, %%%s\n", x86_get_regname_mmx(reg1),
        x86_get_regname_mmx(reg2));

  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6f;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_reg_mmx (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  ORC_ASM_CODE(program,"  movd %%%s, %%%s\n", x86_get_regname(reg1),
      x86_get_regname_mmx(reg2));
  x86_emit_rex(program, 0, reg1, 0, reg2);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x6e;
  x86_emit_modrm_reg (program, reg1, reg2);
}

void x86_emit_mov_mmx_reg (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  ORC_ASM_CODE(program,"  movd %%%s, %%%s\n", x86_get_regname_mmx(reg1),
      x86_get_regname(reg2));
  x86_emit_rex(program, 0, reg2, 0, reg1);
  *program->codeptr++ = 0x0f;
  *program->codeptr++ = 0x7e;
  x86_emit_modrm_reg (program, reg2, reg1);
}

void x86_emit_mov_reg_sse (OrcProgram *program, int reg1, int reg2)
{
  /* FIXME */
  ORC_ASM_CODE(program,"  movd %%%s, %%%s\n", x86_get_regname(reg1),
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
  ORC_ASM_CODE(program,"  movd %%%s, %%%s\n", x86_get_regname_sse(reg1),
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
    ORC_ASM_CODE(program,"  testw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  testl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    ORC_ASM_CODE(program,"  test %%%s, %%%s\n", x86_get_regname(reg1),
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
    ORC_ASM_CODE(program,"  sarw $%d, %%%s\n", value, x86_get_regname_16(reg));
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  sarl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    ORC_ASM_CODE(program,"  sar $%d, %%%s\n", value, x86_get_regname_64(reg));
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
    ORC_ASM_CODE(program,"  andw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  andl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    ORC_ASM_CODE(program,"  and $%d, %d(%%%s)\n", value, offset,
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
    ORC_ASM_CODE(program,"  andw $%d, %%%s\n", value, x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  andl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    ORC_ASM_CODE(program,"  and $%d, %%%s\n", value, x86_get_regname_64(reg));
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
    ORC_ASM_CODE(program,"  addw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  addl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    ORC_ASM_CODE(program,"  add $%d, %d(%%%s)\n", value, offset,
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
    ORC_ASM_CODE(program,"  addw $%d, %%%s\n", value, x86_get_regname_16(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  addl $%d, %%%s\n", value, x86_get_regname(reg));
  } else {
    ORC_ASM_CODE(program,"  add $%d, %%%s\n", value, x86_get_regname_64(reg));
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
    ORC_ASM_CODE(program,"  subw %%%s, %%%s\n", x86_get_regname_16(reg1),
        x86_get_regname_16(reg2));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  subl %%%s, %%%s\n", x86_get_regname(reg1),
        x86_get_regname(reg2));
  } else {
    ORC_ASM_CODE(program,"  sub %%%s, %%%s\n", x86_get_regname_64(reg1),
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
    ORC_ASM_CODE(program,"  subw %d(%%%s), %%%s\n", offset,
        x86_get_regname_ptr(reg),
        x86_get_regname_16(destreg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  subl %d(%%%s), %%%s\n", offset,
        x86_get_regname_ptr(reg),
        x86_get_regname(destreg));
  } else {
    ORC_ASM_CODE(program,"  sub %d(%%%s), %%%s\n", offset,
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
    ORC_ASM_CODE(program,"  cmpw %%%s, %d(%%%s)\n", x86_get_regname_16(reg1), offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  cmpl %%%s, %d(%%%s)\n", x86_get_regname(reg1), offset,
        x86_get_regname_ptr(reg));
  } else {
    ORC_ASM_CODE(program,"  cmp %%%s, %d(%%%s)\n", x86_get_regname_64(reg1), offset,
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
    ORC_ASM_CODE(program,"  cmpw $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  cmpl $%d, %d(%%%s)\n", value, offset,
        x86_get_regname_ptr(reg));
  } else {
    ORC_ASM_CODE(program,"  cmp $%d, %d(%%%s)\n", value, offset,
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
    ORC_ASM_CODE(program,"  decw %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
    *program->codeptr++ = 0x66;
  } else if (size == 4) {
    ORC_ASM_CODE(program,"  decl %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
  } else {
    ORC_ASM_CODE(program,"  dec %d(%%%s)\n", offset, x86_get_regname_ptr(reg));
  }

  x86_emit_rex(program, size, 0, 0, reg);
  *program->codeptr++ = 0xff;
  x86_emit_modrm_memoffset (program, 1, offset, reg);
}

void x86_emit_ret (OrcProgram *program)
{
  if (x86_64) {
    ORC_ASM_CODE(program,"  retq\n");
  } else {
    ORC_ASM_CODE(program,"  ret\n");
  }
  *program->codeptr++ = 0xc3;
}

void x86_emit_emms (OrcProgram *program)
{
  ORC_ASM_CODE(program,"  emms\n");
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
  ORC_ASM_CODE(program,"  jmp .L%d\n", label);

  if (program->long_jumps) {
    ORC_PROGRAM_ERROR(program, "unimplemented");
  } else {
    *program->codeptr++ = 0xeb;
    x86_add_fixup (program, program->codeptr, label, 0);
    *program->codeptr++ = 0xff;
  }
}

void x86_emit_jle (OrcProgram *program, int label)
{
  ORC_ASM_CODE(program,"  jle .L%d\n", label);

  if (program->long_jumps) {
    ORC_PROGRAM_ERROR(program, "unimplemented");
  } else {
    *program->codeptr++ = 0x7e;
    x86_add_fixup (program, program->codeptr, label, 0);
    *program->codeptr++ = 0xff;
  }
}

void x86_emit_je (OrcProgram *program, int label)
{
  ORC_ASM_CODE(program,"  je .L%d\n", label);

  if (program->long_jumps) {
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x84;
    x86_add_fixup (program, program->codeptr, label, 1);
    *program->codeptr++ = 0xfc;
    *program->codeptr++ = 0xff;
    *program->codeptr++ = 0xff;
    *program->codeptr++ = 0xff;
  } else {
    *program->codeptr++ = 0x74;
    x86_add_fixup (program, program->codeptr, label, 0);
    *program->codeptr++ = 0xff;
  }
}

void x86_emit_jne (OrcProgram *program, int label)
{
  ORC_ASM_CODE(program,"  jne .L%d\n", label);

  if (program->long_jumps) {
    *program->codeptr++ = 0x0f;
    *program->codeptr++ = 0x85;
    x86_add_fixup (program, program->codeptr, label, 1);
    *program->codeptr++ = 0xfc;
    *program->codeptr++ = 0xff;
    *program->codeptr++ = 0xff;
    *program->codeptr++ = 0xff;
  } else {
    *program->codeptr++ = 0x75;
    x86_add_fixup (program, program->codeptr, label, 0);
    *program->codeptr++ = 0xff;
  }
}

void x86_emit_label (OrcProgram *program, int label)
{
  ORC_ASM_CODE(program,".L%d:\n", label);

  x86_add_label (program, program->codeptr, label);
}

void
x86_do_fixups (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_fixups;i++){
    if (program->fixups[i].type == 0) {
      unsigned char *label = program->labels[program->fixups[i].label];
      unsigned char *ptr = program->fixups[i].ptr;
      int diff;

      diff = ((int8_t)ptr[0]) + (label - ptr);
      if (diff != (int8_t)diff) {
        ORC_WARNING("short jump too long");
        program->error = TRUE;
      }

      ptr[0] = diff;
    } else if (program->fixups[i].type == 1) {
      unsigned char *label = program->labels[program->fixups[i].label];
      unsigned char *ptr = program->fixups[i].ptr;
      int diff;

      diff = ORC_READ_UINT32_LE (ptr) + (label - ptr);
      ORC_WRITE_UINT32_LE(ptr, diff);
    }
  }
}

void
x86_emit_prologue (OrcProgram *program)
{
  orc_program_append_code(program,".global _binary_dump_start\n");
  orc_program_append_code(program,"_binary_dump_start:\n");
  if (x86_64) {
    int i;
    for(i=0;i<16;i++){
      if (program->used_regs[ORC_GP_REG_BASE+i] &&
          program->save_regs[ORC_GP_REG_BASE+i]) {
        x86_emit_push (program, 8, ORC_GP_REG_BASE+i);
      }
    }
  } else {
    x86_emit_push (program, 4, X86_EBP);
    x86_emit_mov_memoffset_reg (program, 4, 8, X86_ESP, X86_EBP);
    if (program->used_regs[X86_EDI]) {
      x86_emit_push (program, 4, X86_EDI);
    }
    if (program->used_regs[X86_ESI]) {
      x86_emit_push (program, 4, X86_ESI);
    }
    if (program->used_regs[X86_EBX]) {
      x86_emit_push (program, 4, X86_EBX);
    }
  }
}

void
x86_emit_epilogue (OrcProgram *program)
{
  if (x86_64) {
    int i;
    for(i=15;i>=0;i--){
      if (program->used_regs[ORC_GP_REG_BASE+i] &&
          program->save_regs[ORC_GP_REG_BASE+i]) {
        x86_emit_push (program, 8, ORC_GP_REG_BASE+i);
      }
    }
  } else {
    if (program->used_regs[X86_EBX]) {
      x86_emit_pop (program, 4, X86_EBX);
    }
    if (program->used_regs[X86_ESI]) {
      x86_emit_pop (program, 4, X86_ESI);
    }
    if (program->used_regs[X86_EDI]) {
      x86_emit_pop (program, 4, X86_EDI);
    }
    x86_emit_pop (program, 4, X86_EBP);
  }
  x86_emit_ret (program);
}

void
x86_emit_align (OrcProgram *program)
{
  int diff;
  int align_shift = 4;

  diff = (program->code - program->codeptr)&((1<<align_shift) - 1);
  while (diff) {
    ORC_ASM_CODE(program,"  nop\n");
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

