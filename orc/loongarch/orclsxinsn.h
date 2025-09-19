/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: hecai yuan, yuanhecai@loongson.cn
  Author: jinbo, jinbo@loongson.cn

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

#ifndef _ORC_LSX_INSN_H_
#define _ORC_LSX_INSN_H_

#include <orc/orc.h>
#include <orc/orcutils.h>
#include <orc/loongarch/orcloongarch.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

ORC_API void orc_lsx_insn_emit_vaddb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vaddh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vaddw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vpickevw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vpickodw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vaddd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vreplgr2vrb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vreplgr2vrh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vreplgr2vrw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vreplgr2vrd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vldreplb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vldreplh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vldreplw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vldrepld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vst (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12);
ORC_API void orc_lsx_insn_emit_vldx (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_lsx_insn_emit_vstx (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, OrcLoongRegister rk);
ORC_API void orc_lsx_insn_emit_vsubb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsubh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsubw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsubd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsaddwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsadddu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssubdu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vorv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vxorv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vandnv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vandv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmind (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmaxdu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vminwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmindu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmulb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmulh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmulw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vmuhwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vabsdb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vabsdh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vabsdw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vabsdwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrbu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vavgrwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vdivhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsigncovb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsigncovh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsigncovw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vreplveih (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwilhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwilwh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwildw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwilhubu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwilwuhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vsllwilduwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui3);
ORC_API void orc_lsx_insn_emit_vilvlb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvlh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvlw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvhh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvhw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vilvhd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsltb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vslth (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsltw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsltd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vseqb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vseqh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vseqw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vseqd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vssrarnibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrarnihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrarniwd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlnibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlnihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlniwd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vshuf4ib (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vshuf4ih (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vshuf4iw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsllib (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsllih (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vslliw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsllid (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsllb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsllh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsllw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vslld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrlib (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlih (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrliw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlid (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrlb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrlh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrlw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsraib (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsraih (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsraiw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsraid (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrab (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrah (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsraw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsrad (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vsranibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsranihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrarnibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrarnihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vsrarniwd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlnibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlnihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlniwd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlnibuh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlnihuw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vssrlniwud (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm8);
ORC_API void orc_lsx_insn_emit_vbsrlv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui5);
ORC_API void orc_lsx_insn_emit_vbsllv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui5);
ORC_API void orc_lsx_insn_emit_vstelmb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx);
ORC_API void orc_lsx_insn_emit_vstelmh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx);
ORC_API void orc_lsx_insn_emit_vstelmw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx);
ORC_API void orc_lsx_insn_emit_vstelmd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx);
ORC_API void orc_lsx_insn_emit_vssranibh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui4);
ORC_API void orc_lsx_insn_emit_vssranihw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui5);
ORC_API void orc_lsx_insn_emit_vssraniwd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui6);
ORC_API void orc_lsx_insn_emit_vssranibuh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui4);
ORC_API void orc_lsx_insn_emit_vssranihuw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui5);
ORC_API void orc_lsx_insn_emit_vssraniwud (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int ui6);
ORC_API void orc_lsx_insn_emit_vfmuls (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfmuld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfdivs (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfdivd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vffintsw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vffintldw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vffinthdw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vfsqrts (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vfsqrtd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vfadds (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfaddd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfsubs (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfsubd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vftintrzws (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vhaddwhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vftintrnewd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcvtsd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcvtlds (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vfcvthds (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vfcmpceqs (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpceqd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpclts (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpcltd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpcles (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpcled (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpcuns (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfcmpcund (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfmins (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfmind (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfmaxs (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vfmaxd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwwh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwdw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwqd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwhubu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwwuhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwduwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vhaddwqudu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vbitselv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk, OrcLoongRegister va);
ORC_API void orc_lsx_insn_emit_vseqiw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm5);
ORC_API void orc_lsx_insn_emit_vseqid (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, int imm5);
ORC_API void orc_lsx_insn_emit_vnorv (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk);
ORC_API void orc_lsx_insn_emit_vexthhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vexthwh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vexthdw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vexthhubu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vexthwuhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);
ORC_API void orc_lsx_insn_emit_vexthduwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj);

/* Pseudoinstructions */
ORC_API void orc_lsx_insn_emit_flush_subnormals (OrcCompiler *c, int element_width, OrcLoongRegister vs, OrcLoongRegister vd);
ORC_API OrcLoongRegister orc_lsx_insn_emit_normalize (OrcCompiler *c, OrcLoongRegister src, OrcLoongRegister dest, int element_width);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_LSX_INSN_H_ */
