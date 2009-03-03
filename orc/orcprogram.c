
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <orc/orcprogram.h>

void orc_program_assign_rules (OrcProgram *program);
void orc_program_global_reg_alloc (OrcProgram *program);
void orc_program_rewrite_vars (OrcProgram *program);
void orc_program_rewrite_vars2 (OrcProgram *program);
void orc_program_do_regs (OrcProgram *program);

int _orc_default_target = ORC_TARGET_SSE;

OrcProgram *
orc_program_new (void)
{
  OrcProgram *p;

  p = malloc(sizeof(OrcProgram));
  memset (p, 0, sizeof(OrcProgram));

  p->target = _orc_default_target;

  return p;
}

OrcProgram *
orc_program_new_dss (int size1, int size2, int size3)
{
  OrcProgram *p;

  p = orc_program_new ();

  orc_program_add_destination (p, size1, "d1");
  orc_program_add_source (p, size2, "s1");
  orc_program_add_source (p, size3, "s2");

  return p;
}

void
orc_program_free (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_vars;i++){
    free (program->vars[i].name);
  }
  free (program);
}

int
orc_program_add_temporary (OrcProgram *program, int size, const char *name)
{
  int i = program->n_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_vars++;

  return i;
}

int
orc_program_dup_temporary (OrcProgram *program, int var, int j)
{
  int i = program->n_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  program->vars[i].size = program->vars[var].size;
  program->vars[i].name = malloc (strlen(program->vars[var].name) + 10);
  sprintf(program->vars[i].name, "%s.dup%d", program->vars[var].name, j);
  program->n_vars++;

  return i;
}

int
orc_program_add_source (OrcProgram *program, int size, const char *name)
{
  int i = program->n_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_SRC;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_vars++;

  return i;
}

int
orc_program_add_destination (OrcProgram *program, int size, const char *name)
{
  int i = program->n_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_DEST;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_vars++;

  return i;
}

int
orc_program_add_constant (OrcProgram *program, int size, int value, const char *name)
{
  int i = program->n_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_CONST;
  program->vars[i].size = size;
  program->vars[i].value = value;
  program->vars[i].name = strdup(name);
  program->n_vars++;

  return i;
}

int
orc_program_add_parameter (OrcProgram *program, int size, int value,
    const char *name)
{

  return 0;
}

void
orc_program_append (OrcProgram *program, const char *name, int arg0,
    int arg1, int arg2)
{
  OrcInstruction *insn;

  insn = program->insns + program->n_insns;

  insn->opcode = orc_opcode_find_by_name (name);
  if (!insn->opcode) {
    printf("unknown opcode: %s\n", name);
  }
  insn->args[0] = arg0;
  insn->args[1] = arg1;
  insn->args[2] = arg2;
  
  program->n_insns++;
}

int
orc_program_find_var_by_name (OrcProgram *program, const char *name)
{
  int i;

  for(i=0;i<program->n_vars;i++){
    if (strcmp (program->vars[i].name, name) == 0) {
      return i;
    }
  }

  return -1;
}

void
orc_program_append_str (OrcProgram *program, const char *name,
    const char *arg1, const char *arg2, const char *arg3)
{
  OrcInstruction *insn;

  insn = program->insns + program->n_insns;

  insn->opcode = orc_opcode_find_by_name (name);
  if (!insn->opcode) {
    printf("unknown opcode: %s\n", name);
  }
  insn->args[0] = orc_program_find_var_by_name (program, arg1);
  insn->args[1] = orc_program_find_var_by_name (program, arg2);
  insn->args[2] = orc_program_find_var_by_name (program, arg3);
  
  program->n_insns++;
}

int
orc_program_allocate_register (OrcProgram *program, int data_reg)
{
  int i;
  int offset;

  if (data_reg) {
    offset = ORC_VEC_REG_BASE;
  } else {
    offset = ORC_GP_REG_BASE;
  }

  for(i=offset;i<offset+32;i++){
    if (program->valid_regs[i] &&
        !program->save_regs[i] &&
        program->alloc_regs[i] == 0) {
      program->alloc_regs[i]++;
      program->used_regs[i] = 1;
      return i;
    }
  }
  for(i=offset;i<offset+32;i++){
    if (program->valid_regs[i] &&
        program->alloc_regs[i] == 0) {
      program->alloc_regs[i]++;
      program->used_regs[i] = 1;
      return i;
    }
  }

  printf("register overflow\n");
  return 0;
}

