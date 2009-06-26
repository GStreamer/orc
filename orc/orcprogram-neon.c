
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcarm.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#include "neon.h"

#define SIZE 65536

void orc_neon_emit_loop (OrcCompiler *compiler);

void orc_compiler_neon_register_rules (OrcTarget *target);
unsigned int orc_compiler_neon_get_default_flags (void);

void orc_compiler_neon_init (OrcCompiler *compiler);
void orc_compiler_neon_assemble (OrcCompiler *compiler);

void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);
void orc_neon_save_accumulators (OrcCompiler *compiler);


void
orc_neon_emit_prologue (OrcCompiler *compiler)
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
  if (regs) orc_arm_emit_push (compiler, regs);

}

void
orc_neon_dump_insns (OrcCompiler *compiler)
{

  orc_arm_emit_label (compiler, 0);

  orc_arm_emit_add (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_A4);
  orc_arm_emit_sub (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_A4);
  orc_arm_emit_push (compiler, 0x06);
  orc_arm_emit_mov (compiler, ORC_ARM_A2, ORC_ARM_A3);

  orc_arm_emit_branch (compiler, ORC_ARM_COND_LE, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, 0);

  orc_arm_emit_load_imm (compiler, ORC_ARM_A3, 0xa500);
  orc_arm_loadw (compiler, ORC_ARM_A3, ORC_ARM_A4, 0xa5);
  orc_arm_emit_load_reg (compiler, ORC_ARM_A3, ORC_ARM_A4, 0x5a5);
}

void
orc_neon_emit_epilogue (OrcCompiler *compiler)
{
  int i;
  unsigned int regs = 0;

  for(i=0;i<16;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) orc_arm_emit_pop (compiler, regs);
  orc_arm_emit_bx_lr (compiler);

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
  orc_compiler_neon_get_default_flags,
  orc_compiler_neon_init,
  orc_compiler_neon_assemble
};

void
orc_neon_init (void)
{
  orc_target_register (&neon_target);

  orc_compiler_neon_register_rules (&neon_target);
}

unsigned int
orc_compiler_neon_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_neon_init (OrcCompiler *compiler)
{
  int i;
  int loop_shift;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
    compiler->valid_regs[i] = 1;
  }
  for(i=ORC_VEC_REG_BASE+0;i<ORC_VEC_REG_BASE+32;i+=2){
    compiler->valid_regs[i] = 1;
  }
  //compiler->valid_regs[ORC_ARM_SB] = 0;
  compiler->valid_regs[ORC_ARM_IP] = 0;
  compiler->valid_regs[ORC_ARM_SP] = 0;
  compiler->valid_regs[ORC_ARM_LR] = 0;
  compiler->valid_regs[ORC_ARM_PC] = 0;
  for(i=4;i<12;i++) {
    compiler->save_regs[ORC_GP_REG_BASE+i] = 1;
  }
  
  for(i=0;i<ORC_N_REGS;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->exec_reg = ORC_ARM_A1;
  compiler->valid_regs[compiler->exec_reg] = 0;
  compiler->gp_tmpreg = ORC_ARM_A2;
  compiler->valid_regs[compiler->gp_tmpreg] = 0;
  compiler->tmpreg = ORC_VEC_REG_BASE + 0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  loop_shift = 0;
  switch (orc_program_get_max_var_size (compiler->program)) {
    case 1:
      compiler->loop_shift = 4;
      break;
    case 2:
      compiler->loop_shift = 3;
      break;
    case 4:
      compiler->loop_shift = 2;
      break;
    case 8:
      compiler->loop_shift = 1;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }

  switch (orc_program_get_max_array_size (compiler->program)) {
    case 1:
      loop_shift = 3;
      break;
    case 2:
      loop_shift = 2;
      break;
    case 4:
      loop_shift = 1;
      break;
    case 8:
      loop_shift = 0;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }
  if (loop_shift < compiler->loop_shift) {
    compiler->loop_shift = loop_shift;
  }

  switch (orc_program_get_max_accumulator_size (compiler->program)) {
    case 0:
      loop_shift = 4;
      break;
    case 1:
      loop_shift = 3;
      break;
    case 2:
      loop_shift = 2;
      break;
    case 4:
      loop_shift = 1;
      break;
    case 8:
      loop_shift = 0;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }
  if (loop_shift < compiler->loop_shift) {
    compiler->loop_shift = loop_shift;
  }

  compiler->need_mask_regs = TRUE;
}

