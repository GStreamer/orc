
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>

#define SIZE 65536

void powerpc_emit_addi (OrcProgram *program, int regd, int rega, int imm);
void powerpc_emit_lwz (OrcProgram *program, int regd, int rega, int imm);
void powerpc_emit_stwu (OrcProgram *program, int regs, int rega, int offset);

void powerpc_emit_ret (OrcProgram *program);
void powerpc_emit_beq (OrcProgram *program, int label);
void powerpc_emit_bne (OrcProgram *program, int label);
void powerpc_emit_label (OrcProgram *program, int label);

void orc_program_powerpc_register_rules (void);

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
powerpc_emit(OrcProgram *program, unsigned int insn)
{
  *program->codeptr++ = (insn>>24);
  *program->codeptr++ = (insn>>16);
  *program->codeptr++ = (insn>>8);
  *program->codeptr++ = (insn>>0);
}

void
powerpc_emit_prologue (OrcProgram *program)
{
  int i;

  printf (".global test\n");
  printf ("test:\n");

  powerpc_emit_stwu (program, POWERPC_R1, POWERPC_R1, -16);

  for(i=POWERPC_R13;i<=POWERPC_R31;i++){
    if (program->used_regs[i]) {
      //powerpc_emit_push (program, 4, i);
    }
  }
}

void
powerpc_emit_addi (OrcProgram *program, int regd, int rega, int imm)
{
  unsigned int insn;

  printf("  addi %s, %s, %d\n",
      powerpc_get_regname(regd),
      powerpc_get_regname(rega), imm);
  insn = (14<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= imm&0xffff;

  powerpc_emit (program, insn);
}

void
powerpc_emit_lwz (OrcProgram *program, int regd, int rega, int imm)
{
  unsigned int insn;

  printf("  lwz %s, %d(%s)\n",
      powerpc_get_regname(regd),
      imm, powerpc_get_regname(rega));
  insn = (32<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= imm&0xffff;

  powerpc_emit (program, insn);
}

void
powerpc_emit_stwu (OrcProgram *program, int regs, int rega, int offset)
{
  unsigned int insn;

  printf("  stwu %s, %d(%s)\n",
      powerpc_get_regname(regs),
      offset, powerpc_get_regname(rega));
  insn = (37<<26) | (powerpc_regnum (regs)<<21) | (powerpc_regnum (rega)<<16);
  insn |= offset&0xffff;

  powerpc_emit (program, insn);
}

void
powerpc_emit_srawi (OrcProgram *program, int regd, int rega, int shift,
    int record)
{
  unsigned int insn;

  printf("  srawi%s %s, %s, %d\n", (record)?".":"",
      powerpc_get_regname(regd),
      powerpc_get_regname(rega), shift);

  insn = (31<<26) | (powerpc_regnum (regd)<<21) | (powerpc_regnum (rega)<<16);
  insn |= (shift<<11) | (824<<1) | record;

  powerpc_emit (program, insn);
}

void
powerpc_emit_655510 (OrcProgram *program, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<0);

  powerpc_emit (program, insn);
}

void
powerpc_emit_X (OrcProgram *program, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<1) | (0<<0);

  powerpc_emit (program, insn);
}

void
powerpc_emit_VA (OrcProgram *program, int major, int d, int a, int b,
    int c, int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (c<<6) | (minor<<0);

  powerpc_emit (program, insn);
}

void
powerpc_emit_VX (OrcProgram *program, int major, int d, int a, int b,
    int minor)
{
  unsigned int insn;

  insn = (major<<26) | (d<<21) | (a<<16);
  insn |= (b<<11) | (minor<<0);

  powerpc_emit (program, insn);
}


void
powerpc_emit_epilogue (OrcProgram *program)
{
  int i;

  for(i=POWERPC_R31;i>=POWERPC_R31;i--){
    if (program->used_regs[i]) {
      //powerpc_emit_pop (program, 4, i);
    }
  }

  powerpc_emit_addi (program, POWERPC_R1, POWERPC_R1, 16);
  printf("  blr\n");
  powerpc_emit(program, 0x4e800020);
}

void
powerpc_do_fixups (OrcProgram *program)
{
  int i;
  unsigned int insn;

  for(i=0;i<program->n_fixups;i++){
    if (program->fixups[i].type == 0) {
      unsigned char *label = program->labels[program->fixups[i].label];
      unsigned char *ptr = program->fixups[i].ptr;

      insn = *(unsigned int *)ptr;
      *(unsigned int *)ptr = (insn&0xffff0000) | ((insn + (label-ptr))&0xffff);
    }
  }
}

