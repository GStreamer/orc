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
#include <orc/orcinternal.h>
#include <orc/orcdebug.h>
#include <orc/orclimits.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>
#include <orc/loongarch/orcloongarch-internal.h>
#include <orc/loongarch/orcloongarch.h>
#include <orc/loongarch/orclsxinsn.h>
#include <orc/loongarch/orcloongarchinsn.h>

typedef enum
{
  OPCODE_VREPLGR2VRB = 0b0111001010011111000000,
  OPCODE_VREPLGR2VRH = 0b0111001010011111000001,
  OPCODE_VREPLGR2VRW = 0b0111001010011111000010,
  OPCODE_VREPLGR2VRD = 0b0111001010011111000011,
  OPCODE_VLDREPLB    = 0b00110000,
  OPCODE_VLDREPLH    = 0b00110000,
  OPCODE_VLDREPLW    = 0b00110000,
  OPCODE_VLDREPLD    = 0b00110000,
  OPCODE_VLD         = 0b0010110000,
  OPCODE_VST         = 0b0010110001,
  OPCODE_VADDB       = 0b01110000000010100,
  OPCODE_VADDH       = 0b01110000000010101,
  OPCODE_VADDW       = 0b01110000000010110,
  OPCODE_VADDD       = 0b01110000000010111,
  OPCODE_VSUBB       = 0b01110000000011000,
  OPCODE_VSUBH       = 0b01110000000011001,
  OPCODE_VSUBW       = 0b01110000000011010,
  OPCODE_VSUBD       = 0b01110000000011011,
  OPCODE_VSADDB      = 0b01110000010001100,
  OPCODE_VSADDH      = 0b01110000010001101,
  OPCODE_VSADDW      = 0b01110000010001110,
  OPCODE_VSADDD      = 0b01110000010001111,
  OPCODE_VSSUBB      = 0b01110000010010000,
  OPCODE_VSSUBH      = 0b01110000010010001,
  OPCODE_VSSUBW      = 0b01110000010010010,
  OPCODE_VSSUBD      = 0b01110000010010011,
  OPCODE_VSADDBU     = 0b01110000010010100,
  OPCODE_VSADDHU     = 0b01110000010010101,
  OPCODE_VSADDWU     = 0b01110000010010110,
  OPCODE_VSADDDU     = 0b01110000010010111,
  OPCODE_VSSUBBU     = 0b01110000010011000,
  OPCODE_VSSUBHU     = 0b01110000010011001,
  OPCODE_VSSUBWU     = 0b01110000010011010,
  OPCODE_VSSUBDU     = 0b01110000010011011,
  OPCODE_VANDV       = 0b01110001001001100,
  OPCODE_VORV        = 0b01110001001001101,
  OPCODE_VXORV       = 0b01110001001001110,
  OPCODE_VANDNV      = 0b01110001001010000,
  OPCODE_VMAXB       = 0b01110000011100000,
  OPCODE_VMAXH       = 0b01110000011100001,
  OPCODE_VMAXW       = 0b01110000011100010,
  OPCODE_VMAXD       = 0b01110000011100011,
  OPCODE_VMINB       = 0b01110000011100100,
  OPCODE_VMINH       = 0b01110000011100101,
  OPCODE_VMINW       = 0b01110000011100110,
  OPCODE_VMIND       = 0b01110000011100111,
  OPCODE_VMAXBU      = 0b01110000011101000,
  OPCODE_VMAXHU      = 0b01110000011101001,
  OPCODE_VMAXWU      = 0b01110000011101010,
  OPCODE_VMAXDU      = 0b01110000011101011,
  OPCODE_VMINBU      = 0b01110000011101100,
  OPCODE_VMINHU      = 0b01110000011101101,
  OPCODE_VMINWU      = 0b01110000011101110,
  OPCODE_VMINDU      = 0b01110000011101111,
  OPCODE_VMULB       = 0b01110000100001000,
  OPCODE_VMULH       = 0b01110000100001001,
  OPCODE_VMULW       = 0b01110000100001010,
  OPCODE_VMULD       = 0b01110000100001011,
  OPCODE_VMUHB       = 0b01110000100001100,
  OPCODE_VMUHH       = 0b01110000100001101,
  OPCODE_VMUHW       = 0b01110000100001110,
  OPCODE_VMUHBU      = 0b01110000100010000,
  OPCODE_VMUHHU      = 0b01110000100010001,
  OPCODE_VMUHWU      = 0b01110000100010010,
  OPCODE_VABSDB      = 0b01110000011000000,
  OPCODE_VABSDH      = 0b01110000011000001,
  OPCODE_VABSDW      = 0b01110000011000010,
  OPCODE_VABSDWU     = 0b01110000011000110,
  OPCODE_VAVGRB      = 0b01110000011010000,
  OPCODE_VAVGRH      = 0b01110000011010001,
  OPCODE_VAVGRW      = 0b01110000011010010,
  OPCODE_VAVGRBU     = 0b01110000011010100,
  OPCODE_VAVGRHU     = 0b01110000011010101,
  OPCODE_VAVGRWU     = 0b01110000011010110,
  OPCODE_VDIVHU      = 0b01110000111001001,
  OPCODE_VSIGNCOVB   = 0b01110001001011100,
  OPCODE_VSIGNCOVH   = 0b01110001001011101,
  OPCODE_VSIGNCOVW   = 0b01110001001011110,
  OPCODE_VREPLVEIH   = 0b01110010111101,
  OPCODE_VSLLWILHB   = 0b01110011000010,
  OPCODE_VSLLWILWH   = 0b01110011000010,
  OPCODE_VSLLWILDW   = 0b01110011000010,
  OPCODE_VSLLWILHUBU = 0b01110011000011,
  OPCODE_VSLLWILWUHU = 0b01110011000011,
  OPCODE_VSLLWILDUWU = 0b01110011000011,
  OPCODE_VILVLB      = 0b01110001000110100,
  OPCODE_VILVLH      = 0b01110001000110101,
  OPCODE_VILVLW      = 0b01110001000110110,
  OPCODE_VILVLD      = 0b01110001000110111,
  OPCODE_VILVHB      = 0b01110001000111000,
  OPCODE_VILVHH      = 0b01110001000111001,
  OPCODE_VILVHW      = 0b01110001000111010,
  OPCODE_VILVHD      = 0b01110001000111011,
  OPCODE_VSLTB       = 0b01110000000001100,
  OPCODE_VSLTH       = 0b01110000000001101,
  OPCODE_VSLTW       = 0b01110000000001110,
  OPCODE_VSLTD       = 0b01110000000001111,
  OPCODE_VSEQB       = 0b01110000000000000,
  OPCODE_VSEQH       = 0b01110000000000001,
  OPCODE_VSEQW       = 0b01110000000000010,
  OPCODE_VSEQD       = 0b01110000000000011,
  OPCODE_VSSRARNIBH  = 0b01110011011010,
  OPCODE_VSSRARNIHW  = 0b01110011011010,
  OPCODE_VSSRARNIWD  = 0b01110011011010,
  OPCODE_VSRLNIBH    = 0b01110011010000,
  OPCODE_VSRLNIHW    = 0b01110011010000,
  OPCODE_VSRLNIWD    = 0b01110011010000,
  OPCODE_VSHUF4IB    = 0b01110011100100,
  OPCODE_VSHUF4IH    = 0b01110011100101,
  OPCODE_VSHUF4IW    = 0b01110011100110,
  OPCODE_VSLLIB      = 0b01110011001011,
  OPCODE_VSLLIH      = 0b01110011001011,
  OPCODE_VSLLIW      = 0b01110011001011,
  OPCODE_VSLLID      = 0b01110011001011,
  OPCODE_VSLLB       = 0b01110000111010000,
  OPCODE_VSLLH       = 0b01110000111010001,
  OPCODE_VSLLW       = 0b01110000111010010,
  OPCODE_VSLLD       = 0b01110000111010011,
  OPCODE_VSRLIB      = 0b01110011001100,
  OPCODE_VSRLIH      = 0b01110011001100,
  OPCODE_VSRLIW      = 0b01110011001100,
  OPCODE_VSRLID      = 0b01110011001100,
  OPCODE_VSRLB       = 0b01110000111010100,
  OPCODE_VSRLH       = 0b01110000111010101,
  OPCODE_VSRLW       = 0b01110000111010110,
  OPCODE_VSRLD       = 0b01110000111010111,
  OPCODE_VSRAIB      = 0b01110011001101,
  OPCODE_VSRAIH      = 0b01110011001101,
  OPCODE_VSRAIW      = 0b01110011001101,
  OPCODE_VSRAID      = 0b01110011001101,
  OPCODE_VSRAB       = 0b01110000111011000,
  OPCODE_VSRAH       = 0b01110000111011001,
  OPCODE_VSRAW       = 0b01110000111011010,
  OPCODE_VSRAD       = 0b01110000111011011,
  OPCODE_VBSLLV      = 0b01110010100011,
  OPCODE_VBSRLV      = 0b01110010100011,
  OPCODE_VSRANIBH    = 0b01110011010110,
  OPCODE_VSRANIHW    = 0b01110011010110,
  OPCODE_VSRARNIBH   = 0b01110011010111,
  OPCODE_VSRARNIHW   = 0b01110011010111,
  OPCODE_VSRARNIWD   = 0b01110011010111,
  OPCODE_VSSRANIBUH  = 0b01110011011001,
  OPCODE_VSSRANIHUW  = 0b01110011011001,
  OPCODE_VSSRANIWUD  = 0b01110011011001,
  OPCODE_VSSRLNIBH   = 0b01110011010010,
  OPCODE_VSSRLNIHW   = 0b01110011010010,
  OPCODE_VSSRLNIWD   = 0b01110011010010,
  OPCODE_VSSRLNIBUH  = 0b01110011010011,
  OPCODE_VSSRLNIHUW  = 0b01110011010011,
  OPCODE_VSSRLNIWUD  = 0b01110011010011,
  OPCODE_VSTELMB     = 0b00110001,
  OPCODE_VSTELMH     = 0b00110001,
  OPCODE_VSTELMW     = 0b00110001,
  OPCODE_VSTELMD     = 0b00110001,
  OPCODE_VSSRANIBH   = 0b01110011011000,
  OPCODE_VSSRANIHW   = 0b01110011011000,
  OPCODE_VSSRANIWD   = 0b01110011011000,
  OPCODE_VFMULS      = 0b01110001001110001,
  OPCODE_VFMULD      = 0b01110001001110010,
  OPCODE_VFDIVS      = 0b01110001001110101,
  OPCODE_VFDIVD      = 0b01110001001110110,
  OPCODE_VFMAXS      = 0b01110001001111001,
  OPCODE_VFMAXD      = 0b01110001001111010,
  OPCODE_VFMINS      = 0b01110001001111101,
  OPCODE_VFMIND      = 0b01110001001111110,
  OPCODE_VFSQRTS     = 0b0111001010011100111001,
  OPCODE_VFSQRTD     = 0b0111001010011100111010,
  OPCODE_VFFINTSW    = 0b0111001010011110000000,
  OPCODE_VFFINTLDW   = 0b0111001010011110000100,
  OPCODE_VFFINTHDW   = 0b0111001010011110000101,
  OPCODE_VFADDS      = 0b01110001001100001,
  OPCODE_VFADDD      = 0b01110001001100010,
  OPCODE_VFSUBS      = 0b01110001001100101,
  OPCODE_VFSUBD      = 0b01110001001100110,
  OPCODE_VFTINTRZWS  = 0b0111001010011110010010,
  OPCODE_VFTINTRNEWD = 0b01110001010010111,
  OPCODE_VFCVTSD     = 0b01110001010001101,
  OPCODE_VCFCMPCEQS  = 0b00001100010100100,
  OPCODE_VCFCMPCEQD  = 0b00001100011000100,
  OPCODE_VCFCMPCLTS  = 0b00001100010100010,
  OPCODE_VCFCMPCLTD  = 0b00001100011000010,
  OPCODE_VCFCMPCLES  = 0b00001100010100110,
  OPCODE_VCFCMPCLED  = 0b00001100011000110,
  OPCODE_VCFCMPCUNS  = 0b00001100010101000,
  OPCODE_VCFCMPCUND  = 0b00001100011001000,
  OPCODE_VHADDWHB    = 0b01110000010101000,
  OPCODE_VHADDWWH    = 0b01110000010101001,
  OPCODE_VHADDWDW    = 0b01110000010101010,
  OPCODE_VHADDWQD    = 0b01110000010101011,
  OPCODE_VHADDWHUBU  = 0b01110000010110000,
  OPCODE_VHADDWWUHU  = 0b01110000010110001,
  OPCODE_VHADDWDUWU  = 0b01110000010110010,
  OPCODE_VHADDWQUDU  = 0b01110000010110011,
  OPCODE_VBITSELV    = 0b000011010001,
  OPCODE_VSEQIW      = 0b01110010100000010,
  OPCODE_VSEQID      = 0b01110010100000011,
  OPCODE_VNORV       = 0b01110001001001111,
  OPCODE_VPICKEVW    = 0b01110001000111110,
  OPCODE_VPICKODW    = 0b01110001001000010,
  OPCODE_VFCVTLDS    = 0b0111001010011101111100,
  OPCODE_VFCVTHDS    = 0b0111001010011101111101,
  OPCODE_VEXTHHB     = 0b0111001010011110111000,
  OPCODE_VEXTHWH     = 0b0111001010011110111001,
  OPCODE_VEXTHDW     = 0b0111001010011110111010,
  OPCODE_VEXTHHUBU   = 0b0111001010011110111100,
  OPCODE_VEXTHWUHU   = 0b0111001010011110111101,
  OPCODE_VEXTHDUWU   = 0b0111001010011110111110
} LsxOpcode;

