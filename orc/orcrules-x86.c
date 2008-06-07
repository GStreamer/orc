
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>

#define SIZE 65536

/* rules */

static void
x86_rule_loadi_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  x86_emit_mov_imm_reg (p, 2,  p->vars[insn->args[2]].s16,
      p->vars[insn->args[0]].alloc);
}

static void
x86_rule_add_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  if (insn->rule_flag == ORC_RULE_REG_IMM) {
    int value = p->vars[insn->args[2]].s16;
    printf("  addw $%d, %%%s\n", value,
        x86_get_regname_16(p->vars[insn->args[0]].alloc));

    if (value >= -128 && value < 128) {
      *p->codeptr++ = 0x66;
      *p->codeptr++ = 0x83;
      x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 0);
      *p->codeptr++ = value;
    } else {
      *p->codeptr++ = 0x66;
      *p->codeptr++ = 0x81;
      x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 0);
      *p->codeptr++ = value & 0xff;
      *p->codeptr++ = value >> 8;
    }
  } else {
    printf("  addw %%%s, %%%s\n",
        x86_get_regname_16(p->vars[insn->args[2]].alloc),
        x86_get_regname_16(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x03;
    x86_emit_modrm_reg (p, p->vars[insn->args[2]].alloc,
        p->vars[insn->args[0]].alloc);
  }
}

static void
x86_rule_sub_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  subw %%%s, %%%s\n",
      x86_get_regname_16(p->vars[insn->args[2]].alloc),
      x86_get_regname_16(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x2b;
  x86_emit_modrm_reg (p, p->vars[insn->args[2]].alloc,
      p->vars[insn->args[0]].alloc);
}

static void
x86_rule_mul_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  imulw %%%s, %%%s\n",
      x86_get_regname_16(p->vars[insn->args[2]].alloc),
      x86_get_regname_16(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xaf;
  x86_emit_modrm_reg (p, p->vars[insn->args[2]].alloc,
      p->vars[insn->args[0]].alloc);
}

static void
x86_rule_lshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  x86_emit_mov_reg_reg(p, 4, p->vars[insn->args[2]].alloc, X86_ECX);

  printf("  shlw %%cl, %%%s\n",
      x86_get_regname_16(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0xd3;
  x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 4);
}

static void
x86_rule_rshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  if (insn->rule_flag == ORC_RULE_REG_IMM) {
    printf("  sarw $%d, %%%s\n",
        p->vars[insn->args[2]].s16,
        x86_get_regname_16(p->vars[insn->args[0]].alloc));

    if (p->vars[insn->args[2]].s16 == 1) {
      *p->codeptr++ = 0x66;
      *p->codeptr++ = 0xd1;
      x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 7);
    } else {
      *p->codeptr++ = 0x66;
      *p->codeptr++ = 0xc1;
      x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 7);
      *p->codeptr++ = p->vars[insn->args[2]].s16;
    }
  } else {
    x86_emit_mov_reg_reg(p, 4, p->vars[insn->args[2]].alloc, X86_ECX);

    printf("  sarw %%cl, %%%s\n",
        x86_get_regname_16(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0xd3;
    x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 7);
  }
}


void
orc_program_x86_register_rules (void)
{
  orc_rule_register ("_loadi_s16", ORC_RULE_SCALAR_1, x86_rule_loadi_s16, NULL,
      ORC_RULE_REG_IMM);

  orc_rule_register ("add_s16", ORC_RULE_SCALAR_1, x86_rule_add_s16, NULL,
      ORC_RULE_REG_REG | ORC_RULE_REG_IMM);
  orc_rule_register ("sub_s16", ORC_RULE_SCALAR_1, x86_rule_sub_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("mul_s16", ORC_RULE_SCALAR_1, x86_rule_mul_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("lshift_s16", ORC_RULE_SCALAR_1, x86_rule_lshift_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("rshift_s16", ORC_RULE_SCALAR_1, x86_rule_rshift_s16, NULL,
      ORC_RULE_REG_REG | ORC_RULE_REG_IMM);
}

