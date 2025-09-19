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

#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orclimits.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>
#include <orc/orcinternal.h>
#include <orc/loongarch/orclasx-internal.h>
#include <orc/loongarch/orcloongarch.h>
#include <orc/loongarch/orcloongarchinsn.h>
#include <orc/loongarch/orclasxinsn.h>

typedef enum
{
  OPCODE_XVLDREPLB     = 0b00110010,
  OPCODE_XVLDREPLH     = 0b00110010,
  OPCODE_XVLDREPLW     = 0b00110010,
  OPCODE_XVLDREPLD     = 0b00110010,
  OPCODE_XVREPLGR2VRB  = 0b0111011010011111000000,
  OPCODE_XVREPLGR2VRH  = 0b0111011010011111000001,
  OPCODE_XVREPLGR2VRW  = 0b0111011010011111000010,
  OPCODE_XVREPLGR2VRD  = 0b0111011010011111000011,
  OPCODE_XVLD          = 0b0010110010,
  OPCODE_XVST          = 0b0010110011,
  OPCODE_XVORV         = 0b01110101001001101,
  OPCODE_XVXORV        = 0b01110101001001110,
  OPCODE_XVANDNV       = 0b01110101001010000,
  OPCODE_XVADDB        = 0b01110100000010100,
  OPCODE_XVADDH        = 0b01110100000010101,
  OPCODE_XVADDW        = 0b01110100000010110,
  OPCODE_XVADDD        = 0b01110100000010111,
  OPCODE_XVSADDB       = 0b01110100010001100,
  OPCODE_XVSADDH       = 0b01110100010001101,
  OPCODE_XVSADDW       = 0b01110100010001110,
  OPCODE_XVSADDBU      = 0b01110100010010100,
  OPCODE_XVSADDHU      = 0b01110100010010101,
  OPCODE_XVSADDWU      = 0b01110100010010110,
  OPCODE_XVSUBB        = 0b01110100000011000,
  OPCODE_XVSUBH        = 0b01110100000011001,
  OPCODE_XVSUBW        = 0b01110100000011010,
  OPCODE_XVSUBD        = 0b01110100000011011,
  OPCODE_XVSSUBB       = 0b01110100010010000,
  OPCODE_XVSSUBH       = 0b01110100010010001,
  OPCODE_XVSSUBW       = 0b01110100010010010,
  OPCODE_XVSSUBBU      = 0b01110100010011000,
  OPCODE_XVSSUBHU      = 0b01110100010011001,
  OPCODE_XVSSUBWU      = 0b01110100010011010,
  OPCODE_XVANDV        = 0b01110101001001100,
  OPCODE_XVMINB        = 0b01110100011100100,
  OPCODE_XVMINH        = 0b01110100011100101,
  OPCODE_XVMINW        = 0b01110100011100110,
  OPCODE_XVMINBU       = 0b01110100011101100,
  OPCODE_XVMINHU       = 0b01110100011101101,
  OPCODE_XVMINWU       = 0b01110100011101110,
  OPCODE_XVMAXB        = 0b01110100011100000,
  OPCODE_XVMAXH        = 0b01110100011100001,
  OPCODE_XVMAXW        = 0b01110100011100010,
  OPCODE_XVMAXBU       = 0b01110100011101000,
  OPCODE_XVMAXHU       = 0b01110100011101001,
  OPCODE_XVMAXWU       = 0b01110100011101010,
  OPCODE_XVABSDB       = 0b01110100011000000,
  OPCODE_XVABSDH       = 0b01110100011000001,
  OPCODE_XVABSDW       = 0b01110100011000010,
  OPCODE_XVAVGRB       = 0b01110100011010000,
  OPCODE_XVAVGRH       = 0b01110100011010001,
  OPCODE_XVAVGRW       = 0b01110100011010010,
  OPCODE_XVAVGRBU      = 0b01110100011010100,
  OPCODE_XVAVGRHU      = 0b01110100011010101,
  OPCODE_XVAVGRWU      = 0b01110100011010110,
  OPCODE_XVMULB        = 0b01110100100001000,
  OPCODE_XVMULH        = 0b01110100100001001,
  OPCODE_XVMULW        = 0b01110100100001010,
  OPCODE_XVMULD        = 0b01110100100001011,
  OPCODE_XVMUHB        = 0b01110100100001100,
  OPCODE_XVMUHH        = 0b01110100100001101,
  OPCODE_XVMUHW        = 0b01110100100001110,
  OPCODE_XVSIGNCOVB    = 0b01110101001011100,
  OPCODE_XVSIGNCOVH    = 0b01110101001011101,
  OPCODE_XVSIGNCOVW    = 0b01110101001011110,
  OPCODE_XVDIVHU       = 0b01110100111001001,
  OPCODE_VEXT2XVHB     = 0b0111011010011111000100,
  OPCODE_VEXT2XVWH     = 0b0111011010011111000111,
  OPCODE_VEXT2XVDW     = 0b0111011010011111001001,
  OPCODE_VEXT2XVHUBU   = 0b0111011010011111001010,
  OPCODE_VEXT2XVWUHU   = 0b0111011010011111001101,
  OPCODE_VEXT2XVDUWU   = 0b0111011010011111001111,
  OPCODE_XVSHUFB       = 0b000011010110,
  OPCODE_XVSHUFH       = 0b01110101011110101,
  OPCODE_XVSHUFW       = 0b01110101011110110,
  OPCODE_XVSLTB        = 0b01110100000001100,
  OPCODE_XVSLTH        = 0b01110100000001101,
  OPCODE_XVSLTW        = 0b01110100000001110,
  OPCODE_XVSLTD        = 0b01110100000001111,
  OPCODE_XVSEQB        = 0b01110100000000000,
  OPCODE_XVSEQH        = 0b01110100000000001,
  OPCODE_XVSEQW        = 0b01110100000000010,
  OPCODE_XVSEQD        = 0b01110100000000011,
  OPCODE_XVPERMID      = 0b01110111111010,
  OPCODE_XVPERMIQ      = 0b01110111111011,
  OPCODE_XVSSRARNIBH   = 0b01110111011010,
  OPCODE_XVSSRARNIHW   = 0b01110111011010,
  OPCODE_XVSSRARNIWD   = 0b01110111011010,
  OPCODE_XVREPL128VEIH = 0b01110110111101,
  OPCODE_XVSRLNIBH     = 0b01110111010000,
  OPCODE_XVSRLNIHW     = 0b01110111010000,
  OPCODE_XVSRLNIWD     = 0b01110111010000,
  OPCODE_XVSHUF4IB     = 0b01110111100100,
  OPCODE_XVSHUF4IH     = 0b01110111100101,
  OPCODE_XVSHUF4IW     = 0b01110111100110,
  OPCODE_XVSLLIB       = 0b01110111001011,
  OPCODE_XVSLLIH       = 0b01110111001011,
  OPCODE_XVSLLIW       = 0b01110111001011,
  OPCODE_XVSLLID       = 0b01110111001011,
  OPCODE_XVSLLB        = 0b01110100111010000,
  OPCODE_XVSLLH        = 0b01110100111010001,
  OPCODE_XVSLLW        = 0b01110100111010010,
  OPCODE_XVSLLD        = 0b01110100111010011,
  OPCODE_XVSRAIB       = 0b01110111001101,
  OPCODE_XVSRAIH       = 0b01110111001101,
  OPCODE_XVSRAIW       = 0b01110111001101,
  OPCODE_XVSRAID       = 0b01110111001101,
  OPCODE_XVSRAB        = 0b01110100111011000,
  OPCODE_XVSRAH        = 0b01110100111011001,
  OPCODE_XVSRAW        = 0b01110100111011010,
  OPCODE_XVSRAD        = 0b01110100111011011,
  OPCODE_XVSRLIB       = 0b01110111001100,
  OPCODE_XVSRLIH       = 0b01110111001100,
  OPCODE_XVSRLIW       = 0b01110111001100,
  OPCODE_XVSRLID       = 0b01110111001100,
  OPCODE_XVSRLB        = 0b01110100111010100,
  OPCODE_XVSRLH        = 0b01110100111010101,
  OPCODE_XVSRLW        = 0b01110100111010110,
  OPCODE_XVSRLD        = 0b01110100111010111,
  OPCODE_XVSRARNIBH    = 0b01110111010111,
  OPCODE_XVSRARNIHW    = 0b01110111010111,
  OPCODE_XVSRARNIWD    = 0b01110111010111,
  OPCODE_XVSLLWILHB    = 0b01110111000010,
  OPCODE_XVSLLWILWH    = 0b01110111000010,
  OPCODE_XVSLLWILDW    = 0b01110111000010,
  OPCODE_XVSLLWILHUBU  = 0b01110111000011,
  OPCODE_XVSLLWILWUHU  = 0b01110111000011,
  OPCODE_XVSLLWILDUWU  = 0b01110111000011,
  OPCODE_XVSRANIBH     = 0b01110111010110,
  OPCODE_XVSRANIHW     = 0b01110111010110,
  OPCODE_XVSSRANIBH    = 0b01110111011000,
  OPCODE_XVSSRANIHW    = 0b01110111011000,
  OPCODE_XVSSRANIWD    = 0b01110111011000,
  OPCODE_XVSSRANIBUH   = 0b01110111011001,
  OPCODE_XVSSRANIHUW   = 0b01110111011001,
  OPCODE_XVSSRANIWUD   = 0b01110111011001,
  OPCODE_XVSSRLNIBUH   = 0b01110111010011,
  OPCODE_XVSSRLNIHUW   = 0b01110111010011,
  OPCODE_XVSSRLNIWUD   = 0b01110111010011,
  OPCODE_XVSSRLNIBH    = 0b01110111010010,
  OPCODE_XVSSRLNIHW    = 0b01110111010010,
  OPCODE_XVSSRLNIWD    = 0b01110111010010,
  OPCODE_XVILVLB       = 0b01110101000110100,
  OPCODE_XVILVLH       = 0b01110101000110101,
  OPCODE_XVILVLW       = 0b01110101000110110,
  OPCODE_XVILVLD       = 0b01110101000110111,
  OPCODE_XVILVHB       = 0b01110101000111000,
  OPCODE_XVILVHH       = 0b01110101000111001,
  OPCODE_XVILVHW       = 0b01110101000111010,
  OPCODE_XVILVHD       = 0b01110101000111011,
  OPCODE_XVMUHBU       = 0b01110100100010000,
  OPCODE_XVMUHHU       = 0b01110100100010001,
  OPCODE_XVMUHWU       = 0b01110100100010010,
  OPCODE_XVSTELMB      = 0b00110011,
  OPCODE_XVSTELMH      = 0b00110011,
  OPCODE_XVSTELMW      = 0b00110011,
  OPCODE_XVSTELMD      = 0b00110011,
  OPCODE_VEXT2XVWUBU   = 0b0111011010011111001011,
  OPCODE_XVABSDWU      = 0b01110100011000110,
  OPCODE_XVBSLLV       = 0b01110110100011100,
  OPCODE_XVHADDWWH     = 0b01110100010101001,
  OPCODE_XVHADDWDW     = 0b01110100010101010,
  OPCODE_XVHADDWQD     = 0b01110100010101011,
  OPCODE_XVHADDWDUWU   = 0b01110100010110010,
  OPCODE_XVHADDWQUDU   = 0b01110100010110011,
  OPCODE_XVFFINTSW     = 0b0111011010011110000000,
  OPCODE_XVFTINTRZWS   = 0b0111011010011110010010,
  OPCODE_XVFTINTRNEWD  = 0b01110101010010111,
  OPCODE_XVFCVTSD      = 0b01110101010001101,
  OPCODE_XVFADDS       = 0b01110101001100001,
  OPCODE_XVFADDD       = 0b01110101001100010,
  OPCODE_XVFSUBS       = 0b01110101001100101,
  OPCODE_XVFSUBD       = 0b01110101001100110,
  OPCODE_XVFMULS       = 0b01110101001110001,
  OPCODE_XVFMULD       = 0b01110101001110010,
  OPCODE_XVFDIVS       = 0b01110101001110101,
  OPCODE_XVFDIVD       = 0b01110101001110110,
  OPCODE_XVFSQRTS      = 0b0111011010011100111001,
  OPCODE_XVFSQRTD      = 0b0111011010011100111010,
  OPCODE_XVCFCMPCEQS   = 0b00001100100100100,
  OPCODE_XVCFCMPCEQD   = 0b00001100101000100,
  OPCODE_XVCFCMPCLTS   = 0b00001100100100010,
  OPCODE_XVCFCMPCLTD   = 0b00001100101000010,
  OPCODE_XVCFCMPCLES   = 0b00001100100100110,
  OPCODE_XVCFCMPCLED   = 0b00001100101000110,
  OPCODE_XVCFCMPCUNS   = 0b00001100100101000,
  OPCODE_XVCFCMPCUND   = 0b00001100101001000,
  OPCODE_XVFMINS       = 0b01110101001111101,
  OPCODE_XVFMIND       = 0b01110101001111110,
  OPCODE_XVFMAXS       = 0b01110101001111001,
  OPCODE_XVFMAXD       = 0b01110101001111010,
  OPCODE_XVSEQIW       = 0b01110110100000010,
  OPCODE_XVSEQID       = 0b01110110100000011,
  OPCODE_XVNORV        = 0b01110101001001111,
  OPCODE_XVBITSELV     = 0b000011010010
} LasxOpcode;

