
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

void orc_compiler_assign_rules (OrcCompiler *compiler);
void orc_compiler_global_reg_alloc (OrcCompiler *compiler);
void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_rewrite_vars2 (OrcCompiler *compiler);
void orc_compiler_do_regs (OrcCompiler *compiler);
int orc_compiler_dup_temporary (OrcCompiler *compiler, int var, int j);
void orc_compiler_check_sizes (OrcCompiler *compiler);


int
orc_compiler_allocate_register (OrcCompiler *compiler, int data_reg)
{
  int i;
  int offset;

  if (data_reg) {
    offset = compiler->target->data_register_offset;
  } else {
    offset = ORC_GP_REG_BASE;
  }

  for(i=offset;i<offset+32;i++){
    if (compiler->valid_regs[i] &&
        !compiler->save_regs[i] &&
        compiler->alloc_regs[i] == 0) {
      compiler->alloc_regs[i]++;
      compiler->used_regs[i] = 1;
      return i;
    }
  }
  for(i=offset;i<offset+32;i++){
    if (compiler->valid_regs[i] &&
        compiler->alloc_regs[i] == 0) {
      compiler->alloc_regs[i]++;
      compiler->used_regs[i] = 1;
      return i;
    }
  }

  ORC_ERROR ("register overflow");
  return 0;
}

orc_bool
orc_program_compile (OrcProgram *program)
{
  return orc_program_compile_for_target (program, orc_target_get_default());
}

orc_bool
orc_program_compile_for_target (OrcProgram *program, OrcTarget *target)
{
  OrcCompiler *compiler;
  int i;

  compiler = malloc (sizeof(OrcCompiler));
  memset (compiler, 0, sizeof(OrcCompiler));

  compiler->program = program;
  compiler->target = target;

  {
    for(i=0;i<ORC_N_VARIABLES;i++){
      ORC_INFO("%d: %s %d %d", i,
          program->vars[i].name,
          program->vars[i].size,
          program->vars[i].vartype);
    }
    for(i=0;i<program->n_insns;i++){
      ORC_INFO("%d: %s %d %d %d %d", i,
          program->insns[i].opcode->name,
          program->insns[i].dest_args[0],
          program->insns[i].dest_args[1],
          program->insns[i].src_args[0],
          program->insns[i].src_args[1]);
    }
  }

  memcpy (compiler->insns, program->insns,
      program->n_insns * sizeof(OrcInstruction));
  compiler->n_insns = program->n_insns;

  memcpy (compiler->vars, program->vars,
      ORC_N_VARIABLES * sizeof(OrcVariable));
  compiler->n_temp_vars = program->n_temp_vars;

  for(i=0;i<32;i++) {
    compiler->valid_regs[i] = 1;
  }

  compiler->target->compiler_init (compiler);

  orc_compiler_check_sizes (compiler);
  if (compiler->error) goto error;

  orc_compiler_assign_rules (compiler);
  if (compiler->error) goto error;

  orc_compiler_rewrite_vars (compiler);
  if (compiler->error) goto error;

  //if (compiler->target != ORC_TARGET_C) {
    orc_compiler_global_reg_alloc (compiler);

    orc_compiler_do_regs (compiler);
  //}

  orc_compiler_rewrite_vars2 (compiler);
  if (compiler->error) goto error;

  //if (compiler->target != ORC_TARGET_C) {
    orc_compiler_allocate_codemem (compiler);
  //}

  compiler->target->compile (compiler);
  if (compiler->error) goto error;

  program->asm_code = compiler->asm_code;
  program->code_size = compiler->codeptr - program->code;

  return TRUE;
error:
  ORC_ERROR("program failed to compile");
  return FALSE;
}