void
powerpc_flush (OrcProgram *program)
{
#ifdef HAVE_POWERPC
  unsigned char *ptr;
  int cache_line_size = 32;
  int i;
  int size = program->codeptr - program->code;

  ptr = program->code;
  for (i=0;i<size;i+=cache_line_size) {
    __asm__ __volatile__ ("dcbst %0,%1" :: "r" (ptr), "r" (i));
  }
  __asm__ __volatile ("sync");

  ptr = program->code_exec;
  for (i=0;i<size;i+=cache_line_size) {
    __asm__ __volatile__ ("icbi %0,%1" :: "r" (ptr), "r" (i));
  }
  __asm__ __volatile ("isync");
#endif
}

void
orc_powerpc_init (void)
{
  orc_program_powerpc_register_rules ();
}

void
orc_program_powerpc_init (OrcProgram *program)
{
  int i;

  for(i=0;i<32;i++){
    program->valid_regs[POWERPC_R0+i] = 1;
    program->valid_regs[POWERPC_V0+i] = 1;
  }
  program->valid_regs[POWERPC_R0] = 0; /* used for temp space */
  program->valid_regs[POWERPC_R1] = 0; /* stack pointer */
  program->valid_regs[POWERPC_R2] = 0; /* TOC pointer */
  program->valid_regs[POWERPC_R3] = 0; /* pointer to OrcExecutor */
  program->valid_regs[POWERPC_R13] = 0; /* reserved */
  program->valid_regs[POWERPC_V0] = 0; /* used for temp space */

  for(i=14;i<32;i++){
    program->save_regs[POWERPC_R0 + i] = 1;
  }
  for(i=20;i<32;i++){
    program->save_regs[POWERPC_V0 + i] = 1;
  }

  program->loop_shift = 2;
}

void
powerpc_load_constants (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_vars;i++){
    switch (program->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        printf("  vspltish %s, %d\n",
            powerpc_get_regname(program->vars[i].alloc),
            program->vars[i].s16);
        powerpc_emit_655510 (program, 4,
            powerpc_regnum(program->vars[i].alloc),
            program->vars[i].s16, 0, 844);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (program->vars[i].ptr_register) {
          powerpc_emit_lwz (program,
              program->vars[i].ptr_register,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        } else {
          /* FIXME */
          printf("ERROR");
        }
        break;
      default:
        break;
    }
  }
}

void
powerpc_emit_load_src (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (program->loop_shift) {
    case 0:
      printf("  lvehx %s, 0, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (program, 31, powerpc_regnum(var->alloc),
          0, powerpc_regnum(ptr_reg), 39);
      printf("  lvsl %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (program, 31, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg), 6);
      printf("  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (program, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      break;
    default:
      printf("ERROR\n");
  }
}

void
powerpc_emit_store_dest (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (program->loop_shift) {
    case 0:
      printf("  lvsr %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (program, 31, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg), 38);
      printf("  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (program, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      printf("  stvehx %s, 0, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (program, 31,
          powerpc_regnum(var->alloc),
          0, powerpc_regnum(ptr_reg), 167);
      break;
    default:
      printf("ERROR\n");
  }
}

void
orc_program_assemble_powerpc (OrcProgram *program)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  OrcVariable *args[10];
  OrcRule *rule;

  powerpc_emit_prologue (program);

  powerpc_emit_lwz (program, POWERPC_R0, POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
  powerpc_emit_srawi (program, POWERPC_R0, POWERPC_R0,
      program->loop_shift, 1);

  powerpc_emit_beq (program, 1);

  powerpc_emit (program, 0x7c0903a6);
  printf ("  mtctr %s\n", powerpc_get_regname(POWERPC_R0));

  powerpc_load_constants (program);

  powerpc_emit_label (program, 0);

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    printf("# %d: %s", j, insn->opcode->name);

    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = program->vars + insn->args[k];
      printf(" %d", args[k]->alloc);
      if (args[k]->is_chained) {
        printf(" (chained)");
      }
    }
    printf(" rule_flag=%d", insn->rule_flag);
    printf("\n");

    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_SRC:
          powerpc_emit_load_src (program, args[k]);
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
      rule->emit (program, rule->emit_user, insn);
    } else {
      printf("No rule for: %s\n", opcode->name);
    }

    for(k=0;k<opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_DEST:
          powerpc_emit_store_dest (program, args[k]);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  for(k=0;k<program->n_vars;k++){
    if (program->vars[k].vartype == ORC_VAR_TYPE_SRC ||
        program->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (program->vars[k].ptr_register) {
        powerpc_emit_addi (program,
            program->vars[k].ptr_register,
            program->vars[k].ptr_register,
            orc_variable_get_size(program->vars + k) << program->loop_shift);
      } else {
        printf("ERROR\n");
      }
    }
  }

  powerpc_emit_bne (program, 0);
  powerpc_emit_label (program, 1);

  powerpc_emit_epilogue (program);

  powerpc_do_fixups (program);

  powerpc_flush (program);
}


/* rules */

static void
powerpc_rule_addw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  unsigned int x;

  printf("  vadduhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->args[0]].alloc),
      powerpc_get_regname(p->vars[insn->args[1]].alloc),
      powerpc_get_regname(p->vars[insn->args[2]].alloc));

  x = (4<<26);
  x |= (powerpc_regnum (p->vars[insn->args[0]].alloc)<<21);
  x |= (powerpc_regnum (p->vars[insn->args[1]].alloc)<<16);
  x |= (powerpc_regnum (p->vars[insn->args[2]].alloc)<<11);
  x |= 64;

  powerpc_emit (p, x);
}

static void
powerpc_rule_subw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  vsubuhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->args[0]].alloc),
      powerpc_get_regname(p->vars[insn->args[1]].alloc),
      powerpc_get_regname(p->vars[insn->args[2]].alloc));
}

