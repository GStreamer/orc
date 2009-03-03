
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>

#define SIZE 65536


void sse_emit_loop (OrcProgram *program);

void orc_program_sse_register_rules (void);


void orc_program_rewrite_vars (OrcProgram *program);
void orc_program_dump (OrcProgram *program);

void
sse_emit_prologue (OrcProgram *program)
{
  orc_program_append_code(program,".global test\n");
  orc_program_append_code(program,"test:\n");
  if (x86_64) {

  } else {
    x86_emit_push (program, 4, X86_EBP);
    x86_emit_mov_memoffset_reg (program, 4, 8, X86_ESP, X86_EBP);
    if (program->used_regs[X86_EDI]) {
      x86_emit_push (program, 4, X86_EDI);
    }
    if (program->used_regs[X86_ESI]) {
      x86_emit_push (program, 4, X86_ESI);
    }
    if (program->used_regs[X86_EBX]) {
      x86_emit_push (program, 4, X86_EBX);
    }
  }
}

void
sse_emit_epilogue (OrcProgram *program)
{
  if (x86_64) {

  } else {
    if (program->used_regs[X86_EBX]) {
      x86_emit_pop (program, 4, X86_EBX);
    }
    if (program->used_regs[X86_ESI]) {
      x86_emit_pop (program, 4, X86_ESI);
    }
    if (program->used_regs[X86_EDI]) {
      x86_emit_pop (program, 4, X86_EDI);
    }
    x86_emit_pop (program, 4, X86_EBP);
  }
  x86_emit_ret (program);
}

void
sse_do_fixups (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_fixups;i++){
    if (program->fixups[i].type == 0) {
      unsigned char *label = program->labels[program->fixups[i].label];
      unsigned char *ptr = program->fixups[i].ptr;

      ptr[0] += label - ptr;
    }
  }
}

void
orc_sse_init (void)
{
  orc_program_sse_register_rules ();
}

void
orc_program_sse_init (OrcProgram *program)
{
  int i;

  if (x86_64) {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
      program->valid_regs[i] = 1;
    }
    program->valid_regs[X86_ECX] = 0;
    program->valid_regs[X86_EDI] = 0;
    program->valid_regs[X86_ESP] = 0;
    for(i=X86_XMM0;i<X86_XMM0+16;i++){
      program->valid_regs[i] = 1;
    }
    program->save_regs[X86_EBX] = 1;
    program->save_regs[X86_EBP] = 1;
    program->save_regs[X86_R12] = 1;
    program->save_regs[X86_R13] = 1;
    program->save_regs[X86_R14] = 1;
    program->save_regs[X86_R15] = 1;
  } else {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+8;i++){
      program->valid_regs[i] = 1;
    }
    program->valid_regs[X86_ECX] = 0;
    program->valid_regs[X86_ESP] = 0;
    program->valid_regs[X86_EBP] = 0;
    for(i=X86_XMM0;i<X86_XMM0+8;i++){
      program->valid_regs[i] = 1;
    }
    program->save_regs[X86_EBX] = 1;
    program->save_regs[X86_EDI] = 1;
    program->save_regs[X86_EBP] = 1;
  }
  for(i=X86_MM0;i<X86_MM0+8;i++){
    program->valid_regs[i] = 1;
  }
  for(i=0;i<128;i++){
    program->alloc_regs[i] = 0;
    program->used_regs[i] = 0;
  }

  program->loop_shift = 3;
}

