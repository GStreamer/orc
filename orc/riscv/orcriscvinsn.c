/*
  Copyright © 2024 Samsung Electronics

  Author: Maksymilian Knust, m.knust@samsung.com
  Author: Filip Wasil, f.wasil@samsung.com

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orclimits.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include <orc/orcinternal.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>
#include <orc/riscv/orcriscvinsn.h>

static const char *
riscv_reg_name (OrcRiscvRegister reg)
{
  static const char *names[] = {
    [ORC_GP_REG_BASE] = "zero",
    "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0",
    "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6",
    "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8",
    "s9", "s10", "s11", "t3", "t4", "t5", "t6",

    [ORC_VEC_REG_BASE] = "v0",
    "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8",
    "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16",
    "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24",
    "v25", "v26", "v27", "v28", "v29", "v30", "v31"
  };

  ORC_ASSERT (reg < ARRAY_SIZE (names));
  ORC_ASSERT (names[reg] != NULL);

  return names[reg];
}

#define NAME(reg) riscv_reg_name(reg)

void
orc_riscv_insn_emit_add (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  add %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_addi (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  addi %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sub (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sub %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sll (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sll %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_slli (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  slli %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_or (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  or %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_and (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  and %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_xor (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  xor %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_slt (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  slt %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sra (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sra %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sb (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sb %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  /* TODO: emit machine code */
}


void
orc_riscv_insn_emit_lb (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lb %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sh (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sh %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_lh (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lh %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sw (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sw %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  /* TODO: emit machine code */
}


void
orc_riscv_insn_emit_lw (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lw %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_sd (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sd %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_ld (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  ld %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_lui (OrcCompiler *c, OrcRiscvRegister rd, int imm)
{
  ORC_ASM_CODE (c, "  lui %s, %d\n", NAME (rd), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_beq (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  beq %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_beqz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_beq (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bne (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bne %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_bnez (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bne (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_blt (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  blt %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_bltz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_blt (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bltu (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bltu %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_bltuz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bltu (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bge (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bge %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_bgez (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bge (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bgeu (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bgeu %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_bgeuz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bgeu (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_jal (OrcCompiler *c, OrcRiscvRegister rd, int imm)
{
  ORC_ASM_CODE (c, "  jal %s, %d\n", NAME (rd), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_jalr (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  jalr %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_csrrw (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrw %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_csrrs (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrs %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_csrrc (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrc %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_shift_add (OrcCompiler *c, OrcRiscvRegister rd,
    orc_uint32 imm)
{
  const int top10 = (imm >> 22) & 0x3FF;
  const int mid11 = (imm >> 11) & 0x7FF;
  const int bot11 = imm & 0x7FF;

  int shifted = 0;

  if (top10 != 0) {
    orc_riscv_insn_emit_slli (c, rd, rd, 10);
    orc_riscv_insn_emit_addi (c, rd, rd, top10);
    shifted = 10;
  }

  if (mid11 != 0) {
    orc_riscv_insn_emit_slli (c, rd, rd, 21 - shifted);
    orc_riscv_insn_emit_addi (c, rd, rd, mid11);
    shifted = 21;
  }

  orc_riscv_insn_emit_slli (c, rd, rd, 32 - shifted);
  if (bot11 != 0) {
    orc_riscv_insn_emit_addi (c, rd, rd, bot11);
  }
}

void
orc_riscv_insn_emit_load_word (OrcCompiler *c, OrcRiscvRegister rd,
    orc_uint32 imm)
{
  const int lower = (imm & 0xFFF) | -(imm & 0x800);
  const int upper = ((imm >> 12) + (lower < 0)) & 0xFFFFF;

  if (upper != 0)
    orc_riscv_insn_emit_lui (c, rd, upper);

  if (lower != 0 || upper == 0)
    orc_riscv_insn_emit_addi (c, rd, upper ? rd : ORC_RISCV_ZERO, lower);
}

void
orc_riscv_insn_emit_load_immediate (OrcCompiler *c,
    OrcRiscvRegister rd, orc_uint64 imm)
{
  const uint32_t bit31 = (imm >> 31) & 1;
  const uint32_t lower = imm & 0xFFFFFFFFu;
  const uint32_t upper = sizeof (long) == 4 ? 0 : (imm >> 32);

  if ((upper ^ -bit31) == 0) {
    orc_riscv_insn_emit_load_word (c, rd, lower);
  } else if (lower == 0) {
    orc_riscv_insn_emit_load_word (c, rd, upper);
    orc_riscv_insn_emit_slli (c, rd, rd, 32);
  } else if (rd != c->gp_tmpreg) {
    orc_riscv_insn_emit_load_word (c, rd, (upper ^ -bit31));
    orc_riscv_insn_emit_slli (c, rd, rd, 32);
    orc_riscv_insn_emit_load_word (c, c->gp_tmpreg, lower);
    orc_riscv_insn_emit_xor (c, rd, rd, c->gp_tmpreg);
  } else {
    orc_riscv_insn_emit_load_word (c, rd, upper);
    orc_riscv_insn_emit_shift_add (c, rd, lower);
  }
}
