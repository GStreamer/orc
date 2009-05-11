
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

#define SIZE 65536

void powerpc_emit_addi (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_lwz (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_stwu (OrcCompiler *compiler, int regs, int rega, int offset);

void powerpc_emit_ret (OrcCompiler *compiler);
void powerpc_emit_b (OrcCompiler *compiler, int label);
void powerpc_emit_beq (OrcCompiler *compiler, int label);
void powerpc_emit_bne (OrcCompiler *compiler, int label);
void powerpc_emit_label (OrcCompiler *compiler, int label);
void powerpc_add_fixup (OrcCompiler *compiler, int type, unsigned char *ptr, int label);

void orc_compiler_powerpc_init (OrcCompiler *compiler);
void orc_compiler_powerpc_assemble (OrcCompiler *compiler);
void orc_compiler_powerpc_register_rules (OrcTarget *target);

enum {
  POWERPC_R0 = ORC_GP_REG_BASE,
  POWERPC_R1,
  POWERPC_R2,
  POWERPC_R3,
  POWERPC_R4,
  POWERPC_R5,
  POWERPC_R6,
  POWERPC_R7,
  POWERPC_R8,
  POWERPC_R9,
  POWERPC_R10,
  POWERPC_R11,
  POWERPC_R12,
  POWERPC_R13,
  POWERPC_R14,
  POWERPC_R15,
  POWERPC_R16,
  POWERPC_R17,
  POWERPC_R18,
  POWERPC_R19,
  POWERPC_R20,
  POWERPC_R21,
  POWERPC_R22,
  POWERPC_R23,
  POWERPC_R24,
  POWERPC_R25,
  POWERPC_R26,
  POWERPC_R27,
  POWERPC_R28,
  POWERPC_R29,
  POWERPC_R30,
  POWERPC_R31,
  POWERPC_V0 = ORC_VEC_REG_BASE,
  POWERPC_V1,
  POWERPC_V2,
  POWERPC_V3,
  POWERPC_V4,
  POWERPC_V5,
  POWERPC_V6,
  POWERPC_V7,
  POWERPC_V8,
  POWERPC_V9,
  POWERPC_V10,
  POWERPC_V11,
  POWERPC_V12,
  POWERPC_V13,
  POWERPC_V14,
  POWERPC_V15,
  POWERPC_V16,
  POWERPC_V17,
  POWERPC_V18,
  POWERPC_V19,
  POWERPC_V20,
  POWERPC_V21,
  POWERPC_V22,
  POWERPC_V23,
  POWERPC_V24,
  POWERPC_V25,
  POWERPC_V26,
  POWERPC_V27,
  POWERPC_V28,
  POWERPC_V29,
  POWERPC_V30,
  POWERPC_V31
};

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
powerpc_emit_prologue (OrcCompiler *compiler)
{
  int i;

  ORC_ASM_CODE (compiler, ".global %s\n", compiler->program->name);
  ORC_ASM_CODE (compiler, "%s:\n", compiler->program->name);

  powerpc_emit_stwu (compiler, POWERPC_R1, POWERPC_R1, -16);

  for(i=POWERPC_R13;i<=POWERPC_R31;i++){
    if (compiler->used_regs[i]) {
      //powerpc_emit_push (compiler, 4, i);
    }
  }
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
powerpc_emit_epilogue (OrcCompiler *compiler)
{
  int i;

  for(i=POWERPC_R31;i>=POWERPC_R31;i--){
    if (compiler->used_regs[i]) {
      //powerpc_emit_pop (compiler, 4, i);
    }
  }

  powerpc_emit_addi (compiler, POWERPC_R1, POWERPC_R1, 16);
  ORC_ASM_CODE(compiler,"  blr\n");
  powerpc_emit(compiler, 0x4e800020);
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

static OrcTarget powerpc_target = {
  "powerpc",
#ifdef HAVE_POWERPC
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_powerpc_init,
  orc_compiler_powerpc_assemble
};

void
orc_powerpc_init (void)
{
  orc_target_register (&powerpc_target);

  orc_compiler_powerpc_register_rules (&powerpc_target);
}

void
orc_compiler_powerpc_init (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<32;i++){
    compiler->valid_regs[POWERPC_R0+i] = 1;
    compiler->valid_regs[POWERPC_V0+i] = 1;
  }
  compiler->valid_regs[POWERPC_R0] = 0; /* used for temp space */
  compiler->valid_regs[POWERPC_R1] = 0; /* stack pointer */
  compiler->valid_regs[POWERPC_R2] = 0; /* TOC pointer */
  compiler->valid_regs[POWERPC_R3] = 0; /* pointer to OrcExecutor */
  compiler->valid_regs[POWERPC_R13] = 0; /* reserved */

  compiler->tmpreg = POWERPC_V0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  for(i=14;i<32;i++){
    compiler->save_regs[POWERPC_R0 + i] = 1;
  }
  for(i=20;i<32;i++){
    compiler->save_regs[POWERPC_V0 + i] = 1;
  }

  compiler->loop_shift = 0;
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
        ORC_PROGRAM_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_W:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltish %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000024c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_PROGRAM_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_L:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltisw %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000028c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_PROGRAM_ERROR(p,"can't load constant");
      }
      break;
    default:
      ORC_PROGRAM_ERROR(p,"unhandled");
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

static int
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

void
powerpc_load_constants (OrcCompiler *compiler)
{
  OrcVariable *var;
  int i;
  int j;
  int greg = POWERPC_R0;

  for(i=0;i<ORC_N_VARIABLES;i++){
    var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        j = compiler->n_constants;
        compiler->n_constants++;
        if (compiler->vars[i].size == 1) {
          compiler->constants[j].type = ORC_CONST_SPLAT_B;
        } else if (compiler->vars[i].size == 2) {
          compiler->constants[j].type = ORC_CONST_SPLAT_W;
        } else {
          compiler->constants[j].type = ORC_CONST_SPLAT_L;
        }
        compiler->constants[j].value = compiler->vars[i].value;
        compiler->constants[j].alloc_reg = compiler->vars[i].alloc;
        break;
      case ORC_VAR_TYPE_PARAM:
        powerpc_emit_addi (compiler,
            greg, POWERPC_R3,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]));
        ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n", 
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (greg));
        powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(var->alloc),
            0, powerpc_regnum(greg));

        ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n", 
            powerpc_get_regname (POWERPC_V0),
            powerpc_get_regname (greg));
        powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
            0, powerpc_regnum(greg));

        ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (POWERPC_V0));
        powerpc_emit_VA (compiler, 4,
            powerpc_regnum(var->alloc),
            powerpc_regnum(var->alloc),
            powerpc_regnum(var->alloc),
            powerpc_regnum(POWERPC_V0), 43);
        switch (var->size) {
          case 1:
            ORC_ASM_CODE(compiler,"  vspltb %s, %s, 3\n",
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000020c,
                powerpc_regnum(var->alloc), 3, powerpc_regnum(var->alloc));
            break;
          case 2:
            ORC_ASM_CODE(compiler,"  vsplth %s, %s, 1\n", 
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000024c,
                powerpc_regnum(var->alloc), 1, powerpc_regnum(var->alloc));
            break;
          case 4:
            ORC_ASM_CODE(compiler,"  vspltw %s, %s, 0\n", 
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000028c,
                powerpc_regnum(var->alloc), 0, powerpc_regnum(var->alloc));
            break;
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          powerpc_emit_lwz (compiler,
              compiler->vars[i].ptr_register,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        } else {
          /* FIXME */
          ORC_ASM_CODE(compiler,"ERROR");
        }
        break;
      default:
        break;
    }
  }

  for(i=0;i<compiler->n_constants;i++){
    if (compiler->constants[i].alloc_reg > 0) {
      powerpc_load_constant (compiler, i, compiler->constants[i].alloc_reg);
    }
  }
}

