
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcc64x.h>
#include <orc/orcdebug.h>


void
orc_c64x_loadb (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
#if 0
  uint32_t code;
  int i;
#endif

  ORC_ASM_CODE(compiler,"  ldb *+%s(0), %s\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  if (compiler->loop_shift == 3) {
    ORC_ASM_CODE(compiler,"  ldw 0(%s), %s\n",
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (dest));
    code = 0xf42007cd;
    code |= (src1&0xf) << 16;
    code |= (dest&0xf) << 12;
    code |= ((dest>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.8 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (dest), i,
          orc_c64x_reg_name (src1),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= (src1&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= i << 5;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_loadw (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  ORC_ASM_CODE(compiler,"  ldh *+%s(0), %s\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_c64x_reg_name (dest),
        orc_c64x_reg_name (src1),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (src1&0xf) << 16;
    code |= (dest&0xf) << 12;
    code |= ((dest>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.16 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (dest), i,
          orc_c64x_reg_name (src1),
          update ? "!" : "");
      code = 0xf4a0040d;
      code |= (src1&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= i << 6;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_loadl (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  ORC_ASM_CODE(compiler,"  ldw *+%s(0), %s\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 1) {
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_c64x_reg_name (dest),
        orc_c64x_reg_name (src1),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (src1&0xf) << 16;
    code |= (dest&0xf) << 12;
    code |= ((dest>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.32 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (dest), i,
          orc_c64x_reg_name (src1),
          update ? "!" : "");
      code = 0xf4a0080d;
      code |= (src1&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= i<<7;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_storeb (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  ORC_ASM_CODE(compiler,"  stb %s, *+%s(0)\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 3) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.8 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (src1), i,
          orc_c64x_reg_name (dest),
          update ? "!" : "");
      code = 0xf480000d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<5;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_storew (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  ORC_ASM_CODE(compiler,"  sth %s, *+%s(0)\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (src1), i,
          orc_c64x_reg_name (dest),
          update ? "!" : "");
      code = 0xf480040d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<6;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_storel (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  ORC_ASM_CODE(compiler,"  stw %s, *+%s(0)\n",
      orc_c64x_reg_name (src1),
      orc_c64x_reg_name (dest));
#if 0
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_c64x_reg_name (src1),
        orc_c64x_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_c64x_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s]%s\n",
          orc_c64x_reg_name (src1), i,
          orc_c64x_reg_name (dest),
          update ? "!" : "");
      code = 0xf480080d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<7;
      code |= (!update) << 1;
      orc_c64x_emit (compiler, code);
    }
  }
#endif
}

void
orc_c64x_emit_loadib (OrcCompiler *compiler, int dest, int value)
{
  if (value == 0) {
    ORC_ASM_CODE(compiler,"  zero %s\n",
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  mvk %d, %s\n",
        value,
        orc_c64x_reg_name (dest));
  }
}

void
orc_c64x_emit_loadiw (OrcCompiler *compiler, int dest, int value)
{
  if (value == 0) {
    ORC_ASM_CODE(compiler,"  zero %s\n",
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  mvk %d, %s\n",
        value,
        orc_c64x_reg_name (dest));
  }
}

void
orc_c64x_emit_loadil (OrcCompiler *compiler, int dest, int value)
{
  if (value == 0) {
    ORC_ASM_CODE(compiler,"  zero %s\n",
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  mvk %d, %s\n",
        value,
        orc_c64x_reg_name (dest));
  }
}

void
orc_c64x_emit_loadpb (OrcCompiler *compiler, int dest, int param)
{
  int offset = ORC_STRUCT_OFFSET(OrcExecutor, params[param]);

  if (offset >= 256) {
    orc_c64x_emit_add_imm (compiler, compiler->tmpreg, compiler->exec_reg,
        offset);
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->tmpreg), 0,
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->exec_reg),
        offset,
        orc_c64x_reg_name (dest));
  }
}

void
orc_c64x_emit_loadpw (OrcCompiler *compiler, int dest, int param)
{
  int offset = ORC_STRUCT_OFFSET(OrcExecutor, params[param]);

  if (offset >= 256) {
    orc_c64x_emit_add_imm (compiler, compiler->tmpreg, compiler->exec_reg,
        offset);
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->tmpreg), 0,
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->exec_reg),
        offset,
        orc_c64x_reg_name (dest));
  }
}

void
orc_c64x_emit_loadpl (OrcCompiler *compiler, int dest, int param)
{
  int offset = ORC_STRUCT_OFFSET(OrcExecutor, params[param]);

  if (offset >= 256) {
    orc_c64x_emit_add_imm (compiler, compiler->tmpreg, compiler->exec_reg,
        offset);
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->tmpreg), 0,
        orc_c64x_reg_name (dest));
  } else {
    ORC_ASM_CODE(compiler,"  ldw *+%s(%d), %s\n",
        orc_c64x_reg_name (compiler->exec_reg),
        offset,
        orc_c64x_reg_name (dest));
  }
}

#define UNARY(opcode,insn_name,code) \
static void \
orc_c64x_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc), \
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5; \
  orc_c64x_emit (p, x); \
}

#define BINARY(opcode,insn_name,code) \
static void \
orc_c64x_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, %s\n", \
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc), \
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc), \
      orc_c64x_reg_name (p->vars[insn->src_args[1]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<7; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5; \
  orc_c64x_emit (p, x); \
}

//UNARY(absb,"abs4",0x00000000)
BINARY(addb,"add4",0x00000000)
//BINARY(addssb,"sadds4",0x00000000)
BINARY(addusb,"saddu4",0x00000000)
BINARY(andb,"and",0x00000000)
BINARY(andnb,"andn",0x00000000)
//BINARY(avgsb,"avgs4",0x00000000)
BINARY(avgub,"avgu4",0x00000000)
BINARY(cmpeqb,"cmpeq4",0x00000000)
//BINARY(cmpgtsb,"cmpgt4",0x00000000)
UNARY(copyb,"mv",0x00000000)
//BINARY(maxsb,"maxs4",0x00000000)
BINARY(maxub,"maxu4",0x00000000)
//BINARY(minsb,"mins4",0x00000000)
BINARY(minub,"minu4",0x00000000)
//BINARY(mullb,"mpy",0x00000000)
BINARY(orb,"or",0x00000000)
//LSHIFT(shlb,"shl4",0x00000000)
//RSHIFT(shrsb,"shr4",0x00000000,8)
//RSHIFT(shrub,"shr4",0x00000000,8)
BINARY(subb,"sub4",0x00000000)
BINARY(subssb,"sub4",0x00000000)
BINARY(subusb,"sub4",0x00000000)
BINARY(xorb,"xor",0x00000000)

UNARY(absw,"abs2",0x00000000)
BINARY(addw,"add2",0x00000000)
BINARY(addssw,"sadd2",0x00000000)
//BINARY(addusw,"saddu2",0x00000000)
BINARY(andw,"and",0x00000000)
BINARY(andnw,"andn",0x00000000)
BINARY(avgsw,"avg2",0x00000000)
//BINARY(avguw,"avgu2",0x00000000)
BINARY(cmpeqw,"cmpeq2",0x00000000)
BINARY(cmpgtsw,"cmpgt2",0x00000000)
UNARY(copyw,"mv",0x00000000)
BINARY(maxsw,"max2",0x00000000)
//BINARY(maxuw,"maxu2",0x00000000)
BINARY(minsw,"min2",0x00000000)
//BINARY(minuw,"minu2",0x00000000)
//BINARY(mullw,"mpy2",0x00000000)
BINARY(orw,"or",0x00000000)
//LSHIFT(shlw,"shl2",0x00000000)
//RSHIFT(shrsw,"shr2",0x00000000,16)
//RSHIFT(shruw,"shru2",0x00000000,16)
BINARY(subw,"sub2",0x00000000)
BINARY(subssw,"sub2",0x00000000)
BINARY(subusw,"sub2",0x00000000)
BINARY(xorw,"xor",0x00000000)

UNARY(absl,"abs",0x00000000)
BINARY(addl,"add",0x00000000)
BINARY(addssl,"sadd",0x00000000)
BINARY(addusl,"add",0x00000000)
BINARY(andl,"and",0x00000000)
//BINARY(andnl,"andn",0x00000000)
//BINARY(avgsl,"avgs",0x00000000)
//BINARY(avgul,"avgu",0x00000000)
BINARY(cmpeql,"cmpeq",0x00000000)
BINARY(cmpgtsl,"cmpgt",0x00000000)
UNARY(copyl,"mv",0x00000000)
//BINARY(maxsl,"maxs",0x00000000)
//BINARY(maxul,"maxu",0x00000000)
//BINARY(minsl,"mins",0x00000000)
//BINARY(minul,"minu",0x00000000)
BINARY(mulll,"mpy32",0x00000000)
BINARY(orl,"or",0x00000000)
//LSHIFT(shll,"shl",0x00000000)
//RSHIFT(shrsl,"shr",0x00000000,32)
//RSHIFT(shrul,"shru",0x00000000,32)
BINARY(subl,"sub",0x00000000)
//BINARY(subssl,"ssub",0x00000000)
//BINARY(subusl,"ssub",0x00000000)
BINARY(xorl,"xor",0x00000000)

UNARY(swapw,"swap4",0x00000000)
//UNARY(swapl,"swap2;swap4",0x00000000)

#if 0
static void
orc_c64x_emit_binary (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_c64x_reg_name (dest), orc_c64x_reg_name (src1), orc_c64x_reg_name (src2));
  code |= (dest&0xf)<<16;
  code |= ((dest>>4)&0x1)<<7;
  code |= (src1&0xf)<<12;
  code |= ((src1>>4)&0x1)<<22;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_c64x_emit (p, code);

}
#endif

