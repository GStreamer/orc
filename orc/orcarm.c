
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcarm.h>
#include <orc/orcutils.h>

/**
 * SECTION:orcarm
 * @title: ARM
 * @short_description: code generation for ARM
 */

const char *
orc_arm_cond_name (OrcArmCond cond)
{
  static const char *cond_names[] = {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", "", ""
  };
  if (cond < 0 || cond >= 16) {
    return "ERROR";
  }
  return cond_names[cond&0xf];
}

const char *
orc_arm_reg_name (int reg)
{
#if 0
  static const char *gp_regs[] = {
    "a1", "a2", "a3", "a4",
    "v1", "v2", "v3", "v4",
    "v5", "v6", "v7", "v8",
    "ip", "sp", "lr", "pc" };
#else
  static const char *gp_regs[] = {
    "r0", "r1", "r2", "r3",
    "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11",
    "ip", "sp", "lr", "pc" };
    //"r12", "r13", "r14", "r15" };
#endif

  if (reg < ORC_GP_REG_BASE || reg >= ORC_GP_REG_BASE+16) {
    return "ERROR";
  }

  return gp_regs[reg&0xf];
}

void
orc_arm_emit (OrcCompiler *compiler, uint32_t insn)
{
  ORC_WRITE_UINT32_LE (compiler->codeptr, insn);
  compiler->codeptr+=4;
}

void
orc_arm_emit_bx_lr (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  bx lr\n");
  orc_arm_emit (compiler, 0xe12fff1e);
}

void
orc_arm_emit_push (OrcCompiler *compiler, int regs)
{
  int i;
  int x = 0;

  ORC_ASM_CODE(compiler,"  push {");
  for(i=0;i<16;i++){
    if (regs & (1<<i)) {
      x |= (1<<i);
      ORC_ASM_CODE(compiler,"r%d", i);
      if (x != regs) {
        ORC_ASM_CODE(compiler,", ");
      }
    }
  }
  ORC_ASM_CODE(compiler,"}\n");

  orc_arm_emit (compiler, 0xe92d0000 | regs);
}

void
orc_arm_emit_pop (OrcCompiler *compiler, int regs)
{
  int i;
  int x = 0;

  ORC_ASM_CODE(compiler,"  pop {");
  for(i=0;i<16;i++){
    if (regs & (1<<i)) {
      x |= (1<<i);
      ORC_ASM_CODE(compiler,"r%d", i);
      if (x != regs) {
        ORC_ASM_CODE(compiler,", ");
      }
    }
  }
  ORC_ASM_CODE(compiler,"}\n");

  orc_arm_emit (compiler, 0xe8bd0000 | regs);
}

void
orc_arm_emit_label (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,".L%d:\n", label);

  compiler->labels[label] = compiler->codeptr;
}

void
orc_arm_add_fixup (OrcCompiler *compiler, int label, int type)
{
  compiler->fixups[compiler->n_fixups].ptr = compiler->codeptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = type;
  compiler->n_fixups++;
}

void
orc_arm_do_fixups (OrcCompiler *compiler)
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
orc_arm_emit_branch (OrcCompiler *compiler, int cond, int label)
{
  uint32_t code;

  code = 0x0afffffe;
  code |= (cond&0xf) << 28;
  orc_arm_add_fixup (compiler, label, 0);
  orc_arm_emit (compiler, code);

  ORC_ASM_CODE(compiler,"  b%s .L%d\n", orc_arm_cond_name(cond), label);
}

void
orc_arm_emit_loadimm (OrcCompiler *compiler, int dest, int imm)
{
  uint32_t code;
  int shift2;

  shift2 = 0;
  while (imm && ((imm&3)==0)) {
    imm >>= 2;
    shift2++;
  }

  code = 0xe3a00000;
  code |= (dest&0xf) << 12;
  code |= (((16-shift2)&0xf) << 8);
  code |= (imm&0xff);

  ORC_ASM_CODE(compiler,"  mov %s, #0x%08x\n", orc_arm_reg_name (dest), imm << (shift2*2));
  orc_arm_emit (compiler, code);
}

void
orc_arm_emit_load_reg (OrcCompiler *compiler, int dest, int src1, int offset)
{
  uint32_t code;

  code = 0xe5900000;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= offset&0xfff;

  ORC_ASM_CODE(compiler,"  ldr %s, [%s, #%d]\n",
      orc_arm_reg_name (dest),
      orc_arm_reg_name (src1), offset);
  orc_arm_emit (compiler, code);
}