void
sse_load_constants (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_vars;i++){
    switch (program->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        sse_emit_loadiw (program, program->vars[i].alloc,
            program->vars[i].s16);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (program->vars[i].ptr_register) {
          x86_emit_mov_memoffset_reg (program, x86_ptr_size,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), x86_exec_ptr,
              program->vars[i].ptr_register);
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
sse_emit_load_src (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    x86_emit_mov_memoffset_reg (program, x86_ptr_size,
        var->ptr_offset, x86_exec_ptr, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (program->loop_shift) {
    case 0:
      x86_emit_mov_memoffset_reg (program, 2, 0, ptr_reg, X86_ECX);
      x86_emit_mov_reg_sse (program, X86_ECX, var->alloc);
      break;
    case 1:
      x86_emit_mov_memoffset_sse (program, 4, 0, ptr_reg, var->alloc);
      break;
    case 2:
      x86_emit_mov_memoffset_sse (program, 8, 0, ptr_reg, var->alloc);
      break;
    case 3:
      x86_emit_mov_memoffset_sse (program, 16, 0, ptr_reg, var->alloc);
      break;
    default:
      printf("ERROR\n");
  }
}

void
sse_emit_store_dest (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    x86_emit_mov_memoffset_reg (program, x86_ptr_size,
        var->ptr_offset, x86_exec_ptr, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (program->loop_shift) {
    case 0:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        printf("ERROR\n");
      }
      x86_emit_mov_sse_reg (program, var->alloc, X86_ECX);
      x86_emit_mov_reg_memoffset (program, 2, X86_ECX, 0, ptr_reg);
      break;
    case 1:
      x86_emit_mov_sse_memoffset (program, 4, var->alloc, 0, ptr_reg);
      break;
    case 2:
      x86_emit_mov_sse_memoffset (program, 8, var->alloc, 0, ptr_reg);
      break;
    case 3:
      x86_emit_mov_sse_memoffset (program, 16, var->alloc, 0, ptr_reg);
      break;
    default:
      printf("ERROR\n");
  }
}

void
orc_program_sse_assemble (OrcProgram *program)
{
  sse_emit_prologue (program);

  x86_emit_mov_memoffset_reg (program, 4, (int)ORC_STRUCT_OFFSET(OrcExecutor,n),
      x86_exec_ptr, X86_ECX);
  x86_emit_sar_imm_reg (program, 4, program->loop_shift, X86_ECX);
  x86_emit_mov_reg_memoffset (program, 4, X86_ECX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);

  x86_emit_mov_memoffset_reg (program, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n), x86_exec_ptr, X86_ECX);
  x86_emit_and_imm_reg (program, 4, (1<<program->loop_shift)-1, X86_ECX);
  x86_emit_mov_reg_memoffset (program, 4, X86_ECX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), x86_exec_ptr);

  sse_load_constants (program);

  if (program->loop_shift > 0) {
    int save_loop_shift;

    x86_emit_cmp_imm_memoffset (program, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), x86_exec_ptr);
    x86_emit_je (program, 1);

    save_loop_shift = program->loop_shift;
    program->loop_shift = 0;

    x86_emit_label (program, 0);
    sse_emit_loop (program);
    x86_emit_dec_memoffset (program, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1),
        x86_exec_ptr);
    x86_emit_jne (program, 0);

    program->loop_shift = save_loop_shift;
  }

  x86_emit_label (program, 1);

  x86_emit_cmp_imm_memoffset (program, 4, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);
  x86_emit_je (program, 3);

  x86_emit_label (program, 2);
  sse_emit_loop (program);
  x86_emit_dec_memoffset (program, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2),
      x86_exec_ptr);
  x86_emit_jne (program, 2);
  x86_emit_label (program, 3);

  sse_emit_epilogue (program);

  sse_do_fixups (program);
}

void
sse_emit_loop (OrcProgram *program)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  OrcVariable *args[10];
  OrcRule *rule;

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    orc_program_append_code(program,"# %d: %s", j, insn->opcode->name);

    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = program->vars + insn->args[k];
      orc_program_append_code(program," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        orc_program_append_code(program," (chained)");
      }
    }
    orc_program_append_code(program," rule_flag=%d", insn->rule_flag);
    orc_program_append_code(program,"\n");

    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_SRC:
          sse_emit_load_src (program, args[k]);
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
      if (args[0]->alloc != args[1]->alloc) {
        x86_emit_mov_sse_reg_reg (program, args[1]->alloc, args[0]->alloc);
      }
      rule->emit (program, rule->emit_user, insn);
    } else {
      orc_program_append_code(program,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_DEST:
          sse_emit_store_dest (program, args[k]);
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
        x86_emit_add_imm_reg (program, x86_ptr_size,
            orc_variable_get_size(program->vars + k) << program->loop_shift,
            program->vars[k].ptr_register);
      } else {
        x86_emit_add_imm_memoffset (program, x86_ptr_size,
            orc_variable_get_size(program->vars + k) << program->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
            x86_exec_ptr);
      }
    }
  }
}

