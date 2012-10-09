#include <orc/orcmips.h>
#include <orc/orcdebug.h>

const char *
orc_mips_reg_name (int reg)
{
  static const char *regs[] = {
    "$0", "$at",
    "$v0", "$v1",
    "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3","$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3","$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9",
    "$k0", "$k1",
    "$gp", "$sp", "$fp", "$ra"
  };

  if (reg < ORC_GP_REG_BASE || reg > ORC_GP_REG_BASE + 32)
    return "ERROR";

  return regs[reg-32];
}

void
orc_mips_emit_label (OrcCompiler *compiler, unsigned int label)
{
  ORC_ASSERT (label < ORC_N_LABELS);
  ORC_ASM_CODE(compiler,".L%d:\n", label);
  //compiler->labels[label] = compiler->codeptr;
}

void
orc_mips_emit_nop (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  nop\n");
}

void
orc_mips_emit_sw (OrcCompiler *compiler, OrcMipsRegister reg,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  sw      %s, %d(%s)\n",
                orc_mips_reg_name (reg),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_lw (OrcCompiler *compiler, OrcMipsRegister dest,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lw      %s, %d(%s)\n",
                orc_mips_reg_name (dest),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_jr (OrcCompiler *compiler, OrcMipsRegister address_reg)
{
  ORC_ASM_CODE (compiler, "  jr      %s\n", orc_mips_reg_name (address_reg));
}

void
orc_mips_emit_blez (OrcCompiler *compiler,
                    OrcMipsRegister reg, unsigned int label)
{
  ORC_ASM_CODE (compiler, "  blez    %s, .L%d\n",
                orc_mips_reg_name (reg), label);
}

void
orc_mips_emit_bnez (OrcCompiler *compiler,
                    OrcMipsRegister reg, unsigned int label)
{
  ORC_ASM_CODE (compiler, "  bnez    %s, .L%d\n",
                orc_mips_reg_name (reg), label);
}

void
orc_mips_emit_addiu (OrcCompiler *compiler,
                     OrcMipsRegister dest, OrcMipsRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  addiu   %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), value);
}
void
orc_mips_emit_add (OrcCompiler *compiler,
                   OrcMipsRegister dest,
                   OrcMipsRegister source1, OrcMipsRegister source2)
{
  ORC_ASM_CODE (compiler, "  add     %s, %s, %s\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source1),
                orc_mips_reg_name (source2));
}

void
orc_mips_emit_move (OrcCompiler *compiler,
                    OrcMipsRegister dest, OrcMipsRegister source)
{
  ORC_ASM_CODE (compiler, "  move    %s, %s\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source));
}