void
orc_lsx_insn_emit_vaddb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vadd.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VADDB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vaddh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vadd.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VADDH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vaddw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vadd.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VADDW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vaddd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vadd.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VADDD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vreplgr2vrb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vreplgr2vr.b %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VREPLGR2VRB, VREG(vd), GREG(vj)));
}

void
orc_lsx_insn_emit_vreplgr2vrh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vreplgr2vr.h %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VREPLGR2VRH, VREG(vd), GREG(vj)));
}

void
orc_lsx_insn_emit_vreplgr2vrw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vreplgr2vr.w %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VREPLGR2VRW, VREG(vd), GREG(vj)));
}

void
orc_lsx_insn_emit_vreplgr2vrd (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vreplgr2vr.d %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VREPLGR2VRD, VREG(vd), GREG(vj)));
}

void
orc_lsx_insn_emit_vldreplb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12)
{
  ORC_ASM_CODE (c, "  vldrepl.b %s, %s, %d\n", NAME (vd), NAME (rj), imm12);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VLDREPLB, VREG(vd), GREG(rj), imm12 | 0b10000000000000));
}

void
orc_lsx_insn_emit_vldreplh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm11)
{
  ORC_ASM_CODE (c, "  vldrepl.h %s, %s, %d\n", NAME (vd), NAME (rj), imm11);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VLDREPLH, VREG(vd), GREG(rj), (imm11 >> 1) | 0b01000000000000));
}

