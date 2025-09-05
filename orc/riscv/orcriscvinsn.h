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
ORC_API void orc_riscv_insn_emit_srli (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm);
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
ORC_API void orc_riscv_insn_emit_ret (OrcCompiler *c);
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
ORC_API void orc_riscv_insn_emit_vadd_vvm (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vadd_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vadd_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vadd_vim (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, int imm);
ORC_API void orc_riscv_insn_emit_vdiv_vv (OrcCompiler *c, OrcRiscvRegister vd,  OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vdivu_vv (OrcCompiler *c, OrcRiscvRegister vd,  OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vdivu_vx (OrcCompiler *c, OrcRiscvRegister vd,  OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vaadd_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vaaddu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vsll_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vsll_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vsll_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vwsll_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, int imm);
ORC_API void orc_riscv_insn_emit_vsrl_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vsrl_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vsrl_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vnsrl_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vnsrl_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vsra_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vsra_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vsra_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vmv_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmv_vvm (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmv_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vmv_vi (OrcCompiler *c, OrcRiscvRegister vd, int imm);
ORC_API void orc_riscv_insn_emit_vnmv_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vwmv_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vsadd_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vsaddu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vsub_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vsub_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vrsub_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vssubu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vssub_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vand_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vand_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vandn_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vand_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vor_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vor_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vxor_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vxor_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vxor_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmax_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmaxu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmax_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmaxu_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmin_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vminu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmin_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vminu_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vneg (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmsne_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmsne_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vmslt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmslt_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vmsle_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmsgt_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vmseq_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmseq_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1);
ORC_API void orc_riscv_insn_emit_vrev8 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmul_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmul_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmulh_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmulhu_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmulhu_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_vnclip_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, int imm);
ORC_API void orc_riscv_insn_emit_vnclip_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister rs2);
ORC_API void orc_riscv_insn_emit_vnclipu_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vnclipu_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, int imm);
ORC_API void orc_riscv_insn_emit_vnsra_vi (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm);
ORC_API void orc_riscv_insn_emit_vwadd_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vwaddu_vx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vext_z2 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vext_z4 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vext_s2 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vext_s4 (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfadd_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfadd_vvm (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfsub_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vfdiv_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vfmul_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfsqrt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmaxf_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vminf_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmflt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmfle_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmfeq_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmfeq_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vmfeq_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1);
ORC_API void orc_riscv_insn_emit_vfcvt_rtz_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfxcvt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vfxncvt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vffncvt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vxfncvt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vffwcvt_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vredsum_vv (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmv_xs (OrcCompiler *c, OrcRiscvRegister rd, OrcRiscvRegister vs2);
ORC_API void orc_riscv_insn_emit_vmv_sx (OrcCompiler *c, OrcRiscvRegister vd, OrcRiscvRegister rs1);

/* Pseudoinstructions */
ORC_API void orc_riscv_insn_emit_shift_add (OrcCompiler *c, OrcRiscvRegister rd, orc_uint32 imm);
ORC_API void orc_riscv_insn_emit_load_word (OrcCompiler *c, OrcRiscvRegister rd, orc_uint32 imm);
ORC_API void orc_riscv_insn_emit_load_immediate (OrcCompiler *c, OrcRiscvRegister rd, orc_uint64 imm);
ORC_API void orc_riscv_insn_emit_flush_subnormals(OrcCompiler *c, int element_width, OrcRiscvRegister vs, OrcRiscvRegister vd, OrcRiscvRegister vtemp);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_RISCV_INSN_H_ */
