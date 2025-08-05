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

#ifndef ORC_LOONGARCH_INSN_H_
#define ORC_LOONGARCH_INSN_H_

#include <orc/orc.h>
#include <orc/orcutils.h>
#include <orc/orcinternal.h>
#include <orc/loongarch/orcloongarch.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

ORC_API void orc_loongarch_insn_emit_addi_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int imm12);
ORC_API void orc_loongarch_insn_emit_add_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_loongarch_insn_emit_sub_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_loongarch_insn_emit_sll_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_loongarch_insn_emit_slli_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int imm8);
ORC_API void orc_loongarch_insn_emit_srl_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_loongarch_insn_emit_srli_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int imm8);
ORC_API void orc_loongarch_insn_emit_and (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_loongarch_insn_emit_andi (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int imm12);
ORC_API void orc_loongarch_insn_emit_ld_w (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int offset);
ORC_API void orc_loongarch_insn_emit_ld_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int offset);
ORC_API void orc_loongarch_insn_emit_beqz (OrcCompiler *c, OrcLoongRegister rj, int label, int type);
ORC_API void orc_loongarch_insn_emit_bnez (OrcCompiler *c, OrcLoongRegister rj, int label, int type);
ORC_API void orc_loongarch_insn_emit_blt (OrcCompiler *c, OrcLoongRegister rj, OrcLoongRegister rd, int label, int type);
ORC_API void orc_loongarch_insn_emit_st_d (OrcCompiler *c, OrcLoongRegister rd, OrcLoongRegister rj, int offset);
ORC_API void orc_loongarch_insn_emit_ret (OrcCompiler *c);
ORC_API void orc_loongarch_insn_emit_lu12i_w (OrcCompiler *p, OrcLoongRegister rd, int imm20);
ORC_API void orc_loongarch_insn_emit_lu32i_d (OrcCompiler *p, OrcLoongRegister rd, int imm20);
ORC_API void orc_loongarch_insn_emit_lu52i_d (OrcCompiler *p, OrcLoongRegister rd, OrcLoongRegister rj, int imm12);
ORC_API void orc_loongarch_insn_emit_ori (OrcCompiler *p, OrcLoongRegister rd, OrcLoongRegister rj, int imm12);
ORC_API void orc_loongarch_insn_emit_load_word (OrcCompiler *p, OrcLoongRegister rd, orc_uint32 imm);
ORC_API void orc_loongarch_insn_emit_load_imm (OrcCompiler *p, OrcLoongRegister rd, orc_uint64 imm);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif //ORC_LOONGARCH_INSN_H_
