
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc/orccpuinsn.h>
#include <stdlib.h>


static const OrcSysOpcode orc_x86_opcodes[] = {
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
  { "pshufd", ORC_X86_INSN_TYPE_SDI, 0, 0x660f70 },
  { "pshuflw", ORC_X86_INSN_TYPE_SDI, 0, 0xf20f70 },
  { "pshufhw", ORC_X86_INSN_TYPE_SDI, 0, 0xf30f70 },
  { "palignr", ORC_X86_INSN_TYPE_SDI, 0, 0x660f3a0f },
  { "pinsrw", ORC_X86_INSN_TYPE_EDI, 0, 0x660fc4 },
  { "movd", ORC_X86_INSN_TYPE_ED, 0, 0x660f6e },
  { "movq", ORC_X86_INSN_TYPE_SD, 0, 0xf30f7e },
  { "movdqa", ORC_X86_INSN_TYPE_SD, 0, 0x660f6f },
  { "movdqu", ORC_X86_INSN_TYPE_SD, 0, 0xf30f6f },
  { "movhps", ORC_X86_INSN_TYPE_SD, 0, 0x0f16 },
  { "pextrw", ORC_X86_INSN_TYPE_SDI_REV, 0, 0x660f3a15 },
  { "movd", ORC_X86_INSN_TYPE_ED_REV, 0, 0x660f7e },
  { "movq", ORC_X86_INSN_TYPE_SD_REV, 0, 0x660fd6 },
  { "movdqa", ORC_X86_INSN_TYPE_SD_REV, 0, 0x660f7f },
  { "movdqu", ORC_X86_INSN_TYPE_SD_REV, 0, 0xf30f7f },
  { "movntdq", ORC_X86_INSN_TYPE_SD_REV, 0, 0x660fe7 },
  { "ldmxcsr", ORC_X86_INSN_TYPE_MEM, 0, 0x0fae, 2 },
  { "stmxcsr", ORC_X86_INSN_TYPE_MEM, 0, 0x0fae, 3 },
  { "add", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 0 },
  { "add", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 0 },
  { "add", ORC_X86_INSN_TYPE_rm_r, 0, 0x03 },
  { "add", ORC_X86_INSN_TYPE_r_rm, 0, 0x01 },
  { "or", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 1 },
  { "or", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 1 },
  { "or", ORC_X86_INSN_TYPE_rm_r, 0, 0x0b },
  { "or", ORC_X86_INSN_TYPE_r_rm, 0, 0x09 },
  { "adc", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 2 },
  { "adc", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 2 },
  { "adc", ORC_X86_INSN_TYPE_rm_r, 0, 0x13 },
  { "adc", ORC_X86_INSN_TYPE_r_rm, 0, 0x11 },
  { "sbb", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 3 },
  { "sbb", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 3 },
  { "sbb", ORC_X86_INSN_TYPE_rm_r, 0, 0x1b },
  { "sbb", ORC_X86_INSN_TYPE_r_rm, 0, 0x19 },
  { "and", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 4 },
  { "and", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 4 },
  { "and", ORC_X86_INSN_TYPE_rm_r, 0, 0x23 },
  { "and", ORC_X86_INSN_TYPE_r_rm, 0, 0x21 },
  { "sub", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 5 },
  { "sub", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 5 },
  { "sub", ORC_X86_INSN_TYPE_rm_r, 0, 0x2b },
  { "sub", ORC_X86_INSN_TYPE_r_rm, 0, 0x29 },
  { "xor", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 6 },
  { "xor", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 6 },
  { "xor", ORC_X86_INSN_TYPE_rm_r, 0, 0x33 },
  { "xor", ORC_X86_INSN_TYPE_r_rm, 0, 0x31 },
  { "cmp", ORC_X86_INSN_TYPE_imm8_rm, 0, 0x83, 7 },
  { "cmp", ORC_X86_INSN_TYPE_imm32_rm, 0, 0x81, 7 },
  { "cmp", ORC_X86_INSN_TYPE_rm_r, 0, 0x3b },
  { "cmp", ORC_X86_INSN_TYPE_r_rm, 0, 0x39 },
  { "jo", ORC_X86_INSN_TYPE_LABEL, 0, 0x70 },
  { "jno", ORC_X86_INSN_TYPE_LABEL, 0, 0x71 },
  { "jc", ORC_X86_INSN_TYPE_LABEL, 0, 0x72 },
  { "jnc", ORC_X86_INSN_TYPE_LABEL, 0, 0x73 },
  { "jz", ORC_X86_INSN_TYPE_LABEL, 0, 0x74 },
  { "jnz", ORC_X86_INSN_TYPE_LABEL, 0, 0x75 },
  { "jbe", ORC_X86_INSN_TYPE_LABEL, 0, 0x76 },
  { "ja", ORC_X86_INSN_TYPE_LABEL, 0, 0x77 },
  { "js", ORC_X86_INSN_TYPE_LABEL, 0, 0x78 },
  { "jns", ORC_X86_INSN_TYPE_LABEL, 0, 0x79 },
  { "jp", ORC_X86_INSN_TYPE_LABEL, 0, 0x7a },
  { "jnp", ORC_X86_INSN_TYPE_LABEL, 0, 0x7b },
  { "jl", ORC_X86_INSN_TYPE_LABEL, 0, 0x7c },
  { "jge", ORC_X86_INSN_TYPE_LABEL, 0, 0x7d },
  { "jle", ORC_X86_INSN_TYPE_LABEL, 0, 0x7e },
  { "jg", ORC_X86_INSN_TYPE_LABEL, 0, 0x7f },
  { "jmp", ORC_X86_INSN_TYPE_LABEL, 0, 0xeb },
  { "", ORC_X86_INSN_TYPE_LABEL, 0, 0x00 },
  { "ret", ORC_X86_INSN_TYPE_NONE, 0, 0xc3 },
  { "retq", ORC_X86_INSN_TYPE_NONE, 0, 0xc3 },
  { "emms", ORC_X86_INSN_TYPE_NONE, 0, 0x0f77 },
  { "rdtsc", ORC_X86_INSN_TYPE_NONE, 0, 0x0f31 },
  { "nop", ORC_X86_INSN_TYPE_NONE, 0, 0x90 },
  { "rep movsb", ORC_X86_INSN_TYPE_NONE, 0, 0xf3a4 },
  { "rep movsw", ORC_X86_INSN_TYPE_NONE, 0, 0x66f3a5 },
  { "rep movsl", ORC_X86_INSN_TYPE_NONE, 0, 0xf3a5 },
  { "push", ORC_X86_INSN_TYPE_STACK, 0, 0x50 },
  { "pop", ORC_X86_INSN_TYPE_STACK, 0, 0x58 },
  { "movzx", ORC_X86_INSN_TYPE_rm_r, 0, 0x0fb6 },
  { "movw", ORC_X86_INSN_TYPE_rm_r, 0, 0x668b },
  { "movl", ORC_X86_INSN_TYPE_rm_r, 0, 0x8b },
  { "mov", ORC_X86_INSN_TYPE_rm_r, 0, 0x8b },
  { "mov", ORC_X86_INSN_TYPE_mov_imm32, 0, 0xb8 },
  { "movb", ORC_X86_INSN_TYPE_r_rm_byte, 0, 0x88 },
  { "movw", ORC_X86_INSN_TYPE_r_rm_word, 0, 0x6689 },
  { "movl", ORC_X86_INSN_TYPE_r_rm, 0, 0x89 },
  { "mov", ORC_X86_INSN_TYPE_r_rm, 0, 0x89 },
  { "test", ORC_X86_INSN_TYPE_rm_r, 0, 0x85 },
  { "testl", ORC_X86_INSN_TYPE_imm32_rm, 0, 0xf7, 0 },
  { "leal", ORC_X86_INSN_TYPE_rm_r, 0, 0x8d },
  { "leaq", ORC_X86_INSN_TYPE_rm_r, 0, 0x8d },
  { "imul", ORC_X86_INSN_TYPE_rm_r, 0, 0x0faf },
  { "imul", ORC_X86_INSN_TYPE_MEM, 0, 0xf7, 5 },
  { "incl", ORC_X86_INSN_TYPE_MEM, 0, 0xff, 0 },
  { "decl", ORC_X86_INSN_TYPE_MEM, 0, 0xff, 1 },
  { "sar", ORC_X86_INSN_TYPE_imm8_rm, 0, 0xc1, 7 },
  { "sar", ORC_X86_INSN_TYPE_MEM, 0, 0xd1, 7 },
  { "and", ORC_X86_INSN_TYPE_imm32_a, 0, 0x25, 4 },
};

