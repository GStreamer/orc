
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


OrcExecutor *
orc_executor_new (OrcProgram *program)
{
  OrcExecutor *ex;

  ex = malloc(sizeof(OrcExecutor));
  memset(ex,0,sizeof(OrcExecutor));

  ex->program = program;

  memcpy (ex->vars, program->vars, 10*sizeof(OrcVariable));

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
  func (ex);
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
  OrcOpcode *opcode;

  //printf("n %d\n", ex->n);
  //printf("n_insns %d\n", program->n_insns);

  for(i=0;i<ex->n;i++){
    for(j=0;j<program->n_insns;j++){
      insn = program->insns + j;
      opcode = insn->opcode;

      //printf("%d: %s\n", j, insn->opcode->name);

      /* set up args */
      for(k=0;k<opcode->n_src + opcode->n_dest;k++){
        ex->args[k] = ex->vars + insn->args[k];

        //printf("setting up arg %d as var %d vartype %d\n",
        //    k, insn->args[k], ex->args[k]->vartype);

        if (ex->args[k]->vartype == ORC_VAR_TYPE_SRC) {
          void *ptr = ex->arrays[insn->args[k]] + 2*i;

          //printf("load %p\n", ptr);
          ex->args[k]->s16 = *(int16_t *)ptr;
        }
      }

      opcode->emulate (ex, opcode->emulate_user);
      //printf("emulate: %d %d %d\n", ex->args[0]->s16,
      //    ex->args[1]->s16, ex->args[2]->s16);

      for(k=0;k<opcode->n_src + opcode->n_dest;k++){
        if (ex->args[k]->vartype == ORC_VAR_TYPE_DEST) {
          void *ptr = ex->arrays[insn->args[k]] + 2*i;

          //printf("store %p\n", ptr);
          *(int16_t *)ptr = ex->args[k]->s16;
        }
      }
    }
  }
}