void
orc_neon_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;

    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        if (compiler->vars[i].size == 1) {
          orc_neon_emit_loadib (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 2) {
          orc_neon_emit_loadiw (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 4) {
          orc_neon_emit_loadil (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else {
          ORC_PROGRAM_ERROR(compiler,"unimplemented");
        }
        break;
      case ORC_VAR_TYPE_PARAM:
        if (compiler->vars[i].size == 1) {
          orc_neon_emit_loadpb (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 2) {
          orc_neon_emit_loadpw (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 4) {
          orc_neon_emit_loadpl (compiler, compiler->vars[i].alloc, i);
        } else {
          ORC_PROGRAM_ERROR(compiler,"unimplemented");
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_arm_emit_load_reg (compiler, 
            compiler->vars[i].ptr_register,
            compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_neon_emit_loadil (compiler, compiler->vars[i].alloc, 0);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler,"bad vartype");
        break;
    }
  }
}

void
orc_neon_load_alignment_masks (OrcCompiler *compiler)
{
  int i;
  //int j;
  unsigned int code;
  int size;
  int b = 0;

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = &compiler->vars[i];

    if (var->name == NULL) continue;

    switch (var->vartype) {
      case ORC_VAR_TYPE_SRC:
        if (var->is_aligned) continue;

        if (compiler->loop_shift > 1) {
          int j;

          size = var->size << compiler->loop_shift;

          orc_arm_emit_and_imm (compiler, compiler->gp_tmpreg,
              var->ptr_register, size-1);
          orc_arm_emit_lsl_imm (compiler, compiler->gp_tmpreg,
              compiler->gp_tmpreg, 3);

          if (compiler->target_flags & ORC_TARGET_NEON_CLEAN_COMPILE) {
            for(j=0;j<size;j++){
              ORC_ASM_CODE(compiler,"  vmov.8 %s[%d], %s\n",
                  orc_neon_reg_name (var->mask_alloc), j,
                  orc_arm_reg_name (compiler->gp_tmpreg));
              code = 0xee400b10;
              code |= (var->mask_alloc&0xf)<<16;
              code |= ((var->mask_alloc>>4)&0x1)<<7;
              code |= (compiler->gp_tmpreg&0xf)<<12;
              code |= (j&3)<<5;
              code |= (j>>2)<<21;
              orc_arm_emit (compiler, code);

              orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
                  compiler->gp_tmpreg, 1);
            }
          } else {
            orc_arm_emit_align (compiler, 3);
            orc_arm_emit_add (compiler, compiler->gp_tmpreg,
                compiler->gp_tmpreg, ORC_ARM_PC);
            orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
                compiler->gp_tmpreg, 16-8);

            if (size != 4 && size != 8) {
              ORC_ERROR("strange size %d", size);
            }

            ORC_ASM_CODE(compiler, "  vld1.64 %s, [%s]\n",
                orc_neon_reg_name (var->mask_alloc),
                orc_arm_reg_name (compiler->gp_tmpreg));
            code = 0xf42007cf;
            code |= (compiler->gp_tmpreg&0xf) << 16;
            code |= (var->mask_alloc&0xf) << 12;
            code |= ((var->mask_alloc>>4)&0x1) << 22;
            orc_arm_emit (compiler, code);

            orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, 8+b);
            for(j=0;j<8;j++){
              ORC_ASM_CODE(compiler, "  .word 0x%02x%02x%02x%02x\n", j+3, j+2, j+1, j+0);
              orc_arm_emit (compiler, ((j+0)<<0) | ((j+1)<<8) | ((j+2)<<16) | ((j+3)<<24));
              ORC_ASM_CODE(compiler, "  .word 0x%02x%02x%02x%02x\n", j+7, j+6, j+5, j+4);
              orc_arm_emit (compiler, ((j+4)<<0) | ((j+5)<<8) | ((j+6)<<16) | ((j+7)<<24));
            }
            orc_arm_emit_label (compiler, 8+b);
            b++;

          }

          orc_arm_emit_and_imm (compiler, var->ptr_offset, var->ptr_register,
              size - 1);
          orc_arm_emit_sub (compiler, var->ptr_register, var->ptr_register,
              var->ptr_offset);

          if (size == 4) {
            int update = 1;
            ORC_ASM_CODE(compiler,"  vld1.32 %s[0], [%s]%s\n",
                orc_neon_reg_name (var->aligned_data),
                orc_arm_reg_name (var->ptr_register),
                update ? "!" : "");
            code = 0xf4a0080d;
            code |= (var->ptr_register&0xf) << 16;
            code |= ((var->aligned_data)&0xf) << 12;
            code |= (((var->aligned_data)>>4)&0x1) << 22;
            code |= (!update) << 1;
            orc_arm_emit (compiler, code);
          } else {
            int update = 1;
            ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
                orc_neon_reg_name (var->aligned_data),
                orc_arm_reg_name (var->ptr_register),
                update ? "!" : "");
            code = 0xf42007cd;
            code |= (var->ptr_register&0xf) << 16;
            code |= ((var->aligned_data)&0xf) << 12;
            code |= (((var->aligned_data)>>4)&0x1) << 22;
            code |= (!update) << 1;
            orc_arm_emit (compiler, code);
          }
        }

        break;
      case ORC_VAR_TYPE_DEST:
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler,"bad vartype");
        break;
    }
  }
}