static void
output_opcode (OrcCompiler *p, const OrcSysOpcode *opcode, int size,
    int src, int dest)
{
  ORC_ASSERT(opcode->code != 0);

  if (opcode->code & 0xff000000) {
    if ((opcode->code & 0xff000000) == 0x66000000 ||
        (opcode->code & 0xff000000) == 0xf3000000 ||
        (opcode->code & 0xff000000) == 0xf2000000) {
      *p->codeptr++ = (opcode->code >> 24) & 0xff;
      orc_x86_emit_rex (p, size, dest, 0, src);
    } else {
      *p->codeptr++ = (opcode->code >> 24) & 0xff;
      orc_x86_emit_rex (p, size, dest, 0, src);
    }
    *p->codeptr++ = (opcode->code >> 16) & 0xff;
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  } else if (opcode->code & 0xff0000) {
    if ((opcode->code & 0xff0000) == 0x660000 ||
        (opcode->code & 0xff0000) == 0xf30000 ||
        (opcode->code & 0xff0000) == 0xf20000) {
      *p->codeptr++ = (opcode->code >> 16) & 0xff;
      orc_x86_emit_rex (p, size, dest, 0, src);
    } else {
      orc_x86_emit_rex (p, size, dest, 0, src);
      *p->codeptr++ = (opcode->code >> 16) & 0xff;
    }
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  } else if (opcode->code & 0xff00) {
    if ((opcode->code & 0xff00) == 0x6600 ||
        (opcode->code & 0xff00) == 0xf300 ||
        (opcode->code & 0xff00) == 0xf200) {
      *p->codeptr++ = (opcode->code >> 8) & 0xff;
      orc_x86_emit_rex (p, size, dest, 0, src);
    } else {
      orc_x86_emit_rex (p, size, dest, 0, src);
      *p->codeptr++ = (opcode->code >> 8) & 0xff;
    }
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  } else {
    orc_x86_emit_rex (p, size, dest, 0, src);
    *p->codeptr++ = (opcode->code >> 0) & 0xff;
  }
}