/* shifter operands */
/*    1
 *  1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 * |rotimm |   immed_8     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define arm_so_imm(rot,imm) (((rot)<<8)|(imm))
/*    1
 *  1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Si   | St  |0| Rm  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define arm_so_shift_imm(Si,St,Rm) (((Si)<<7)|((St)<<5)|(Rm))
#define arm_so_rrx_reg(Rm)     arm_so_shift_imm(0,ORC_ARM_ROR,Rm)
#define arm_so_reg(Rm)        (Rm)
/*    1
 *  1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Rs   |0| St  |1| Rm  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define arm_so_shift_reg(Rs,St,Rm) (0x008|((Rs)<<8)|((St)<<5)|(Rm))

/* data processing instructions */
/*    3   2 2 2 2 2     2 2 1     1 1     1   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | cond  |0 0|I| opcode|S|   Rn  |  Rd   |   shifter_operand     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define arm_dp(cond,I,opcode,S,Rn,Rd,So) (((cond)<<28)|((I)<<25)|((opcode)<<21)|((S)<<20)|((Rn)<<16)|((Rd)<<12)|So)
#define arm_dp_reg(cond,opcode,S,Rn,Rd,So) arm_dp (cond,0,opcode,S,Rn,Rd,So)
#define arm_dp_imm(cond,opcode,S,Rn,Rd,So) arm_dp (cond,1,opcode,S,Rn,Rd,So)

/*
 * type 0:  <op>{<cond>}{s} {<Rd>}, <Rn>, #imm   (imm = (val>>(shift*2))|(val<<(32-(shift*2))))
 * type 1:  <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm>
 * type 2:  <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm>, [LSL|LSR|ASR] #imm
 * type 3:  <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm>, [LSL|LSR|ASR] <Rs>
 * type 4:  <op>{<cond>}{s} {<Rd>,} <Rn>, <Rm>, RRX
 */
void
orc_arm_emit_dp (OrcCompiler *p, int type, OrcArmCond cond, OrcArmDP opcode,
    int S, int Rd, int Rn, int Rm, int shift, int val)
{
  uint32_t code;
  int I = 0;
  int shifter_op;
  char shifter[100];
  uint32_t imm;
  static const char *shift_names[] = {
    "LSL", "LSR", "ASR", "ROR"
  };
  /* opcodes with Rd */
  static const int op_Rd[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1
  };
  /* opcodes using Rn */
  static const int op_Rn[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0
  };
  static const char *dp_insn_names[] = {
    "and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
    "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
  };

  switch (type) {
    case 0:
      /* #imm */
      imm = val & 0xff;
      shifter_op = arm_so_imm (shift, imm);
      if (shift > 0)
        imm = (imm >> (shift*2)) | (imm << (32-(shift*2)));
      snprintf (shifter, sizeof(shifter), "#%08x", imm);
      I = 1;
      break;
    case 1:
      /* <Rm> */
      shifter_op = arm_so_reg (Rm);
      snprintf (shifter, sizeof(shifter), "%s", orc_arm_reg_name (Rm));
      break;
    case 2:
      /* <Rm>, [LSL|LSR|ASR] #imm */
      shifter_op = arm_so_shift_imm (val,shift,Rm);
      snprintf (shifter, sizeof(shifter), "%s, %s #%d",
          orc_arm_reg_name (Rm), shift_names[shift], val);
      break;
    case 3:
      /* <Rm>, [LSL|LSR|ASR] <Rs> */
      shifter_op = arm_so_shift_reg (val,shift,Rm);
      snprintf (shifter, sizeof(shifter), "%s, %s %s",
          orc_arm_reg_name (Rm), shift_names[shift], orc_arm_reg_name (val));
      break;
    case 4:
      /* <Rm>, RRX */
      shifter_op = arm_so_rrx_reg (Rm);
      snprintf (shifter, sizeof(shifter), "%s, RRX",
          orc_arm_reg_name (Rm));
      break;
    default:
      ORC_COMPILER_ERROR(p,"unknown data processing type %d", type);
      return;
  }

  if (op_Rd[opcode]) {
    if (op_Rn[opcode]) {
      /* opcode using Rn */
      code = arm_dp (cond, I, opcode, S, Rn, Rd, shifter_op);
      ORC_ASM_CODE(p,"  %s%s%s %s, %s, %s\n",
          dp_insn_names[opcode], orc_arm_cond_name(cond), (S ? "s" : ""),
          orc_arm_reg_name (Rd), orc_arm_reg_name (Rn), shifter);
    } else {
      /* opcode using Rd and val (mov, mvn) */
      code = arm_dp (cond, I, opcode, S, Rn, Rd, shifter_op);
      ORC_ASM_CODE(p,"  %s%s%s %s, %s\n",
          dp_insn_names[opcode], orc_arm_cond_name(cond), (S ? "s" : ""),
          orc_arm_reg_name (Rd), shifter);
    }
  } else {
    /* opcode does not change Rd, change status register (cmp, tst, ..) */
    code = arm_dp (cond, I, opcode, 1, Rn, 0, shifter_op);
    ORC_ASM_CODE(p,"  %s%s %s, %s\n",
        dp_insn_names[opcode], orc_arm_cond_name(cond), orc_arm_reg_name (Rn), shifter);
  }
  orc_arm_emit (p, code);
}

/* parallel instructions */
/*    3   2 2 2 2 2     2 2 1     1 1     1   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | cond  |      mode     |   Rn  |  Rd   |0 0 0 0|  op   |  Rm   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define arm_code_mm(cond,mode,Rn,Rd,op,Rm) (((cond)<<28)|((mode)<<20)|((Rn)<<16)|((Rd)<<12)|((op)<<4)|(Rm))

void
orc_arm_emit_mm (OrcCompiler *p, const char *name, OrcArmCond cond, int mode,
            int op, int dest, int src1, int src2)
{
  uint32_t code;

  code = arm_code_mm (cond, mode, src1, dest, op, src2);
  ORC_ASM_CODE(p,"  %s%s %s, %s, %s\n",
      name, orc_arm_cond_name(cond),
      orc_arm_reg_name (dest),
      orc_arm_reg_name (src1),
      orc_arm_reg_name (src2));
  orc_arm_emit (p, code);
}
