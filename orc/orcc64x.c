
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcc64x.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>


const char *
orc_c64x_reg_name (int reg)
{
  static const char *gp_regs[] = {
    "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7",
    "a8", "a9", "a10", "a11",
    "a12", "a13", "a14", "a15",
    "b0", "b1", "b2", "b3",
    "b4", "b5", "b6", "b7",
    "b8", "b9", "b10", "b11",
    "b12", "b13", "b14", "b15",
  };

  if (reg < ORC_GP_REG_BASE || reg >= ORC_GP_REG_BASE+16) {
    return "ERROR";
  }

  return gp_regs[reg&0x1f];
}

void
orc_c64x_emit (OrcCompiler *compiler, uint32_t insn)
{
  ORC_WRITE_UINT32_LE (compiler->codeptr, insn);
  compiler->codeptr+=4;
}

void
orc_c64x_emit_bx_lr (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  ret.s2 b3\n");
  ORC_ASM_CODE(compiler,"  addk.s2 24, b15\n");
  ORC_ASM_CODE(compiler,"  nop\n");
  orc_c64x_emit (compiler, 0xe12fff1e);
}

void
orc_c64x_emit_push (OrcCompiler *compiler, int regs)
{
  int i;

  for(i=0;i<16;i++){
    if (regs & (1<<i)) {
      ORC_ASM_CODE(compiler,"  stw  %s,*+b15(16)\n",
          orc_c64x_reg_name(ORC_GP_REG_BASE + i));
      orc_c64x_emit (compiler, 0xe92d0000 | regs);
    }
  }

}

void
orc_c64x_emit_pop (OrcCompiler *compiler, int regs)
{
  int i;

  for(i=0;i<16;i++){
    if (regs & (1<<i)) {
      ORC_ASM_CODE(compiler,"  ldw *+b15(8),%s\n",
          orc_c64x_reg_name(ORC_GP_REG_BASE + i));
      orc_c64x_emit (compiler, 0xe8bd0000 | regs);
    }
  }

}

void
orc_c64x_emit_mov (OrcCompiler *compiler, int dest, int src)
{
  uint32_t code;

  code = 0xe1a00000;
  code |= (src&0xf) << 0;
  code |= (dest&0xf) << 12;

  ORC_ASM_CODE(compiler,"  or %s, %s, %s\n",
      orc_c64x_reg_name (src), orc_c64x_reg_name (src),
      orc_c64x_reg_name (dest));

  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_label (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"local%d:\n", label);

  compiler->labels[label] = compiler->codeptr;
}

void
orc_c64x_add_fixup (OrcCompiler *compiler, int label, int type)
{
  compiler->fixups[compiler->n_fixups].ptr = compiler->codeptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = type;
  compiler->n_fixups++;
}

void
orc_c64x_do_fixups (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<compiler->n_fixups;i++){
    unsigned char *label = compiler->labels[compiler->fixups[i].label];
    unsigned char *ptr = compiler->fixups[i].ptr;
    uint32_t code;
    int diff;

    code = ORC_READ_UINT32_LE (ptr);
    diff = ORC_READ_UINT32_LE (ptr) + ((label - ptr) >> 2);
    ORC_WRITE_UINT32_LE(ptr, (code&0xff000000) | (diff&0x00ffffff));
  }

}