void
orc_lsx_insn_emit_vldreplw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm10)
{
  ORC_ASM_CODE (c, "  vldrepl.w %s, %s, %d\n", NAME (vd), NAME (rj), imm10);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VLDREPLW, VREG(vd), GREG(rj), (imm10 >> 2) | 0b00100000000000));
}

void
orc_lsx_insn_emit_vldrepld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm9)
{
  ORC_ASM_CODE (c, "  vldrepl.d %s, %s, %d\n", NAME (vd), NAME (rj), imm9);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VLDREPLD, VREG(vd), GREG(rj), (imm9 >> 3) | 0b00010000000000));
}

void
orc_lsx_insn_emit_vld (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12)
{
  ORC_ASM_CODE (c, "  vld %s, %s, %d\n", NAME (vd), NAME (rj), imm12);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(OPCODE_VLD, VREG(vd), GREG(rj), imm12));
}

void
orc_lsx_insn_emit_vst (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister rj, int imm12)
{
  ORC_ASM_CODE (c, "  vst %s, %s, %d\n", NAME (vd), NAME (rj), imm12);
  orc_loongarch_insn_emit32 (c, LOONG_2RI12_INSTRUCTION(OPCODE_VST, VREG(vd), GREG(rj), imm12));
}

void
orc_lsx_insn_emit_vsubb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsub.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSUBB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsubh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsub.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSUBH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsubw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsub.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSUBW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsubd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsub.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSUBD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsaddwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsadddu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsadd.du %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSADDDU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssubdu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vssub.du %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSSUBDU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vorv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vor.v %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VORV, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vxorv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vxor.v %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VXORV, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vandnv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vandn.v %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VANDNV, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vandv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vand.v %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VANDV, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmind (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMIND, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmaxdu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmax.du %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMAXDU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vminwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmindu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmin.du %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMINDU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmulb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmul.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMULB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmulh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmul.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMULH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmulw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmul.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMULW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuld (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmul.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMULD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vmuhwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vmuh.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VMUHWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vabsdb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vabsd.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VABSDB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vabsdh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vabsd.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VABSDH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vabsdw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vabsd.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VABSDW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vabsdwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vabsd.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VABSDWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrbu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vavgrwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vavgr.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VAVGRWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vdivhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vdiv.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VDIVHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsigncovb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsigncov.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSIGNCOVB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsigncovh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsigncov.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSIGNCOVH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsigncovw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsigncov.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSIGNCOVW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vreplveih (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui3)
{
  ORC_ASM_CODE (c, "  vreplvei.h %s, %s, %d\n", NAME (vd), NAME (vj), ui3);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VREPLVEIH, VREG(vd), VREG(vj), ui3 | 0b11110000));
}

