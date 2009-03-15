
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

void orc_program_assign_rules (OrcProgram *program);
void orc_program_global_reg_alloc (OrcProgram *program);
void orc_program_rewrite_vars (OrcProgram *program);
void orc_program_rewrite_vars2 (OrcProgram *program);
void orc_program_do_regs (OrcProgram *program);

#if defined(HAVE_I386)
int _orc_default_target = ORC_TARGET_SSE;
#elif defined(HAVE_AMD64)
int _orc_default_target = ORC_TARGET_SSE;
#elif defined(HAVE_ARM)
int _orc_default_target = ORC_TARGET_ARM;
#elif defined(HAVE_POWERPC)
int _orc_default_target = ORC_TARGET_ALTIVEC;
#else
int _orc_default_target = ORC_TARGET_C;
#endif


int
orc_program_allocate_register (OrcProgram *program, int data_reg)
{
  int i;
  int offset;

  if (data_reg) {
    if (program->target == ORC_TARGET_ARM ||
        program->target == ORC_TARGET_C) {
      offset = ORC_GP_REG_BASE;
    } else {
      offset = ORC_VEC_REG_BASE;
    }
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

  ORC_ERROR ("register overflow");
  return 0;
}

orc_bool
orc_program_compile (OrcProgram *program)
{
  return orc_program_compile_for_target (program, _orc_default_target);
}

orc_bool
orc_program_compile_for_target (OrcProgram *program, int target)
{
  int i;

  program->target = target;

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
    case ORC_TARGET_ARM:
      orc_program_arm_init (program);
      break;
    default:
      break;
  }

  orc_program_assign_rules (program);
  if (program->error) goto error;

  orc_program_rewrite_vars (program);
  if (program->error) goto error;

  if (program->target != ORC_TARGET_C) {
    orc_program_global_reg_alloc (program);

    orc_program_do_regs (program);
  }

  orc_program_rewrite_vars2 (program);
  if (program->error) goto error;

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
    case ORC_TARGET_ARM:
      orc_program_arm_assemble (program);
      break;
    default:
      break;
  }
  if (program->error) goto error;

  return TRUE;
error:
  ORC_ERROR("program failed to compile");
  return FALSE;
}

void
orc_program_assign_rules (OrcProgram *program)
{
  int i;

  for(i=0;i<program->n_insns;i++) {
    OrcInstruction *insn = program->insns + i;

    insn->rule = insn->opcode->rules + program->target;

    if (insn->rule == NULL || insn->rule->emit == NULL) {
      ORC_PROGRAM_ERROR(program, "No rule for: %s", insn->opcode->name);
    }
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
        ORC_PROGRAM_ERROR(program, "using dest var as source");
      }

      actual_var = var;
      if (program->vars[var].replaced) {
        actual_var = program->vars[var].replacement;
        insn->args[k] = actual_var;
      }

      if (!program->vars[var].used) {
        if (program->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          ORC_PROGRAM_ERROR(program, "using uninitialized temp var");
        }
        program->vars[var].used = TRUE;
        program->vars[var].first_use = j;
      }
      program->vars[actual_var].last_use = j;
    }

    for(k=0;k<opcode->n_dest;k++){
      var = insn->args[k];

      if (program->vars[var].vartype == ORC_VAR_TYPE_SRC) {
        ORC_PROGRAM_ERROR(program,"using src var as dest");
      }
      if (program->vars[var].vartype == ORC_VAR_TYPE_CONST) {
        ORC_PROGRAM_ERROR(program,"using const var as dest");
      }
      if (program->vars[var].vartype == ORC_VAR_TYPE_PARAM) {
        ORC_PROGRAM_ERROR(program,"using param var as dest");
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
          ORC_PROGRAM_ERROR(program,"writing dest more than once");
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
      case ORC_VAR_TYPE_PARAM:
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

const char *
orc_program_get_asm_code (OrcProgram *program)
{
  return program->asm_code;
}

void
orc_program_dump_asm (OrcProgram *program)
{
  printf("%s", program->asm_code);
}

void
orc_program_append_code (OrcProgram *p, const char *fmt, ...)
{
  char tmp[100];
  va_list varargs;
  int n;

  va_start (varargs, fmt);
  vsnprintf(tmp, 100 - 1, fmt, varargs);
  va_end (varargs);

  n = strlen (tmp);
  p->asm_code = realloc (p->asm_code, p->asm_code_len + n + 1);
  memcpy (p->asm_code + p->asm_code_len, tmp, n + 1);
  p->asm_code_len += n;
}