void
orc_compiler_check_sizes (OrcCompiler *compiler)
{
  int i;
  int j;

  for(i=0;i<compiler->n_insns;i++) {
    OrcInstruction *insn = compiler->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;

    for(j=0;j<ORC_STATIC_OPCODE_N_DEST;j++){
      if (opcode->dest_size[j] == 0) continue;
      if (opcode->dest_size[j] != compiler->vars[insn->dest_args[j]].size) {
        ORC_PROGRAM_ERROR(compiler, "size mismatch, opcode %s dest[%d] is %d should be %d",
            opcode->name, j, compiler->vars[insn->dest_args[j]].size,
            opcode->dest_size[j]);
        return;
      }
    }
    for(j=0;j<ORC_STATIC_OPCODE_N_SRC;j++){
      if (opcode->src_size[j] == 0) continue;
      if (opcode->src_size[j] != compiler->vars[insn->src_args[j]].size &&
          compiler->vars[insn->src_args[j]].vartype != ORC_VAR_TYPE_PARAM) {
        ORC_PROGRAM_ERROR(compiler, "size mismatch, opcode %s src[%d] is %d should be %d",
            opcode->name, j, compiler->vars[insn->src_args[j]].size,
            opcode->src_size[j]);
        return;
      }
    }
  }
}

void
orc_compiler_assign_rules (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<compiler->n_insns;i++) {
    OrcInstruction *insn = compiler->insns + i;

    insn->rule = orc_target_get_rule (compiler->target, insn->opcode);

    if (insn->rule == NULL || insn->rule->emit == NULL) {
      ORC_PROGRAM_ERROR(compiler, "No rule for: %s", insn->opcode->name);
      return;
    }
  }
}

void
orc_compiler_rewrite_vars (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  int var;
  int actual_var;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    /* set up args */
    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      if (opcode->src_size[k] == 0) continue;

      var = insn->src_args[k];
#if 0
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_DEST) {
        ORC_PROGRAM_ERROR(compiler, "using dest var as source");
      }
#endif

      actual_var = var;
      if (compiler->vars[var].replaced) {
        actual_var = compiler->vars[var].replacement;
        insn->src_args[k] = actual_var;
      }

      if (!compiler->vars[var].used) {
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          ORC_PROGRAM_ERROR(compiler, "using uninitialized temp var");
        }
        compiler->vars[var].used = TRUE;
        compiler->vars[var].first_use = j;
      }
      compiler->vars[actual_var].last_use = j;
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      if (opcode->dest_size[k] == 0) continue;

      var = insn->dest_args[k];

      if (compiler->vars[var].vartype == ORC_VAR_TYPE_SRC) {
        ORC_PROGRAM_ERROR(compiler,"using src var as dest");
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_CONST) {
        ORC_PROGRAM_ERROR(compiler,"using const var as dest");
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_PARAM) {
        ORC_PROGRAM_ERROR(compiler,"using param var as dest");
      }
      if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
        if (compiler->vars[var].vartype != ORC_VAR_TYPE_ACCUMULATOR) {
          ORC_PROGRAM_ERROR(compiler,"accumulating opcode to non-accumulator dest");
        }
      } else {
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
          ORC_PROGRAM_ERROR(compiler,"non-accumulating opcode to accumulator dest");
        }
      }

      actual_var = var;
      if (compiler->vars[var].replaced) {
        actual_var = compiler->vars[var].replacement;
        insn->dest_args[k] = actual_var;
      }

      if (!compiler->vars[var].used) {
        compiler->vars[actual_var].used = TRUE;
        compiler->vars[actual_var].first_use = j;
      } else {
#if 0
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_DEST) {
          ORC_PROGRAM_ERROR(compiler,"writing dest more than once");
        }
#endif
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          actual_var = orc_compiler_dup_temporary (compiler, var, j);
          compiler->vars[var].replaced = TRUE;
          compiler->vars[var].replacement = actual_var;
          insn->dest_args[k] = actual_var;
          compiler->vars[actual_var].used = TRUE;
          compiler->vars[actual_var].first_use = j;
        }
      }
      compiler->vars[actual_var].last_use = j;
    }
  }
}