void
orc_neon_restore_unalignment (OrcCompiler *compiler)
{
  int i;
  int size;

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = &compiler->vars[i];

    if (var->name == NULL) continue;

    switch (var->vartype) {
      case ORC_VAR_TYPE_SRC:
        if (var->is_aligned) continue;

        if (compiler->loop_shift > 1) {
          size = var->size << compiler->loop_shift;

          orc_arm_emit_add (compiler, var->ptr_register, var->ptr_register,
              var->ptr_offset);
          orc_arm_emit_sub_imm (compiler, var->ptr_register, var->ptr_register,
              size);
        }
        break;
      case ORC_VAR_TYPE_DEST:
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler,"bad vartype");
        break;
    }
  }
}

void
orc_neon_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  int update;

  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
    //    p->exec_reg, X86_ECX);
    ptr_reg = ORC_ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  if (var->vartype == ORC_VAR_TYPE_DEST) {
    update = FALSE;
  } else {
    update = TRUE;
  }
  switch (var->size) {
    case 1:
      orc_neon_loadb (compiler, var, update);
      break;
    case 2:
      orc_neon_loadw (compiler, var, update);
      break;
    case 4:
      orc_neon_loadl (compiler, var, update);
      break;
    case 8:
      orc_neon_loadq (compiler, var->alloc, ptr_reg, update, var->is_aligned);
      break;
    default:
      ORC_ERROR("bad size");
  }
}

void
orc_neon_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    //arm_emit_mov_memoffset_reg (compiler, arm_ptr_size,
    //    var->ptr_offset, p->exec_reg, X86_ECX);
    ptr_reg = ORC_ARM_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size) {
    case 1:
      orc_neon_storeb (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
      break;
    case 2:
      orc_neon_storew (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
      break;
    case 4:
      orc_neon_storel (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
      break;
    case 8:
      orc_neon_storeq (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
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
    case 8:
      return 3;
    default:
      ORC_ERROR("bad size %d", size);
  }
  return -1;
}

static int
get_align_var (OrcCompiler *compiler)
{
  if (compiler->vars[ORC_VAR_D1].size) return ORC_VAR_D1;
  if (compiler->vars[ORC_VAR_S1].size) return ORC_VAR_S1;

  ORC_PROGRAM_ERROR(compiler, "could not find alignment variable");

  return -1;
}

void
orc_compiler_neon_assemble (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  int var_size_shift;
  
  align_var = get_align_var (compiler);
  var_size_shift = get_shift (compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  compiler->vars[align_var].is_aligned = FALSE;

  orc_neon_emit_prologue (compiler);

  if (compiler->loop_shift > 0) {
    int align_shift = 3;

    orc_arm_emit_load_imm (compiler, ORC_ARM_IP, 1<<align_shift);

    orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
    orc_arm_emit_load_reg (compiler, ORC_ARM_A2, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[align_var]));
    orc_arm_emit_sub (compiler, ORC_ARM_IP, ORC_ARM_IP, ORC_ARM_A2);
    orc_arm_emit_and_imm (compiler, ORC_ARM_IP, ORC_ARM_IP,
        (1<<align_shift)-1);
    if (var_size_shift > 0) {
      orc_arm_emit_asr_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, var_size_shift);
    }

    orc_arm_emit_cmp (compiler, ORC_ARM_A3, ORC_ARM_IP);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_LE, 6);

    orc_arm_emit_store_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
    orc_arm_emit_sub (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_IP);

    orc_arm_emit_asr_imm (compiler, ORC_ARM_A3, ORC_ARM_A2, compiler->loop_shift);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    orc_arm_emit_and_imm (compiler, ORC_ARM_A3, ORC_ARM_A2, (1<<compiler->loop_shift)-1);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, 7);
    orc_arm_emit_label (compiler, 6);

    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_arm_emit_load_imm (compiler, ORC_ARM_A3, 0);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_label (compiler, 7);
  }

  orc_neon_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, 1);

    orc_arm_emit_label (compiler, 0);
    orc_neon_emit_loop (compiler);
    orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1);
    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, 0);
    orc_arm_emit_label (compiler, 1);

    compiler->loop_shift = save_loop_shift;
    compiler->vars[align_var].is_aligned = TRUE;
  }

  if (compiler->loop_shift > 0) {
    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
  } else {
    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  }

  orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, 3);

  orc_neon_load_alignment_masks (compiler);

  orc_arm_emit_label (compiler, 2);
  orc_neon_emit_loop (compiler);
  orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1);
  orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, 2);

  orc_neon_restore_unalignment (compiler);

  orc_arm_emit_label (compiler, 3);


  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    compiler->vars[align_var].is_aligned = FALSE;

    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, 5);

    orc_arm_emit_label (compiler, 4);
    orc_neon_emit_loop (compiler);
    orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1);
    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, 4);
    orc_arm_emit_label (compiler, 5);

    compiler->loop_shift = save_loop_shift;
  }

  orc_neon_save_accumulators (compiler);

  orc_neon_emit_epilogue (compiler);

  orc_arm_emit_align (compiler, 3);

  orc_arm_do_fixups (compiler);

  orc_arm_flush_cache (compiler);
}

