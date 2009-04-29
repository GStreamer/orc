
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>

#define SIZE 65536

/* mmx rules */

void
orc_mmx_emit_loadiw (OrcCompiler *p, int reg, int value)
{
  if (value == 0) {
    ORC_ASM_CODE(p,"  pxor %%%s, %%%s\n", orc_x86_get_regname_mmx(reg),
        orc_x86_get_regname_mmx(reg));
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xef;
    orc_x86_emit_modrm_reg (p, reg, reg);
  } else {
#if 1
    orc_x86_emit_mov_imm_reg (p, 4, value, X86_ECX);

    ORC_ASM_CODE(p,"  movd %%ecx, %%%s\n", orc_x86_get_regname_mmx(reg));
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x6e;
    orc_x86_emit_modrm_reg (p, X86_ECX, reg);
#else
    ORC_ASM_CODE(p,"  pinsrw $0, %%ecx, %%%s\n", orc_x86_get_regname_mmx(reg));
#endif

    ORC_ASM_CODE(p,"  pshufw $0, %%%s, %%%s\n", orc_x86_get_regname_mmx(reg),
        orc_x86_get_regname_mmx(reg));

    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x70;
    orc_x86_emit_modrm_reg (p, reg, reg);
    *p->codeptr++ = 0x00;
  }
}

static void
mmx_rule_addw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  paddw %%%s, %%%s\n",
      orc_x86_get_regname_mmx(p->vars[insn->src_args[1]].alloc),
      orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xfd;
  orc_x86_emit_modrm_reg (p, p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
mmx_rule_subw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  psubw %%%s, %%%s\n",
      orc_x86_get_regname_mmx(p->vars[insn->src_args[1]].alloc),
      orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xf9;
  orc_x86_emit_modrm_reg (p, p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
mmx_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  pmullw %%%s, %%%s\n",
      orc_x86_get_regname_mmx(p->vars[insn->src_args[1]].alloc),
      orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xd5;
  orc_x86_emit_modrm_reg (p, p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
mmx_rule_shlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    ORC_ASM_CODE(p,"  psllw $%d, %%%s\n",
        p->vars[insn->src_args[1]].value,
        orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x71;
    orc_x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 6);
    *p->codeptr++ = p->vars[insn->src_args[1]].value;
  } else {
    /* FIXME this doesn't work quite right */
    ORC_ASM_CODE(p,"  psllw %%%s, %%%s\n",
        orc_x86_get_regname_mmx(p->vars[insn->src_args[1]].alloc),
        orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xf1;
    orc_x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
  }
}

static void
mmx_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    ORC_ASM_CODE(p,"  psraw $%d, %%%s\n",
        p->vars[insn->src_args[1]].value,
        orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x71;
    orc_x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 4);
    *p->codeptr++ = p->vars[insn->src_args[1]].value;
  } else {
    /* FIXME this doesn't work quite right */
    ORC_ASM_CODE(p,"  psraw %%%s, %%%s\n",
        orc_x86_get_regname_mmx(p->vars[insn->src_args[1]].alloc),
        orc_x86_get_regname_mmx(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xe1;
    orc_x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
  }
}

void
orc_compiler_mmx_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

  orc_rule_register (rule_set, "addw", mmx_rule_addw, NULL);
  orc_rule_register (rule_set, "subw", mmx_rule_subw, NULL);
  orc_rule_register (rule_set, "mullw", mmx_rule_mullw, NULL);
  orc_rule_register (rule_set, "shlw", mmx_rule_shlw, NULL);
  orc_rule_register (rule_set, "shrsw", mmx_rule_shrsw, NULL);
}