void
orc_lasx_insn_emit_xvldreplb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvldrepl.b %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVLDREPLB, XREG (xd), GREG (rj), imm | 0b10000000000000));
}

void
orc_lasx_insn_emit_xvldreplh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvldrepl.h %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVLDREPLH, XREG (xd), GREG (rj), (imm >> 1) | 0b01000000000000));
}

void
orc_lasx_insn_emit_xvldreplw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvldrepl.w %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVLDREPLW, XREG (xd), GREG (rj), (imm >> 2) | 0b00100000000000));
}

void
orc_lasx_insn_emit_xvldrepld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvldrepl.d %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVLDREPLD, XREG (xd), GREG (rj), (imm >> 3) | 0b00010000000000));
}

void
orc_lasx_insn_emit_xvreplgr2vrb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister rj)
{
  ORC_ASM_CODE (c, "  xvreplgr2vr.b %s, %s\n", NAME (xd), NAME (rj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVREPLGR2VRB, XREG (xd), GREG (rj)));
}

void
orc_lasx_insn_emit_xvreplgr2vrh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister rj)
{
  ORC_ASM_CODE (c, "  xvreplgr2vr.h %s, %s\n", NAME (xd), NAME (rj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVREPLGR2VRH, XREG (xd), GREG (rj)));
}

