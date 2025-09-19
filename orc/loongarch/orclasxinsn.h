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

#ifndef _ORC_LASX_INSN_H_
#define _ORC_LASX_INSN_H_

#include <orc/loongarch/orcloongarch.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

ORC_API void orc_lasx_insn_emit_xvldreplb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvldreplh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvldreplw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvldrepld (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvreplgr2vrb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj);
ORC_API void orc_lasx_insn_emit_xvreplgr2vrh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj);
ORC_API void orc_lasx_insn_emit_xvreplgr2vrw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj);
ORC_API void orc_lasx_insn_emit_xvreplgr2vrd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj);
ORC_API void orc_lasx_insn_emit_xvld (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvst (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister rj, int imm);
ORC_API void orc_lasx_insn_emit_xvorv (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvxorv (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvandnv (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvandv (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvaddb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvaddh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvaddw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvaddd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsaddwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsubb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsubh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsubw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsubd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvssubwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvminwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmaxwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvabsdb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvabsdh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvabsdw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvavgrwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmulb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmulh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmulw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuld (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhbu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvmuhwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsigncovb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsigncovh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsigncovw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvdivhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_vext2xvhb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_vext2xvwh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_vext2xvdw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_vext2xvhubu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_vext2xvwuhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_vext2xvduwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvshufb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk, OrcLoongRegister xa);
ORC_API void orc_lasx_insn_emit_xvshufh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvshufw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsltb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvslth (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsltw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsltd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvseqb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvseqh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvseqw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvseqd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvpermid (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvpermiq (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrarnibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrarnihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrarniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvrepl128veih (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlnibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlnihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvshuf4ib (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvshuf4iw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvshuf4ih (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllib (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllih (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvslliw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllid (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsllh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsllw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvslld (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrlib (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlih (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrliw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlid (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrlb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrlh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrlw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrld (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsraib (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsraih (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsraiw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsraid (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrab (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrah (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsraw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrad (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvsrarnibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrarnihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsrarniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwilhb (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwilwh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwildw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwilhubu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwilwuhu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsllwilduwu (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsranibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsranihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvsraniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssranibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssranihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssraniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssranibuh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssranihuw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssraniwud (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlnibuh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlnihuw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlniwud (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlnibh (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlnihw (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvssrlniwd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvilvlb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvlh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvlw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvld (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvstelmb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx);
ORC_API void orc_lasx_insn_emit_xvstelmh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx);
ORC_API void orc_lasx_insn_emit_xvstelmw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx);
ORC_API void orc_lasx_insn_emit_xvstelmd (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx);
ORC_API void orc_lasx_insn_emit_xvilvhb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvhh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvhw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvilvhd (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_vext2xvwubu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvabsdwu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvbsllv (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvhaddwwh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvhaddwdw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvhaddwqd (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvhaddwduwu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvhaddwqudu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvffintsw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvftintrzws (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvftintrnewd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcvtsd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfadds (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfaddd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfsubs (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfsubd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmuls (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmuld (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfdivs (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfdivd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfsqrts (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvfsqrtd (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj);
ORC_API void orc_lasx_insn_emit_xvfcmpceqs (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpceqd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpcles (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpcled (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpclts (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpcltd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpcuns (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfcmpcund (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmins (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmind (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmaxs (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvfmaxd (OrcCompiler *p, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lsx_insn_emit_xvseqiw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lsx_insn_emit_xvseqid (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, int imm);
ORC_API void orc_lasx_insn_emit_xvnorv (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk);
ORC_API void orc_lasx_insn_emit_xvbitselv (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk, OrcLoongRegister xa);

/* Pseudoinstructions */
ORC_API void orc_lasx_insn_emit_flush_subnormals (OrcCompiler *c, int element_width, OrcLoongRegister xs, OrcLoongRegister xd);
ORC_API OrcLoongRegister orc_lasx_insn_emit_normalize (OrcCompiler *c, OrcLoongRegister src, OrcLoongRegister dest, int element_width);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_LASX_INSN_H_ */