void
powerpc_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (compiler->loop_shift) {
    case 0:
      switch (var->size) {
        case 1:
          ORC_ASM_CODE(compiler,"  lvebx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00000e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 2:
          ORC_ASM_CODE(compiler,"  lvehx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00004e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 4:
          ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
      }
      ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg));
      ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (compiler, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      break;
    default:
      ORC_ASM_CODE(compiler,"ERROR\n");
  }
}

void
powerpc_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (compiler->loop_shift) {
    case 0:
      ORC_ASM_CODE(compiler,"  lvsr %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 0x7c00004c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg));
      ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (compiler, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      switch (var->size) {
        case 1:
          ORC_ASM_CODE(compiler,"  stvebx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00010e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 2:
          ORC_ASM_CODE(compiler,"  stvehx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00014e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 4:
          ORC_ASM_CODE(compiler,"  stvewx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00018e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
      }
      break;
    default:
      ORC_ASM_CODE(compiler,"ERROR\n");
  }
}

void
orc_compiler_powerpc_assemble (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  //OrcVariable *args[10];
  OrcRule *rule;
  int label_loop_start;
  int label_leave;

  label_loop_start = orc_compiler_label_new (compiler);
  label_leave = orc_compiler_label_new (compiler);

  powerpc_emit_prologue (compiler);

  powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
  powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
      compiler->loop_shift, 1);

  powerpc_emit_beq (compiler, label_leave);

  powerpc_emit (compiler, 0x7c0903a6);
  ORC_ASM_CODE (compiler, "  mtctr %s\n", powerpc_get_regname(POWERPC_R0));

  powerpc_load_constants (compiler);

  powerpc_emit_label (compiler, label_loop_start);

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(compiler,"# %d: %s\n", j, insn->opcode->name);

#if 0
    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = compiler->vars + insn->args[k];
      ORC_ASM_CODE(compiler," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        ORC_ASM_CODE(compiler," (chained)");
      }
    }
    ORC_ASM_CODE(compiler,"\n");
#endif

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      OrcVariable *var = compiler->vars + insn->src_args[k];

      if (opcode->src_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
        case ORC_VAR_TYPE_DEST:
          powerpc_emit_load_src (compiler, var);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }

    rule = insn->rule;
    if (rule) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_ASM_CODE(compiler,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      OrcVariable *var = compiler->vars + insn->dest_args[k];

      if (opcode->dest_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_DEST:
          powerpc_emit_store_dest (compiler, var);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (compiler->vars[k].name == NULL) continue;
    if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
        compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (compiler->vars[k].ptr_register) {
        powerpc_emit_addi (compiler,
            compiler->vars[k].ptr_register,
            compiler->vars[k].ptr_register,
            compiler->vars[k].size << compiler->loop_shift);
      } else {
        ORC_ASM_CODE(compiler,"ERROR\n");
      }
    }
  }

  powerpc_emit_bne (compiler, label_loop_start);
  powerpc_emit_label (compiler, label_leave);

  powerpc_emit_epilogue (compiler);

  powerpc_do_fixups (compiler);

  powerpc_flush (compiler);
}


/* rules */

#define RULE(name, opcode, code) \
static void \
powerpc_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  ORC_ASM_CODE(p,"  " opcode " %s, %s, %s\n", \
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc)); \
  powerpc_emit_VX(p, code , \
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[1]].alloc)); \
}