void
orc_lasx_insn_emit_xvreplgr2vrw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister rj)
{
  ORC_ASM_CODE (c, "  xvreplgr2vr.w %s, %s\n", NAME (xd), NAME (rj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVREPLGR2VRW, XREG (xd), GREG (rj)));
}

void
orc_lasx_insn_emit_xvreplgr2vrd (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister rj)
{
  ORC_ASM_CODE (c, "  xvreplgr2vr.d %s, %s\n", NAME (xd), NAME (rj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVREPLGR2VRD, XREG (xd), GREG (rj)));
}

void
orc_lasx_insn_emit_xvld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvld %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(OPCODE_XVLD, XREG (xd), GREG (rj), imm));
}

void
orc_lasx_insn_emit_xvst (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister rj, int imm)
{
  ORC_ASM_CODE (c, "  xvst %s, %s, %d\n", NAME (xd), NAME (rj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(OPCODE_XVST, XREG (xd), GREG (rj), imm));
}

void
orc_lasx_insn_emit_xvorv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvor.v %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVORV, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvxorv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvxor.v %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVXORV, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvandnv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvandn.v %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVANDNV, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvandv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvand.v %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVANDV, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvaddb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvadd.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVADDB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvaddh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvadd.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVADDH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvaddw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvadd.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVADDW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvaddd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvadd.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVADDD, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsaddwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsadd.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSADDWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsubb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsub.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSUBB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsubh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsub.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSUBH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsubw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsub.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSUBW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsubd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsub.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSUBD, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvssubwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvssub.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSSUBWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvminwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmin.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMINWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmaxwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmax.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMAXWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvabsdb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvabsd.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVABSDB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvabsdh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvabsd.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVABSDH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvabsdw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvabsd.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVABSDW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvavgrwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvavgr.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVAVGRWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmulb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmul.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMULB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmulh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmul.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMULH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmulw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmul.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMULW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmul.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMULD, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhbu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.bu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHBU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvmuhwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvmuh.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVMUHWU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsigncovb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsigncov.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSIGNCOVB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsigncovh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsigncov.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSIGNCOVH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsigncovw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsigncov.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSIGNCOVW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvdivhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvdiv.hu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVDIVHU, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_vext2xvhb (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.h.b %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVHB, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_vext2xvwh (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.w.h %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVWH, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_vext2xvdw (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.d.w %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVDW, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_vext2xvhubu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.hu.bu %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVHUBU, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_vext2xvwuhu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.wu.hu %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVWUHU, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_vext2xvduwu (OrcCompiler *c, OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.du.wu %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVDUWU, XREG (xd), XREG (xj)));
}

void
orc_lasx_insn_emit_xvshufb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk, OrcLoongRegister xa)
{
  ORC_ASM_CODE (c, "  xvshuf.b %s, %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk), NAME (xa));
  orc_loongarch_insn_emit32 (c, LOONG_4R_INSTRUCTION(OPCODE_XVSHUFB, XREG (xd), XREG (xj), XREG (xk), XREG (xa)));
}

void
orc_lasx_insn_emit_xvshufh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvshuf.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSHUFH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvshufw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvshuf.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSHUFW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsltb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvslt.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLTB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvslth (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvslt.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLTH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsltw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvslt.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLTW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvsltd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvslt.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLTD, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvseqb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvseq.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSEQB, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvseqh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvseq.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSEQH, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvseqw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvseq.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSEQW, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvseqd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvseq.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSEQD, XREG (xd), XREG (xj), XREG (xk)));
}