void
orc_lsx_insn_emit_vsllwilhb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui3)
{
  ORC_ASM_CODE (c, "  vsllwil.h.b %s, %s, %d\n", NAME (vd), NAME (vj), ui3);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILHB, VREG(vd), VREG(vj), ui3 | 0b00001000));
}

void
orc_lsx_insn_emit_vsllwilwh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui4)
{
  ORC_ASM_CODE (c, "  vsllwil.w.h %s, %s, %d\n", NAME (vd), NAME (vj), ui4);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILWH, VREG(vd), VREG(vj), ui4 | 0b00010000));
}

void
orc_lsx_insn_emit_vsllwildw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vsllwil.d.w %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILDW, VREG(vd), VREG(vj), ui5 | 0b00100000));
}

void
orc_lsx_insn_emit_vsllwilhubu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui3)
{
  ORC_ASM_CODE (c, "  vsllwil.hu.bu %s, %s, %d\n", NAME (vd), NAME (vj), ui3);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILHUBU, VREG(vd), VREG(vj), ui3 | 0b00001000));
}

void
orc_lsx_insn_emit_vsllwilwuhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui4)
{
  ORC_ASM_CODE (c, "  vsllwil.wu.hu %s, %s, %d\n", NAME (vd), NAME (vj), ui4);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILWUHU, VREG(vd), VREG(vj), ui4 | 0b00010000));
}

