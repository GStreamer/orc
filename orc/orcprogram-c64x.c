
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcc64x.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#define SIZE 65536

void orc_c64x_emit_loop (OrcCompiler *compiler);

void orc_compiler_c64x_register_rules (OrcTarget *target);
unsigned int orc_compiler_c64x_get_default_flags (void);

void orc_compiler_c64x_init (OrcCompiler *compiler);
void orc_compiler_c64x_assemble (OrcCompiler *compiler);

void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);
void orc_c64x_save_accumulators (OrcCompiler *compiler);


void
orc_c64x_emit_prologue (OrcCompiler *compiler)
{
  unsigned int regs = 0;
  int i;

  orc_compiler_append_code(compiler,"  .global %s\n", compiler->program->name);
  orc_compiler_append_code(compiler,"%s:\n", compiler->program->name);

  for(i=0;i<16;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) orc_c64x_emit_push (compiler, regs);

}

void
orc_c64x_emit_epilogue (OrcCompiler *compiler)
{
  int i;
  unsigned int regs = 0;

  for(i=0;i<16;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }
  if (regs) orc_c64x_emit_pop (compiler, regs);
  orc_c64x_emit_bx_lr (compiler);

  //c64x_dump_insns (compiler);
}

static OrcTarget c64x_target = {
  "c64x",
#ifdef HAVE_C64X
  TRUE,
#else
  FALSE,
#endif
  ORC_GP_REG_BASE,
  orc_compiler_c64x_get_default_flags,
  orc_compiler_c64x_init,
  orc_compiler_c64x_assemble
};

void
orc_c64x_init (void)
{
  orc_target_register (&c64x_target);

  orc_compiler_c64x_register_rules (&c64x_target);
}

unsigned int
orc_compiler_c64x_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_c64x_init (OrcCompiler *compiler)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+32;i++){
    compiler->valid_regs[i] = 1;
  }
  compiler->valid_regs[C64X_SP] = 0;
  compiler->valid_regs[C64X_B3] = 0; /* return register */
  for(i=10;i<16;i++) {
    compiler->save_regs[ORC_GP_REG_BASE+i] = 1;
    compiler->save_regs[ORC_GP_REG_BASE+16+i] = 1;
  }
  
  for(i=0;i<ORC_N_REGS;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->exec_reg = C64X_A4;
  compiler->valid_regs[compiler->exec_reg] = 0;
  compiler->gp_tmpreg = C64X_A0;
  compiler->valid_regs[compiler->gp_tmpreg] = 0;
  compiler->tmpreg = C64X_B0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  switch (orc_program_get_max_var_size (compiler->program)) {
    case 1:
      compiler->loop_shift = 2;
      break;
    case 2:
      compiler->loop_shift = 1;
      break;
    case 4:
      compiler->loop_shift = 0;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }
  compiler->loop_shift = 0;
}