void
orc_x86_emit_cpuinsn (OrcCompiler *p, int index, int imm, int src, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_SD2:
    case ORC_X86_INSN_TYPE_SD_REV:
      ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", opcode->name,
          orc_x86_get_regname_sse(src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_ED:
      ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", opcode->name,
          orc_x86_get_regname(src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_ED_REV:
      ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", opcode->name,
          orc_x86_get_regname_sse(src),
          orc_x86_get_regname(dest));
      break;
    case ORC_X86_INSN_TYPE_SHIFTIMM:
      ORC_ASM_CODE(p,"  %s $%d, %%%s\n", opcode->name,
          imm,
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_SDI:
      ORC_ASM_CODE(p,"  %s $%d, %%%s, %%%s\n", opcode->name,
          imm,
          orc_x86_get_regname_sse(src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_EDI:
      ORC_ASM_CODE(p,"  %s $%d, %%%s, %%%s\n", opcode->name,
          imm,
          orc_x86_get_regname(src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_rm_r:
    case ORC_X86_INSN_TYPE_r_rm:
      ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", opcode->name,
          orc_x86_get_regname(src),
          orc_x86_get_regname(dest));
      break;
    case ORC_X86_INSN_TYPE_STACK:
      ORC_ASM_CODE(p,"  %s %%%s\n", opcode->name,
          orc_x86_get_regname(dest));
      break;
    case ORC_X86_INSN_TYPE_MEM:
    default:
      ORC_ASSERT(0);
      break;
  }

  if (opcode->type == ORC_X86_INSN_TYPE_SHIFTIMM) {
    output_opcode (p, opcode, 4, dest, 0);
  } else if (opcode->type == ORC_X86_INSN_TYPE_ED_REV ||
      opcode->type == ORC_X86_INSN_TYPE_r_rm) {
    output_opcode (p, opcode, 4, dest, src);
  } else if (opcode->type != ORC_X86_INSN_TYPE_STACK) {
    output_opcode (p, opcode, 4, src, dest);
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_rm_r:
    case ORC_X86_INSN_TYPE_ED:
    case ORC_X86_INSN_TYPE_SD:
      orc_x86_emit_modrm_reg (p, src, dest);
      break;
    case ORC_X86_INSN_TYPE_r_rm:
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_ED_REV:
      orc_x86_emit_modrm_reg (p, dest, src);
      break;
    case ORC_X86_INSN_TYPE_SHIFTIMM:
      orc_x86_emit_modrm_reg (p, dest, opcode->code2);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
      orc_x86_emit_modrm_reg (p, src, dest);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SD2:
      orc_x86_emit_modrm_reg (p, src, dest);
      *p->codeptr++ = opcode->code2;
      break;
    case ORC_X86_INSN_TYPE_STACK:
      *p->codeptr++ = opcode->code + (dest&0x7);
      break;
    case ORC_X86_INSN_TYPE_MEM:
    default:
      ORC_ASSERT(0);
      break;
  }

}

void
orc_x86_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_SD2:
    case ORC_X86_INSN_TYPE_ED:
    case ORC_X86_INSN_TYPE_ED_REV:
      ORC_ASM_CODE(p,"  %s %d(%%%s), %%%s\n", opcode->name,
          offset,
          orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
    case ORC_X86_INSN_TYPE_SDI_REV:
      ORC_ASM_CODE(p,"  %s $%d, %d(%%%s), %%%s\n", opcode->name,
          imm, offset,
          orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_MEM:
      ORC_ASM_CODE(p,"  %s %d(%%%s)\n", opcode->name,
          offset,
          orc_x86_get_regname_ptr(p, src));
      break;
    case ORC_X86_INSN_TYPE_rm_r:
      ORC_ASM_CODE(p,"  %s %d(%%%s), %%%s\n", opcode->name,
          offset, orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_size(src,size));
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  output_opcode (p, opcode, size, src, dest);

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_ED:
    case ORC_X86_INSN_TYPE_rm_r:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SDI_REV:
      orc_x86_emit_modrm_memoffset (p, offset, dest, src);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_ED_REV:
      orc_x86_emit_modrm_memoffset (p, offset, dest, src);
      break;
    case ORC_X86_INSN_TYPE_SD2:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      *p->codeptr++ = opcode->code2;
      break;
    case ORC_X86_INSN_TYPE_MEM:
      orc_x86_emit_modrm_memoffset (p, offset, src, opcode->code2);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_SD2:
    case ORC_X86_INSN_TYPE_ED_REV:
      ORC_ASM_CODE(p,"  %s %%%s, %d(%%%s)\n", opcode->name,
          orc_x86_get_regname_sse(src),
          offset,
          orc_x86_get_regname_ptr(p, dest));
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
    case ORC_X86_INSN_TYPE_SDI_REV:
      ORC_ASM_CODE(p,"  %s $%d, %%%s, %d(%%%s)\n", opcode->name,
          imm, orc_x86_get_regname_sse(src),
          offset,
          orc_x86_get_regname_ptr(p, dest));
      break;
    case ORC_X86_INSN_TYPE_ED:
    default:
      ORC_ASSERT(0);
      break;
  }

  if (opcode->type == ORC_X86_INSN_TYPE_SD_REV ||
      opcode->type == ORC_X86_INSN_TYPE_SDI_REV ||
      opcode->type == ORC_X86_INSN_TYPE_ED_REV) {
    output_opcode (p, opcode, 4, dest, src);
  } else {
    output_opcode (p, opcode, 4, src, dest);
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SDI_REV:
      orc_x86_emit_modrm_memoffset (p, offset, dest, src);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_ED_REV:
      orc_x86_emit_modrm_memoffset (p, offset, dest, src);
      break;
    case ORC_X86_INSN_TYPE_SD2:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      *p->codeptr++ = opcode->code2;
      break;
    case ORC_X86_INSN_TYPE_ED:
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int src_index, int shift, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_SD_REV:
    case ORC_X86_INSN_TYPE_SD2:
    case ORC_X86_INSN_TYPE_ED:
    case ORC_X86_INSN_TYPE_ED_REV:
      ORC_ASM_CODE(p,"  %s %d(%%%s,%%%s,%d), %%%s\n", opcode->name,
          offset,
          orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_ptr(p, src_index), 1<<shift,
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
    case ORC_X86_INSN_TYPE_SDI_REV:
      ORC_ASM_CODE(p,"  %s $%d, %d(%%%s,%%%s,%d), %%%s\n", opcode->name,
          imm, offset,
          orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_ptr(p, src_index), 1<<shift,
          orc_x86_get_regname_sse(dest));
      break;
    case ORC_X86_INSN_TYPE_rm_r:
      ORC_ASM_CODE(p,"  %s %d(%%%s,%%%s,%d), %%%s\n", opcode->name,
          offset,
          orc_x86_get_regname_ptr(p, src),
          orc_x86_get_regname_ptr(p, src_index), 1<<shift,
          orc_x86_get_regname_size(dest,size));
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  output_opcode (p, opcode, size, src, dest);

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_SD:
    case ORC_X86_INSN_TYPE_ED:
    case ORC_X86_INSN_TYPE_ED_REV:
    case ORC_X86_INSN_TYPE_rm_r:
      orc_x86_emit_modrm_memindex2 (p, offset, src, src_index, shift, dest);
      break;
    case ORC_X86_INSN_TYPE_EDI:
    case ORC_X86_INSN_TYPE_SDI:
      orc_x86_emit_modrm_memindex2 (p, offset, src, src_index, shift, dest);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SDI_REV:
      orc_x86_emit_modrm_memindex2 (p, offset, dest, src_index, shift, src);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_SD_REV:
      orc_x86_emit_modrm_memindex2 (p, offset, dest, src_index, shift, src);
      break;
    case ORC_X86_INSN_TYPE_SD2:
      orc_x86_emit_modrm_memindex2 (p, offset, src, src_index, shift, dest);
      *p->codeptr++ = opcode->code2;
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_imm_reg (OrcCompiler *p, int index, int size, int imm,
    int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_imm8_rm:
    case ORC_X86_INSN_TYPE_imm32_rm:
    case ORC_X86_INSN_TYPE_mov_imm32:
    case ORC_X86_INSN_TYPE_imm32_a:
      if (size == 4) {
        ORC_ASM_CODE(p,"  %s $%d, %%%s\n", opcode->name,
            imm,
            orc_x86_get_regname(dest));
      } else {
        ORC_ASM_CODE(p,"  %s $%d, %%%s\n", opcode->name,
            imm,
            orc_x86_get_regname_64(dest));
      }
      break;
    case ORC_X86_INSN_TYPE_MEM:
      if (size == 4) {
        ORC_ASM_CODE(p,"  %s %%%s\n", opcode->name,
            orc_x86_get_regname(dest));
      } else {
        ORC_ASM_CODE(p,"  %s %%%s\n", opcode->name,
            orc_x86_get_regname_64(dest));
      }
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  if (opcode->type != ORC_X86_INSN_TYPE_mov_imm32 &&
      opcode->type != ORC_X86_INSN_TYPE_imm32_a) {
    output_opcode (p, opcode, size, dest, 0);
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_imm8_rm:
      orc_x86_emit_modrm_reg (p, dest, opcode->code2);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_imm32_rm:
      orc_x86_emit_modrm_reg (p, dest, opcode->code2);
      *p->codeptr++ = imm&0xff;
      *p->codeptr++ = (imm>>8)&0xff;
      *p->codeptr++ = (imm>>16)&0xff;
      *p->codeptr++ = (imm>>24)&0xff;
      break;
    case ORC_X86_INSN_TYPE_imm32_a:
      *p->codeptr++ = imm&0xff;
      *p->codeptr++ = (imm>>8)&0xff;
      *p->codeptr++ = (imm>>16)&0xff;
      *p->codeptr++ = (imm>>24)&0xff;
      break;
    case ORC_X86_INSN_TYPE_mov_imm32:
      *p->codeptr++ = opcode->code + (dest&7);
      *p->codeptr++ = imm&0xff;
      *p->codeptr++ = (imm>>8)&0xff;
      *p->codeptr++ = (imm>>16)&0xff;
      *p->codeptr++ = (imm>>24)&0xff;
      break;
    case ORC_X86_INSN_TYPE_MEM:
      orc_x86_emit_modrm_reg (p, dest, opcode->code2);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_imm_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_imm8_rm:
    case ORC_X86_INSN_TYPE_imm32_rm:
      ORC_ASM_CODE(p,"  %s $%d, %d(%%%s)\n", opcode->name,
          imm, offset,
          orc_x86_get_regname_ptr(p,dest));
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  output_opcode (p, opcode, size, dest, 0);

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_imm8_rm:
      orc_x86_emit_modrm_memoffset (p, offset, dest, opcode->code2);
      *p->codeptr++ = imm;
      break;
    case ORC_X86_INSN_TYPE_imm32_rm:
      orc_x86_emit_modrm_memoffset (p, offset, dest, opcode->code2);
      *p->codeptr++ = imm&0xff;
      *p->codeptr++ = (imm>>8)&0xff;
      *p->codeptr++ = (imm>>16)&0xff;
      *p->codeptr++ = (imm>>24)&0xff;
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_reg_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;
  int size = 4;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_r_rm:
      if (size == 4) {
        ORC_ASM_CODE(p,"  %s %%%s, %d(%%%s)\n", opcode->name,
            orc_x86_get_regname(src), offset,
            orc_x86_get_regname_ptr(p,dest));
      } else {
        ORC_ASM_CODE(p,"  %s %%%s, %d(%%%s)\n", opcode->name,
            orc_x86_get_regname_64(src), offset,
            orc_x86_get_regname_ptr(p,dest));
      }
      break;
    case ORC_X86_INSN_TYPE_r_rm_byte:
      ORC_ASM_CODE(p,"  %s %%%s, %d(%%%s)\n", opcode->name,
          orc_x86_get_regname_8(src), offset,
          orc_x86_get_regname_ptr(p,dest));
      break;
    case ORC_X86_INSN_TYPE_r_rm_word:
      ORC_ASM_CODE(p,"  %s %%%s, %d(%%%s)\n", opcode->name,
          orc_x86_get_regname_16(src), offset,
          orc_x86_get_regname_ptr(p,dest));
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  if (opcode->type == ORC_X86_INSN_TYPE_r_rm_byte ||
      opcode->type == ORC_X86_INSN_TYPE_r_rm_word) {
    output_opcode (p, opcode, size, dest, 0);
  } else {
    output_opcode (p, opcode, size, 0, dest);
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_r_rm:
    case ORC_X86_INSN_TYPE_r_rm_word:
    case ORC_X86_INSN_TYPE_r_rm_byte:
      orc_x86_emit_modrm_memoffset (p, offset, dest, src);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_memoffset_reg (OrcCompiler *p, int index, int size,
    int offset, int src, int dest)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_rm_r:
      ORC_ASM_CODE(p,"  %s %d(%%%s), %%%s\n", opcode->name,
          offset, orc_x86_get_regname_ptr(p,src),
          orc_x86_get_regname_size(dest,size));
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  output_opcode (p, opcode, size, src, dest);

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_rm_r:
      orc_x86_emit_modrm_memoffset (p, offset, src, dest);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_branch (OrcCompiler *p, int index, int label)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_LABEL:
      ORC_ASM_CODE(p,"  %s %d%c\n", opcode->name, label,
          (p->labels[label]!=NULL) ? 'b' : 'f');
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  if (p->long_jumps) {
    if (index == ORC_X86_jmp) {
      *p->codeptr++ = 0xe9;
    } else {
      *p->codeptr++ = 0x0f;
      *p->codeptr++ = opcode->code + 0x10;
    }
  } else {
    *p->codeptr++ = opcode->code;
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_LABEL:
      if (p->long_jumps) {
        x86_add_fixup (p, p->codeptr, label, 1);
        *p->codeptr++ = 0xfc;
        *p->codeptr++ = 0xff;
        *p->codeptr++ = 0xff;
        *p->codeptr++ = 0xff;
      } else {
        x86_add_fixup (p, p->codeptr, label, 0);
        *p->codeptr++ = 0xff;
      }
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_label (OrcCompiler *p, int index, int label)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_LABEL:
      ORC_ASM_CODE(p,"%d:\n", label);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_LABEL:
      x86_add_label (p, p->codeptr, label);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }
}

void
orc_x86_emit_cpuinsn_none (OrcCompiler *p, int index)
{
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;
  int size = 4;

  switch (opcode->type) {
    case ORC_X86_INSN_TYPE_NONE:
      ORC_ASM_CODE(p,"  %s\n", opcode->name);
      break;
    default:
      ORC_ASSERT(0);
      break;
  }

  output_opcode (p, opcode, size, 0, 0);
}

