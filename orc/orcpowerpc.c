
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcpowerpc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orcpowerpc
 * @title: PowerPC
 * @short_description: code generation for PowerPC
 */


void orc_compiler_powerpc_init (OrcCompiler *compiler);
void orc_compiler_powerpc_assemble (OrcCompiler *compiler);
void orc_compiler_powerpc_register_rules (OrcTarget *target);

const char *
powerpc_get_regname(int i)
{
  static const char *powerpc_regs[] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
    "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
    "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
    "r30", "r31",
    "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",
    "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19",
    "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29",
    "v30", "v31",
  };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 64) {
    return powerpc_regs[i - ORC_GP_REG_BASE];
  }
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

int
powerpc_regnum (int i)
{
  return (i-ORC_GP_REG_BASE)&0x1f;
}

void
powerpc_emit(OrcCompiler *compiler, unsigned int insn)
{
  *compiler->codeptr++ = (insn>>24);
  *compiler->codeptr++ = (insn>>16);
  *compiler->codeptr++ = (insn>>8);
  *compiler->codeptr++ = (insn>>0);
}

void
powerpc_emit_addi (OrcCompiler *compiler, int regd, int rega, int imm)
{
  unsigned int insn;

  ORC_ASM_CODE(compiler,"  addi %s, %s, %d\n",
      powerpc_get_regname(regd),
      powerpc_get_regname(rega), imm);
  insn = (14<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= imm&0xffff;

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_lwz (OrcCompiler *compiler, int regd, int rega, int imm)
{
  unsigned int insn;

  ORC_ASM_CODE(compiler,"  lwz %s, %d(%s)\n",
      powerpc_get_regname(regd),
      imm, powerpc_get_regname(rega));
  insn = (32<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= imm&0xffff;

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_stwu (OrcCompiler *compiler, int regs, int rega, int offset)
{
  unsigned int insn;

  ORC_ASM_CODE(compiler,"  stwu %s, %d(%s)\n",
      powerpc_get_regname(regs),
      offset, powerpc_get_regname(rega));
  insn = (37<<26) | (powerpc_regnum (regs)<<21) | (powerpc_regnum (rega)<<16);
  insn |= offset&0xffff;

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_srawi (OrcCompiler *compiler, int regd, int rega, int shift,
    int record)
{
  unsigned int insn;

  ORC_ASM_CODE(compiler,"  srawi%s %s, %s, %d\n", (record)?".":"",
      powerpc_get_regname(regd),
      powerpc_get_regname(rega), shift);

  insn = (31<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= (shift<<11) | (824<<1) | record;

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_655510 (OrcCompiler *compiler, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<0);

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_X (OrcCompiler *compiler, unsigned int insn, int d, int a, int b)
{
#if 0
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<1) | (0<<0);

  powerpc_emit (compiler, insn);
#endif
  insn |= ((d&0x1f)<<21);
  insn |= ((a&0x1f)<<16);
  insn |= ((b&0x1f)<<11);
  powerpc_emit (compiler, insn);
}

void
powerpc_emit_VA (OrcCompiler *compiler, int major, int d, int a, int b,
    int c, int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (c<<6) | (minor<<0);

  powerpc_emit (compiler, insn);
}

void
powerpc_emit_VA_2 (OrcCompiler *p, const char *name, int major, int d, int a, int b,
    int c, int minor)
{
  ORC_ASM_CODE(p,"  %s %s, %s, %s, %s\n", name,
      powerpc_get_regname(d),
      powerpc_get_regname(a),
      powerpc_get_regname(b),
      powerpc_get_regname(c));
  powerpc_emit_VA(p, major,
      powerpc_regnum(d),
      powerpc_regnum(a),
      powerpc_regnum(b),
      powerpc_regnum(c), minor);
}

void
powerpc_emit_VX (OrcCompiler *compiler, unsigned int insn, int d, int a, int b)
{
  insn |= ((d&0x1f)<<21);
  insn |= ((a&0x1f)<<16);
  insn |= ((b&0x1f)<<11);
  powerpc_emit (compiler, insn);
}

void
powerpc_emit_VX_2 (OrcCompiler *p, const char *name,
    unsigned int insn, int d, int a, int b)
{
  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      powerpc_get_regname(d),
      powerpc_get_regname(a),
      powerpc_get_regname(b));
  powerpc_emit_VX(p, insn,
      powerpc_regnum(d),
      powerpc_regnum(a),
      powerpc_regnum(b));
}

void
powerpc_emit_VX_3 (OrcCompiler *p, const char *name,
    unsigned int insn, int d, int a, int b, int c)
{
  ORC_ASM_CODE(p,"  %s %s, %s, %s, %d\n", name,
      powerpc_get_regname(d),
      powerpc_get_regname(a),
      powerpc_get_regname(b), c);
  powerpc_emit_VX(p, insn,
      powerpc_regnum(d),
      powerpc_regnum(a),
      powerpc_regnum(b));
}

void
powerpc_emit_VX_4 (OrcCompiler *p, const char *name,
    unsigned int insn, int d, int a)
{
  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      powerpc_get_regname(d),
      powerpc_get_regname(a));
  powerpc_emit_VX(p, insn,
      powerpc_regnum(d),
      0,
      powerpc_regnum(a));
}

void
powerpc_do_fixups (OrcCompiler *compiler)
{
  int i;
  unsigned int insn;

  for(i=0;i<compiler->n_fixups;i++){
    if (compiler->fixups[i].type == 0) {
      unsigned char *label = compiler->labels[compiler->fixups[i].label];
      unsigned char *ptr = compiler->fixups[i].ptr;

      insn = *(unsigned int *)ptr;
      *(unsigned int *)ptr = (insn&0xffff0000) | ((insn + (label-ptr))&0xffff);
    } else {
      unsigned char *label = compiler->labels[compiler->fixups[i].label];
      unsigned char *ptr = compiler->fixups[i].ptr;

      insn = *(unsigned int *)ptr;
      *(unsigned int *)ptr = (insn&0xffff0000) | ((insn + (label-compiler->program->code))&0xffff);
    }
  }
}

void
powerpc_flush (OrcCompiler *compiler)
{
#ifdef HAVE_POWERPC
  unsigned char *ptr;
  int cache_line_size = 32;
  int i;
  int size = compiler->codeptr - compiler->program->code;

  ptr = compiler->program->code;
  for (i=0;i<size;i+=cache_line_size) {
    __asm__ __volatile__ ("dcbst %0,%1" :: "r" (ptr), "r" (i));
  }
  __asm__ __volatile ("sync");

  ptr = compiler->program->code_exec;
  for (i=0;i<size;i+=cache_line_size) {
    __asm__ __volatile__ ("icbi %0,%1" :: "r" (ptr), "r" (i));
  }
  __asm__ __volatile ("isync");
#endif
}

static void
powerpc_load_constant (OrcCompiler *p, int i, int reg)
{
  int j;
  int value = p->constants[i].value;
  int greg = POWERPC_R31;
  int label_skip, label_data;

#if 0
  switch (p->constants[i].type) {
    case ORC_CONST_ZERO:
      powerpc_emit_VX_2(p, "vxor", 0x100004c4, reg, reg, reg);
      break;
    case ORC_CONST_SPLAT_B:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltisb %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000020c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_W:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltish %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000024c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_L:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltisw %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000028c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    default:
      ORC_COMPILER_ERROR(p,"unhandled");
      break;
  }
#endif

  switch (p->constants[i].type) {
    case ORC_CONST_ZERO:
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = 0;
      }
      break;
    case ORC_CONST_SPLAT_B:
      value &= 0xff;
      value |= (value<<8);
      value |= (value<<16);
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    case ORC_CONST_SPLAT_W:
      value &= 0xffff;
      value |= (value<<16);
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    case ORC_CONST_SPLAT_L:
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    default:
      break;
  }

  label_skip = orc_compiler_label_new (p);
  label_data = orc_compiler_label_new (p);

  powerpc_emit_b (p, label_skip);

  while ((p->codeptr - p->program->code) & 0xf) {
    ORC_ASM_CODE(p,"  .long 0x00000000\n");
    powerpc_emit (p, 0x00000000);
  }

  powerpc_emit_label (p, label_data);
  for(j=0;j<4;j++){
    ORC_ASM_CODE(p,"  .long 0x%08x\n", p->constants[i].full_value[j]);
    powerpc_emit (p, p->constants[i].full_value[j]);
  }

  powerpc_emit_label (p, label_skip);
  powerpc_emit_lwz (p,
      greg,
      POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, program));
  powerpc_emit_lwz (p,
      greg, greg,
      (int)ORC_STRUCT_OFFSET(OrcProgram, code_exec));

  powerpc_add_fixup (p, 1, p->codeptr, label_data);
  {
    unsigned int insn;

    ORC_ASM_CODE(p,"  addi %s, %s, %db - %s\n",
        powerpc_get_regname(greg),
        powerpc_get_regname(greg), label_data, p->program->name);
    insn = (14<<26) | (powerpc_regnum (greg)<<21) | (powerpc_regnum (greg)<<16);
    insn |= 0;

    powerpc_emit (p, insn);
  }

  ORC_ASM_CODE(p,"  lvx %s, 0, %s\n",
      powerpc_get_regname(reg),
      powerpc_get_regname(greg));
  powerpc_emit_X (p, 0x7c0000ce, reg, 0, greg);

}

int
powerpc_get_constant (OrcCompiler *p, int type, int value)
{
  int reg = p->tmpreg;
  int i;

  for(i=0;i<p->n_constants;i++){
    if (p->constants[i].type == type &&
        p->constants[i].value == value) {
      if (p->constants[i].alloc_reg != 0) {
        return p->constants[i].alloc_reg;
      }
      break;
    }
  }
  if (i == p->n_constants) {
    p->n_constants++;
    p->constants[i].type = type;
    p->constants[i].value = value;
    p->constants[i].alloc_reg = 0;
  }

  powerpc_load_constant (p, i, reg);

  return reg;
}

void powerpc_emit_ret (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  ret\n");
  //*compiler->codeptr++ = 0xc3;
}

void
powerpc_add_fixup (OrcCompiler *compiler, int type, unsigned char *ptr, int label)
{
  compiler->fixups[compiler->n_fixups].ptr = ptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = type;
  compiler->n_fixups++;
}

void
powerpc_add_label (OrcCompiler *compiler, unsigned char *ptr, int label)
{
  compiler->labels[label] = ptr;
}

void powerpc_emit_b (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"  b %d%c\n", label,
      (compiler->labels[label]!=NULL) ? 'b' : 'f');

  powerpc_add_fixup (compiler, 0, compiler->codeptr, label);
  powerpc_emit (compiler, 0x48000000);
}

void powerpc_emit_beq (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"  ble- %d%c\n", label,
      (compiler->labels[label]!=NULL) ? 'b' : 'f');

  powerpc_add_fixup (compiler, 0, compiler->codeptr, label);
  powerpc_emit (compiler, 0x40810000);
}

void powerpc_emit_bne (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"  bdnz+ %d%c\n", label,
      (compiler->labels[label]!=NULL) ? 'b' : 'f');

  powerpc_add_fixup (compiler, 0, compiler->codeptr, label);
  powerpc_emit (compiler, 0x42000000);
}

void powerpc_emit_label (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"%d:\n", label);

  powerpc_add_label (compiler, compiler->codeptr, label);
}

