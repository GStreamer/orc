
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orccpu.h>

#define SIZE 65536


void orc_sse_emit_loop (OrcCompiler *compiler);

void orc_compiler_sse_init (OrcCompiler *compiler);
unsigned int orc_compiler_sse_get_default_flags (void);
void orc_compiler_sse_assemble (OrcCompiler *compiler);
void orc_compiler_sse_register_rules (OrcTarget *target);


void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);

static OrcTarget sse_target = {
  "sse",
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_sse_get_default_flags,
  orc_compiler_sse_init,
  orc_compiler_sse_assemble
};


void
orc_sse_init (void)
{
  orc_target_register (&sse_target);

  orc_compiler_sse_register_rules (&sse_target);
}

unsigned int
orc_compiler_sse_get_default_flags (void)
{
  unsigned int flags = 0;

#ifdef HAVE_AMD64
  flags |= ORC_TARGET_SSE_64BIT;
#endif
  flags &= ~ORC_TARGET_SSE_FRAME_POINTER;
  flags |= ORC_TARGET_SSE_SHORT_JUMPS;
  
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  flags = orc_sse_get_cpu_flags ();
#else
  flags = ORC_TARGET_SSE_SSE2;
  flags |= ORC_TARGET_SSE_SSE3;
  flags |= ORC_TARGET_SSE_SSSE3;
#endif

  return flags;
}

void
orc_compiler_sse_init (OrcCompiler *compiler)
{
  int i;

  if (compiler->target_flags & ORC_TARGET_SSE_64BIT) {
    compiler->is_64bit = TRUE;
  }
  if (compiler->target_flags & ORC_TARGET_SSE_FRAME_POINTER) {
    compiler->use_frame_pointer = TRUE;
  }
  if (!(compiler->target_flags & ORC_TARGET_SSE_SHORT_JUMPS)) {
    compiler->long_jumps = TRUE;
  }
  

  if (compiler->is_64bit) {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_EDI] = 0;
    compiler->valid_regs[X86_ESP] = 0;
    for(i=X86_XMM0;i<X86_XMM0+16;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EBP] = 1;
    compiler->save_regs[X86_R12] = 1;
    compiler->save_regs[X86_R13] = 1;
    compiler->save_regs[X86_R14] = 1;
    compiler->save_regs[X86_R15] = 1;
  } else {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+8;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_ESP] = 0;
    if (compiler->use_frame_pointer) {
      compiler->valid_regs[X86_EBP] = 0;
    }
    for(i=X86_XMM0;i<X86_XMM0+8;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EDI] = 1;
    compiler->save_regs[X86_EBP] = 1;
  }
  for(i=X86_MM0;i<X86_MM0+8;i++){
    compiler->valid_regs[i] = 1;
  }
  for(i=0;i<128;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->tmpreg = X86_XMM0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  compiler->gp_tmpreg = X86_ECX;
  compiler->valid_regs[compiler->gp_tmpreg] = 0;

  if (compiler->is_64bit) {
    compiler->exec_reg = X86_EDI;
  } else {
    if (compiler->use_frame_pointer) {
      compiler->exec_reg = X86_EBX;
    } else {
      compiler->exec_reg = X86_EBP;
    }
  }
  compiler->valid_regs[compiler->exec_reg] = 0;

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

}

void
sse_save_accumulators (OrcCompiler *compiler)
{
  int i;
  int src;

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        src = compiler->vars[i].alloc;

        orc_sse_emit_pshufd (compiler, 0xee, src, compiler->tmpreg);

        if (compiler->vars[i].size == 2) {
          orc_sse_emit_660f (compiler, "paddw", 0xfd, compiler->tmpreg, src);
        } else {
          orc_sse_emit_660f (compiler, "paddd", 0xfe, compiler->tmpreg, src);
        }

        orc_sse_emit_pshufd (compiler, 0x55, src, compiler->tmpreg);

        if (compiler->vars[i].size == 2) {
          orc_sse_emit_660f (compiler, "paddw", 0xfd, compiler->tmpreg, src);
        } else {
          orc_sse_emit_660f (compiler, "paddd", 0xfe, compiler->tmpreg, src);
        }

        if (compiler->vars[i].size == 2) {
          orc_sse_emit_pshuflw (compiler, 0x55, src, compiler->tmpreg);

          orc_sse_emit_660f (compiler, "paddw", 0xfd, compiler->tmpreg, src);
        }

        if (compiler->vars[i].size == 2) {
          orc_x86_emit_mov_sse_reg (compiler, src, X86_ECX);
          orc_x86_emit_mov_reg_memoffset (compiler, 2, X86_ECX,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
              compiler->exec_reg);
        } else {
          orc_x86_emit_mov_sse_memoffset (compiler, 4, src,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
              compiler->exec_reg,
              var->is_aligned, var->is_uncached);
        }

        break;
      default:
        break;
    }
  }
}