void
orc_lasx_insn_emit_xvpermid (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvpermi.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVPERMID, XREG (xd), XREG (xj), imm));
}

void
orc_lasx_insn_emit_xvpermiq (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvpermi.q %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVPERMIQ, XREG (xd), XREG (xj), imm));
}

void
orc_lasx_insn_emit_xvssrarnibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrarni.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRARNIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvssrarnihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrarni.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRARNIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssrarniwd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrarni.w.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRARNIBH, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvrepl128veih (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvrepl128vei.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVREPL128VEIH, XREG(xd), XREG(xj), imm | 0b11110000));
}

void
orc_lasx_insn_emit_xvsrlnibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrlni.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLNIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsrlnihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrlni.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLNIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsrlniwd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrlni.w.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLNIWD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvshuf4ib (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvshuf4i.b %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSHUF4IB, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvshuf4ih (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvshuf4i.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSHUF4IH, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvshuf4iw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvshuf4i.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSHUF4IW, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvsllib (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvslli.b %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLIB, XREG(xd), XREG(xj), imm | 0b00001000));
}

void
orc_lasx_insn_emit_xvsllih (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvslli.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLIH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvslliw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvslli.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLIB, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsllid (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvslli.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLIB, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvsllb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsll.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLLB, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsllh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsll.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLLH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsllw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsll.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLLW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvslld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsll.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSLLD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrlib (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrli.b %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLIB, XREG(xd), XREG(xj), imm | 0b00001000));
}

