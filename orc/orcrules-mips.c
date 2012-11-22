#include <orc/orcmips.h>
#include <orc/orcdebug.h>
#include <stdlib.h>

#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)

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
      orc_mips_emit_lb (compiler, compiler->tmpreg, src, 0);
      orc_mips_emit_lb (compiler, dest, src, 1);
      orc_mips_emit_append (compiler, dest, compiler->tmpreg, 8);
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
      orc_mips_emit_srl (compiler, compiler->tmpreg, src, 8);
      orc_mips_emit_sb (compiler, compiler->tmpreg, dest, 1);
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
mips_rule_mul (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_mul (compiler, dest, src1, src2);
}

void
mips_rule_shrs (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int shift = ORC_SRC_VAL (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_sra (compiler, dest, src, shift);
}

void
mips_rule_convssslw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (dest != src)
    orc_mips_emit_move (compiler, dest, src);
  orc_mips_emit_ori (compiler, ORC_MIPS_T3, ORC_MIPS_ZERO, ORC_SW_MAX);
  orc_mips_emit_slt (compiler, ORC_MIPS_T4, ORC_MIPS_T3, src);
  orc_mips_emit_movn (compiler, dest, ORC_MIPS_T3, ORC_MIPS_T4);
  orc_mips_emit_lui (compiler, ORC_MIPS_T3, (ORC_SW_MIN >> 16) & 0xffff);
  orc_mips_emit_ori (compiler, ORC_MIPS_T3, ORC_MIPS_T3, ORC_SW_MAX & 0xffff);
  /* this still works if src == dest since in that case, its value is either
   * the original src or ORC_SW_MAX, which works as well here */
  orc_mips_emit_slt (compiler, ORC_MIPS_T4, src, ORC_MIPS_T3);
  orc_mips_emit_movn (compiler, dest, ORC_MIPS_T3, ORC_MIPS_T4);
}

void
mips_rule_mergewl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (dest != src1)
    orc_mips_emit_move (compiler, dest, src1);
  orc_mips_emit_append (compiler, dest, src2, 16);
}

void
mips_rule_addssw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addq_s_ph (compiler, dest, src1, src2);
}

void
mips_rule_loadp (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    if (size == 1 || size == 2) {
      orc_mips_emit_ori (compiler, dest->alloc, ORC_MIPS_ZERO, src->value.i);
    } else if (size == 4) {
      orc_int16 high_bits;
      high_bits = ((src->value.i >> 16) & 0xffff);
      if (high_bits) {
        orc_mips_emit_lui (compiler, dest->alloc, high_bits);
        orc_mips_emit_ori (compiler, dest->alloc, dest->alloc, src->value.i & 0xffff);
      } else {
        orc_mips_emit_ori (compiler, dest->alloc, ORC_MIPS_ZERO, src->value.i & 0xffff);
      }
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  } else {
    if (size == 1) {
      orc_mips_emit_lb (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
    } else if (size == 2) {
      orc_mips_emit_lh (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
    } else if (size == 4) {
      orc_mips_emit_lw (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  }
}


void
orc_compiler_orc_mips_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

  orc_rule_register (rule_set, "loadl", mips_rule_load, (void *) 2);
  orc_rule_register (rule_set, "loadw", mips_rule_load, (void *) 1);
  orc_rule_register (rule_set, "loadb", mips_rule_load, (void *) 0);
  orc_rule_register (rule_set, "loadpl", mips_rule_loadp, (void *) 4);
  orc_rule_register (rule_set, "loadpw", mips_rule_loadp, (void *) 2);
  orc_rule_register (rule_set, "loadpb", mips_rule_loadp, (void *) 1);
  orc_rule_register (rule_set, "storel", mips_rule_store, (void *)2);
  orc_rule_register (rule_set, "storew", mips_rule_store, (void *)1);
  orc_rule_register (rule_set, "storeb", mips_rule_store, (void *)0);
  orc_rule_register (rule_set, "addl", mips_rule_addl, NULL);
  orc_rule_register (rule_set, "addw", mips_rule_addw, NULL);
  orc_rule_register (rule_set, "addb", mips_rule_addb, NULL);
  orc_rule_register (rule_set, "copyl", mips_rule_copyl, NULL);
  orc_rule_register (rule_set, "copyw", mips_rule_copyw, NULL);
  orc_rule_register (rule_set, "copyb", mips_rule_copyb, NULL);
  orc_rule_register (rule_set, "mulswl", mips_rule_mul, NULL);
  orc_rule_register (rule_set, "shrsl", mips_rule_shrs, NULL);
  orc_rule_register (rule_set, "convssslw", mips_rule_convssslw, NULL);
  orc_rule_register (rule_set, "mergewl", mips_rule_mergewl, NULL);
  orc_rule_register (rule_set, "addssw", mips_rule_addssw, NULL);
}