void
orc_program_compile (OrcProgram *program)
{
  int i;

  for(i=0;i<32;i++) {
    program->valid_regs[i] = 1;
  }

  switch (program->target) {
    case ORC_TARGET_C:
      orc_program_c_init (program);
      break;
    case ORC_TARGET_ALTIVEC:
      orc_program_powerpc_init (program);
      break;
    case ORC_TARGET_SSE:
      orc_program_sse_init (program);
      break;
    case ORC_TARGET_MMX:
      orc_program_mmx_init (program);
      break;
    default:
      break;
  }

  orc_program_assign_rules (program);
  orc_program_rewrite_vars (program);

  if (program->target != ORC_TARGET_C) {
    orc_program_global_reg_alloc (program);

    orc_program_do_regs (program);
  }

  orc_program_rewrite_vars2 (program);

  if (program->target != ORC_TARGET_C) {
    orc_program_allocate_codemem (program);
  }

  switch (program->target) {
    case ORC_TARGET_C:
      orc_program_assemble_c (program);
      break;
    case ORC_TARGET_ALTIVEC:
      orc_program_assemble_powerpc (program);
      break;
    case ORC_TARGET_MMX:
      orc_program_mmx_assemble (program);
      break;
    case ORC_TARGET_SSE:
      orc_program_sse_assemble (program);
      break;
    default:
      break;
  }

  orc_program_dump_code (program);
}

void
orc_program_assign_rules (OrcProgram *program)
{
  int i;

  for(i=0;i<program->n_insns;i++) {
    OrcInstruction *insn = program->insns + i;

    insn->rule = insn->opcode->rules + program->target;
  }
}

void
orc_program_rewrite_vars (OrcProgram *program)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  int var;
  int actual_var;

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    /* set up args */
    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      var = insn->args[k];
      if (program->vars[var].vartype == ORC_VAR_TYPE_DEST) {
        printf("ERROR: using dest var as source\n");
      }

      actual_var = var;
      if (program->vars[var].replaced) {
        actual_var = program->vars[var].replacement;
        insn->args[k] = actual_var;
      }

      if (!program->vars[var].used) {
        if (program->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          printf("ERROR: using uninitialized temp var\n");
        }
        program->vars[var].used = TRUE;
        program->vars[var].first_use = j;
      }
      program->vars[actual_var].last_use = j;
    }

    for(k=0;k<opcode->n_dest;k++){
      var = insn->args[k];

      if (program->vars[var].vartype == ORC_VAR_TYPE_SRC) {
        printf("ERROR: using src var as dest\n");
      }
      if (program->vars[var].vartype == ORC_VAR_TYPE_CONST) {
        printf("ERROR: using const var as dest\n");
      }
      if (program->vars[var].vartype == ORC_VAR_TYPE_PARAM) {
        printf("ERROR: using param var as dest\n");
      }

      actual_var = var;
      if (program->vars[var].replaced) {
        actual_var = program->vars[var].replacement;
        insn->args[k] = actual_var;
      }

      if (!program->vars[var].used) {
        program->vars[actual_var].used = TRUE;
        program->vars[actual_var].first_use = j;
      } else {
        if (program->vars[var].vartype == ORC_VAR_TYPE_DEST) {
          printf("ERROR: writing dest more than once\n");
        }
        if (program->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          actual_var = orc_program_dup_temporary (program, var, j);
          program->vars[var].replaced = TRUE;
          program->vars[var].replacement = actual_var;
          insn->args[k] = actual_var;
          program->vars[actual_var].used = TRUE;
          program->vars[actual_var].first_use = j;
        }
      }
      program->vars[actual_var].last_use = j;
    }
  }
}