void
orc_lasx_insn_emit_xvsrlih (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrli.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLIH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsrliw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrli.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLIW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsrlid (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrli.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRLID, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvsrlb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsrl.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRLB, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrlh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsrl.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRLH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrlw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsrl.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRLW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsrl.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRLD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsraib (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrai.b %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRAIB, XREG(xd), XREG(xj), imm | 0b00001000));
}

void
orc_lasx_insn_emit_xvsraih (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrai.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRAIH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsraiw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrai.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRAIW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsraid (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrai.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRAID, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvsrab (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsra.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRAB, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrah (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsra.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRAH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsraw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsra.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRAW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrad (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvsra.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVSRAD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvsrarnibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrarni.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRARNIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsrarnihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrarni.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRARNIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsrarniwd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrarni.w.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRARNIWD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvsllwilhb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.h.b %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILHB, XREG(xd), XREG(xj), imm | 0b00001000));
}

void
orc_lasx_insn_emit_xvsllwilwh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.w.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILWH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsllwildw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.d.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILDW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsllwilhubu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.hu.bu %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILHUBU, XREG(xd), XREG(xj), imm | 0b00001000));
}

void
orc_lasx_insn_emit_xvsllwilwuhu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.wu.hu %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILWUHU, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsllwilduwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsllwil.du.wu %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSLLWILDUWU, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvsranibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrani.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRANIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvsranihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvsrani.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSRANIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssranibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvssranihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssraniwd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.w.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIWD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvssranibuh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.bu.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIBUH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvssranihuw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.hu.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIHUW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssraniwud (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrani.wu.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRANIWUD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvssrlnibuh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.bu.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIBUH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvssrlnihuw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.hu.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIHUW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssrlniwud (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.wu.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIWUD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvssrlnibh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.b.h %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIBH, XREG(xd), XREG(xj), imm | 0b00010000));
}

