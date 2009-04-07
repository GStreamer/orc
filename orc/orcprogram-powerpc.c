
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>

#define SIZE 65536

void powerpc_emit_addi (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_lwz (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_stwu (OrcCompiler *compiler, int regs, int rega, int offset);

void powerpc_emit_ret (OrcCompiler *compiler);
void powerpc_emit_beq (OrcCompiler *compiler, int label);
void powerpc_emit_bne (OrcCompiler *compiler, int label);
void powerpc_emit_label (OrcCompiler *compiler, int label);

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

  ORC_ASM_CODE (compiler, ".global test\n");
  ORC_ASM_CODE (compiler, "test:\n");

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
powerpc_emit_X (OrcCompiler *compiler, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<1) | (0<<0);

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
powerpc_emit_VX (OrcCompiler *compiler, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<0);

  powerpc_emit (compiler, insn);
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
  int size = compiler->codeptr - compiler->code;

  ptr = compiler->code;
  for (i=0;i<size;i+=cache_line_size) {
    __asm__ __volatile__ ("dcbst %0,%1" :: "r" (ptr), "r" (i));
  }
  __asm__ __volatile ("sync");

  ptr = compiler->code_exec;
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
  compiler->valid_regs[POWERPC_V0] = 0; /* used for temp space */

  for(i=14;i<32;i++){
    compiler->save_regs[POWERPC_R0 + i] = 1;
  }
  for(i=20;i<32;i++){
    compiler->save_regs[POWERPC_V0 + i] = 1;
  }

  compiler->loop_shift = 2;
}

void
powerpc_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        ORC_ASM_CODE(compiler,"  vspltish %s, %d\n",
            powerpc_get_regname(compiler->vars[i].alloc),
            (int)compiler->vars[i].value);
        powerpc_emit_655510 (compiler, 4,
            powerpc_regnum(compiler->vars[i].alloc),
            (int)compiler->vars[i].value, 0, 844);
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
}

void
powerpc_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (compiler->loop_shift) {
    case 0:
      ORC_ASM_CODE(compiler,"  lvehx %s, 0, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 31, powerpc_regnum(var->alloc),
          0, powerpc_regnum(ptr_reg), 39);
      ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 31, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg), 6);
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
      powerpc_emit_X (compiler, 31, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg), 38);
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
      ORC_ASM_CODE(compiler,"  stvehx %s, 0, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 31,
          powerpc_regnum(var->alloc),
          0, powerpc_regnum(ptr_reg), 167);
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

  powerpc_emit_prologue (compiler);

  powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
  powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
      compiler->loop_shift, 1);

  powerpc_emit_beq (compiler, 1);

  powerpc_emit (compiler, 0x7c0903a6);
  ORC_ASM_CODE (compiler, "  mtctr %s\n", powerpc_get_regname(POWERPC_R0));

  powerpc_load_constants (compiler);

  powerpc_emit_label (compiler, 0);

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(compiler,"# %d: %s", j, insn->opcode->name);

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

  powerpc_emit_bne (compiler, 0);
  powerpc_emit_label (compiler, 1);

  powerpc_emit_epilogue (compiler);

  powerpc_do_fixups (compiler);

  powerpc_flush (compiler);
}


/* rules */

static void
powerpc_rule_addw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int x;

  ORC_ASM_CODE(p,"  vadduhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));

  x = (4<<26);
  x |= (powerpc_regnum (p->vars[insn->dest_args[0]].alloc)<<21);
  x |= (powerpc_regnum (p->vars[insn->src_args[0]].alloc)<<16);
  x |= (powerpc_regnum (p->vars[insn->src_args[1]].alloc)<<11);
  x |= 64;

  powerpc_emit (p, x);
}

static void
powerpc_rule_subw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vsubuhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vxor %s, %s, %s\n",
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VX(p, 4,
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0), 1220);

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

static void
powerpc_rule_shlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vrlh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 4,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc), 68);
}

static void
powerpc_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int x;

  ORC_ASM_CODE(p,"  vsrah %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));

  x = (4<<26);
  x |= (powerpc_regnum (p->vars[insn->dest_args[0]].alloc)<<21);
  x |= (powerpc_regnum (p->vars[insn->src_args[0]].alloc)<<16);
  x |= (powerpc_regnum (p->vars[insn->src_args[1]].alloc)<<11);
  x |= 836;

  powerpc_emit (p, x);
}


void
orc_compiler_powerpc_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target);

  orc_rule_register (rule_set, "addw", powerpc_rule_addw, NULL);
  orc_rule_register (rule_set, "subw", powerpc_rule_subw, NULL);
  orc_rule_register (rule_set, "mullw", powerpc_rule_mullw, NULL);
  orc_rule_register (rule_set, "shlw",  powerpc_rule_shlw, NULL);
  orc_rule_register (rule_set, "shrsw", powerpc_rule_shrsw, NULL);
}

/* code generation */

void powerpc_emit_ret (OrcCompiler *compiler)
{
  ORC_ASM_CODE(compiler,"  ret\n");
  //*compiler->codeptr++ = 0xc3;
}

void
powerpc_add_fixup (OrcCompiler *compiler, unsigned char *ptr, int label)
{
  compiler->fixups[compiler->n_fixups].ptr = ptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = 0;
  compiler->n_fixups++;
}

void
powerpc_add_label (OrcCompiler *compiler, unsigned char *ptr, int label)
{
  compiler->labels[label] = ptr;
}

void powerpc_emit_beq (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"  ble- .L%d\n", label);

  powerpc_add_fixup (compiler, compiler->codeptr, label);
  powerpc_emit (compiler, 0x40810000);
}

void powerpc_emit_bne (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,"  bdnz+ .L%d\n", label);

  powerpc_add_fixup (compiler, compiler->codeptr, label);
  powerpc_emit (compiler, 0x42000000);
}

void powerpc_emit_label (OrcCompiler *compiler, int label)
{
  ORC_ASM_CODE(compiler,".L%d:\n", label);

  powerpc_add_label (compiler, compiler->codeptr, label);
}

