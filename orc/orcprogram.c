
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orcprogram
 * @title: OrcProgram
 * @short_description: Creating and manipulating Orc programs
 */


/**
 * orc_program_new:
 * 
 * Create a new OrcProgram.  The program should be freed using
 * @orc_program_free().
 *
 * Returns: a pointer to an OrcProgram structure
 */
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

/**
 * orc_program_new_dss:
 * @size1: size of destination array members
 * @size2: size of first source array members
 * @size3: size of second source array members
 * 
 * Create a new OrcProgram, with a destination named "d1" and
 * two sources named "s1" and "s2".
 *
 * Returns: a pointer to an OrcProgram structure
 */
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

/**
 * orc_program_new_ds:
 * @size1: size of destination array members
 * @size2: size of source array members
 * 
 * Create a new OrcProgram, with a destination named "d1" and
 * one source named "s1".
 *
 * Returns: a pointer to an OrcProgram structure
 */
OrcProgram *
orc_program_new_ds (int size1, int size2)
{
  OrcProgram *p;

  p = orc_program_new ();

  orc_program_add_destination (p, size1, "d1");
  orc_program_add_source (p, size2, "s1");

  return p;
}

/**
 * orc_program_new_ass:
 * @size1: size of destination array members
 * @size2: size of first source array members
 * @size3: size of second source array members
 * 
 * Create a new OrcProgram, with an accumulator named "a1" and
 * two source named "s1" and "s2".
 *
 * Returns: a pointer to an OrcProgram structure
 */
OrcProgram *
orc_program_new_ass (int size1, int size2, int size3)
{
  OrcProgram *p;

  p = orc_program_new ();

  orc_program_add_accumulator (p, size1, "a1");
  orc_program_add_source (p, size2, "s1");
  orc_program_add_source (p, size3, "s2");

  return p;
}

/**
 * orc_program_new_as:
 * @size1: size of destination array members
 * @size2: size of source array members
 * 
 * Create a new OrcProgram, with an accumulator named "a1" and
 * one source named "s1".
 *
 * Returns: a pointer to an OrcProgram structure
 */
OrcProgram *
orc_program_new_as (int size1, int size2)
{
  OrcProgram *p;

  p = orc_program_new ();

  orc_program_add_accumulator (p, size1, "a1");
  orc_program_add_source (p, size2, "s1");

  return p;
}

/**
 * orc_program_free:
 * @program: a pointer to an OrcProgram structure
 *
 * Frees an OrcProgram.
 */
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

/**
 * orc_program_set_name:
 * @program: a pointer to an OrcProgram structure
 * @name: string to set the name to
 *
 * Sets the name of the program.  The string is copied.
 */
void
orc_program_set_name (OrcProgram *program, const char *name)
{
  if (program->name) {
    free (program->name);
  }
  program->name = strdup (name);
}

/**
 * orc_program_get_name:
 * @program: a pointer to an OrcProgram structure
 *
 * Gets the name of the program.  The string is valid until the name
 * is changed or the program is freed.
 *
 * Returns: a character string
 */
const char *
orc_program_get_name (OrcProgram *program)
{
  return program->name;
}

/**
 * orc_program_add_temporary:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data values
 * @name: name of variable
 *
 * Creates a new variable holding temporary values.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_dup_temporary:
 * @program: a pointer to an OrcProgram structure
 * @var: variable to duplicate
 * @j: index
 *
 * Internal function.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_add_source:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data values
 * @name: name of variable
 *
 * Creates a new variable representing a source array.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_add_destination:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data values
 * @name: name of variable
 *
 * Creates a new variable representing a destination array.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_add_constant:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data value
 * @value: the value
 * @name: name of variable
 *
 * Creates a new variable representing a constant value.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_add_parameter:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data value
 * @name: name of variable
 *
 * Creates a new variable representing a scalar parameter.
 *
 * Returns: the index of the new variable
 */
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

/**
 * orc_program_add_accumulator:
 * @program: a pointer to an OrcProgram structure
 * @size: size of data value
 * @name: name of variable
 *
 * Creates a new variable representing an accumulator.
 *
 * Returns: the index of the new variable
 */
int
orc_program_add_accumulator (OrcProgram *program, int size, const char *name)
{
  int i = ORC_VAR_A1 + program->n_accum_vars;

  program->vars[i].vartype = ORC_VAR_TYPE_ACCUMULATOR;
  program->vars[i].size = size;
  program->vars[i].name = strdup(name);
  program->n_param_vars++;

  return i;
}

void
orc_program_set_type_name (OrcProgram *program, int var, const char *type_name)
{
  program->vars[var].type_name = strdup(type_name);
}

/**
 * orc_program_append_ds:
 * @program: a pointer to an OrcProgram structure
 * @name: name of instruction
 * @arg0: index of first variable
 * @arg1: index of second variable
 *
 * Appends an instruction to the program, with arguments @arg0 and
 * @arg1.  The instruction must take 2 operands.
 */
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

/**
 * orc_program_append_ds:
 * @program: a pointer to an OrcProgram structure
 * @name: name of instruction
 * @arg0: index of first variable
 * @arg1: index of second variable
 * @arg2: index of second variable
 *
 * Appends an instruction to the program, with arguments @arg0,
 * @arg1, and @arg2.  The instruction must take 3 operands.
 */
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

/**
 * orc_program_find_var_by_name:
 * @program: a pointer to an OrcProgram structure
 * @name: name of instruction
 *
 * Finds the variable with the name @name.  If no variable with the
 * given name exists in the program, -1 is returned.
 *
 * Returns: the index of the variable
 */
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

/**
 * orc_program_append_str:
 * @program: a pointer to an OrcProgram structure
 * @name: name of instruction
 * @arg0: name of first variable
 * @arg1: name of second variable
 * @arg2: name of third variable
 *
 * Appends an instruction to the program, with arguments @arg0,
 * @arg1, and @arg2.  The instruction must take 3 operands.
 */
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

/**
 * orc_program_append_ds_str:
 * @program: a pointer to an OrcProgram structure
 * @name: name of instruction
 * @arg0: name of first variable
 * @arg1: name of second variable
 *
 * Appends an instruction to the program, with arguments @arg0 and
 * @arg2.  The instruction must take 2 operands.
 */
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

/**
 * orc_program_get_asm_code:
 * @program: a pointer to an OrcProgram structure
 *
 * Returns a character string containing the assembly code created
 * by compiling the program.  This string is valid until the program
 * is compiled again or the program is freed.
 * 
 * Returns: a character string
 */
const char *
orc_program_get_asm_code (OrcProgram *program)
{
  return program->asm_code;
}

/**
 * orc_program_get_max_var_size:
 * @program: a pointer to an OrcProgram structure
 *
 * Returns the size of the largest variable used in the program.
 * 
 * Returns: the number of bytes
 */
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

/**
 * orc_program_get_max_array_size:
 * @program: a pointer to an OrcProgram structure
 *
 * Returns the size of the largest array used in the program.
 * 
 * Returns: the number of bytes
 */
int
orc_program_get_max_array_size (OrcProgram *program)
{
  int i;
  int max;

  max = 0;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].size) {
      if (program->vars[i].vartype == ORC_VAR_TYPE_SRC ||
          program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
        max = MAX(max, program->vars[i].size);
      }
    }
  }

  return max;
}