void
orc_lasx_insn_emit_xvssrlnihw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.h.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIHW, XREG(xd), XREG(xj), imm | 0b00100000));
}

void
orc_lasx_insn_emit_xvssrlniwd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvssrlni.w.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVSSRLNIWD, XREG(xd), XREG(xj), imm | 0b01000000));
}

void
orc_lasx_insn_emit_xvilvlb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvl.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVLB, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvlh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvl.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVLH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvlw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvl.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVLW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvl.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVLD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvstelmb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  xvstelm.b %s, %s, %d, %d\n", NAME (xd), NAME (xj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVSTELMB, XREG(xd), GREG(xj), 0b0011000111000000000000 | (idx << 9 ) | si8));
}

void
orc_lasx_insn_emit_xvstelmh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  xvstelm.h %s, %s, %d, %d\n", NAME (xd), NAME (xj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVSTELMH, XREG(xd), GREG(xj), 0b0011000110100000000000 | (idx << 9 ) | si8));
}

void
orc_lasx_insn_emit_xvstelmw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  xvstelm.w %s, %s, %d, %d\n", NAME (xd), NAME (xj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVSTELMW, XREG(xd), GREG(xj), 0b0011000110010000000000 | (idx << 9 ) | si8));
}

void
orc_lasx_insn_emit_xvstelmd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  xvstelm.d %s, %s, %d, %d\n", NAME (xd), NAME (xj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_XVSTELMD, XREG(xd), GREG(xj), 0b0011000110001000000000 | (idx << 9 ) | si8));
}

void
orc_lasx_insn_emit_xvilvhb (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvh.b %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVHB, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvhh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvh.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVHH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvhw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvh.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVHW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvilvhd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvilvh.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVILVHD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_vext2xvwubu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  vext2xv.wu.bu %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXT2XVWUBU, XREG(xd), XREG(xj)));
}

void
orc_lasx_insn_emit_xvabsdwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvabsd.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVABSDWU, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvbsllv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvbsll.v %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_XVBSLLV, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvhaddwwh (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvhaddw.w.h %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVHADDWWH, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvhaddwdw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvhaddw.d.w %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVHADDWDW, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvhaddwqd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvhaddw.q.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVHADDWQD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvhaddwduwu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvhaddw.du.wu %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVHADDWDUWU, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvhaddwqudu (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvhaddw.qu.du %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVHADDWQUDU, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvffintsw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  xvffint.s.w %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVFFINTSW, XREG(xd), XREG(xj)));
}

void
orc_lasx_insn_emit_xvftintrzws (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  xvftintrz.w.s %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVFTINTRZWS, XREG(xd), XREG(xj)));
}