#define RULE_SHIFT(name, opcode, code) \
static void \
powerpc_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST && \
      p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_PARAM) { \
    ORC_PROGRAM_ERROR(p,"rule only works with constants or params"); \
  } \
  ORC_ASM_CODE(p,"  " opcode " %s, %s, %s\n", \
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc)); \
  powerpc_emit_VX(p, code , \
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[1]].alloc)); \
}

RULE(addb, "vaddubm", 0x10000000)
RULE(addssb, "vaddsbs", 0x10000300)
RULE(addusb, "vaddubs", 0x10000200)
RULE(andb, "vand", 0x10000404)
//RULE(andnb, "vandc", 0x10000444)
RULE(avgsb, "vavgsb", 0x10000502)
RULE(avgub, "vavgub", 0x10000402)
RULE(cmpeqb, "vcmpequb", 0x10000006)
RULE(cmpgtsb, "vcmpgtsb", 0x10000306)
RULE(maxsb, "vmaxsb", 0x10000102)
RULE(maxub, "vmaxub", 0x10000002)
RULE(minsb, "vminsb", 0x10000302)
RULE(minub, "vminub", 0x10000202)
RULE(orb, "vor", 0x10000484)
RULE_SHIFT(shlb, "vslb", 0x10000104)
RULE_SHIFT(shrsb, "vsrab", 0x10000304)
RULE_SHIFT(shrub, "vsrb", 0x10000204)
RULE(subb, "vsububm", 0x10000400)
RULE(subssb, "vsubsbs", 0x10000700)
RULE(subusb, "vsububs", 0x10000600)
RULE(xorb, "vxor", 0x100004c4)

