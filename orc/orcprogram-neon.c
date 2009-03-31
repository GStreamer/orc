
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

int neon_exec_ptr = ARM_R0;
int neon_tmp_reg = ARM_A2;

void neon_emit_loop (OrcCompiler *compiler);

void orc_compiler_neon_register_rules (OrcTarget *target);

void orc_compiler_neon_init (OrcCompiler *compiler);
void orc_compiler_neon_assemble (OrcCompiler *compiler);

void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);

void neon_loadb (OrcCompiler *compiler, int dest, int src1, int offset);
void neon_loadw (OrcCompiler *compiler, int dest, int src1, int offset);
void neon_loadl (OrcCompiler *compiler, int dest, int src1, int offset);
void neon_neg (OrcCompiler *compiler, int dest);
void neon_storeb (OrcCompiler *compiler, int dest, int offset, int src1);
void neon_storew (OrcCompiler *compiler, int dest, int offset, int src1);
void neon_storel (OrcCompiler *compiler, int dest, int offset, int src1);
void neon_emit_loadiw (OrcCompiler *p, int reg, int value);

void
neon_emit_prologue (OrcCompiler *compiler)
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
neon_dump_insns (OrcCompiler *compiler)
{

  arm_emit_label (compiler, 0);

  arm_emit_add (compiler, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_sub (compiler, ARM_A2, ARM_A3, ARM_A4);
  arm_emit_push (compiler, 0x06);
  arm_emit_mov (compiler, ARM_A2, ARM_A3);

  arm_emit_branch (compiler, ARM_COND_LE, 0);
  arm_emit_branch (compiler, ARM_COND_AL, 0);

  arm_emit_load_imm (compiler, ARM_A3, 0xa500);
  arm_loadw (compiler, ARM_A3, ARM_A4, 0xa5);
  arm_emit_load_reg (compiler, ARM_A3, ARM_A4, 0x5a5);
}

void
neon_emit_epilogue (OrcCompiler *compiler)
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

static OrcTarget neon_target = {
  "neon",
#ifdef HAVE_ARM
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_neon_init,
  orc_compiler_neon_assemble
};

void
orc_neon_init (void)
{
  orc_target_register (&neon_target);

  orc_compiler_neon_register_rules (&neon_target);
}

void
orc_compiler_neon_init (OrcCompiler *compiler)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+9;i++){
    compiler->valid_regs[i] = 1;
  }
  for(i=ORC_VEC_REG_BASE+0;i<ORC_VEC_REG_BASE+32;i+=2){
    compiler->valid_regs[i] = 1;
  }
  compiler->valid_regs[neon_exec_ptr] = 0;
  compiler->valid_regs[neon_tmp_reg] = 0;
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

  switch (orc_program_get_max_var_size (compiler->program)) {
    case 1:
      compiler->loop_shift = 3;
      break;
    case 2:
      compiler->loop_shift = 2;
      break;
    case 4:
      compiler->loop_shift = 1;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }
}

void
neon_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<compiler->n_vars;i++){
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        ORC_ASSERT (compiler->vars[i].size == 2);
        neon_emit_loadiw (compiler, compiler->vars[i].alloc,
            (int)compiler->vars[i].value);
        break;
      case ORC_VAR_TYPE_PARAM:
        arm_emit_add_imm (compiler, neon_tmp_reg,
            neon_exec_ptr, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        neon_loadl (compiler, compiler->vars[i].alloc, neon_exec_ptr, 0);
        neon_neg (compiler, compiler->vars[i].alloc);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        arm_emit_load_reg (compiler, 
            compiler->vars[i].ptr_register,
            neon_exec_ptr, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        break;
      default:
        break;
    }
  }
}

void
neon_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
    //    neon_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size) {
    case 1:
      neon_loadb (compiler, var->alloc, ptr_reg, 0);
      break;
    case 2:
      neon_loadw (compiler, var->alloc, ptr_reg, 0);
      break;
    case 4:
      neon_loadl (compiler, var->alloc, ptr_reg, 0);
      break;
    default:
      ORC_ERROR("bad size");
  }
}

void
neon_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    var->ptr_offset, neon_exec_ptr, X86_ECX);
    ptr_reg = ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size) {
    case 1:
      neon_storeb (compiler, ptr_reg, 0, var->alloc);
      break;
    case 2:
      neon_storew (compiler, ptr_reg, 0, var->alloc);
      break;
    case 4:
      neon_storel (compiler, ptr_reg, 0, var->alloc);
      break;
    default:
      ORC_ERROR("bad size");
  }
}