void
orc_lasx_insn_emit_xvftintrnewd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvftintrne.w.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFTINTRNEWD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcvtsd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcvt.s.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFCVTSD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfadds (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfadd.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFADDS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfaddd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfadd.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFADDD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfsubs (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfsub.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFSUBS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfsubd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfsub.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFSUBD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmuls (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmul.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMULS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmuld (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmul.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMULD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfdivs (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfdiv.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFDIVS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfdivd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfdiv.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFDIVD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfsqrts (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  xvfsqrt.s %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVFSQRTS, XREG(xd), XREG(xj)));
}

void
orc_lasx_insn_emit_xvfsqrtd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj)
{
  ORC_ASM_CODE (c, "  xvfsqrt.d %s, %s\n", NAME (xd), NAME (xj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_XVFSQRTD, XREG(xd), XREG(xj)));
}

void
orc_lasx_insn_emit_xvfcmpceqs (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.ceq.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCEQS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpceqd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.ceq.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCEQD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpclts (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.clt.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCLTS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpcltd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.clt.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCLTD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpcles (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.cle.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCLES, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpcled (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.cle.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCLED, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpcuns (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.cun.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCUNS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfcmpcund (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfcmp.cun.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVCFCMPCUND, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmins (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmin.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMINS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmind (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmin.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMIND, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmaxs (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmax.s %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMAXS, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvfmaxd (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvfmax.d %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVFMAXD, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvseqiw (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvseqi.w %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI5_INSTRUCTION(OPCODE_XVSEQIW, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvseqid (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, int imm)
{
  ORC_ASM_CODE (c, "  xvseqi.d %s, %s, %d\n", NAME (xd), NAME (xj), imm);
  orc_loongarch_insn_emit32 (c, LOONG_2RI5_INSTRUCTION(OPCODE_XVSEQID, XREG(xd), XREG(xj), imm));
}

void
orc_lasx_insn_emit_xvnorv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk)
{
  ORC_ASM_CODE (c, "  xvnor.v %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_XVNORV, XREG(xd), XREG(xj), XREG(xk)));
}

void
orc_lasx_insn_emit_xvbitselv (OrcCompiler *c,
    OrcLoongRegister xd, OrcLoongRegister xj, OrcLoongRegister xk, OrcLoongRegister xa)
{
  ORC_ASM_CODE (c, "  xvbitsel.v %s, %s, %s, %s\n", NAME (xd), NAME (xj), NAME (xk), NAME (xa));
  orc_loongarch_insn_emit32 (c, LOONG_4R_INSTRUCTION(OPCODE_XVBITSELV, XREG(xd), XREG(xj), XREG(xk), XREG(xa)));
}

void
orc_lasx_insn_emit_flush_subnormals (OrcCompiler *c, int element_width,
    OrcLoongRegister xs, OrcLoongRegister xd)
{
  const orc_uint64 upper =
      element_width ==
      4 ? 0xff800000 : ORC_UINT64_C (0xfff) << 52;
  const orc_uint64 exponent = upper & (upper >> 1);
  const OrcLoongRegister tmp1 = ORC_LOONG_XR0, tmp2 = ORC_LOONG_XR15;

  orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, exponent);

  if (element_width == 8) {
    orc_lasx_insn_emit_xvreplgr2vrd (c, tmp1, c->gp_tmpreg);
  } else {
    orc_lasx_insn_emit_xvreplgr2vrw (c, tmp1, c->gp_tmpreg);
  }
  orc_lasx_insn_emit_xvandv (c, tmp1, tmp1, xs);
  orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, upper);

  if (element_width == 8) {
    orc_lasx_insn_emit_xvseqid (c, tmp1, tmp1, 0);
    orc_lasx_insn_emit_xvreplgr2vrd (c, tmp2, c->gp_tmpreg);
  } else {
    orc_lasx_insn_emit_xvseqiw (c, tmp1, tmp1, 0);
    orc_lasx_insn_emit_xvreplgr2vrw (c, tmp2, c->gp_tmpreg);
  }

  if (xs != xd) {
    orc_lasx_insn_emit_xvnorv (c, tmp1, tmp1, tmp1);
    orc_lasx_insn_emit_xvandv (c, tmp2, tmp2, xs);
    orc_lasx_insn_emit_xvbitselv (c, xd, tmp2, xs, tmp1);
  } else {
    orc_lasx_insn_emit_xvandv (c, tmp2, tmp2, xs);
    orc_lasx_insn_emit_xvbitselv (c, xd, xd, tmp2, tmp1);
  }
}

OrcLoongRegister
orc_lasx_insn_emit_normalize (OrcCompiler *c, OrcLoongRegister src,
    OrcLoongRegister dest, int element_width)
{
  if (c->target_flags & ORC_TARGET_FAST_DENORMAL) {
    return src;
  } else {
    orc_lasx_insn_emit_flush_subnormals (c, element_width, src, dest);
    return dest;
  }
}