void
orc_c64x_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;

    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        if (compiler->vars[i].size == 1) {
          orc_c64x_emit_loadib (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 2) {
          orc_c64x_emit_loadiw (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 4) {
          orc_c64x_emit_loadil (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else {
          ORC_PROGRAM_ERROR(compiler,"unimplemented");
        }
        break;
      case ORC_VAR_TYPE_PARAM:
        if (compiler->vars[i].size == 1) {
          orc_c64x_emit_loadpb (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 2) {
          orc_c64x_emit_loadpw (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 4) {
          orc_c64x_emit_loadpl (compiler, compiler->vars[i].alloc, i);
        } else {
          ORC_PROGRAM_ERROR(compiler,"unimplemented");
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_c64x_emit_load_reg (compiler, 
            compiler->vars[i].ptr_register,
            compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_c64x_emit_loadil (compiler, compiler->vars[i].alloc, 0);
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
orc_c64x_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  int update;

  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    //c64x_emit_mov_memoffset_reg (compiler, c64x_ptr_size,
    //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
    //    p->exec_reg, X86_ECX);
    ptr_reg = C64X_PC;
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
      orc_c64x_loadb (compiler, var->alloc, ptr_reg, update, var->is_aligned);
      break;
    case 2:
      orc_c64x_loadw (compiler, var->alloc, ptr_reg, update, var->is_aligned);
      break;
    case 4:
      orc_c64x_loadl (compiler, var->alloc, ptr_reg, update, var->is_aligned);
      break;
    default:
      ORC_ERROR("bad size");
  }
}

void
orc_c64x_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    //c64x_emit_mov_memoffset_reg (compiler, c64x_ptr_size,
    //    var->ptr_offset, p->exec_reg, X86_ECX);
    ptr_reg = C64X_PC;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size) {
    case 1:
      orc_c64x_storeb (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
      break;
    case 2:
      orc_c64x_storew (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
      break;
    case 4:
      orc_c64x_storel (compiler, ptr_reg, TRUE, var->alloc, var->is_aligned);
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
orc_compiler_c64x_assemble (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  
  align_var = get_align_var (compiler);
  align_shift = get_shift (compiler->vars[align_var].size);

  compiler->vars[align_var].is_aligned = FALSE;

  orc_c64x_emit_prologue (compiler);

  if (compiler->loop_shift > 0) {
    int align_shift = 3;

    orc_c64x_emit_load_imm (compiler, C64X_IP, 1<<align_shift);

    orc_c64x_emit_load_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
    orc_c64x_emit_load_reg (compiler, C64X_A2, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[align_var]));
    orc_c64x_emit_sub (compiler, C64X_IP, C64X_IP, C64X_A2);
    orc_c64x_emit_and_imm (compiler, C64X_IP, C64X_IP, (1<<align_shift)-1);
    if (align_shift > 0) {
      orc_c64x_emit_asr_imm (compiler, C64X_IP, C64X_IP, align_shift);
    }

    orc_c64x_emit_cmp (compiler, C64X_A3, C64X_IP);
    orc_c64x_emit_branch (compiler, C64X_COND_LE, 6);

    orc_c64x_emit_store_reg (compiler, C64X_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
    orc_c64x_emit_sub (compiler, C64X_A2, C64X_A3, C64X_IP);

    orc_c64x_emit_asr_imm (compiler, C64X_A3, C64X_A2, compiler->loop_shift);
    orc_c64x_emit_store_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    orc_c64x_emit_and_imm (compiler, C64X_A3, C64X_A2, (1<<compiler->loop_shift)-1);
    orc_c64x_emit_store_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_c64x_emit_branch (compiler, C64X_COND_AL, 7);
    orc_c64x_emit_label (compiler, 6);

    orc_c64x_emit_store_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_c64x_emit_load_imm (compiler, C64X_A3, 0);
    orc_c64x_emit_store_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
    orc_c64x_emit_store_reg (compiler, C64X_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_c64x_emit_label (compiler, 7);
  }

  orc_c64x_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_c64x_emit_load_reg (compiler, C64X_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
    orc_c64x_emit_branch (compiler, C64X_COND_EQ, 1);

    orc_c64x_emit_label (compiler, 0);
    orc_c64x_emit_loop (compiler);
    orc_c64x_emit_sub_imm (compiler, C64X_IP, C64X_IP, 1);
    orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
    orc_c64x_emit_branch (compiler, C64X_COND_NE, 0);
    orc_c64x_emit_label (compiler, 1);

    compiler->loop_shift = save_loop_shift;
    compiler->vars[align_var].is_aligned = TRUE;
  }

  if (compiler->loop_shift > 0) {
    orc_c64x_emit_load_reg (compiler, C64X_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
  } else {
    orc_c64x_emit_load_reg (compiler, C64X_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  }

  orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
  orc_c64x_emit_branch (compiler, C64X_COND_EQ, 3);

  orc_c64x_emit_label (compiler, 2);
  orc_c64x_emit_loop (compiler);
  orc_c64x_emit_sub_imm (compiler, C64X_IP, C64X_IP, 1);
  orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
  orc_c64x_emit_branch (compiler, C64X_COND_NE, 2);
  orc_c64x_emit_label (compiler, 3);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    compiler->vars[align_var].is_aligned = FALSE;

    orc_c64x_emit_load_reg (compiler, C64X_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
    orc_c64x_emit_branch (compiler, C64X_COND_EQ, 5);

    orc_c64x_emit_label (compiler, 4);
    orc_c64x_emit_loop (compiler);
    orc_c64x_emit_sub_imm (compiler, C64X_IP, C64X_IP, 1);
    orc_c64x_emit_cmp_imm (compiler, C64X_IP, 0);
    orc_c64x_emit_branch (compiler, C64X_COND_NE, 4);
    orc_c64x_emit_label (compiler, 5);

    compiler->loop_shift = save_loop_shift;
  }

  orc_c64x_save_accumulators (compiler);

  orc_c64x_emit_epilogue (compiler);

  orc_c64x_do_fixups (compiler);
}

void
orc_c64x_emit_loop (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    orc_compiler_append_code(compiler,"; %d: %s", j, insn->opcode->name);

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
          orc_c64x_emit_load_src (compiler, &compiler->vars[insn->src_args[k]]);
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
        orc_c64x_emit_mov (compiler, compiler->vars[insn->src_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc);
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
          orc_c64x_emit_store_dest (compiler, &compiler->vars[insn->dest_args[k]]);
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
        orc_c64x_emit_add_imm (compiler,
            compiler->vars[k].ptr_register,
            compiler->vars[k].ptr_register,
            compiler->vars[k].size << compiler->loop_shift);
      } else {
        //c64x_emit_add_imm_memoffset (compiler, c64x_ptr_size,
        //    compiler->vars[k].size << compiler->loop_shift,
        //    (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
        //    p->exec_reg);
      }
    }
  }
#endif
}

void
orc_c64x_save_accumulators (OrcCompiler *compiler)
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

        orc_c64x_emit_load_imm (compiler, compiler->gp_tmpreg,
            ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]));
        switch (var->size) {
          case 2:
            ORC_ASM_CODE(compiler,"  vpaddl.u16 %s, %s\n",
                orc_c64x_reg_name (src),
                orc_c64x_reg_name (src));
            code = 0xf3b40280;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            code |= (src&0xf) << 0;
            orc_c64x_emit (compiler, code);

            ORC_ASM_CODE(compiler,"  vpaddl.u32 %s, %s\n",
                orc_c64x_reg_name (src),
                orc_c64x_reg_name (src));
            code = 0xf3b80280;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            code |= (src&0xf) << 0;
            orc_c64x_emit (compiler, code);

            ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s], %s\n",
                orc_c64x_reg_name (src), 0,
                orc_c64x_reg_name (compiler->gp_tmpreg),
                orc_c64x_reg_name (compiler->exec_reg));
            code = 0xf4800400;
            code |= (compiler->gp_tmpreg&0xf) << 16;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            orc_c64x_emit (compiler, code);
            break;
          case 4:
            ORC_ASM_CODE(compiler,"  vpaddl.u32 %s, %s\n",
                orc_c64x_reg_name (src),
                orc_c64x_reg_name (src));
            code = 0xf3b80280;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            code |= (src&0xf) << 0;
            orc_c64x_emit (compiler, code);

            ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s], %s\n",
                orc_c64x_reg_name (src), 0,
                orc_c64x_reg_name (compiler->gp_tmpreg),
                orc_c64x_reg_name (compiler->exec_reg));
            code = 0xf4800800;
            code |= (compiler->gp_tmpreg&0xf) << 16;
            code |= (src&0xf) << 12;
            code |= ((src>>4)&0x1) << 22;
            orc_c64x_emit (compiler, code);
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

