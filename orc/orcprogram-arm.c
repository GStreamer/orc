
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

int arm_exec_ptr = ARM_R0;

void arm_emit_loop (OrcCompiler *compiler);

void orc_compiler_arm_register_rules (OrcTarget *target);

void orc_compiler_arm_init (OrcCompiler *compiler);
unsigned int orc_compiler_arm_get_default_flags (void);
void orc_compiler_arm_assemble (OrcCompiler *compiler);

void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);

void
arm_emit_prologue (OrcCompiler *compiler)
{
  unsigned int regs = 0;
  int i;

  orc_compiler_append_code(compiler,".global %s\n", compiler->program->name);
  orc_compiler_append_code(compiler,"%s:\n", compiler->program->name);

  for(i=0;i<16;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) arm_emit_push (compiler, regs);

}

void
arm_dump_insns (OrcCompiler *compiler)
{

  arm_emit_label (compiler, 0);

  arm_emit_add (compiler, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_sub (compiler, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_push (compiler, 0x06);
  arm_emit_mov (compiler, ARM_A2, ARM_A3);

  arm_emit_branch (compiler, ARM_COND_LE, 0);
  arm_emit_branch (compiler, ARM_COND_AL, 0);

  arm_emit_loadimm (compiler, ARM_A3, 0xa500);
  arm_loadw (compiler, ARM_A3, ARM_A4, 0xa5);
  arm_emit_load_reg (compiler, ARM_A3, ARM_A4, 0x5a5);
}

void
arm_emit_epilogue (OrcCompiler *compiler)
{
  int i;
  unsigned int regs = 0;

  for(i=0;i<16;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) arm_emit_pop (compiler, regs);
  arm_emit_bx_lr (compiler);

  //arm_dump_insns (compiler);
}

static OrcTarget arm_target = {
  "arm",
#ifdef HAVE_ARM
  TRUE,
#else
  FALSE,
#endif
  ORC_GP_REG_BASE,
  orc_compiler_arm_get_default_flags,
  orc_compiler_arm_init,
  orc_compiler_arm_assemble
};

void
orc_arm_init (void)
{
  orc_target_register (&arm_target);

  orc_compiler_arm_register_rules (&arm_target);
}

unsigned int
orc_compiler_arm_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_arm_init (OrcCompiler *compiler)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+9;i++){
    compiler->valid_regs[i] = 1;
  }
  compiler->valid_regs[ARM_R0] = 0;
  //compiler->valid_regs[ARM_SB] = 0;
  compiler->valid_regs[ARM_IP] = 0;
  compiler->valid_regs[ARM_SP] = 0;
  compiler->valid_regs[ARM_LR] = 0;
  compiler->valid_regs[ARM_PC] = 0;
  for(i=4;i<11;i++) {
    compiler->save_regs[ORC_GP_REG_BASE+i] = 1;
  }
  
  for(i=0;i<ORC_N_REGS;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->loop_shift = 0;
}

void
arm_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        //arm_emit_loadiw (compiler, compiler->vars[i].alloc,
        //    (int)compiler->vars[i].value);
        break;
      case ORC_VAR_TYPE_PARAM:
        //arm_emit_loadw (compiler, compiler->vars[i].alloc,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]), arm_exec_ptr);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        arm_emit_load_reg (compiler, 
            compiler->vars[i].ptr_register,
            arm_exec_ptr, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        break;
      default:
        break;
    }
  }
}

void
arm_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
    //    arm_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    //case 1:
      //arm_emit_mov_memoffset_reg (compiler, 1, 0, ptr_reg, X86_ECX);
      //arm_emit_mov_reg_arm (compiler, X86_ECX, var->alloc);
      break;
    case 2:
      arm_loadw (compiler, var->alloc, ptr_reg, 0);
      //arm_emit_mov_memoffset_reg (compiler, 2, 0, ptr_reg, X86_ECX);
      //arm_emit_mov_reg_arm (compiler, X86_ECX, var->alloc);
      break;
    //case 4:
      //arm_emit_mov_memoffset_arm (compiler, 4, 0, ptr_reg, var->alloc);
      break;
    //case 8:
      //arm_emit_mov_memoffset_arm (compiler, 8, 0, ptr_reg, var->alloc);
      break;
    //case 16:
      //arm_emit_mov_memoffset_arm (compiler, 16, 0, ptr_reg, var->alloc);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size %d\n", var->size << compiler->loop_shift);
  }
}