static int
get_shift (int size)
{
  switch (size) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    default:
      ORC_ERROR("bad size %d", size);
  }
  return -1;
}

void
orc_compiler_neon_assemble (OrcCompiler *compiler)
{
  int dest_var;
  int dest_shift;
  
  dest_var = orc_compiler_get_dest (compiler);
  dest_shift = get_shift (compiler->vars[dest_var].size);

  compiler->vars[dest_var].is_aligned = FALSE;

  neon_emit_prologue (compiler);

  if (compiler->loop_shift > 0) {
    int align_shift = 3;

    arm_emit_load_imm (compiler, ARM_IP, 1<<align_shift);

    arm_emit_load_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
    arm_emit_load_reg (compiler, ARM_A2, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[dest_var]));
    arm_emit_sub (compiler, ARM_IP, ARM_IP, ARM_A2);
    arm_emit_and_imm (compiler, ARM_IP, ARM_IP, (1<<align_shift)-1);
    if (dest_shift > 0) {
      arm_emit_asr_imm (compiler, ARM_IP, ARM_IP, dest_shift);
    }

    arm_emit_cmp (compiler, ARM_A3, ARM_IP);
    arm_emit_branch (compiler, ARM_COND_LE, 6);

    arm_emit_store_reg (compiler, ARM_IP, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
    arm_emit_sub (compiler, ARM_A2, ARM_A3, ARM_IP);

    arm_emit_asr_imm (compiler, ARM_A3, ARM_A2, compiler->loop_shift);
    arm_emit_store_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    arm_emit_and_imm (compiler, ARM_A3, ARM_A2, (1<<compiler->loop_shift)-1);
    arm_emit_store_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    arm_emit_branch (compiler, ARM_COND_AL, 7);
    arm_emit_label (compiler, 6);

    arm_emit_store_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    arm_emit_load_imm (compiler, ARM_A3, 0);
    arm_emit_store_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
    arm_emit_store_reg (compiler, ARM_A3, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    arm_emit_label (compiler, 7);
  }

  neon_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    arm_emit_load_reg (compiler, ARM_IP, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    arm_emit_cmp_imm (compiler, ARM_IP, 0);
    arm_emit_branch (compiler, ARM_COND_EQ, 3);

    arm_emit_label (compiler, 0);
    neon_emit_loop (compiler);
    arm_emit_sub_imm (compiler, ARM_IP, ARM_IP, 1);
    arm_emit_cmp_imm (compiler, ARM_IP, 0);
    arm_emit_branch (compiler, ARM_COND_NE, 0);
    arm_emit_label (compiler, 1);

    compiler->loop_shift = save_loop_shift;
  }

  if (compiler->loop_shift > 0) {
    arm_emit_load_reg (compiler, ARM_IP, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
  } else {
    arm_emit_load_reg (compiler, ARM_IP, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  }

  arm_emit_cmp_imm (compiler, ARM_IP, 0);
  arm_emit_branch (compiler, ARM_COND_EQ, 3);

  arm_emit_label (compiler, 2);
  neon_emit_loop (compiler);
  arm_emit_sub_imm (compiler, ARM_IP, ARM_IP, 1);
  arm_emit_cmp_imm (compiler, ARM_IP, 0);
  arm_emit_branch (compiler, ARM_COND_NE, 2);
  arm_emit_label (compiler, 3);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    arm_emit_load_reg (compiler, ARM_IP, neon_exec_ptr,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    arm_emit_cmp_imm (compiler, ARM_IP, 0);
    arm_emit_branch (compiler, ARM_COND_EQ, 5);

    arm_emit_label (compiler, 4);
    neon_emit_loop (compiler);
    arm_emit_sub_imm (compiler, ARM_IP, ARM_IP, 1);
    arm_emit_cmp_imm (compiler, ARM_IP, 0);
    arm_emit_branch (compiler, ARM_COND_NE, 4);
    arm_emit_label (compiler, 5);

    compiler->loop_shift = save_loop_shift;
  }

  neon_emit_epilogue (compiler);

  arm_do_fixups (compiler);
}

void
neon_emit_loop (OrcCompiler *compiler)
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
          neon_emit_load_src (compiler, &compiler->vars[insn->src_args[k]]);
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
          neon_emit_store_dest (compiler, &compiler->vars[insn->dest_args[k]]);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

#if 0
  for(k=0;k<compiler->n_vars;k++){
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
        //    neon_exec_ptr);
      }
    }
  }
#endif
}