void
orc_lsx_insn_emit_vsllwilduwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vsllwil.du.wu %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLWILDUWU, VREG(vd), VREG(vj), ui5 | 0b00100000));
}

void
orc_lsx_insn_emit_vexthhb (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.h.b %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHHB, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vexthwh (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.w.h %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHWH, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vexthdw (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.d.w %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHDW, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vexthhubu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.hu.bu %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHHUBU, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vexthwuhu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.wu.hu %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHWUHU, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vexthduwu (OrcCompiler *c, OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vexth.du.wu %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VEXTHDUWU, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vilvlb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvl.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVLB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvlh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvl.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVLH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvlw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvl.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVLW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvld (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvl.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVLD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsltb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vslt.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLTB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vslth (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vslt.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLTH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsltw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vslt.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLTW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsltd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vslt.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLTD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vseqb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vseq.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSEQB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vseqh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vseq.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSEQH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vseqw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vseq.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSEQW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vseqd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vseq.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSEQD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vssrarnibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrarni.b.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRARNIBH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vssrarnihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrarni.h.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRARNIHW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vssrarniwd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrarni.w.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRARNIWD, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vsrlnibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrlni.b.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLNIBH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vsrlnihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrlni.h.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLNIHW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsrlniwd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrlni.w.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLNIWD, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vshuf4ib (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vshuf4i.b %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSHUF4IB, VREG(vd), VREG(vj), imm8));
}

void
orc_lsx_insn_emit_vshuf4ih (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vshuf4i.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSHUF4IH, VREG(vd), VREG(vj), imm8));
}

void
orc_lsx_insn_emit_vshuf4iw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vshuf4i.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSHUF4IW, VREG(vd), VREG(vj), imm8));
}