void
orc_neon_emit_loop (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    compiler->insn_index = j;
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
        case ORC_VAR_TYPE_DEST:
          orc_neon_emit_load_src (compiler, &compiler->vars[insn->src_args[k]]);
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
#if 0
      if (compiler->vars[insn->dest_args[0]].alloc !=
          compiler->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (compiler, compiler->vars[insn->dest_args[0]].alloc,
            compiler->vars[insn->src_args[0]].alloc);
      }
#endif
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_append_code(compiler,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      if (opcode->dest_size[k] == 0) continue;

      switch (compiler->vars[insn->dest_args[k]].vartype) {
        case ORC_VAR_TYPE_DEST:
          orc_neon_emit_store_dest (compiler, &compiler->vars[insn->dest_args[k]]);
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
        orc_arm_emit_add_imm (compiler,
            compiler->vars[k].ptr_register,
            compiler->vars[k].ptr_register,
            compiler->vars[k].size << compiler->loop_shift);
      } else {
        //arm_emit_add_imm_memoffset (compiler, arm_ptr_size,
        //    compiler->vars[k].size << compiler->loop_shift,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
        //    p->exec_reg);
      }
    }
  }
#endif
}

#define NEON_BINARY(code,a,b,c) \
  ((code) | \
   (((a)&0xf)<<12) | \
   ((((a)>>4)&0x1)<<22) | \
   (((b)&0xf)<<16) | \
   ((((b)>>4)&0x1)<<7) | \
   (((c)&0xf)<<0) | \
   ((((c)>>4)&0x1)<<5))

void
orc_neon_save_accumulators (OrcCompiler *compiler)
{
  int i;
  int src;
  unsigned int code;

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        src = compiler->vars[i].alloc;

        orc_arm_emit_load_imm (compiler, compiler->gp_tmpreg,
            ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]));
        orc_arm_emit_add (compiler, compiler->gp_tmpreg,
            compiler->gp_tmpreg, compiler->exec_reg);
        switch (var->size) {
          case 2:
            if (compiler->loop_shift > 0) {
              ORC_ASM_CODE(compiler,"  vpaddl.u16 %s, %s\n",
                  orc_neon_reg_name (src),
                  orc_neon_reg_name (src));
              code = 0xf3b40280;
              code |= (src&0xf) << 12;
              code |= ((src>>4)&0x1) << 22;
              code |= (src&0xf) << 0;
              orc_arm_emit (compiler, code);

              ORC_ASM_CODE(compiler,"  vpaddl.u32 %s, %s\n",
                  orc_neon_reg_name (src),
                  orc_neon_reg_name (src));
              code = 0xf3b80280;
              code |= (src&0xf) << 12;
              code |= ((src>>4)&0x1) << 22;
              code |= (src&0xf) << 0;
              orc_arm_emit (compiler, code);
            }

            ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s]\n",
                orc_neon_reg_name (src), 0,
                orc_arm_reg_name (compiler->gp_tmpreg));
            code = 0xf480040f;
            code |= (compiler->gp_tmpreg&0xf) << 16;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            orc_arm_emit (compiler, code);
            break;
          case 4:
            if (compiler->loop_shift > 0) {
              ORC_ASM_CODE(compiler,"  vpadd.u32 %s, %s, %s\n",
                  orc_neon_reg_name (src),
                  orc_neon_reg_name (src),
                  orc_neon_reg_name (src));
              code = NEON_BINARY(0xf2200b10, src, src, src);
              orc_arm_emit (compiler, code);
            }

            ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s]\n",
                orc_neon_reg_name (src), 0,
                orc_arm_reg_name (compiler->gp_tmpreg));
            code = 0xf480080f;
            code |= (compiler->gp_tmpreg&0xf) << 16;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            orc_arm_emit (compiler, code);
            break;
          default:
            ORC_ERROR("bad size");
        }

        break;
      default:
        break;
    }
  }
}

