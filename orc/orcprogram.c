
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


OrcProgram *
orc_program_new (void)
{
  OrcProgram *p;

  p = malloc(sizeof(OrcProgram));
  memset (p, 0, sizeof(OrcProgram));

  p->name = malloc (40);
  sprintf(p->name, "func_%p", p);

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

OrcProgram *
orc_program_new_ds (int size1, int size2)
{
  OrcProgram *p;

  p = orc_program_new ();

  orc_program_add_destination (p, size1, "d1");
  orc_program_add_source (p, size2, "s1");

  return p;
}

void
orc_program_free (OrcProgram *program)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name) free (program->vars[i].name);
  }
  if (program->name) {
    free (program->name);
  }
  free (program);
}

void
orc_program_set_name (OrcProgram *program, const char *name)
{
  if (program->name) {
    free (program->name);
  }
  program->name = strdup (name);
}

const char *
orc_program_get_name (OrcProgram *program)
{
  return program->name;
}

int
orc_program_add_temporary (OrcProgram *program, int size, const char *name)
{
  int i = ORC_VAR_T1 + program->n_temp_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_temp_vars++;

  return i;
}

int
orc_program_dup_temporary (OrcProgram *program, int var, int j)
{
  int i = ORC_VAR_T1 + program->n_temp_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  program->vars[i].size = program->vars[var].size;
  program->vars[i].name = malloc (strlen(program->vars[var].name) + 10);
  sprintf(program->vars[i].name, "%s.dup%d", program->vars[var].name, j);
  program->n_temp_vars++;

  return i;
}

int
orc_program_add_source (OrcProgram *program, int size, const char *name)
{
  int i = ORC_VAR_S1 + program->n_src_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_SRC;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_src_vars++;

  return i;
}

int
orc_program_add_destination (OrcProgram *program, int size, const char *name)
{
  int i = ORC_VAR_D1 + program->n_dest_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_DEST;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_dest_vars++;

  return i;
}

int
orc_program_add_constant (OrcProgram *program, int size, int value, const char *name)
{
  int i = ORC_VAR_C1 + program->n_const_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_CONST;
  program->vars[i].size = size;
  program->vars[i].value = value;
  program->vars[i].name = strdup(name);
  program->n_const_vars++;

  return i;
}

int
orc_program_add_parameter (OrcProgram *program, int size, const char *name)
{
  int i = ORC_VAR_P1 + program->n_param_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_PARAM;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_param_vars++;

  return i;
}

void
orc_program_append_ds (OrcProgram *program, const char *name, int arg0,
    int arg1)
{
  OrcInstruction *insn;

  insn = program->insns + program->n_insns;

  insn->opcode = orc_opcode_find_by_name (name);
  if (!insn->opcode) {
    ORC_ERROR ("unknown opcode: %s", name);
  }
  insn->dest_args[0] = arg0;
  insn->src_args[0] = arg1;
  
  program->n_insns++;
}

void
orc_program_append (OrcProgram *program, const char *name, int arg0,
    int arg1, int arg2)
{
  OrcInstruction *insn;

  insn = program->insns + program->n_insns;

  insn->opcode = orc_opcode_find_by_name (name);
  if (!insn->opcode) {
    ORC_ERROR ("unknown opcode: %s", name);
  }
  insn->dest_args[0] = arg0;
  insn->src_args[0] = arg1;
  insn->src_args[1] = arg2;
  
  program->n_insns++;
}

int
orc_program_find_var_by_name (OrcProgram *program, const char *name)
{
  int i;

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name && strcmp (program->vars[i].name, name) == 0) {
      return i;
    }
  }

  return -1;
}

int
orc_compiler_get_dest (OrcCompiler *compiler)
{
  int k;

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (compiler->vars[k].name &&
        compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      return k;
    }
  }

  ORC_PROGRAM_ERROR(compiler, "failed to find destination array");
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
    ORC_ERROR ("unknown opcode: %s", name);
  }
  insn->dest_args[0] = orc_program_find_var_by_name (program, arg1);
  insn->src_args[0] = orc_program_find_var_by_name (program, arg2);
  insn->src_args[1] = orc_program_find_var_by_name (program, arg3);
  
  program->n_insns++;
}

void
orc_program_append_ds_str (OrcProgram *program, const char *name,
    const char *arg1, const char *arg2)
{
  OrcInstruction *insn;

  insn = program->insns + program->n_insns;

  insn->opcode = orc_opcode_find_by_name (name);
  if (!insn->opcode) {
    ORC_ERROR ("unknown opcode: %s", name);
  }
  insn->dest_args[0] = orc_program_find_var_by_name (program, arg1);
  insn->src_args[0] = orc_program_find_var_by_name (program, arg2);
  
  program->n_insns++;
}

const char *
orc_program_get_asm_code (OrcProgram *program)
{
  return program->asm_code;
}

int
orc_program_get_max_var_size (OrcProgram *program)
{
  int i;
  int max;

  max = 0;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].size) {
      max = MAX(max, program->vars[i].size);
    }
  }

  return max;
}