void
arm_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    var->ptr_offset, arm_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      //arm_emit_mov_arm_reg (compiler, var->alloc, X86_ECX);
      //arm_emit_mov_reg_memoffset (compiler, 1, X86_ECX, 0, ptr_reg);
      break;
    case 2:
      arm_storew (compiler, ptr_reg, 0, var->alloc);
      //arm_emit_mov_arm_reg (compiler, var->alloc, X86_ECX);
      //arm_emit_mov_reg_memoffset (compiler, 2, X86_ECX, 0, ptr_reg);
      break;
    case 4:
      //arm_emit_mov_arm_memoffset (compiler, 4, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    case 8:
      //arm_emit_mov_arm_memoffset (compiler, 8, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    case 16:
      //arm_emit_mov_arm_memoffset (compiler, 16, var->alloc, 0, ptr_reg,
      //    var->is_aligned, var->is_uncached);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size %d\n", var->size << compiler->loop_shift);
  }
}

void
orc_compiler_arm_assemble (OrcCompiler *compiler)
{
  int dest_var = orc_compiler_get_dest (compiler);

  compiler->vars[dest_var].is_aligned = FALSE;

  arm_emit_prologue (compiler);

  arm_emit_load_reg (compiler, ARM_IP, arm_exec_ptr,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  arm_load_constants (compiler);

  arm_emit_label (compiler, 1);

  arm_emit_cmp_imm (compiler, ARM_IP, 0);
  arm_emit_branch (compiler, ARM_COND_EQ, 3);

  arm_emit_label (compiler, 2);
  arm_emit_loop (compiler);
  arm_emit_sub_imm (compiler, ARM_IP, ARM_IP, 1);
  arm_emit_cmp_imm (compiler, ARM_IP, 0);
  arm_emit_branch (compiler, ARM_COND_NE, 2);
  arm_emit_label (compiler, 3);

  arm_emit_epilogue (compiler);

  arm_do_fixups (compiler);
}

void
arm_emit_loop (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    orc_compiler_append_code(compiler,"# %d: %s", j, insn->opcode->name);

    /* set up args */
#if 0
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = compiler->vars + insn->args[k];
      orc_compiler_append_code(compiler," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        orc_compiler_append_code(compiler," (chained)");
      }
    }
#endif
    orc_compiler_append_code(compiler,"\n");

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      if (opcode->src_size[k] == 0) continue;

      switch (compiler->vars[insn->src_args[k]].vartype) {
        case ORC_VAR_TYPE_SRC:
          arm_emit_load_src (compiler, &compiler->vars[insn->src_args[k]]);
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
      if (compiler->vars[insn->dest_args[0]].alloc !=
          compiler->vars[insn->src_args[0]].alloc) {
        arm_emit_mov (compiler, compiler->vars[insn->src_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc);
      }
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_append_code(compiler,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      if (opcode->dest_size[k] == 0) continue;

      switch (compiler->vars[insn->dest_args[k]].vartype) {
        case ORC_VAR_TYPE_DEST:
          arm_emit_store_dest (compiler, &compiler->vars[insn->dest_args[k]]);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (compiler->vars[k].name == NULL) continue;
    if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
        compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (compiler->vars[k].ptr_register) {
        arm_emit_add_imm (compiler,
            compiler->vars[k].ptr_register,
            compiler->vars[k].ptr_register,
            compiler->vars[k].size << compiler->loop_shift);
      } else {
        //arm_emit_add_imm_memoffset (compiler, arm_ptr_size,
        //    compiler->vars[k].size << compiler->loop_shift,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
        //    arm_exec_ptr);
      }
    }
  }
}