void
sse_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        if (compiler->vars[i].size == 1) {
          orc_sse_emit_loadib (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 2) {
          orc_sse_emit_loadiw (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 4) {
          orc_sse_emit_loadil (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else {
          ORC_COMPILER_ERROR(compiler, "unimplemented");
        }
        break;
      case ORC_VAR_TYPE_PARAM:
        if (compiler->vars[i].size == 1) {
          orc_sse_emit_loadpb (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 2) {
          orc_sse_emit_loadpw (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 4) {
          orc_sse_emit_loadpl (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 8) {
          orc_sse_emit_loadpq (compiler, compiler->vars[i].alloc, i);
        } else {
          ORC_COMPILER_ERROR(compiler, "unimplemented");
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), compiler->exec_reg,
              compiler->vars[i].ptr_register);
        } else {
          ORC_COMPILER_ERROR(compiler,"unimplemented");
        }
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_sse_emit_660f (compiler, "pxor", 0xef,
            compiler->vars[i].alloc, compiler->vars[i].alloc);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_COMPILER_ERROR(compiler,"bad vartype");
        break;
    }
  }
}

void
orc_sse_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      orc_x86_emit_mov_memoffset_reg (compiler, 1, 0, ptr_reg, X86_ECX);
      orc_x86_emit_mov_reg_sse (compiler, X86_ECX, var->alloc);
      break;
    case 2:
      orc_x86_emit_mov_memoffset_reg (compiler, 2, 0, ptr_reg, X86_ECX);
      orc_x86_emit_mov_reg_sse (compiler, X86_ECX, var->alloc);
      break;
    case 4:
      orc_x86_emit_mov_memoffset_sse (compiler, 4, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_sse (compiler, 8, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_sse (compiler, 16, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    default:
      ORC_COMPILER_ERROR(compiler,"bad load size %d",
          var->size << compiler->loop_shift);
      break;
  }
}

void
orc_sse_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        var->ptr_offset, compiler->exec_reg, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        ORC_COMPILER_ERROR(compiler,"unimplemented");
      }
      orc_x86_emit_mov_sse_reg (compiler, var->alloc, X86_ECX);
      orc_x86_emit_mov_reg_memoffset (compiler, 1, X86_ECX, 0, ptr_reg);
      break;
    case 2:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        ORC_COMPILER_ERROR(compiler,"unimplemented");
      }
      orc_x86_emit_mov_sse_reg (compiler, var->alloc, X86_ECX);
      orc_x86_emit_mov_reg_memoffset (compiler, 2, X86_ECX, 0, ptr_reg);
      break;
    case 4:
      orc_x86_emit_mov_sse_memoffset (compiler, 4, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 8:
      orc_x86_emit_mov_sse_memoffset (compiler, 8, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 16:
      orc_x86_emit_mov_sse_memoffset (compiler, 16, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    default:
      ORC_COMPILER_ERROR(compiler,"bad size");
      break;
  }
}

static int
get_align_var (OrcCompiler *compiler)
{
  if (compiler->vars[ORC_VAR_D1].size) return ORC_VAR_D1;
  if (compiler->vars[ORC_VAR_S1].size) return ORC_VAR_S1;

  ORC_COMPILER_ERROR(compiler, "could not find alignment variable");

  return -1;
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

void
orc_compiler_sse_assemble (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;

  align_var = get_align_var (compiler);
  align_shift = get_shift (compiler->vars[align_var].size);

  compiler->vars[align_var].is_aligned = FALSE;

  orc_x86_emit_prologue (compiler);

  if (compiler->loop_shift > 0) {
    orc_x86_emit_mov_imm_reg (compiler, 4, 16, X86_EAX);
    orc_x86_emit_sub_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[align_var]),
        compiler->exec_reg, X86_EAX);
    orc_x86_emit_and_imm_reg (compiler, 4, 15, X86_EAX);
    orc_x86_emit_sar_imm_reg (compiler, 4, align_shift, X86_EAX);

    orc_x86_emit_cmp_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg);

    orc_x86_emit_jle (compiler, 6);

    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
    
    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg, X86_ECX);
    orc_x86_emit_sub_reg_reg (compiler, 4, X86_EAX, X86_ECX);

    orc_x86_emit_mov_reg_reg (compiler, 4, X86_ECX, X86_EAX);

    orc_x86_emit_sar_imm_reg (compiler, 4, compiler->loop_shift, X86_ECX);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);

    orc_x86_emit_and_imm_reg (compiler, 4, (1<<compiler->loop_shift)-1, X86_EAX);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), compiler->exec_reg);

    orc_x86_emit_jmp (compiler, 7);
    orc_x86_emit_label (compiler, 6);

    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg, X86_EAX);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
    orc_x86_emit_mov_imm_reg (compiler, 4, 0, X86_EAX);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), compiler->exec_reg);

    orc_x86_emit_label (compiler, 7);
  } else {
    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg, X86_ECX);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
  }

  sse_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift;

    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
    orc_x86_emit_je (compiler, 1);

    save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_x86_emit_label (compiler, 0);
    orc_sse_emit_loop (compiler);
    orc_x86_emit_dec_memoffset (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, 0);

    compiler->loop_shift = save_loop_shift;
    compiler->vars[align_var].is_aligned = TRUE;
  }

  orc_x86_emit_label (compiler, 1);

  orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
  orc_x86_emit_je (compiler, 3);

  orc_x86_emit_align (compiler);
  orc_x86_emit_label (compiler, 2);
  orc_sse_emit_loop (compiler);
  orc_x86_emit_dec_memoffset (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2),
      compiler->exec_reg);
  orc_x86_emit_jne (compiler, 2);
  orc_x86_emit_label (compiler, 3);

  if (compiler->loop_shift > 0) {
    int save_loop_shift;

    compiler->vars[align_var].is_aligned = FALSE;
    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), compiler->exec_reg);
    orc_x86_emit_je (compiler, 5);

    save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_x86_emit_label (compiler, 4);
    orc_sse_emit_loop (compiler);
    orc_x86_emit_dec_memoffset (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, 4);

    orc_x86_emit_label (compiler, 5);

    compiler->loop_shift = save_loop_shift;
  }

  sse_save_accumulators (compiler);

  orc_x86_emit_epilogue (compiler);

  x86_do_fixups (compiler);
}