#if 0
static void
orc_c64x_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_c64x_emit_binary (p, "vadd.i16", 0xf2100800,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_c64x_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_c64x_emit_binary (p, "vadd.i32", 0xf2200800,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_c64x_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t x;
  
  x = 0xf3b00100;
  ORC_ASM_CODE(p,"  vrev16.i8 %s, %s\n",
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc));
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
  //x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  //x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
  orc_c64x_emit (p, x);

  x = 0xf3b20200;
  ORC_ASM_CODE(p,"  vmovn.i16 %s, %s\n",
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc));
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
  //x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  //x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
  orc_c64x_emit (p, x);
}

static void
orc_c64x_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t x;
  
  x = 0xf3b40080;
  ORC_ASM_CODE(p,"  vrev32.i16 %s, %s\n",
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc));
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
  //x |= (p->vars[insn->src_args[1]].alloc&0xf)<<16;
  //x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
  orc_c64x_emit (p, x);

  x = 0xf3b60200;
  ORC_ASM_CODE(p,"  vmovn.i32 %s, %s\n",
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc));
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
  //x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  //x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
  orc_c64x_emit (p, x);
}

static void
orc_c64x_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t x;
  
  x = 0xf3800700;
  ORC_ASM_CODE(p,"  vabdl.u8 %s, %s, %s\n",
      orc_c64x_reg_name (p->tmpreg),
      orc_c64x_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_c64x_reg_name (p->vars[insn->src_args[1]].alloc));
  x |= (p->tmpreg&0xf)<<12;
  x |= ((p->tmpreg>>4)&0x1)<<22;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5;
  orc_c64x_emit (p, x);

  x = 0xf3b40680;
  ORC_ASM_CODE(p,"  vpadal.u16 %s, %s\n",
      orc_c64x_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_c64x_reg_name (p->tmpreg));
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
  //x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  //x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->tmpreg&0xf)<<0;
  x |= ((p->tmpreg>>4)&0x1)<<5;
  orc_c64x_emit (p, x);
}
#endif

