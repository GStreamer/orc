
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc/orcsysinsn.h>


OrcSysOpcode orc_x86_opcodes[] = {
  { "punpcklbw", ORC_X86_INSN_TYPE_SD, 0, 0x660f60 },
  { "punpcklwd", ORC_X86_INSN_TYPE_SD, 0, 0x660f61 },
  { "punpckldq", ORC_X86_INSN_TYPE_SD, 0, 0x660f62 },
  { "packsswb", ORC_X86_INSN_TYPE_SD, 0, 0x660f63 },
  { "pcmpgtb", ORC_X86_INSN_TYPE_SD, 0, 0x660f64 },
  { "pcmpgtw", ORC_X86_INSN_TYPE_SD, 0, 0x660f65 },
  { "pcmpgtd", ORC_X86_INSN_TYPE_SD, 0, 0x660f66 },
  { "packuswb", ORC_X86_INSN_TYPE_SD, 0, 0x660f67 },
  { "punpckhbw", ORC_X86_INSN_TYPE_SD, 0, 0x660f68 },
  { "punpckhwd", ORC_X86_INSN_TYPE_SD, 0, 0x660f69 },
  { "punpckhdq", ORC_X86_INSN_TYPE_SD, 0, 0x660f6a },
  { "packssdw", ORC_X86_INSN_TYPE_SD, 0, 0x660f6b },
  { "punpcklqdq", ORC_X86_INSN_TYPE_SD, 0, 0x660f6c },
  { "punpckhqdq", ORC_X86_INSN_TYPE_SD, 0, 0x660f6d },
  { "movdqa", ORC_X86_INSN_TYPE_SD, 0, 0x660f6f },
  { "psraw", ORC_X86_INSN_TYPE_SD, 0, 0x660fe1 },
  { "psrlw", ORC_X86_INSN_TYPE_SD, 0, 0x660fd1 },
  { "psllw", ORC_X86_INSN_TYPE_SD, 0, 0x660ff1 },
  { "psrad", ORC_X86_INSN_TYPE_SD, 0, 0x660fe2 },
  { "psrld", ORC_X86_INSN_TYPE_SD, 0, 0x660fd2 },
  { "pslld", ORC_X86_INSN_TYPE_SD, 0, 0x660ff2 },
  { "psrlq", ORC_X86_INSN_TYPE_SD, 0, 0x660fd3 },
  { "psllq", ORC_X86_INSN_TYPE_SD, 0, 0x660ff3 },
  { "psrldq", ORC_X86_INSN_TYPE_SD, 0, 0x660f73 },
  { "pslldq", ORC_X86_INSN_TYPE_SD, 0, 0x660f73 },
  { "psrlq", ORC_X86_INSN_TYPE_SD, 0, 0x660fd3 },
  { "pcmpeqb", ORC_X86_INSN_TYPE_SD, 0, 0x660f74 },
  { "pcmpeqw", ORC_X86_INSN_TYPE_SD, 0, 0x660f75 },
  { "pcmpeqd", ORC_X86_INSN_TYPE_SD, 0, 0x660f76 },
  { "paddq", ORC_X86_INSN_TYPE_SD, 0, 0x660fd4 },
  { "pmullw", ORC_X86_INSN_TYPE_SD, 0, 0x660fd5 },
  { "psubusb", ORC_X86_INSN_TYPE_SD, 0, 0x660fd8 },
  { "psubusw", ORC_X86_INSN_TYPE_SD, 0, 0x660fd9 },
  { "pminub", ORC_X86_INSN_TYPE_SD, 0, 0x660fda },
  { "pand", ORC_X86_INSN_TYPE_SD, 0, 0x660fdb },
  { "paddusb", ORC_X86_INSN_TYPE_SD, 0, 0x660fdc },
  { "paddusw", ORC_X86_INSN_TYPE_SD, 0, 0x660fdd },
  { "pmaxub", ORC_X86_INSN_TYPE_SD, 0, 0x660fde },
  { "pandn", ORC_X86_INSN_TYPE_SD, 0, 0x660fdf },
  { "pavgb", ORC_X86_INSN_TYPE_SD, 0, 0x660fe0 },
  { "pavgw", ORC_X86_INSN_TYPE_SD, 0, 0x660fe3 },
  { "pmulhuw", ORC_X86_INSN_TYPE_SD, 0, 0x660fe4 },
  { "pmulhw", ORC_X86_INSN_TYPE_SD, 0, 0x660fe5 },
  { "psubsb", ORC_X86_INSN_TYPE_SD, 0, 0x660fe8 },
  { "psubsw", ORC_X86_INSN_TYPE_SD, 0, 0x660fe9 },
  { "pminsw", ORC_X86_INSN_TYPE_SD, 0, 0x660fea },
  { "por", ORC_X86_INSN_TYPE_SD, 0, 0x660feb },
  { "paddsb", ORC_X86_INSN_TYPE_SD, 0, 0x660fec },
  { "paddsw", ORC_X86_INSN_TYPE_SD, 0, 0x660fed },
  { "pmaxsw", ORC_X86_INSN_TYPE_SD, 0, 0x660fee },
  { "pxor", ORC_X86_INSN_TYPE_SD, 0, 0x660fef },
  { "pmuludq", ORC_X86_INSN_TYPE_SD, 0, 0x660ff4 },
  { "pmaddwd", ORC_X86_INSN_TYPE_SD, 0, 0x660ff5 },
  { "psadbw", ORC_X86_INSN_TYPE_SD, 0, 0x660ff6 },
  { "psubb", ORC_X86_INSN_TYPE_SD, 0, 0x660ff8 },
  { "psubw", ORC_X86_INSN_TYPE_SD, 0, 0x660ff9 },
  { "psubd", ORC_X86_INSN_TYPE_SD, 0, 0x660ffa },
  { "psubq", ORC_X86_INSN_TYPE_SD, 0, 0x660ffb },
  { "paddb", ORC_X86_INSN_TYPE_SD, 0, 0x660ffc },
  { "paddw", ORC_X86_INSN_TYPE_SD, 0, 0x660ffd },
  { "paddd", ORC_X86_INSN_TYPE_SD, 0, 0x660ffe },
  { "pshufb", ORC_X86_INSN_TYPE_SD, 0, 0x660f3800 },
  { "phaddw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3801 },
  { "phaddd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3802 },
  { "phaddsw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3803 },
  { "pmaddubsw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3804 },
  { "phsubw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3805 },
  { "phsubd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3806 },
  { "phsubsw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3807 },
  { "psignb", ORC_X86_INSN_TYPE_SD, 0, 0x660f3808 },
  { "psignw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3809 },
  { "psignd", ORC_X86_INSN_TYPE_SD, 0, 0x660f380a },
  { "pmulhrsw", ORC_X86_INSN_TYPE_SD, 0, 0x660f380b },
  { "pabsb", ORC_X86_INSN_TYPE_SD, 0, 0x660f381c },
  { "pabsw", ORC_X86_INSN_TYPE_SD, 0, 0x660f381d },
  { "pabsd", ORC_X86_INSN_TYPE_SD, 0, 0x660f381e },
  { "pmovsxbw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3820 },
  { "pmovsxbd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3821 },
  { "pmovsxbq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3822 },
  { "pmovsxwd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3823 },
  { "pmovsxwq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3824 },
  { "pmovsxdq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3825 },
  { "pmuldq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3828 },
  { "pcmpeqq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3829 },
  { "packusdw", ORC_X86_INSN_TYPE_SD, 0, 0x660f382b },
  { "pmovzxbw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3830 },
  { "pmovzxbd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3831 },
  { "pmovzxbq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3832 },
  { "pmovzxwd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3833 },
  { "pmovzxwq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3834 },
  { "pmovzxdq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3835 },
  { "pmulld", ORC_X86_INSN_TYPE_SD, 0, 0x660f3840 },
  { "phminposuw", ORC_X86_INSN_TYPE_SD, 0, 0x660f3841 },
  { "pminsb", ORC_X86_INSN_TYPE_SD, 0, 0x660f3838 },
  { "pminsd", ORC_X86_INSN_TYPE_SD, 0, 0x660f3839 },
  { "pminuw", ORC_X86_INSN_TYPE_SD, 0, 0x660f383a },
  { "pminud", ORC_X86_INSN_TYPE_SD, 0, 0x660f383b },
  { "pmaxsb", ORC_X86_INSN_TYPE_SD, 0, 0x660f383c },
  { "pmaxsd", ORC_X86_INSN_TYPE_SD, 0, 0x660f383d },
  { "pmaxuw", ORC_X86_INSN_TYPE_SD, 0, 0x660f383e },
  { "pmaxud", ORC_X86_INSN_TYPE_SD, 0, 0x660f383f },
  { "pcmpgtq", ORC_X86_INSN_TYPE_SD, 0, 0x660f3837 },
  { "addps", ORC_X86_INSN_TYPE_SD, 0, 0x0f58 },
  { "subps", ORC_X86_INSN_TYPE_SD, 0, 0x0f5c },
  { "mulps", ORC_X86_INSN_TYPE_SD, 0, 0x0f59 },
  { "divps", ORC_X86_INSN_TYPE_SD, 0, 0x0f5e },
  { "sqrtps", ORC_X86_INSN_TYPE_SD, 0, 0x0f51 },
  { "addpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f58 },
  { "subpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f5c },
  { "mulpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f59 },
  { "divpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f5e },
  { "sqrtpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f51 },
  { "cmpeqps", ORC_X86_INSN_TYPE_SD2, 0, 0x0fc2, 0 },
  { "cmpeqpd", ORC_X86_INSN_TYPE_SD2, 0, 0x660fc2, 0 },
  { "cmpltps", ORC_X86_INSN_TYPE_SD2, 0, 0x0fc2, 1 },
  { "cmpltpd", ORC_X86_INSN_TYPE_SD2, 0, 0x660fc2, 1 },
  { "cmpleps", ORC_X86_INSN_TYPE_SD2, 0, 0x0fc2, 2 },
  { "cmplepd", ORC_X86_INSN_TYPE_SD2, 0, 0x660fc2, 2 },
  { "cvttps2dq", ORC_X86_INSN_TYPE_SD, 0, 0xf30f5b },
  { "cvttpd2dq", ORC_X86_INSN_TYPE_SD, 0, 0x660fe6 },
  { "cvtdq2ps", ORC_X86_INSN_TYPE_SD, 0, 0x0f5b },
  { "cvtdq2pd", ORC_X86_INSN_TYPE_SD, 0, 0xf30fe6 },
  { "cvtps2pd", ORC_X86_INSN_TYPE_SD, 0, 0x0f5a },
  { "cvtpd2ps", ORC_X86_INSN_TYPE_SD, 0, 0x660f5a },
  { "minps", ORC_X86_INSN_TYPE_SD, 0, 0x0f5d },
  { "minpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f5d },
  { "maxps", ORC_X86_INSN_TYPE_SD, 0, 0x0f5f },
  { "maxpd", ORC_X86_INSN_TYPE_SD, 0, 0x660f5f },
  { "psraw", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f71, 4 },
  { "psrlw", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f71, 2 },
  { "psllw", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f71, 6 },
  { "psrad", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f72, 4 },
  { "psrld", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f72, 2 },
  { "pslld", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f72, 6 },
  { "psrlq", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f73, 2 },
  { "psllq", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f73, 6 },
  { "psrldq", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f73, 3 },
  { "pslldq", ORC_X86_INSN_TYPE_SHIFTIMM, 0, 0x660f73, 7 },

  //{ "", ORC_X86_INSN_TYPE_SD, 0, 0x660f58 },
  //{ "", ORC_X86_INSN_TYPE_SD, 0, 0x660f58 },
  //{ "addps", ORC_X86_INSN_TYPE_SD, 0, 0x660f58 },
};


void
orc_sse_emit_sysinsn (OrcCompiler *p, int index, int src, int dest)
{
  OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_SD2:
      ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", opcode->name,
          orc_x86_get_regname_sse(src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_SHIFTIMM:
      ORC_ASM_CODE(p,"  %s $%d, %%%s\n", opcode->name,
          src,
          orc_x86_get_regname_sse(dest));
      break;
  }

  if (opcode->code & 0xff000000) {
    *p->codeptr++ = (opcode->code >> 24) & 0xff;
    orc_x86_emit_rex (p, 0, dest, 0, src);
    *p->codeptr++ = (opcode->code >> 16) & 0xff;
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  } else if (opcode->code & 0xff0000) {
    *p->codeptr++ = (opcode->code >> 16) & 0xff;
    orc_x86_emit_rex (p, 0, dest, 0, src);
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  } else {
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
    orc_x86_emit_rex (p, 0, dest, 0, src);
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
      orc_x86_emit_modrm_reg (p, src, dest);
      break;
    case ORC_X86_INSN_TYPE_SHIFTIMM:
      orc_x86_emit_modrm_reg (p, dest, opcode->code2);
      *p->codeptr++ = src;
      break;
    case ORC_X86_INSN_TYPE_SD2:
      orc_x86_emit_modrm_reg (p, src, dest);
      *p->codeptr++ = opcode->code2;
      break;
  }

}

