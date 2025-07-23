/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: jinbo, jinbo@loongson.cn
  Author: hecai yuan, yuanhecai@loongson.cn

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
#include <orc/orcinternal.h>
#include <orc/loongarch/orcloongarchinsn.h>
#include <orc/loongarch/orcloongarch.h>

//implement scalar functions
void
orc_loongarch_insn_emit_addi_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int imm12) {
  ORC_ASM_CODE (c, "  addi.d %s, %s, %d\n", NAME (rd), NAME (rj), imm12);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(0x0b,GREG(rd),GREG(rj),imm12));
}

void
orc_loongarch_insn_emit_add_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk) {
  ORC_ASM_CODE (c, "  add.d %s, %s, %s\n", NAME (rd), NAME (rj), NAME (rk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(0x21,GREG(rd),GREG(rj),GREG(rk)));
}

void
orc_loongarch_insn_emit_sub_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk) {
  ORC_ASM_CODE (c, "  sub.d %s, %s, %s\n", NAME (rd), NAME (rj), NAME (rk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(0x23,GREG(rd),GREG(rj),GREG(rk)));
}

void
orc_loongarch_insn_emit_sll_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk) {
  ORC_ASM_CODE (c, "  sll.d %s, %s, %s\n", NAME (rd), NAME (rj), NAME (rk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(0x31,GREG(rd),GREG(rj),GREG(rk)));
}

void
orc_loongarch_insn_emit_slli_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int imm8) {
  ORC_ASM_CODE (c, "  slli.d %s, %s, %d\n", NAME (rd), NAME (rj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(0x10,GREG(rd),GREG(rj),(imm8 & 0x3f) | 0x40));
}

void
orc_loongarch_insn_emit_srl_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk) {
  ORC_ASM_CODE (c, "  srl.d %s, %s, %s\n", NAME (rd), NAME (rj), NAME (rk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(0x32,GREG(rd),GREG(rj),GREG(rk)));
}

void
orc_loongarch_insn_emit_srli_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int imm8) {
  ORC_ASM_CODE (c, "  srli.d %s, %s, %d\n", NAME (rd), NAME (rj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(0x11,GREG(rd),GREG(rj),(imm8 & 0x3f) | 0x40));
}

void
orc_loongarch_insn_emit_and (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk) {
  ORC_ASM_CODE (c, "  and %s, %s, %s\n", NAME (rd), NAME (rj), NAME (rk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(0x29,GREG(rd),GREG(rj),GREG(rk)));
}

void
orc_loongarch_insn_emit_andi (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int imm12) {
  ORC_ASM_CODE (c, "  andi %s, %s, %d\n", NAME (rd), NAME (rj), imm12);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(0x0D,GREG(rd),GREG(rj),imm12));
}

void
orc_loongarch_insn_emit_ld_w (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int offset) {
  ORC_ASM_CODE (c, "  ld.w %s, %s, %d\n", NAME (rd), NAME (rj), offset);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(0xA2,GREG(rd),GREG(rj),offset));
}

void
orc_loongarch_insn_emit_ld_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int offset) {
  ORC_ASM_CODE (c, "  ld.d %s, %s, %d\n", NAME (rd), NAME (rj), offset);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(0xA3,GREG(rd),GREG(rj),offset));
}

void
orc_loongarch_insn_emit_beqz (OrcCompiler *c, OrcLoongRegister rj, int label, int type) {
  ORC_ASM_CODE (c, "  beqz %s, .L%s_%d\n", NAME (rj), c->program->name, label);
  //add fixup
  orc_loongarch_insn_emit32 (c, LOONG_1RI21_INSTRUCTION(0x10,GREG(rj),0));
}

void
orc_loongarch_insn_emit_bnez (OrcCompiler *c, OrcLoongRegister rj, int label, int type) {
  ORC_ASM_CODE (c, "  bnez %s, .L%s_%d\n", NAME (rj), c->program->name, label);
  //add fixup
  orc_loongarch_insn_emit32 (c, LOONG_1RI21_INSTRUCTION(0x11,GREG(rj),0));
}

void
orc_loongarch_insn_emit_blt (OrcCompiler *c, OrcLoongRegister rj, OrcLoongRegister rd, int label, int type) {
  ORC_ASM_CODE (c, "  blt %s, %s, .L%s_%d\n", NAME(rj), NAME (rd), c->program->name, label);
  //add fixup
  orc_loongarch_insn_emit32 (c, LOONG_2RI16_INSTRUCTION(0x18,GREG(rd),GREG(rj),0));
}

void
orc_loongarch_insn_emit_st_d (OrcCompiler *c,
     OrcLoongRegister rd, OrcLoongRegister rj, int offset) {
  ORC_ASM_CODE (c, "  st.d %s, %s, %d\n", NAME (rd), NAME (rj), offset);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(0xA7,GREG(rd),GREG(rj),offset));
}

void
orc_loongarch_insn_emit_ret (OrcCompiler *c) {
  ORC_ASM_CODE (c, "  jirl %s, %s, 0x0\n", NAME (ORC_LOONG_ZERO), NAME (ORC_LOONG_RA));
  orc_loongarch_insn_emit32 (c, LOONG_2RI16_INSTRUCTION(0x13,GREG(ORC_LOONG_ZERO),GREG(ORC_LOONG_RA),0x00));
}
