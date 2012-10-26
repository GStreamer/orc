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
  ORC_ASM_CODE(compiler,".L%s%d:\n", compiler->program->name, label);
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
orc_mips_emit_swr (OrcCompiler *compiler, OrcMipsRegister reg,
                   OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  swr     %s, %d(%s)\n",
                orc_mips_reg_name (reg),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_swl (OrcCompiler *compiler, OrcMipsRegister reg,
                   OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  swl     %s, %d(%s)\n",
                orc_mips_reg_name (reg),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_sh (OrcCompiler *compiler, OrcMipsRegister reg,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  sh      %s, %d(%s)\n",
                orc_mips_reg_name (reg),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_sb (OrcCompiler *compiler, OrcMipsRegister reg,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  sb      %s, %d(%s)\n",
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
orc_mips_emit_lwr (OrcCompiler *compiler, OrcMipsRegister dest,
                   OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lwr     %s, %d(%s)\n",
                orc_mips_reg_name (dest),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_lwl (OrcCompiler *compiler, OrcMipsRegister dest,
                   OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lwl     %s, %d(%s)\n",
                orc_mips_reg_name (dest),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_lh (OrcCompiler *compiler, OrcMipsRegister dest,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lh      %s, %d(%s)\n",
                orc_mips_reg_name (dest),
                offset, orc_mips_reg_name (base));
}

void
orc_mips_emit_lb (OrcCompiler *compiler, OrcMipsRegister dest,
                  OrcMipsRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lb      %s, %d(%s)\n",
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
  ORC_ASM_CODE (compiler, "  blez    %s, .L%s%d\n",
                orc_mips_reg_name (reg), compiler->program->name, label);
}

void
orc_mips_emit_bnez (OrcCompiler *compiler,
                    OrcMipsRegister reg, unsigned int label)
{
  ORC_ASM_CODE (compiler, "  bnez    %s, .L%s%d\n",
                orc_mips_reg_name (reg), compiler->program->name, label);
}

void
orc_mips_emit_beqz (OrcCompiler *compiler,
                    OrcMipsRegister reg, unsigned int label)
{
  ORC_ASM_CODE (compiler, "  beqz    %s, .L%s%d\n",
                orc_mips_reg_name (reg), compiler->program->name, label);
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
orc_mips_emit_addi (OrcCompiler *compiler,
                     OrcMipsRegister dest, OrcMipsRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  addi    %s, %s, %d\n",
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
orc_mips_emit_addu (OrcCompiler *compiler,
                    OrcMipsRegister dest,
                    OrcMipsRegister source1, OrcMipsRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu    %s, %s, %s\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source1),
                orc_mips_reg_name (source2));
}

void
orc_mips_emit_addu_qb (OrcCompiler *compiler,
                    OrcMipsRegister dest,
                    OrcMipsRegister source1, OrcMipsRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu.qb %s, %s, %s\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source1),
                orc_mips_reg_name (source2));
}

void
orc_mips_emit_addu_ph (OrcCompiler *compiler,
                    OrcMipsRegister dest,
                    OrcMipsRegister source1, OrcMipsRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu.ph %s, %s, %s\n",
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

void
orc_mips_emit_sub (OrcCompiler *compiler,
                   OrcMipsRegister dest,
                   OrcMipsRegister source1, OrcMipsRegister source2)
{
  ORC_ASM_CODE (compiler, "  sub     %s, %s, %s\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source1),
                orc_mips_reg_name (source2));
}

void
orc_mips_emit_srl (OrcCompiler *compiler,
                     OrcMipsRegister dest, OrcMipsRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  srl     %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), value);
}

void
orc_mips_emit_sll (OrcCompiler *compiler,
                     OrcMipsRegister dest, OrcMipsRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  sll     %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), value);
}

void
orc_mips_emit_andi (OrcCompiler *compiler,
                     OrcMipsRegister dest, OrcMipsRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  andi    %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), value);
}


void
orc_mips_emit_prepend (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister source, int shift_amount)
{
  ORC_ASM_CODE (compiler, "  prepend %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), shift_amount);
}

void
orc_mips_emit_append (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister source, int shift_amount)
{
  ORC_ASM_CODE (compiler, "  append  %s, %s, %d\n",
                orc_mips_reg_name (dest),
                orc_mips_reg_name (source), shift_amount);
}

