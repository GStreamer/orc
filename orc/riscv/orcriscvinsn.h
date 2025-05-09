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

#ifndef _ORC_RISCV_INSN_H_
#define _ORC_RISCV_INSN_H_

#include <orc/orc.h>
#include <orc/orcutils.h>
#include <orc/riscv/orcriscv.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/* Scalar instructions */
ORC_API void orc_riscv_insn_emit_add (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_addi (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_sub (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_sll (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_slli (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_or (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_and (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_xor (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_slt (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_sra (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_sb (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm);
ORC_API void orc_riscv_insn_emit_lb (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_sh (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm);
ORC_API void orc_riscv_insn_emit_lh (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_sw (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm);
ORC_API void orc_riscv_insn_emit_lw (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_sd (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm);
ORC_API void orc_riscv_insn_emit_ld (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_lui (OrcCompiler *c, OrcRiscvRegister rd, int imm);
ORC_API void orc_riscv_insn_emit_beq (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_beqz (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_bne (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_bnez (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_blt (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_bltz (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_bltu (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_bltuz (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_bge (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_bgez (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_bgeu (OrcCompiler *c, OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label);
ORC_API void orc_riscv_insn_emit_bgeuz (OrcCompiler *c, OrcRiscvRegister rs1, int label);
ORC_API void orc_riscv_insn_emit_jal (OrcCompiler *c, OrcRiscvRegister rd, int imm);
ORC_API void orc_riscv_insn_emit_jalr (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_csrrw (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int csr);
ORC_API void orc_riscv_insn_emit_csrrs (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int csr);
ORC_API void orc_riscv_insn_emit_csrrc (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int csr);

/* Vector instructions */
ORC_API void orc_riscv_insn_emit_vsetvli (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvVtype vtype);
ORC_API void orc_riscv_insn_emit_vle8 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vle16 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vle32 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vle64 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vse8 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vse16 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vse32 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vse64 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vadd_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vadd_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vadd_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);

/* Pseudoinstructions */
ORC_API void orc_riscv_insn_emit_shift_add (OrcCompiler *c, OrcRiscvRegister rd, orc_uint32 imm);
ORC_API void orc_riscv_insn_emit_load_word (OrcCompiler *c, OrcRiscvRegister rd, orc_uint32 imm);
ORC_API void orc_riscv_insn_emit_load_immediate (OrcCompiler *c, OrcRiscvRegister rd, orc_uint64 imm);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_RISCV_INSN_H_ */
