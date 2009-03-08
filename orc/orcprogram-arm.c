
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/arm.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#define SIZE 65536

int arm_exec_ptr = ARM_V1;

void arm_emit_loop (OrcProgram *program);

void orc_program_arm_register_rules (void);


void orc_program_rewrite_vars (OrcProgram *program);
void orc_program_dump (OrcProgram *program);

void
arm_emit_prologue (OrcProgram *program)
{
  unsigned int regs = 0;
  int i;

  orc_program_append_code(program,".global _binary_dump_start\n");
  orc_program_append_code(program,"_binary_dump_start:\n");

  for(i=0;i<16;i++){
    if (program->used_regs[ORC_GP_REG_BASE + i] &&
        program->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) arm_emit_push (program, regs);

#if 0
  arm_emit_mov_memoffset_reg (program, 4, 8, X86_ESP, X86_EBP);
  if (program->used_regs[X86_EDI]) {
    arm_emit_push (program, 4, X86_EDI);
  }
  if (program->used_regs[X86_ESI]) {
    arm_emit_push (program, 4, X86_ESI);
  }
  if (program->used_regs[X86_EBX]) {
    arm_emit_push (program, 4, X86_EBX);
  }
#endif
}

void
arm_dump_insns (OrcProgram *program)
{

  arm_emit_label (program, 0);

  arm_emit_add (program, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_sub (program, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_push (program, 0x06);
  arm_emit_mov (program, ARM_A2, ARM_A3);

  arm_emit_branch (program, ARM_COND_LE, 0);
  arm_emit_branch (program, ARM_COND_AL, 0);

  arm_emit_loadimm (program, ARM_A3, 0xa500);
  arm_loadw (program, ARM_A3, ARM_A4, 0xa5);
  arm_emit_load_reg (program, ARM_A3, ARM_A4, 0x5a5);
}

void
arm_emit_epilogue (OrcProgram *program)
{
  int i;
  unsigned int regs = 0;

  for(i=0;i<16;i++){
    if (program->used_regs[ORC_GP_REG_BASE + i] &&
        program->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) arm_emit_pop (program, regs);
  arm_emit_bx_lr (program);

  //arm_dump_insns (program);
}

void
orc_arm_init (void)
{
  orc_program_arm_register_rules ();
}

void
orc_program_arm_init (OrcProgram *program)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
    program->valid_regs[i] = 1;
  }
  program->valid_regs[ARM_V1] = 0;
  //program->valid_regs[ARM_SB] = 0;
  program->valid_regs[ARM_IP] = 0;
  program->valid_regs[ARM_SP] = 0;
  program->valid_regs[ARM_LR] = 0;
  program->valid_regs[ARM_PC] = 0;
  for(i=4;i<11;i++) {
    program->save_regs[ORC_GP_REG_BASE+i] = 1;
  }
  
  for(i=0;i<ORC_N_REGS;i++){
    program->alloc_regs[i] = 0;
    program->used_regs[i] = 0;
  }

  program->loop_shift = 0;
}

void
arm_load_constants (OrcProgram *program)
{
  int i;
  for(i=0;i<program->n_vars;i++){
    switch (program->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        //arm_emit_loadiw (program, program->vars[i].alloc,
        //    (int)program->vars[i].value);
        break;
      case ORC_VAR_TYPE_PARAM:
        //arm_emit_loadw (program, program->vars[i].alloc,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]), arm_exec_ptr);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        arm_emit_load_reg (program, 
            program->vars[i].ptr_register,
            arm_exec_ptr, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        break;
      default:
        break;
    }
  }
}

void
arm_emit_load_src (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - program->vars;
    //arm_emit_mov_memoffset_reg (program, arm_ptr_size,
    //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
    //    arm_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << program->loop_shift) {
    //case 1:
      //arm_emit_mov_memoffset_reg (program, 1, 0, ptr_reg, X86_ECX);
      //arm_emit_mov_reg_arm (program, X86_ECX, var->alloc);
      break;
    case 2:
      arm_loadw (program, var->alloc, ptr_reg, 0);
      //arm_emit_mov_memoffset_reg (program, 2, 0, ptr_reg, X86_ECX);
      //arm_emit_mov_reg_arm (program, X86_ECX, var->alloc);
      break;
    //case 4:
      //arm_emit_mov_memoffset_arm (program, 4, 0, ptr_reg, var->alloc);
      break;
    //case 8:
      //arm_emit_mov_memoffset_arm (program, 8, 0, ptr_reg, var->alloc);
      break;
    //case 16:
      //arm_emit_mov_memoffset_arm (program, 16, 0, ptr_reg, var->alloc);
      break;
    default:
      printf("ERROR bad size %d\n", var->size << program->loop_shift);
  }
}

void
arm_emit_store_dest (OrcProgram *program, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    //arm_emit_mov_memoffset_reg (program, arm_ptr_size,
    //    var->ptr_offset, arm_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << program->loop_shift) {
    case 1:
      //arm_emit_mov_arm_reg (program, var->alloc, X86_ECX);
      //arm_emit_mov_reg_memoffset (program, 1, X86_ECX, 0, ptr_reg);
      break;
    case 2:
      arm_storew (program, ptr_reg, 0, var->alloc);
      //arm_emit_mov_arm_reg (program, var->alloc, X86_ECX);
      //arm_emit_mov_reg_memoffset (program, 2, X86_ECX, 0, ptr_reg);
      break;
    case 4:
      //arm_emit_mov_arm_memoffset (program, 4, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    case 8:
      //arm_emit_mov_arm_memoffset (program, 8, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    case 16:
      //arm_emit_mov_arm_memoffset (program, 16, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    default:
      printf("ERROR\n");
  }
}

void
orc_program_arm_assemble (OrcProgram *program)
{
  int dest_var = orc_program_get_dest (program);

  program->vars[dest_var].is_aligned = FALSE;

  arm_emit_prologue (program);

  arm_emit_load_reg (program, ARM_IP, arm_exec_ptr,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  arm_load_constants (program);

  arm_emit_label (program, 1);

  arm_emit_cmp_imm (program, ARM_IP, 0);
  arm_emit_branch (program, ARM_COND_EQ, 3);

  arm_emit_label (program, 2);
  arm_emit_loop (program);
  arm_emit_sub_imm (program, ARM_IP, ARM_IP, 1);
  arm_emit_cmp_imm (program, ARM_IP, 0);
  arm_emit_branch (program, ARM_COND_NE, 2);
  arm_emit_label (program, 3);

  arm_emit_epilogue (program);

  arm_do_fixups (program);
}

void
arm_emit_loop (OrcProgram *program)
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
    orc_program_append_code(program,"\n");

    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_SRC:
          arm_emit_load_src (program, args[k]);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_PARAM:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }

    rule = insn->rule;
    if (rule && rule->emit) {
      if (args[0]->alloc != args[1]->alloc) {
        arm_emit_mov (program, args[1]->alloc, args[0]->alloc);
      }
      rule->emit (program, rule->emit_user, insn);
    } else {
      orc_program_append_code(program,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_DEST:
          arm_emit_store_dest (program, args[k]);
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
        //arm_emit_add_imm_reg (program, arm_ptr_size,
        //    program->vars[k].size << program->loop_shift,
        //    program->vars[k].ptr_register);
      } else {
        //arm_emit_add_imm_memoffset (program, arm_ptr_size,
        //    program->vars[k].size << program->loop_shift,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
        //    arm_exec_ptr);
      }
    }
  }
}