void
orc_compiler_global_reg_alloc (OrcCompiler *compiler)
{
  int i;
  OrcVariable *var;


  for(i=0;i<ORC_N_VARIABLES;i++){
    var = compiler->vars + i;
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        var->first_use = -1;
        var->last_use = -1;
        var->alloc = orc_compiler_allocate_register (compiler, TRUE);
        break;
      case ORC_VAR_TYPE_PARAM:
        var->first_use = -1;
        var->last_use = -1;
        var->alloc = orc_compiler_allocate_register (compiler, TRUE);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        var->ptr_register = orc_compiler_allocate_register (compiler, FALSE);
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        var->first_use = -1;
        var->last_use = -1;
        var->alloc = orc_compiler_allocate_register (compiler, TRUE);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler, "bad vartype");
        break;
    }
  }
}

void
orc_compiler_do_regs (OrcCompiler *compiler)
{
  int i;
  int k;
  int var;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;

  for(i=0;i<compiler->n_insns;i++){
    insn = compiler->insns + i;
    opcode = insn->opcode;

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      if (opcode->src_size[k] == 0) continue;

      var = insn->src_args[k];

    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      if (opcode->dest_size[k] == 0) continue;

      var = insn->dest_args[k];
    }
  }
}

void
orc_compiler_rewrite_vars2 (OrcCompiler *compiler)
{
  int i;
  int j;
  int k;

  for(j=0;j<compiler->n_insns;j++){
#if 1
    /* must be true to chain src1 to dest:
     *  - rule must handle it
     *  - src1 must be last_use
     */
    if (!(compiler->insns[j].opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR)) {
      int src1 = compiler->insns[j].src_args[0];
      int dest = compiler->insns[j].dest_args[0];

      if (compiler->vars[src1].last_use == j) {
        if (compiler->vars[src1].first_use == j) {
          k = orc_compiler_allocate_register (compiler, TRUE);
          compiler->vars[src1].alloc = k;
        }
        compiler->alloc_regs[compiler->vars[src1].alloc]++;
        compiler->vars[dest].alloc = compiler->vars[src1].alloc;
      }
    }
#endif

    if (0) {
      /* immediate operand, don't load */
      int src2 = compiler->insns[j].src_args[1];
      compiler->vars[src2].alloc = 1;
    } else {
      int src2 = compiler->insns[j].src_args[1];
      if (compiler->vars[src2].alloc == 1) {
        compiler->vars[src2].alloc = 0;
      }
    }

    for(i=0;i<ORC_N_VARIABLES;i++){
      if (compiler->vars[i].name == NULL) continue;
      if (compiler->vars[i].first_use == j) {
        if (compiler->vars[i].alloc) continue;
        k = orc_compiler_allocate_register (compiler, TRUE);
        compiler->vars[i].alloc = k;
      }
    }
    for(i=0;i<ORC_N_VARIABLES;i++){
      if (compiler->vars[i].name == NULL) continue;
      if (compiler->vars[i].last_use == j) {
        compiler->alloc_regs[compiler->vars[i].alloc]--;
      }
    }
  }

}

int
orc_compiler_dup_temporary (OrcCompiler *compiler, int var, int j)
{
  int i = ORC_VAR_T1 + compiler->n_temp_vars;

  compiler->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  compiler->vars[i].size = compiler->vars[var].size;
  compiler->vars[i].name = malloc (strlen(compiler->vars[var].name) + 10);
  sprintf(compiler->vars[i].name, "%s.dup%d", compiler->vars[var].name, j);
  compiler->n_temp_vars++;

  return i;
}

void
orc_compiler_dump_asm (OrcCompiler *compiler)
{
  printf("%s", compiler->asm_code);
}

void
orc_compiler_append_code (OrcCompiler *p, const char *fmt, ...)
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

