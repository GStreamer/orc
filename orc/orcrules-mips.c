#include <orc/orcmips.h>
#include <stdlib.h>

void
mips_rule_loadl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = compiler->vars[insn->src_args[0]].ptr_register;
  int dest = compiler->vars[insn->dest_args[0]].alloc;

  orc_mips_emit_lw (compiler, dest, src, 0);
}

void
mips_rule_storel (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = compiler->vars[insn->src_args[0]].alloc;
  int dest = compiler->vars[insn->dest_args[0]].ptr_register;

  orc_mips_emit_sw (compiler, src, dest, 0);
}

void
mips_rule_addl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_add (compiler, dest, src1, src2);
}

void
mips_rule_copyl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
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

  orc_rule_register (rule_set, "loadl", mips_rule_loadl, NULL);
  orc_rule_register (rule_set, "storel", mips_rule_storel, NULL);
  orc_rule_register (rule_set, "addl", mips_rule_addl, NULL);
  orc_rule_register (rule_set, "copyl", mips_rule_copyl, NULL);
}