void
orc_compiler_c64x_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#define REG(x) \
    orc_rule_register (rule_set, #x , orc_c64x_rule_ ## x, NULL)

  //REG(absb);
  REG(addb);
  //REG(addssb);
  REG(addusb);
  REG(andb);
  REG(andnb);
  //REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  //REG(cmpgtsb);
  REG(copyb);
  //REG(maxsb);
  REG(maxub);
  //REG(minsb);
  REG(minub);
  //REG(mullb);
  REG(orb);
  //REG(shlb);
  //REG(shrsb);
  //REG(shrub);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(absw);
  REG(addw);
  REG(addssw);
  //REG(addusw);
  REG(andw);
  REG(andnw);
  REG(avgsw);
  //REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(copyw);
  REG(maxsw);
  //REG(maxuw);
  REG(minsw);
  //REG(minuw);
  //REG(mullw);
  REG(orw);
  //REG(shlw);
  //REG(shrsw);
  //REG(shruw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(absl);
  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  //REG(andnl);
  //REG(avgsl);
  //REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(copyl);
  //REG(maxsl);
  //REG(maxul);
  //REG(minsl);
  //REG(minul);
  REG(mulll);
  REG(orl);
  //REG(shll);
  //REG(shrsl);
  //REG(shrul);
  REG(subl);
  //REG(subssl);
  //REG(subusl);
  REG(xorl);

#if 0
  REG(convsbw);
  REG(convubw);
  REG(convswl);
  REG(convuwl);
  REG(convlw);
  REG(convssslw);
  REG(convsuslw);
  REG(convuuslw);
  REG(convwb);
  REG(convssswb);
  REG(convsuswb);
  REG(convuuswb);

  REG(mulsbw);
  REG(mulubw);
  REG(mulswl);
  REG(muluwl);

  REG(accw);
  REG(accl);
  REG(accsadubl);
#endif
  REG(swapw);
#if 0
  REG(swapl);
  REG(select0wb);
  REG(select1wb);
  REG(select0lw);
  REG(select1lw);
  REG(mergebw);
  REG(mergewl);
#endif

#if 0
  orc_rule_register (rule_set, "shlb", orc_c64x_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shrsb", orc_c64x_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrub", orc_c64x_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shlw", orc_c64x_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrsw", orc_c64x_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shruw", orc_c64x_rule_shift, (void *)5);
  orc_rule_register (rule_set, "shll", orc_c64x_rule_shift, (void *)6);
  orc_rule_register (rule_set, "shrsl", orc_c64x_rule_shift, (void *)7);
  orc_rule_register (rule_set, "shrul", orc_c64x_rule_shift, (void *)8);
#endif
}