void
orc_lsx_insn_emit_vsllib (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vslli.b %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLIB, VREG(vd), VREG(vj), imm8 | 0b00001000));
}

void
orc_lsx_insn_emit_vsllih (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vslli.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLIH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vslliw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vslli.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLIW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsllid (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vslli.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSLLID, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vsllb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsll.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLLB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsllh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsll.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLLH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsllw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsll.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLLW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vslld (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsll.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSLLD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrlib (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrli.b %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLIB, VREG(vd), VREG(vj), imm8 | 0b00001000));
}

void
orc_lsx_insn_emit_vsrlih (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrli.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLIH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vsrliw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrli.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLIW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsrlid (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrli.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRLID, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vsrlb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsrl.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRLB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrlh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsrl.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRLH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrlw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsrl.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRLW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrld (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsrl.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRLD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsraib (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrai.b %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRAIB, VREG(vd), VREG(vj), imm8 | 0b00001000));
}

void
orc_lsx_insn_emit_vsraih (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrai.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRAIH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vsraiw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrai.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRAIW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsraid (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrai.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRAID, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vsrab (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsra.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRAB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrah (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsra.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRAH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsraw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsra.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRAW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsrad (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vsra.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VSRAD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vsranibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrani.b.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRANIBH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vsranihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrani.h.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRANIHW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsrarnibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrarni.b.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRARNIBH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vsrarnihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrarni.h.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRARNIHW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vsrarniwd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vsrarni.w.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSRARNIWD, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vssrlnibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.b.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIBH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vssrlnihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.h.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIHW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vssrlniwd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.w.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIWD, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vssrlnibuh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.bu.h %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIBUH, VREG(vd), VREG(vj), imm8 | 0b00010000));
}

void
orc_lsx_insn_emit_vssrlnihuw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.hu.w %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIHUW, VREG(vd), VREG(vj), imm8 | 0b00100000));
}

void
orc_lsx_insn_emit_vssrlniwud (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm8)
{
  ORC_ASM_CODE (c, "  vssrlni.wu.d %s, %s, %d\n", NAME (vd), NAME (vj), imm8);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRLNIWUD, VREG(vd), VREG(vj), imm8 | 0b01000000));
}

void
orc_lsx_insn_emit_vbsllv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vbsll.v %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VBSLLV, VREG(vd), VREG(vj), ui5 | 0b10000000));
}

void
orc_lsx_insn_emit_vbsrlv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vbsrl.v %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VBSRLV, VREG(vd), VREG(vj), ui5 | 0b10100000));
}

void
orc_lsx_insn_emit_vstelmb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  vstelm.b %s, %s, %d, %d\n", NAME (vd), NAME (rj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VSTELMB, VREG(vd), GREG(rj), 0b0011000110000000000000 | (idx << 9 ) | si8));
}

void
orc_lsx_insn_emit_vstelmh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  vstelm.h %s, %s, %d, %d\n", NAME (vd), NAME (rj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VSTELMH, VREG(vd), GREG(rj), 0b0011000101000000000000 | (idx << 9 ) | (si8 >> 1)));
}

void
orc_lsx_insn_emit_vstelmw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  vstelm.w %s, %s, %d, %d\n", NAME (vd), NAME (rj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VSTELMW, VREG(vd), GREG(rj), 0b0011000100100000000000 | (idx << 9 ) | (si8 >> 2)));
}

void
orc_lsx_insn_emit_vstelmd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister rj, int si8, int idx)
{
  ORC_ASM_CODE (c, "  vstelm.d %s, %s, %d, %d\n", NAME (vd), NAME (rj), si8, idx);
  orc_loongarch_insn_emit32 (c, LOONG_2RI14_INSTRUCTION(OPCODE_VSTELMD, VREG(vd), GREG(rj), 0b0011000100010000000000 | (idx << 9 ) | (si8 >> 3)));
}