void
orc_program_global_reg_alloc (OrcProgram *program)
{
  int i;
  OrcVariable *var;


  for(i=0;i<program->n_vars;i++){
    var = program->vars + i;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        var->first_use = -1;
        var->last_use = -1;
        var->alloc = orc_program_allocate_register (program, TRUE);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        var->ptr_register = orc_program_allocate_register (program, FALSE);
        break;
      default:
        break;
    }
  }
}

void
orc_program_do_regs (OrcProgram *program)
{
  int i;
  int k;
  int var;
  OrcInstruction *insn;
  OrcOpcode *opcode;

  for(i=0;i<program->n_insns;i++){
    insn = program->insns + i;
    opcode = insn->opcode;

    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      var = insn->args[k];


    }

    for(k=0;k<opcode->n_dest;k++){
      var = insn->args[k];
    }
  }
}

void
orc_program_rewrite_vars2 (OrcProgram *program)
{
  int i;
  int j;
  int k;

  for(j=0;j<program->n_insns;j++){
#if 1
    /* must be true to chain src1 to dest:
     *  - rule must handle it
     *  - src1 must be last_use
     */
    if (1) {
      int src1 = program->insns[j].args[1];
      int dest = program->insns[j].args[0];
      if (program->vars[src1].last_use == j) {
        if (program->vars[src1].first_use == j) {
          k = orc_program_allocate_register (program, TRUE);
          program->vars[src1].alloc = k;
        }
        program->alloc_regs[program->vars[src1].alloc]++;
        program->vars[dest].alloc = program->vars[src1].alloc;
      }
    }
#endif

    if (0) {
      /* immediate operand, don't load */
      int src2 = program->insns[j].args[2];
      program->vars[src2].alloc = 1;
    } else {
      int src2 = program->insns[j].args[2];
      if (program->vars[src2].alloc == 1) {
        program->vars[src2].alloc = 0;
      }
    }

    for(i=0;i<program->n_vars;i++){
      if (program->vars[i].first_use == j) {
        if (program->vars[i].alloc) continue;
        k = orc_program_allocate_register (program, TRUE);
        program->vars[i].alloc = k;
      }
    }
    for(i=0;i<program->n_vars;i++){
      if (program->vars[i].last_use == j) {
        program->alloc_regs[program->vars[i].alloc]--;
      }
    }
  }

#if 0
  for(i=0;i<program->n_vars;i++){
    printf("# %2d: %2d %2d %d\n",
        i,
        program->vars[i].first_use,
        program->vars[i].last_use,
        program->vars[i].alloc);
  }
#endif

}

void
orc_program_dump_code (OrcProgram *program)
{
  FILE *file;
  int n;

  file = fopen("dump","w");

  n = fwrite (program->code, 1, program->codeptr - program->code, file);
  fclose (file);
}

void
orc_program_dump (OrcProgram *program)
{
  int i;
  int j;
  OrcOpcode *opcode;
  OrcInstruction *insn;

  for(i=0;i<program->n_insns;i++){
    insn = program->insns + i;
    opcode = insn->opcode;

    printf("insn: %d\n", i);
    printf("  opcode: %s\n", opcode->name);

    for(j=0;j<opcode->n_dest;j++){
      printf("  dest%d: %d %s\n", j, insn->args[j],
          program->vars[insn->args[j]].name);
    }
    for(j=0;j<opcode->n_src;j++){
      printf("  src%d: %d %s\n", j, insn->args[opcode->n_dest + j],
          program->vars[insn->args[opcode->n_dest + j]].name);
    }

    printf("\n");
  }

  for(i=0;i<program->n_vars;i++){
    printf("var: %d %s\n", i, program->vars[i].name);
    printf("first_use: %d\n", program->vars[i].first_use);
    printf("last_use: %d\n", program->vars[i].last_use);

    printf("\n");
  }

}

void
orc_program_append_code (OrcProgram *p, const char *fmt, ...)
{
  va_list varargs;

  va_start (varargs, fmt);
  vprintf(fmt, varargs);
  va_end (varargs);
}

