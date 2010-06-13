
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcx86.h>
#include <orc/orcsse.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#define SIZE 65536

#define ORC_SSE_ALIGNED_DEST_CUTOFF 64

void orc_sse_emit_loop (OrcCompiler *compiler, int offset, int update);

void orc_compiler_sse_init (OrcCompiler *compiler);
unsigned int orc_compiler_sse_get_default_flags (void);
void orc_compiler_sse_assemble (OrcCompiler *compiler);
void orc_compiler_sse_register_rules (OrcTarget *target);


void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);
void sse_load_constant (OrcCompiler *compiler, int reg, int size, int value);

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
  orc_compiler_sse_assemble,
  { { 0 } },
  0,
  NULL,
  sse_load_constant

};


void
orc_sse_init (void)
{
#if defined(HAVE_I386)
  if (!(orc_sse_get_cpu_flags () & ORC_TARGET_SSE_SSE2)) {
    sse_target.executable = FALSE;
  }
#endif

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
  
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  flags |= orc_sse_get_cpu_flags ();
#else
  flags |= ORC_TARGET_SSE_SSE2;
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

  compiler->unroll_shift = 1;
  compiler->alloc_loop_counter = TRUE;

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
          orc_x86_emit_mov_sse_reg (compiler, src, compiler->gp_tmpreg);
          orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, compiler->gp_tmpreg);
          orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
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
sse_load_constant (OrcCompiler *compiler, int reg, int size, int value)
{
  if (size == 1) {
    orc_sse_emit_loadib (compiler, reg, value);
  } else if (size == 2) {
    orc_sse_emit_loadiw (compiler, reg, value);
  } else if (size == 4) {
    orc_sse_emit_loadil (compiler, reg, value);
  } else {
    ORC_COMPILER_ERROR(compiler, "unimplemented");
  }

}