RULE(addw, "vadduhm", 0x10000040)
RULE(addssw, "vaddshs", 0x10000340)
RULE(addusw, "vadduhs", 0x10000240)
RULE(andw, "vand", 0x10000404)
//RULE(andnw, "vandc", 0x10000444)
RULE(avgsw, "vavgsh", 0x10000542)
RULE(avguw, "vavguh", 0x10000442)
RULE(cmpeqw, "vcmpequh", 0x10000046)
RULE(cmpgtsw, "vcmpgtsh", 0x10000346)
RULE(maxsw, "vmaxsh", 0x10000142)
RULE(maxuw, "vmaxuh", 0x10000042)
RULE(minsw, "vminsh", 0x10000342)
RULE(minuw, "vminuh", 0x10000242)
RULE(orw, "vor", 0x10000484)
RULE_SHIFT(shlw, "vslh", 0x10000144)
RULE_SHIFT(shrsw, "vsrah", 0x10000344)
RULE_SHIFT(shruw, "vsrh", 0x10000244)
RULE(subw, "vsubuhm", 0x10000440)
RULE(subssw, "vsubshs", 0x10000740)
RULE(subusw, "vsubuhs", 0x10000640)
RULE(xorw, "vxor", 0x100004c4)

RULE(addl, "vadduwm", 0x10000080)
RULE(addssl, "vaddsws", 0x10000380)
RULE(addusl, "vadduws", 0x10000280)
RULE(andl, "vand", 0x10000404)
//RULE(andnl, "vandc", 0x10000444)
RULE(avgsl, "vavgsw", 0x10000582)
RULE(avgul, "vavguw", 0x10000482)
RULE(cmpeql, "vcmpequw", 0x10000086)
RULE(cmpgtsl, "vcmpgtsw", 0x10000386)
RULE(maxsl, "vmaxsw", 0x10000182)
RULE(maxul, "vmaxuw", 0x10000082)
RULE(minsl, "vminsw", 0x10000382)
RULE(minul, "vminuw", 0x10000282)
RULE(orl, "vor", 0x10000484)
RULE_SHIFT(shll, "vslw", 0x10000184)
RULE_SHIFT(shrsl, "vsraw", 0x10000384)
RULE_SHIFT(shrul, "vsrw", 0x10000284)
RULE(subl, "vsubuwm", 0x10000480)
RULE(subssl, "vsubsws", 0x10000780)
RULE(subusl, "vsubuws", 0x10000680)
RULE(xorl, "vxor", 0x100004c4)