void
orc_sse_emit_loop (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  //OrcVariable *args[10];
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(compiler,"# %d: %s\n", j, insn->opcode->name);

#if 0
    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = compiler->vars + insn->args[k];
      ORC_ASM_CODE(compiler," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        ORC_ASM_CODE(compiler," (chained)");
      }
    }
    ORC_ASM_CODE(compiler,"\n");
#endif

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      OrcVariable *var = compiler->vars + insn->src_args[k];

      if (opcode->src_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
        case ORC_VAR_TYPE_DEST:
          orc_sse_emit_load_src (compiler, var);
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
      if (!(insn->opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) &&
          compiler->vars[insn->dest_args[0]].alloc !=
          compiler->vars[insn->src_args[0]].alloc) {
        orc_x86_emit_mov_sse_reg_reg (compiler,
            compiler->vars[insn->src_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc);
      }
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_COMPILER_ERROR(compiler,"No rule for: %s", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      OrcVariable *var = compiler->vars + insn->dest_args[k];

      if (opcode->dest_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_DEST:
          orc_sse_emit_store_dest (compiler, var);
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
        orc_x86_emit_add_imm_reg (compiler, compiler->is_64bit ? 8 : 4,
            compiler->vars[k].size << compiler->loop_shift,
            compiler->vars[k].ptr_register);
      } else {
        orc_x86_emit_add_imm_memoffset (compiler, compiler->is_64bit ? 8 : 4,
            compiler->vars[k].size << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
            compiler->exec_reg);
      }
    }
  }
}