void
sse_load_constants_outer (OrcCompiler *compiler)
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
sse_load_constants_inner (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
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
sse_add_strides (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_x86_emit_mov_memoffset_reg (compiler, 4,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]), compiler->exec_reg,
            X86_ECX);
        orc_x86_emit_add_reg_memoffset (compiler, compiler->is_64bit ? 8 : 4,
            X86_ECX,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), compiler->exec_reg);
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
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
orc_sse_emit_load_src (OrcCompiler *compiler, OrcVariable *var, int offset)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      orc_x86_emit_mov_memoffset_reg (compiler, 1, offset, ptr_reg, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_sse (compiler, compiler->gp_tmpreg, var->alloc);
      break;
    case 2:
      orc_x86_emit_mov_memoffset_reg (compiler, 2, offset, ptr_reg, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_sse (compiler, compiler->gp_tmpreg, var->alloc);
      break;
    case 4:
      orc_x86_emit_mov_memoffset_sse (compiler, 4, offset, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_sse (compiler, 8, offset, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_sse (compiler, 16, offset, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    default:
      ORC_COMPILER_ERROR(compiler,"bad load size %d",
          var->size << compiler->loop_shift);
      break;
  }
}

void
orc_sse_emit_store_dest (OrcCompiler *compiler, OrcVariable *var, int offset)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        var->ptr_offset, compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == compiler->gp_tmpreg) {
        ORC_COMPILER_ERROR(compiler,"unimplemented");
      }
      orc_x86_emit_mov_sse_reg (compiler, var->alloc, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_memoffset (compiler, 1, compiler->gp_tmpreg, offset, ptr_reg);
      break;
    case 2:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == compiler->gp_tmpreg) {
        ORC_COMPILER_ERROR(compiler,"unimplemented");
      }
      orc_x86_emit_mov_sse_reg (compiler, var->alloc, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_memoffset (compiler, 2, compiler->gp_tmpreg, offset, ptr_reg);
      break;
    case 4:
      orc_x86_emit_mov_sse_memoffset (compiler, 4, var->alloc, offset, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 8:
      orc_x86_emit_mov_sse_memoffset (compiler, 8, var->alloc, offset, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 16:
      orc_x86_emit_mov_sse_memoffset (compiler, 16, var->alloc, offset, ptr_reg,
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


static void
orc_emit_split_n_regions (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  int var_size_shift;

  align_var = get_align_var (compiler);
  var_size_shift = get_shift (compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  /* determine how many iterations until align array is aligned (n1) */
  orc_x86_emit_mov_imm_reg (compiler, 4, 16, X86_EAX);
  orc_x86_emit_sub_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[align_var]),
      compiler->exec_reg, X86_EAX);
  orc_x86_emit_and_imm_reg (compiler, 4, (1<<align_shift) - 1, X86_EAX);
  orc_x86_emit_sar_imm_reg (compiler, 4, var_size_shift, X86_EAX);

  /* check if n1 is greater than n. */
  orc_x86_emit_cmp_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg);

  orc_x86_emit_jle (compiler, 6);

  /* If so, we have a standard 3-region split. */
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
    
  /* Calculate n2 */
  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg,
      compiler->gp_tmpreg);
  orc_x86_emit_sub_reg_reg (compiler, 4, X86_EAX, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_reg (compiler, 4, compiler->gp_tmpreg, X86_EAX);

  orc_x86_emit_sar_imm_reg (compiler, 4,
      compiler->loop_shift + compiler->unroll_shift,
      compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);

  /* Calculate n3 */
  orc_x86_emit_and_imm_reg (compiler, 4,
      (1<<(compiler->loop_shift + compiler->unroll_shift))-1, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), compiler->exec_reg);

  orc_x86_emit_jmp (compiler, 7);

  /* else, iterations are all unaligned: n1=n, n2=0, n3=0 */
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
}

#define LABEL_REGION1_SKIP 1
#define LABEL_INNER_LOOP_START 2
#define LABEL_REGION2_SKIP 3
#define LABEL_OUTER_LOOP 4
#define LABEL_OUTER_LOOP_SKIP 5
#define LABEL_STEP_DOWN(x) (8+(x))
#define LABEL_STEP_UP(x) (13+(x))


void
orc_compiler_sse_assemble (OrcCompiler *compiler)
{
  int align_var;

  if (0 && orc_x86_assemble_copy_check (compiler)) {
    /* The rep movs implementation isn't faster most of the time */
    orc_x86_assemble_copy (compiler);
    return;
  }

  align_var = get_align_var (compiler);

  compiler->vars[align_var].is_aligned = FALSE;

  orc_x86_emit_prologue (compiler);

  sse_load_constants_outer (compiler);

  if (compiler->program->is_2d) {
    if (compiler->program->constant_m > 0) {
      orc_x86_emit_mov_imm_reg (compiler, 4, compiler->program->constant_m,
          X86_EAX);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    } else {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A1]),
          compiler->exec_reg, X86_EAX);
      orc_x86_emit_test_reg_reg (compiler, 4, X86_EAX, X86_EAX);
      orc_x86_emit_jle (compiler, LABEL_OUTER_LOOP_SKIP);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    }

    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP);
  }

  if (compiler->program->constant_n > 0 &&
      compiler->program->constant_n <= ORC_SSE_ALIGNED_DEST_CUTOFF) {
    /* don't need to load n */
  } else if (compiler->loop_shift > 0) {
    /* split n into three regions, with center region being aligned */
    orc_emit_split_n_regions (compiler);
  } else {
    /* loop shift is 0, no need to split */
    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg, compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
  }

  sse_load_constants_inner (compiler);

  if (compiler->program->constant_n > 0 &&
      compiler->program->constant_n <= ORC_SSE_ALIGNED_DEST_CUTOFF) {
    int n_left = compiler->program->constant_n;
    int save_loop_shift;
    int loop_shift;
    int offset = 0;

    save_loop_shift = compiler->loop_shift;
    while (n_left >= (1<<compiler->loop_shift)) {
      ORC_ASM_CODE(compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);
      orc_sse_emit_loop (compiler, offset, 0);

      n_left -= 1<<compiler->loop_shift;
      offset += 1<<compiler->loop_shift;
    }
    for(loop_shift = compiler->loop_shift-1; loop_shift>=0; loop_shift--) {
      if (n_left >= (1<<loop_shift)) {
        compiler->loop_shift = loop_shift;
        ORC_ASM_CODE(compiler, "# LOOP SHIFT %d\n", loop_shift);
        orc_sse_emit_loop (compiler, offset, 0);
        n_left -= 1<<loop_shift;
        offset += 1<<loop_shift;
      }
    }
    compiler->loop_shift = save_loop_shift;
  } else {
    int ui, ui_max;

    if (compiler->loop_shift > 0) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for (l=0;l<save_loop_shift;l++){
        compiler->loop_shift = l;
        ORC_ASM_CODE(compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1<<compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_UP(compiler->loop_shift));
        orc_sse_emit_loop (compiler, 0, 1<<compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_UP(compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
      compiler->vars[align_var].is_aligned = TRUE;
    }

    orc_x86_emit_label (compiler, LABEL_REGION1_SKIP);

    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
    orc_x86_emit_je (compiler, LABEL_REGION2_SKIP);

    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, counter2), compiler->exec_reg,
          compiler->loop_counter);
    }

    ORC_ASM_CODE(compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);
    orc_x86_emit_align (compiler);
    orc_x86_emit_label (compiler, LABEL_INNER_LOOP_START);
    ui_max = 1<<compiler->unroll_shift;
    for(ui=0;ui<ui_max;ui++) {
      orc_sse_emit_loop (compiler, ui<<compiler->loop_shift,
          (ui==ui_max-1) << (compiler->loop_shift + compiler->unroll_shift));
    }
    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_add_imm_reg (compiler, 4, -1, compiler->loop_counter, TRUE);
    } else {
      orc_x86_emit_dec_memoffset (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2),
          compiler->exec_reg);
    }
    orc_x86_emit_jne (compiler, LABEL_INNER_LOOP_START);
    orc_x86_emit_label (compiler, LABEL_REGION2_SKIP);

    if (compiler->loop_shift > 0) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift + compiler->unroll_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for(l=save_loop_shift - 1; l >= 0; l--) {
        compiler->loop_shift = l;
        ORC_ASM_CODE(compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1<<compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_DOWN(compiler->loop_shift));
        orc_sse_emit_loop (compiler, 0, 1<<compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_DOWN(compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
    }
  }

  if (compiler->program->is_2d) {
    sse_add_strides (compiler);

    orc_x86_emit_add_imm_memoffset (compiler, 4, -1,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, LABEL_OUTER_LOOP);
    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP_SKIP);
  }

  sse_save_accumulators (compiler);

  orc_x86_emit_epilogue (compiler);

  orc_x86_do_fixups (compiler);
}

void
orc_sse_emit_loop (OrcCompiler *compiler, int offset, int update)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
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
          orc_sse_emit_load_src (compiler, var, offset*var->size);
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
          orc_sse_emit_store_dest (compiler, var, offset*var->size);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  if (update) {
    for(k=0;k<ORC_N_VARIABLES;k++){
      if (compiler->vars[k].name == NULL) continue;
      if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
          compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
        if (compiler->vars[k].ptr_register) {
          orc_x86_emit_add_imm_reg (compiler, compiler->is_64bit ? 8 : 4,
              compiler->vars[k].size * update,
              compiler->vars[k].ptr_register, FALSE);
        } else {
          orc_x86_emit_add_imm_memoffset (compiler, compiler->is_64bit ? 8 : 4,
              compiler->vars[k].size * update,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
              compiler->exec_reg);
        }
      }
    }
  }
}