static void
powerpc_rule_andnX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vandc %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000444,
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[1]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_copyX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vor %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000484,
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 1\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000002c | (1<<6),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc));
}

static void
powerpc_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleub %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000208,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 2\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000002c | (2<<6),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc));
}

static void
powerpc_rule_mulhsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulhuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleuh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000248,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}


#ifdef alternate
static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vxor %s, %s, %s\n",
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VX(p, 0x100004c4,
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0));

  ORC_ASM_CODE(p,"  vmladduhm %s, %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VA(p, 4, 
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc),
      powerpc_regnum(POWERPC_V0), 34);

}
#endif

static void
powerpc_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vupkhsb %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000020e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      0,
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vupkhsh %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000024e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      0,
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);

  ORC_ASM_CODE(p,"  vmrghb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(reg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000000c,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(reg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);

  ORC_ASM_CODE(p,"  vmrghh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(reg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000004c,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(reg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkshss %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000018e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkswss %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x100001ce,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkshus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000010e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkswus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000014e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuhus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000008e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuwus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x100000ce,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuhum %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000000e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuwum %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000004e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleub %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000208,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleuh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000248,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vadduhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000040,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vadduwm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000080,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmpreg2 = POWERPC_V31;

  ORC_ASM_CODE(p,"  vmaxub %s, %s, %s\n",
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000002,
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vminub %s, %s, %s\n",
      powerpc_get_regname(tmpreg2),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000202,
      powerpc_regnum(tmpreg2),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsububm %s, %s, %s\n",
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(tmpreg2));
  powerpc_emit_VX(p, 0x10000400,
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(tmpreg2));

  ORC_ASM_CODE(p,"  vsum4ubs %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->tmpreg));
  powerpc_emit_VX(p, 0x10000608,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->tmpreg));
}

static void
powerpc_rule_signb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, 1);
  powerpc_emit_VX_2(p, "vminsb", 0x10000302,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, -1);
  powerpc_emit_VX_2(p, "vmaxsb", 0x10000102,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
}

static void
powerpc_rule_signw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, 1);
  powerpc_emit_VX_2(p, "vminsh", 0x10000342,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, -1);
  powerpc_emit_VX_2(p, "vmaxsh", 0x10000142,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
}

static void
powerpc_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, 1);
  powerpc_emit_VX_2(p, "vminsw", 0x10000382,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, -1);
  powerpc_emit_VX_2(p, "vmaxsw", 0x10000182,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
}

void
orc_compiler_powerpc_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#define REG(name) \
  orc_rule_register (rule_set, #name , powerpc_rule_ ## name , NULL);

  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  REG(orb);
  REG(shlb);
  REG(shrsb);
  REG(shrub);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  REG(avgsw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(maxsw);
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(orw);
  REG(shlw);
  REG(shrsw);
  REG(shruw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  REG(avgsl);
  REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(orl);
  REG(shll);
  REG(shrsl);
  REG(shrul);
  REG(subl);
  REG(subssl);
  REG(subusl);
  REG(xorl);

  REG(mullb);
  REG(mulhsb);
  REG(mulhub);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);

  REG(convsbw);
  REG(convswl);
  REG(convubw);
  REG(convuwl);
  REG(convssswb);
  REG(convssslw);
  REG(convsuswb);
  REG(convsuslw);
  REG(convuuswb);
  REG(convuuslw);
  REG(convwb);
  REG(convlw);

  REG(mulsbw);
  REG(mulubw);
  REG(mulswl);
  REG(muluwl);

  REG(accw);
  REG(accl);
  REG(accsadubl);

  REG(signb);
  REG(signw);
  REG(signl);

  orc_rule_register (rule_set, "andnb", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnw", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnl", powerpc_rule_andnX, NULL);

  orc_rule_register (rule_set, "copyb", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyw", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyl", powerpc_rule_copyX, NULL);
}

/* code generation */

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