void
orc_c64x_emit_branch (OrcCompiler *compiler, int cond, int label)
{
#if 0
  static const char *cond_names[] = {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", "", "" };
#endif
  uint32_t code;

  code = 0x0afffffe;
  code |= (cond&0xf) << 28;
  orc_c64x_add_fixup (compiler, label, 0);
  orc_c64x_emit (compiler, code);

  ORC_ASM_CODE(compiler,"  b local%d\n", label);
  ORC_ASM_CODE(compiler,"  nop\n");
  ORC_ASM_CODE(compiler,"  nop\n");
}

void
orc_c64x_emit_load_imm (OrcCompiler *compiler, int dest, int imm)
{
  uint32_t code;
  int shift2;
  unsigned int x;

  if ((imm & 0xff) == imm) {
    shift2 = 0;
    x = imm;
  } else {
    shift2 = 0;
    x = imm & 0xffffffff;
    while ((x & 3) == 0) {
      x >>= 2;
      shift2++;
    }
    if (x > 0xff) {
      ORC_COMPILER_ERROR(compiler, "bad immediate value");
    }
  }

  code = 0xe3a00000;
  code |= (dest&0xf) << 12;
  code |= (((16-shift2)&0xf) << 8);
  code |= (x&0xff);

  ORC_ASM_CODE(compiler,"  mvk 0x%04x, %s\n", imm, orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_add (OrcCompiler *compiler, int dest, int src1, int src2)
{
  uint32_t code;

  code = 0xe0800000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (src2&0xf) << 0;

  ORC_ASM_CODE(compiler,"  add %s, %s, %s\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (src2),
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_sub (OrcCompiler *compiler, int dest, int src1, int src2)
{
  uint32_t code;

  code = 0xe0400000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (src2&0xf) << 0;

  ORC_ASM_CODE(compiler,"  sub %s, %s, %s\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (src2),
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_add_imm (OrcCompiler *compiler, int dest, int src1, int imm)
{
#if 0
  uint32_t code;

  code = 0xe2800000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (value) << 0;

  ORC_ASM_CODE(compiler,"  add %s, %s, #%d\n",
      orc_c64x_reg_name (dest),
      orc_c64x_reg_name (src1),
      value);
  orc_c64x_emit (compiler, code);
#endif
  uint32_t code;
  int shift2;
  unsigned int x;

  if ((imm & 0xff) == imm) {
    shift2 = 0;
    x = imm;
  } else {
    shift2 = 0;
    x = imm & 0xffffffff;
    while ((x & 3) == 0) {
      x >>= 2;
      shift2++;
    }
    if (x > 0xff) {
      ORC_COMPILER_ERROR(compiler, "bad immediate value");
    }
  }

  if (dest != src1) {
    ORC_ASM_CODE(compiler,"  mv %s, %s\n",
        orc_c64x_reg_name (dest),
        orc_c64x_reg_name (src1));
  }

  code = 0xe2800000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (((16-shift2)&0xf) << 8);
  code |= (x&0xff);

  ORC_ASM_CODE(compiler,"  addk 0x%04x, %s\n", imm,
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_and_imm (OrcCompiler *compiler, int dest, int src1, int value)
{
  uint32_t code;

  code = 0xe2000000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (value) << 0;

  ORC_ASM_CODE(compiler,"  and 0x%04x, %s, %s\n", value,
      orc_c64x_reg_name(src1),
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_sub_imm (OrcCompiler *compiler, int dest, int src1, int value)
{
  uint32_t code;

  code = 0xe2500000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (value) << 0;

  ORC_ASM_CODE(compiler,"  sub 0x%04x, %s, %s\n", value,
      orc_c64x_reg_name(src1),
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_cmp_imm (OrcCompiler *compiler, int src1, int value)
{
  uint32_t code;

  code = 0xe3500000;
  code |= (src1&0xf) << 16;
  code |= (value) << 0;

  ORC_ASM_CODE(compiler,"  sub 0x%04x, %s, a0\n", value,
      orc_c64x_reg_name(src1));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_cmp (OrcCompiler *compiler, int src1, int src2)
{
  uint32_t code;

  code = 0xe1500000;
  code |= (src1&0xf) << 16;
  code |= (src2&0xf) << 0;

  ORC_ASM_CODE(compiler,"  sub %s, %s, a0\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (src2));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_asr_imm (OrcCompiler *compiler, int dest, int src1, int value)
{
  uint32_t code;

  if (value == 0) {
    ORC_ERROR("bad immediate value");
  }
  code = 0xe1a00040;
  code |= (src1&0xf) << 0;
  code |= (dest&0xf) << 12;
  code |= (value) << 7;

  ORC_ASM_CODE(compiler,"  shr %s, %d, %s\n",
      orc_c64x_reg_name (src1),
      value,
      orc_c64x_reg_name (dest));
  orc_c64x_emit (compiler, code);
}

void
orc_c64x_emit_load_reg (OrcCompiler *compiler, int dest, int src1, int offset)
{
  uint32_t code;

  code = 0xe5900000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= offset&0xfff;

  if (offset > 256) {
    orc_c64x_emit_add_imm (compiler, compiler->tmpreg, src1, offset);
    ORC_ASM_CODE(compiler,"  ldw *+%s(0), %s\n",
        orc_c64x_reg_name (compiler->tmpreg),
        orc_c64x_reg_name (dest));
    orc_c64x_emit (compiler, code);
  } else {
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (src1), offset,
        orc_c64x_reg_name (dest));
    orc_c64x_emit (compiler, code);
  }
}

void
orc_c64x_emit_store_reg (OrcCompiler *compiler, int src1, int dest, int offset)
{
  uint32_t code;

  code = 0xe5800000;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= offset&0xfff;

  ORC_ASM_CODE(compiler,"  stw %s, *+%s(%d)\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest), offset);
  orc_c64x_emit (compiler, code);
}


void
orc_c64x_emit_dp_reg (OrcCompiler *compiler, int cond, int opcode, int dest,
    int src1, int src2)
{
  static const char *dp_insn_names[] = {
    "and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
    "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
  };
  static const int shift_expn[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 1, 0, 1
  };
  uint32_t code;
  int update = 0;
  
  code = cond << 28;
  code |= opcode << 21;
  code |= update << 20; /* update condition codes */
  if (opcode >= 8 && opcode < 12) {
    code |= 1 << 20;
  }
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (src2&0xf) << 0;

  if (shift_expn[opcode]) {
    ORC_ASM_CODE(compiler,"  %s%s %s, %s\n",
        dp_insn_names[opcode],
        update ? "s" : "",
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (src2));
  } else {
    ORC_ASM_CODE(compiler,"  %s%s %s, %s, %s\n",
        dp_insn_names[opcode],
        update ? "s" : "",
        orc_c64x_reg_name (dest),
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (src2));
  }
  orc_c64x_emit (compiler, code);
}

