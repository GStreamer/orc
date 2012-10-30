#include <orc/orcmips.h>
#include <orc/orcdebug.h>
#include <stdlib.h>

void
mips_rule_load (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = compiler->vars[insn->src_args[0]].ptr_register;
  int dest = compiler->vars[insn->dest_args[0]].alloc;
  /* such that 2^total_shift is the amount to load at a time */
  int total_shift = compiler->insn_shift + ORC_PTR_TO_INT (user);
  int is_aligned = compiler->vars[insn->src_args[0]].is_aligned;

  if (compiler->vars[insn->src_args[0]].vartype == ORC_VAR_TYPE_CONST) {
    ORC_PROGRAM_ERROR (compiler, "not implemented");
    return;
  }

  ORC_DEBUG ("insn_shift=%d", compiler->insn_shift);
  /* FIXME: Check alignment. We are assuming data is aligned here */
  switch (total_shift) {
  case 0:
    orc_mips_emit_lb (compiler, dest, src, 0);
    break;
  case 1:
    if (is_aligned) {
      orc_mips_emit_lh (compiler, dest, src, 0);
    } else {
      orc_mips_emit_lb (compiler, ORC_MIPS_T3, src, 0);
      orc_mips_emit_lb (compiler, dest, src, 1);
      orc_mips_emit_append (compiler, dest, ORC_MIPS_T3, 8);
    }
    break;
  case 2:
    if (is_aligned) {
      orc_mips_emit_lw (compiler, dest, src, 0);
    } else {
      /* note: the code below is little endian specific */
      orc_mips_emit_lwr (compiler, dest, src, 0);
      orc_mips_emit_lwl (compiler, dest, src, 3);
    }
    break;
  default:
    ORC_PROGRAM_ERROR(compiler, "Don't know how to handle that shift");
  }

}

void
mips_rule_store (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = compiler->vars[insn->src_args[0]].alloc;
  int dest = compiler->vars[insn->dest_args[0]].ptr_register;
  int total_shift = compiler->insn_shift + ORC_PTR_TO_INT (user);
  int is_aligned = compiler->vars[insn->dest_args[0]].is_aligned;

  ORC_DEBUG ("insn_shift=%d", compiler->insn_shift);

  /* FIXME: Check alignment. We are assuming data is aligned here */
  switch (total_shift) {
  case 0:
    orc_mips_emit_sb (compiler, src, dest, 0);
    break;
  case 1:
    if (is_aligned) {
      orc_mips_emit_sh (compiler, src, dest, 0);
    } else {
      /* Note: the code below is little endian specific */
      orc_mips_emit_sb (compiler, src, dest, 0);
      orc_mips_emit_srl (compiler, ORC_MIPS_T3, src, 8);
      orc_mips_emit_sb (compiler, ORC_MIPS_T3, dest, 1);
    }
    break;
  case 2:
    if (is_aligned) {
      orc_mips_emit_sw (compiler, src, dest, 0);
    } else {
      orc_mips_emit_swr (compiler, src, dest, 0);
      orc_mips_emit_swl (compiler, src, dest, 3);
    }
    break;
  default:
    ORC_PROGRAM_ERROR(compiler, "Don't know how to handle that shift");
  }
}


void
mips_rule_addl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addu (compiler, dest, src1, src2);
}

void
mips_rule_addw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  switch (compiler->insn_shift) {
  case 0:
    orc_mips_emit_addu (compiler, dest, src1, src2);
    break;
  case 1:
    orc_mips_emit_addu_ph (compiler, dest, src1, src2);
    break;
  default:
    ORC_PROGRAM_ERROR (compiler, "Don't know how to handle that insn_shift");
  }
}

void
mips_rule_addb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  switch (compiler->insn_shift) {
  case 0:
    orc_mips_emit_addu (compiler, dest, src1, src2);
    break;
  case 1:
  case 2:
    orc_mips_emit_addu_qb (compiler, dest, src1, src2);
    break;
  default:
    ORC_PROGRAM_ERROR (compiler, "Don't know how to handle that insn_shift");
  }

}


void
mips_rule_copyl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_move (compiler, dest, src);
}

void
mips_rule_copyw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_move (compiler, dest, src);
}

void
mips_rule_copyb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_move (compiler, dest, src);
}

void
orc_compiler_orc_mips_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

  orc_rule_register (rule_set, "loadl", mips_rule_load, (void *) 2);
  orc_rule_register (rule_set, "loadw", mips_rule_load, (void *) 1);
  orc_rule_register (rule_set, "loadb", mips_rule_load, (void *) 0);
  orc_rule_register (rule_set, "storel", mips_rule_store, (void *)2);
  orc_rule_register (rule_set, "storew", mips_rule_store, (void *)1);
  orc_rule_register (rule_set, "storeb", mips_rule_store, (void *)0);
  orc_rule_register (rule_set, "addl", mips_rule_addl, NULL);
  orc_rule_register (rule_set, "addw", mips_rule_addw, NULL);
  orc_rule_register (rule_set, "addb", mips_rule_addb, NULL);
  orc_rule_register (rule_set, "copyl", mips_rule_copyl, NULL);
  orc_rule_register (rule_set, "copyw", mips_rule_copyw, NULL);
  orc_rule_register (rule_set, "copyb", mips_rule_copyb, NULL);
}
