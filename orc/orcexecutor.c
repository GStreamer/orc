
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orcexecutor
 * @title: OrcExecutor
 * @short_description: Running Orc programs
 */


OrcExecutor *
orc_executor_new (OrcProgram *program)
{
  OrcExecutor *ex;

  ex = malloc(sizeof(OrcExecutor));
  memset(ex,0,sizeof(OrcExecutor));

  ex->program = program;

  return ex;
}

void
orc_executor_free (OrcExecutor *ex)
{
  free (ex);
}

void
orc_executor_run (OrcExecutor *ex)
{
  void (*func) (OrcExecutor *);

  func = ex->program->code_exec;
  if (func) {
    func (ex);
    //ORC_ERROR("counters %d %d %d", ex->counter1, ex->counter2, ex->counter3);
  } else {
    orc_executor_emulate (ex);
  }
}

void
orc_executor_set_program (OrcExecutor *ex, OrcProgram *program)
{
  ex->program = program;
}

void
orc_executor_set_array (OrcExecutor *ex, int var, void *ptr)
{
  ex->arrays[var] = ptr;
}

void
orc_executor_set_array_str (OrcExecutor *ex, const char *name, void *ptr)
{
  int var;
  var = orc_program_find_var_by_name (ex->program, name);
  ex->arrays[var] = ptr;
}

void
orc_executor_set_param (OrcExecutor *ex, int var, int value)
{
  ex->params[var] = value;
}

void
orc_executor_set_param_str (OrcExecutor *ex, const char *name, int value)
{
  int var;
  var = orc_program_find_var_by_name (ex->program, name);
  ex->params[var] = value;
}

int
orc_executor_get_accumulator (OrcExecutor *ex, int var)
{
  return ex->accumulators[var - ORC_VAR_A1];
}

int
orc_executor_get_accumulator_str (OrcExecutor *ex, const char *name)
{
  int var;
  var = orc_program_find_var_by_name (ex->program, name);
  return ex->accumulators[var];
}

void
orc_executor_set_n (OrcExecutor *ex, int n)
{
  ex->n = n;
}

void
orc_executor_emulate (OrcExecutor *ex)
{
  int i;
  int j;
  int k;
  OrcProgram *program = ex->program;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcOpcodeExecutor opcode_ex;

  ex->accumulators[0] = 0;

  memset (&opcode_ex, 0, sizeof(opcode_ex));

  for(i=0;i<ex->n;i++){
    for(j=0;j<program->n_insns;j++){
      insn = program->insns + j;
      opcode = insn->opcode;

      /* set up args */
      for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++) {
        OrcVariable *var = program->vars + insn->src_args[k];

        if (opcode->src_size[k] == 0) continue;

        if (var->vartype == ORC_VAR_TYPE_CONST) {
          opcode_ex.src_values[k] = var->value;
        } else if (var->vartype == ORC_VAR_TYPE_PARAM) {
          opcode_ex.src_values[k] = ex->params[insn->src_args[k]];
        } else if (var->vartype == ORC_VAR_TYPE_TEMP) {
          /* FIXME shouldn't store executor stuff in program */
          opcode_ex.src_values[k] = var->value;
        } else if (var->vartype == ORC_VAR_TYPE_SRC ||
            var->vartype == ORC_VAR_TYPE_DEST) {
          void *ptr = ex->arrays[insn->src_args[k]] + var->size*i;

          switch (var->size) {
            case 1:
              opcode_ex.src_values[k] = *(int8_t *)ptr;
              break;
            case 2:
              opcode_ex.src_values[k] = *(int16_t *)ptr;
              break;
            case 4:
              opcode_ex.src_values[k] = *(int32_t *)ptr;
              break;
            case 8:
              opcode_ex.src_values[k] = *(int64_t *)ptr;
              break;
            default:
              ORC_ERROR("unhandled size %d", program->vars[insn->src_args[k]].size);
          }
        } else {
          ORC_ERROR("shouldn't be reached (%d)", var->vartype);
        }
      }

      opcode->emulate (&opcode_ex, opcode->emulate_user);

      for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
        OrcVariable *var = program->vars + insn->dest_args[k];

        if (opcode->dest_size[k] == 0) continue;

        if (var->vartype == ORC_VAR_TYPE_TEMP) {
          /* FIXME shouldn't store executor stuff in program */
          var->value = opcode_ex.dest_values[k];
        } else if (var->vartype == ORC_VAR_TYPE_DEST) {
          void *ptr = ex->arrays[insn->dest_args[k]] + var->size*i;

          switch (var->size) {
            case 1:
              *(int8_t *)ptr = opcode_ex.dest_values[k];
              break;
            case 2:
              *(int16_t *)ptr = opcode_ex.dest_values[k];
              break;
            case 4:
              *(int32_t *)ptr = opcode_ex.dest_values[k];
              break;
            case 8:
              *(int64_t *)ptr = opcode_ex.dest_values[k];
              break;
            default:
              ORC_ERROR("unhandled size %d", program->vars[insn->dest_args[k]].size);
          }
        } else if (var->vartype == ORC_VAR_TYPE_ACCUMULATOR) {
          ex->accumulators[0] += opcode_ex.dest_values[k];
        } else {
          ORC_ERROR("shouldn't be reached (%d)", var->vartype);
        }
      }
    }
  }
}