static void
powerpc_rule_mullw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  vxor %s, %s, %s\n",
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VX(p, 4,
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0), 1220);

  printf("  vmladduhm %s, %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->args[0]].alloc),
      powerpc_get_regname(p->vars[insn->args[1]].alloc),
      powerpc_get_regname(p->vars[insn->args[2]].alloc),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VA(p, 4, 
      powerpc_regnum(p->vars[insn->args[0]].alloc),
      powerpc_regnum(p->vars[insn->args[1]].alloc),
      powerpc_regnum(p->vars[insn->args[2]].alloc),
      powerpc_regnum(POWERPC_V0), 34);

}

static void
powerpc_rule_shlw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  vrlh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->args[0]].alloc),
      powerpc_get_regname(p->vars[insn->args[1]].alloc),
      powerpc_get_regname(p->vars[insn->args[2]].alloc));
  powerpc_emit_VX(p, 4,
      powerpc_regnum(p->vars[insn->args[0]].alloc),
      powerpc_regnum(p->vars[insn->args[1]].alloc),
      powerpc_regnum(p->vars[insn->args[2]].alloc), 68);
}

static void
powerpc_rule_shrsw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  unsigned int x;

  printf("  vsrah %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->args[0]].alloc),
      powerpc_get_regname(p->vars[insn->args[1]].alloc),
      powerpc_get_regname(p->vars[insn->args[2]].alloc));

  x = (4<<26);
  x |= (powerpc_regnum (p->vars[insn->args[0]].alloc)<<21);
  x |= (powerpc_regnum (p->vars[insn->args[1]].alloc)<<16);
  x |= (powerpc_regnum (p->vars[insn->args[2]].alloc)<<11);
  x |= 836;

  powerpc_emit (p, x);
}


void
orc_program_powerpc_register_rules (void)
{
  orc_rule_register ("addw", ORC_TARGET_ALTIVEC, powerpc_rule_addw, NULL);
  orc_rule_register ("subw", ORC_TARGET_ALTIVEC, powerpc_rule_subw, NULL);
  orc_rule_register ("mullw", ORC_TARGET_ALTIVEC, powerpc_rule_mullw, NULL);
  orc_rule_register ("shlw", ORC_TARGET_ALTIVEC, powerpc_rule_shlw, NULL);
  orc_rule_register ("shrsw", ORC_TARGET_ALTIVEC, powerpc_rule_shrsw, NULL);
}

/* code generation */

void powerpc_emit_ret (OrcProgram *program)
{
  printf("  ret\n");
  //*program->codeptr++ = 0xc3;
}

void
powerpc_add_fixup (OrcProgram *program, unsigned char *ptr, int label)
{
  program->fixups[program->n_fixups].ptr = ptr;
  program->fixups[program->n_fixups].label = label;
  program->fixups[program->n_fixups].type = 0;
  program->n_fixups++;
}

void
powerpc_add_label (OrcProgram *program, unsigned char *ptr, int label)
{
  program->labels[label] = ptr;
}

void powerpc_emit_beq (OrcProgram *program, int label)
{
  printf("  ble- .L%d\n", label);

  powerpc_add_fixup (program, program->codeptr, label);
  powerpc_emit (program, 0x40810000);
}

void powerpc_emit_bne (OrcProgram *program, int label)
{
  printf("  bdnz+ .L%d\n", label);

  powerpc_add_fixup (program, program->codeptr, label);
  powerpc_emit (program, 0x42000000);
}

void powerpc_emit_label (OrcProgram *program, int label)
{
  printf(".L%d:\n", label);

  powerpc_add_label (program, program->codeptr, label);
}