void
orc_lsx_insn_emit_vssranibh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui4)
{
  ORC_ASM_CODE (c, "  vssrani.b.h %s, %s, %d\n", NAME (vd), NAME (vj), ui4);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIBH, VREG(vd), VREG(vj), ui4 | 0b00010000));
}

void
orc_lsx_insn_emit_vssranihw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vssrani.h.w %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIHW, VREG(vd), VREG(vj), ui5 | 0b00100000));
}

void
orc_lsx_insn_emit_vssraniwd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui6)
{
  ORC_ASM_CODE (c, "  vssrani.w.d %s, %s, %d\n", NAME (vd), NAME (vj), ui6);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIWD, VREG(vd), VREG(vj), ui6 | 0b01000000));
}

void
orc_lsx_insn_emit_vssranibuh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui4)
{
  ORC_ASM_CODE (c, "  vssrani.bu.h %s, %s, %d\n", NAME (vd), NAME (vj), ui4);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIBUH, VREG(vd), VREG(vj), ui4 | 0b00010000));
}

void
orc_lsx_insn_emit_vssranihuw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui5)
{
  ORC_ASM_CODE (c, "  vssrani.hu.w %s, %s, %d\n", NAME (vd), NAME (vj), ui5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIHUW, VREG(vd), VREG(vj), ui5 | 0b00100000));
}

void
orc_lsx_insn_emit_vssraniwud (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int ui6)
{
  ORC_ASM_CODE (c, "  vssrani.wu.d %s, %s, %d\n", NAME (vd), NAME (vj), ui6);
  orc_loongarch_insn_emit32 (c, LOONG_2RI8_INSTRUCTION(OPCODE_VSSRANIWUD, VREG(vd), VREG(vj), ui6 | 0b01000000));
}

void
orc_lsx_insn_emit_vfadds (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vfadd.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VFADDS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfaddd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfadd.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFADDD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfsubs (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vfsub.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VFSUBS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfsubd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfsub.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFSUBD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmuls (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vfmul.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VFMULS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmuld (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfmul.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFMULD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfdivs (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfdiv.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFDIVS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfdivd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfdiv.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFDIVD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfsqrts (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vfsqrt.s %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VFSQRTS, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vfsqrtd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (p, "  vfsqrt.d %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (p, LOONG_2R_INSTRUCTION(OPCODE_VFSQRTD, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vffintsw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vffint.s.w %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VFFINTSW, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vffintldw (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (p, "  vffintl.d.w %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (p, LOONG_2R_INSTRUCTION(OPCODE_VFFINTLDW, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vffinthdw (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (p, "  vffinth.d.w %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (p, LOONG_2R_INSTRUCTION(OPCODE_VFFINTHDW, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vftintrzws (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (c, "  vftintrz.w.s %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (c, LOONG_2R_INSTRUCTION(OPCODE_VFTINTRZWS, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vftintrnewd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vftintrne.w.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFTINTRNEWD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcvtsd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcvt.s.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFCVTSD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcvtlds (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (p, "  vfcvtl.d.s %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (p, LOONG_2R_INSTRUCTION(OPCODE_VFCVTLDS, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vfcvthds (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj)
{
  ORC_ASM_CODE (p, "  vfcvth.d.s %s, %s\n", NAME (vd), NAME (vj));
  orc_loongarch_insn_emit32 (p, LOONG_2R_INSTRUCTION(OPCODE_VFCVTHDS, VREG(vd), VREG(vj)));
}

void
orc_lsx_insn_emit_vfcmpceqs (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.ceq.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCEQS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpceqd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.ceq.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCEQD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpclts (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.clt.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCLTS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpcltd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.clt.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCLTD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpcles (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.cle.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCLES, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpcled (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.cle.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCLED, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpcuns (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.cun.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCUNS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfcmpcund (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfcmp.cun.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VCFCMPCUND, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmins (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfmin.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFMINS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmind (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfmin.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFMIND, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmaxs (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfmax.s %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFMAXS, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vfmaxd (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vfmax.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VFMAXD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwhb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.h.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWHB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwwh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.w.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWWH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwdw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.d.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWDW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwqd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.q.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWQD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwhubu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.hu.bu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWHUBU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwwuhu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.wu.hu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWWUHU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwduwu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.du.wu %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWDUWU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vhaddwqudu (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vhaddw.qu.du %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VHADDWQUDU, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vpickevw (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vpickev.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VPICKEVW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vpickodw (OrcCompiler *p,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (p, "  vpickod.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (p, LOONG_3R_INSTRUCTION(OPCODE_VPICKODW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvhb (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvh.b %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVHB, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvhh (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvh.h %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVHH, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvhw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvh.w %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVHW, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vilvhd (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vilvh.d %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VILVHD, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_vbitselv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk, OrcLoongRegister va)
{
  ORC_ASM_CODE (c, "  vbitsel.v %s, %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk), NAME (va));
  orc_loongarch_insn_emit32 (c, LOONG_4R_INSTRUCTION(OPCODE_VBITSELV, VREG(vd), VREG(vj), VREG(vk), VREG(va)));
}

void
orc_lsx_insn_emit_vseqiw (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm5)
{
  ORC_ASM_CODE (c, "  vseqi.w %s, %s, %d\n", NAME (vd), NAME (vj), imm5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI5_INSTRUCTION(OPCODE_VSEQIW, VREG(vd), VREG(vj), imm5));
}

void
orc_lsx_insn_emit_vseqid (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, int imm5)
{
  ORC_ASM_CODE (c, "  vseqi.d %s, %s, %d\n", NAME (vd), NAME (vj), imm5);
  orc_loongarch_insn_emit32 (c, LOONG_2RI5_INSTRUCTION(OPCODE_VSEQID, VREG(vd), VREG(vj), imm5));
}

void
orc_lsx_insn_emit_vnorv (OrcCompiler *c,
    OrcLoongRegister vd, OrcLoongRegister vj, OrcLoongRegister vk)
{
  ORC_ASM_CODE (c, "  vnor.v %s, %s, %s\n", NAME (vd), NAME (vj), NAME (vk));
  orc_loongarch_insn_emit32 (c, LOONG_3R_INSTRUCTION(OPCODE_VNORV, VREG(vd), VREG(vj), VREG(vk)));
}

void
orc_lsx_insn_emit_flush_subnormals (OrcCompiler *c, int element_width,
    OrcLoongRegister vs, OrcLoongRegister vd)
{
  const orc_uint64 upper =
      element_width ==
      4 ? 0xff800000 : ORC_UINT64_C (0xfff) << 52;
  const orc_uint64 exponent = upper & (upper >> 1);
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0, tmp2 = ORC_LOONG_VR15;

  orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, exponent);

  if (element_width == 8) {
    orc_lsx_insn_emit_vreplgr2vrd (c, tmp1, c->gp_tmpreg);
  } else {
    orc_lsx_insn_emit_vreplgr2vrw (c, tmp1, c->gp_tmpreg);
  }
  orc_lsx_insn_emit_vandv (c, tmp1, tmp1, vs);
  orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, upper);

  if (element_width == 8) {
    orc_lsx_insn_emit_vseqid (c, tmp1, tmp1, 0);
    orc_lsx_insn_emit_vreplgr2vrd (c, tmp2, c->gp_tmpreg);
  } else {
    orc_lsx_insn_emit_vseqiw (c, tmp1, tmp1, 0);
    orc_lsx_insn_emit_vreplgr2vrw (c, tmp2, c->gp_tmpreg);
  }

  if (vs != vd) {
    orc_lsx_insn_emit_vnorv (c, tmp1, tmp1, tmp1);
    orc_lsx_insn_emit_vandv (c, tmp2, tmp2, vs);
    orc_lsx_insn_emit_vbitselv (c, vd, tmp2, vs, tmp1);
  } else {
    orc_lsx_insn_emit_vandv (c, tmp2, tmp2, vs);
    orc_lsx_insn_emit_vbitselv (c, vd, vd, tmp2, tmp1);
  }
}

OrcLoongRegister
orc_lsx_insn_emit_normalize (OrcCompiler *c, OrcLoongRegister src,
    OrcLoongRegister dest, int element_width)
{
  if (c->target_flags & ORC_TARGET_FAST_DENORMAL) {
    return src;
  } else {
    orc_lsx_insn_emit_flush_subnormals (c, element_width, src, dest);
    return dest;
  }
}
