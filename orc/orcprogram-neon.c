
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcarm.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>

#include <orc/orcneon.h>

#define SIZE 65536
#define MAX_UNROLL 16

enum RegionFlags {
  FLAG_NUNROLL = (1 << 0),
  FLAG_REGION1 = (1 << 1),
  FLAG_REGION2 = (1 << 2),
  FLAG_REGION3 = (1 << 3),
};

static void orc_neon_emit_loop (OrcCompiler *compiler, int unroll_index);

extern void orc_compiler_neon_register_rules (OrcTarget *target);
static unsigned int orc_compiler_neon_get_default_flags (void);
static enum RegionFlags orc_compiler_neon_calc_regions (const OrcCompiler *compiler);

static void orc_compiler_neon_init (OrcCompiler *compiler);
static void orc_compiler_neon_assemble (OrcCompiler *compiler);
static void orc_compiler_neon_assemble_32 (OrcCompiler *compiler);
static void orc_compiler_neon_assemble_64 (OrcCompiler *compiler);

static void orc_neon64_short_unaligned_loop (OrcCompiler *compiler);

static void orc_neon_save_accumulators (OrcCompiler *compiler);
static void neon_add_strides (OrcCompiler *compiler);


static void
orc_neon_emit_prologue (OrcCompiler *compiler)
{
  unsigned int regs = 0;
  orc_uint32 vregs = 0;
  int num_gregs;
  int i;

  orc_compiler_append_code(compiler,".global %s\n", compiler->program->name);
  orc_compiler_append_code(compiler,"%s:\n", compiler->program->name);

  num_gregs = compiler->is_64bit ? 32 : 16;

  for(i=0;i<num_gregs;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }

  for(i=0;i<32;i++) {
     if (compiler->used_regs[ORC_VEC_REG_BASE+i] &&
         compiler->save_regs[ORC_VEC_REG_BASE+i]) {
        vregs |= (1U << i);
     }
  }

  orc_arm_emit_push (compiler, regs, vregs);
}

/* unused */
#if 0
static void
orc_neon_dump_insns (OrcCompiler *compiler)
{

  orc_arm_emit_label (compiler, 0);

  orc_arm_emit_add (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_A4);
  orc_arm_emit_sub (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_A4);
  orc_arm_emit_push (compiler, 0x06, 0U);
  orc_arm_emit_mov (compiler, ORC_ARM_A2, ORC_ARM_A3);

  orc_arm_emit_branch (compiler, ORC_ARM_COND_LE, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, 0);

  orc_arm_emit_load_imm (compiler, ORC_ARM_A3, 0xa500);
  orc_arm_loadw (compiler, ORC_ARM_A3, ORC_ARM_A4, 0xa5);
  orc_arm_emit_load_reg (compiler, ORC_ARM_A3, ORC_ARM_A4, 0x5a5);
}
#endif

static void
orc_neon_emit_epilogue (OrcCompiler *compiler)
{
  int i;
  int num_gregs;
  unsigned int regs = 0;
  orc_uint32 vregs = 0;

  num_gregs = compiler->is_64bit ? 32 : 16;

  for(i=0;i<num_gregs;i++){
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i]) {
      regs |= (1<<i);
    }
  }

  for(i=0;i<32;i++) {
     if (compiler->used_regs[ORC_VEC_REG_BASE+i] &&
         compiler->save_regs[ORC_VEC_REG_BASE+i]) {
        vregs |= (1U << i);
     }
  }

  orc_arm_emit_pop (compiler, regs, vregs);
  orc_arm_emit_bx_lr (compiler);

  /* arm_dump_insns (compiler); */
}

static OrcTarget neon_target = {
  "neon",
#if defined(HAVE_ARM) || defined(HAVE_AARCH64)
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_neon_get_default_flags,
  orc_compiler_neon_init,
  orc_compiler_neon_assemble,
  { { 0 } }, 0,
  NULL,
  NULL,
  NULL,
  orc_arm_flush_cache
};

void
orc_neon_init (void)
{
#if defined(HAVE_ARM) || defined(HAVE_AARCH64)
  if (!(orc_arm_get_cpu_flags () & ORC_TARGET_NEON_NEON)) {
    ORC_INFO("marking neon backend non-executable");
    neon_target.executable = FALSE;
  }
#endif

  orc_target_register (&neon_target);

  orc_compiler_neon_register_rules (&neon_target);
}

static unsigned int
orc_compiler_neon_get_default_flags (void)
{
  unsigned int flags = 0;

#if defined(HAVE_AARCH64)
  flags |= ORC_TARGET_NEON_64BIT;
#endif
  flags |= ORC_TARGET_NEON_NEON;

  return flags;
}

static enum RegionFlags
orc_compiler_neon_calc_regions (const OrcCompiler *compiler)
{
  enum RegionFlags flags = 0;

  // NOTE: Assume aligned if we know the input size
  const int constant_n        = compiler->program->constant_n;
  const orc_bool vectorizable = compiler->loop_shift > 0;
  const int num_loops         = constant_n >> compiler->loop_shift;
  const orc_bool n_unrollable = constant_n && (num_loops < MAX_UNROLL);
  const int elm_per_loop      = 1 << compiler->loop_shift;
  const int remaining         = constant_n & (elm_per_loop - 1);
  const orc_bool aligned      = compiler->has_iterator_opcode || (constant_n > 0);
  const orc_bool enough_n     = constant_n >= elm_per_loop;

  if (!vectorizable)
    return FLAG_REGION2; // Only region 2

  orc_bool region1 = !aligned;
  orc_bool region2 = !constant_n || enough_n;
  orc_bool region3 = !n_unrollable || remaining;

  if (region1)
    flags |= FLAG_REGION1;

  if (region2)
    flags |= FLAG_REGION2;

  if (region3)
    flags |= FLAG_REGION3;

  if (n_unrollable)
    flags |= FLAG_NUNROLL;

  return flags;
}

static void
orc_compiler_neon_init (OrcCompiler *compiler)
{
  int i;
  int loop_shift;

  if (compiler->target_flags & ORC_TARGET_NEON_64BIT) {
    compiler->is_64bit = TRUE;
  }

  if (compiler->is_64bit) {
    /** AArch64
     * 31 64-bit generic-purpose registers (R0-R30) and SP
     * 32 128-bit vector registers (do not overlap multiple registers in a narrower view)
     * Note that PC is not a generic-purpose register in AArch64
     */
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+32;i++){
      compiler->valid_regs[i] = 1;
    }
    for(i=ORC_VEC_REG_BASE+0;i<ORC_VEC_REG_BASE+32;i++){
      compiler->valid_regs[i] = 1;
    }

    compiler->valid_regs[ORC_ARM64_IP0] = 0;
    compiler->valid_regs[ORC_ARM64_IP1] = 0;

    compiler->valid_regs[ORC_ARM64_FP] = 0;
    compiler->valid_regs[ORC_ARM64_LR] = 0;
    compiler->valid_regs[ORC_ARM64_SP] = 0;

    /** r19 to r29 are callee-saved */
    for(i=19;i<29;i++) {
      compiler->save_regs[ORC_GP_REG_BASE+i] = 1;
    }
  } else {
    /** AArch32
     * 16 32-bit generic-purpose registers (R0-R15)
     * 32 64-bit vector registers (smaller registers are packed into larger ones)
     */
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
      compiler->valid_regs[i] = 1;
    }
    for(i=ORC_VEC_REG_BASE+0;i<ORC_VEC_REG_BASE+32;i+=2){
      compiler->valid_regs[i] = 1;
    }
    /* compiler->valid_regs[ORC_ARM_SB] = 0; */
    compiler->valid_regs[ORC_ARM_IP] = 0;
    compiler->valid_regs[ORC_ARM_SP] = 0;
    compiler->valid_regs[ORC_ARM_LR] = 0;
    compiler->valid_regs[ORC_ARM_PC] = 0;

    for(i=4;i<12;i++) {
      compiler->save_regs[ORC_GP_REG_BASE+i] = 1;
    }
  }

  /** Both architectures have 8 callee-saved SIMD registers (v8-v15) */
  for(i=8;i<16;i++) {
    compiler->save_regs[ORC_VEC_REG_BASE+i] = 1;
  }

  for(i=0;i<ORC_N_REGS;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->exec_reg = ORC_ARM_A1;
  compiler->gp_tmpreg = ORC_ARM_A2;
  if (compiler->is_64bit) {
    compiler->tmpreg = ORC_VEC_REG_BASE + 0;
    compiler->tmpreg2 = ORC_VEC_REG_BASE + 1;
  } else {
    compiler->tmpreg = ORC_VEC_REG_BASE + 0;
    compiler->tmpreg2 = ORC_VEC_REG_BASE + 2;
  }
  compiler->valid_regs[compiler->exec_reg] = 0;
  compiler->valid_regs[compiler->gp_tmpreg] = 0;
  compiler->valid_regs[compiler->tmpreg] = 0;
  compiler->valid_regs[compiler->tmpreg2] = 0;

  loop_shift = 0;
  switch (compiler->max_var_size) {
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
      ORC_ERROR("unhandled max var size %d", compiler->max_var_size);
      break;
  }

  switch (orc_program_get_max_array_size (compiler->program)) {
    case 0:
    case 1:
      loop_shift = 4;
      break;
    case 2:
      loop_shift = 3;
      break;
    case 4:
      loop_shift = 2;
      break;
    case 8:
      loop_shift = 1;
      break;
    default:
      ORC_ERROR("unhandled max array size %d",
          orc_program_get_max_array_size (compiler->program));
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
      ORC_ERROR("unhandled max accumulator size %d",
          orc_program_get_max_accumulator_size (compiler->program));
      break;
  }
  if (loop_shift < compiler->loop_shift) {
    compiler->loop_shift = loop_shift;
  }

  /* Unrolling isn't helpful until neon gets an instruction
   * scheduler.  This decreases the raw amount of code generated
   * while still keeping the feature active. */
  if (compiler->n_insns < 5) {
    compiler->unroll_shift = 0;
  }

  for(i=0;i<compiler->n_insns;i++){
    OrcInstruction *insn = compiler->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;

    if (strcmp (opcode->name, "loadupdb") == 0) {
      compiler->vars[insn->src_args[0]].need_offset_reg = TRUE;
    }
  }

  if (0) {
    compiler->need_mask_regs = TRUE;
  }
}

static void
orc_neon_load_constants_outer (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;

    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_neon_emit_loadil (compiler, &(compiler->vars[i]), 0);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler,"bad vartype");
        break;
    }
  }

  orc_compiler_emit_invariants (compiler);

  for(i=0;i<compiler->n_insns;i++){
    OrcInstruction *insn = compiler->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;

    if (strcmp (opcode->name, "loadupdb") == 0) {
      if (compiler->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
        if (compiler->is_64bit) {
          orc_arm64_emit_load_reg (compiler, 64,
	        compiler->vars[insn->src_args[0]].ptr_offset,
              compiler->exec_reg,
		ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]));
	  } else {
          orc_arm_emit_load_reg (compiler,
	        compiler->vars[insn->src_args[0]].ptr_offset,
              compiler->exec_reg,
		ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]));
	  }
      } else {
        if (!compiler->vars[insn->src_args[0]].ptr_offset)
            continue;
        if (compiler->is_64bit) {
          if (!compiler->vars[insn->src_args[1]].value.i)
              orc_arm64_emit_eor(compiler, 64,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[0]].ptr_offset);
          else
              orc_arm64_emit_load_imm(compiler, 64,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[1]].value.i);
        } else {
          if (!compiler->vars[insn->src_args[1]].value.i)
              orc_arm_emit_eor_r(compiler, ORC_ARM_COND_AL, 0,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[0]].ptr_offset);
          else
              orc_arm_emit_load_imm(compiler,
                  compiler->vars[insn->src_args[0]].ptr_offset,
                  compiler->vars[insn->src_args[1]].value.i);
        }
      }
    }
  }
}

static void
orc_neon_load_constants_inner (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;

    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->is_64bit) {
          orc_arm64_emit_load_reg (compiler, 64,
              compiler->vars[i].ptr_register,
              compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
          if (compiler->vars[i].ptr_offset)
              orc_arm64_emit_eor(compiler, 64,
                  compiler->vars[i].ptr_offset,
                  compiler->vars[i].ptr_offset,
                  compiler->vars[i].ptr_offset);
        } else {
          orc_arm_emit_load_reg (compiler,
              compiler->vars[i].ptr_register,
              compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
          if (compiler->vars[i].ptr_offset)
              orc_arm_emit_eor_r(compiler, ORC_ARM_COND_AL, 0,
                  compiler->vars[i].ptr_offset,
                  compiler->vars[i].ptr_offset,
                  compiler->vars[i].ptr_offset);
        }
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_PROGRAM_ERROR(compiler,"bad vartype");
        break;
    }
  }
}

#if 0
void
orc_neon_emit_load_src (OrcCompiler *compiler, OrcVariable *var, int unroll_index)
{
  int ptr_reg;
  int update;

  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    /* arm_emit_mov_memoffset_reg (compiler, arm_ptr_size, */
    /*     (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), */
    /*     p->exec_reg, X86_ECX); */
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
  
  if (unroll_index == 0) {
  switch (compiler->size_region) {
    case 0:
    case 1:
      orc_neon_preload (compiler, var, FALSE, 208);
      break;
    case 2:
    case 3:
      orc_neon_preload (compiler, var, FALSE, 208);
      break;
  }
  }
}

void
orc_neon_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    /* arm_emit_mov_memoffset_reg (compiler, arm_ptr_size, */
    /*     var->ptr_offset, p->exec_reg, X86_ECX); */
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

  switch (compiler->size_region) {
    case 0:
      break;
    case 1:
      /* assume hot cache, see below */
      break;
    case 2:
      /* This is only useful for cold cache and for memset-like operations,
         which isn't the usual case, thus it's disabled. */
#if 0
      orc_neon_preload (compiler, var, FALSE, 208);
#endif
      break;
    case 3:
      /* none */
      break;
  }
}
#endif

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

enum {
  LABEL_ONE_REGION = 1,
  LABEL_ONE_REGION_AFTER,
  LABEL_REGION0_LOOP,
  LABEL_REGION0_SKIP,
  LABEL_REGION1_LOOP,
  LABEL_REGION1_SKIP,
  LABEL_REGION2_LOOP_SMALL,
  LABEL_REGION2_LOOP_MEDIUM,
  LABEL_REGION2_LOOP_LARGE,
  LABEL_REGION2_SMALL,
  LABEL_REGION2_MEDIUM,
  LABEL_REGION2_SKIP,
  LABEL_REGION3_LOOP,
  LABEL_REGION3_SKIP,
  LABEL_OUTER_LOOP,
  LABEL_OUTER_LOOP_SKIP,
  LABEL_L1L2_AFTER,
};

#define ORC_NEON_ALIGNED_DEST_CUTOFF 64
#define ORC_NEON_LONG_PROGRAM_CUTOFF 5

static void
orc_neon64_set_region_counters (OrcCompiler *compiler)
{
  int align_var;
  int var_size_shift;
  int align_shift = 4;

  align_var = get_align_var (compiler);
  if (compiler->error) return;
  var_size_shift = get_shift (compiler->vars[align_var].size);

  /** IP0 = 1 << align_shift */
  orc_arm64_emit_mov_imm (compiler, 32, ORC_ARM64_IP0, 1<<align_shift);

  /** r1 == ORC_VAR_D1 */
  orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_R1, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[align_var]));
  /** IP0 = IP0 - r1 */
  orc_arm64_emit_sub (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, ORC_ARM64_R1);
  /** IP0 = IP0 & ((1 << aligned_shift) -1) */
  orc_arm64_emit_and_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0,
      (1<<align_shift)-1);
  if (var_size_shift > 0) {
    /** IP0 = IP0 >> var_size_shift */
    orc_arm64_emit_asr_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, var_size_shift);
  }

  /** r2 = N */
  orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
  /** N <= IP0, go to LABEL_ONE_REGION */
  orc_arm64_emit_cmp (compiler, 32, ORC_ARM64_R2, ORC_ARM64_IP0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_LE, LABEL_ONE_REGION);

  /** counter1 = IP0 */
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_IP0, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
  /** r1 = r2 - IP0 */
  orc_arm64_emit_sub (compiler, 32, ORC_ARM64_R1, ORC_ARM64_R2, ORC_ARM64_IP0);

  /** r2 = r1 >> (loop_shift + unroll_shift) */
  orc_arm64_emit_asr_imm (compiler, 32, ORC_ARM64_R2, ORC_ARM64_R1,
      compiler->loop_shift + compiler->unroll_shift);
  /** counter2 = r2 */
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

  /** r2 = r1 & ((1<<(loop_shift + unroll_shift))-1) */
  orc_arm64_emit_and_imm (compiler, 32, ORC_ARM64_R2, ORC_ARM64_R1,
      (1<<(compiler->loop_shift + compiler->unroll_shift))-1);
  /** counter3 = r2 */
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

  /** go to LABEL_ONE_REGION_AFTER */
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_ONE_REGION_AFTER);
  orc_arm_emit_label (compiler, LABEL_ONE_REGION);

  /** counter1 = r2 */
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
  /** counter2 = counter3 = 0 */
  orc_arm64_emit_mov_uimm (compiler, 32, ORC_ARM64_R2, 0);
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

  orc_arm_emit_label (compiler, LABEL_ONE_REGION_AFTER);
}

static void
orc_neon64_short_unaligned_loop (OrcCompiler *compiler)
{
  /** Get the number of loops (N) from OrcExecutor */
  orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n));

  /** if N > ORC_NEON_ALIGNED_DEST_CUTOFF, go to LABEL_REGION0_SKIP */
  orc_arm64_emit_cmp_imm (compiler, 32, ORC_ARM64_R2, ORC_NEON_ALIGNED_DEST_CUTOFF);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_GT, LABEL_REGION0_SKIP);

  /** counter2 = N >> loop shift */
  orc_arm64_emit_asr_imm (compiler, 32, ORC_ARM64_R1, ORC_ARM64_R2,
      compiler->loop_shift);
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R1, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

  /** counter3 = N & loop shift */
  orc_arm64_emit_and_imm (compiler, 32, ORC_ARM64_R2, ORC_ARM64_R2,
      (1<<compiler->loop_shift)-1);
  orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_R2, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

  /** load function arguments */
  orc_neon_load_constants_inner (compiler);

  /** if counter2 == zero, go to LABEL_REGION2_SKIP */
  orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP0, compiler->exec_reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
  orc_arm64_emit_cmp_imm (compiler, 32, ORC_ARM64_IP0, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SKIP);

  /** vector calculation loop */
  compiler->size_region = 0;
  orc_arm_emit_label (compiler, LABEL_REGION0_LOOP);
  orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1);

  /** vector instructions: @todo port to aarch64 */
  orc_neon_emit_loop (compiler, -1);

  /** if counter2 != zero, repeat loop */
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION0_LOOP);
  /** else go to LABEL_REGION2_SKIP */
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);
  orc_arm_emit_label (compiler, LABEL_REGION0_SKIP);
}

static void
orc_neon64_loop_caches (OrcCompiler *compiler)
{
  int align_var;
  int var_size_shift;
  int i;

  align_var = get_align_var (compiler);
  if (compiler->error) return;
  var_size_shift = get_shift (compiler->vars[align_var].size);

  /** if IP0 == 0, go to LABEL_REGION2_SKIP */
  orc_arm64_emit_cmp_imm (compiler, 32, ORC_ARM64_IP0, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SKIP);

  /** r1 = IP0 >> (17 + var_size_shift - compiler->loop_shift - compiler->unroll_shift) */
  orc_arm64_emit_asr_imm (compiler, 32, compiler->gp_tmpreg, ORC_ARM64_IP0,
      17 + var_size_shift - compiler->loop_shift - compiler->unroll_shift);

  /** if r1 == 0, go to LABEL_REGION2_MEDIUM */
  orc_arm64_emit_cmp_imm (compiler, 32, compiler->gp_tmpreg, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_MEDIUM);

  /** N is larger than L2 cache size */
  compiler->size_region = 3;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_LARGE);
  orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_LARGE);
  /** DONE, let's finish */
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);

  orc_arm_emit_label (compiler, LABEL_REGION2_MEDIUM);
  orc_arm64_emit_asr_imm (compiler, 32, compiler->gp_tmpreg, ORC_ARM64_IP0,
      13 + var_size_shift - compiler->loop_shift - compiler->unroll_shift);
  orc_arm64_emit_cmp_imm (compiler, 32, compiler->gp_tmpreg, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SMALL);

  /* N is smaller than L2 cache size */
  compiler->size_region = 2;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_MEDIUM);
  orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_MEDIUM);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);

  orc_arm_emit_label (compiler, LABEL_REGION2_SMALL);
  /* N is smaller than L1 cache size */
  compiler->size_region = 1;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_SMALL);
  orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_SMALL);

  orc_arm_emit_label (compiler, LABEL_REGION2_SKIP);
}

#define orc_neon64_loop_shift_remainder(compiler,counter,label_loop,label_skip) \
{ \
  int save_loop_shift = compiler->loop_shift; \
  compiler->loop_shift = 0; \
  orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP0, compiler->exec_reg, \
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter)); \
  orc_arm64_emit_cmp_imm (compiler, 32, ORC_ARM64_IP0, 0); \
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, label_skip); \
  orc_arm_emit_label (compiler, label_loop); \
  orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1); \
  orc_neon_emit_loop (compiler, -1); \
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, label_loop); \
  orc_arm_emit_label (compiler, label_skip); \
  compiler->loop_shift = save_loop_shift; \
}

static void
orc_compiler_neon_assemble (OrcCompiler *compiler)
{
  if (compiler->is_64bit)
    orc_compiler_neon_assemble_64 (compiler);
  else
    orc_compiler_neon_assemble_32 (compiler);
}

static void
orc_neon_emit_loop (OrcCompiler *compiler, int unroll_index)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  orc_compiler_append_code(compiler,"# LOOP shift %d (%d)\n",
      compiler->loop_shift, unroll_index);
  for(j=0;j<compiler->n_insns;j++){
    compiler->insn_index = j;
    insn = compiler->insns + j;
    opcode = insn->opcode;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT) continue;

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
          /* orc_neon_emit_load_src (compiler, &compiler->vars[insn->src_args[k]], unroll_index); */
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

    compiler->insn_shift = compiler->loop_shift;
    if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
      compiler->insn_shift += 1;
    }
    if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
      compiler->insn_shift += 2;
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
          /* orc_neon_emit_store_dest (compiler, &compiler->vars[insn->dest_args[k]]); */
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  for(k=0;k<ORC_N_COMPILER_VARIABLES;k++){
    if (compiler->vars[k].name == NULL) continue;
    if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
        compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (compiler->is_64bit) {
        if (compiler->vars[k].ptr_offset) {
          orc_arm64_emit_add_imm (compiler, 64,
              compiler->vars[k].ptr_offset,
              compiler->vars[k].ptr_offset,
              compiler->vars[k].size << compiler->loop_shift);
        } else if (compiler->vars[k].ptr_register) {
          orc_arm64_emit_add_imm (compiler, 64,
              compiler->vars[k].ptr_register,
              compiler->vars[k].ptr_register,
              compiler->vars[k].size << compiler->loop_shift);
        }
      } else {
        if (compiler->vars[k].ptr_offset) {
          orc_arm_emit_add_imm (compiler,
              compiler->vars[k].ptr_offset,
              compiler->vars[k].ptr_offset,
              compiler->vars[k].size << compiler->loop_shift);
        } else if (compiler->vars[k].ptr_register) {
          orc_arm_emit_add_imm (compiler,
              compiler->vars[k].ptr_register,
              compiler->vars[k].ptr_register,
              compiler->vars[k].size << compiler->loop_shift);
        }
      }
    }
  }
}

static void
orc_compiler_neon_assemble_32 (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  int var_size_shift;
  int i;
  int set_fpscr = FALSE;

  align_var = get_align_var (compiler);
  if (compiler->error) return;

  var_size_shift = get_shift (compiler->vars[align_var].size);
  align_shift = 4;

  compiler->vars[align_var].is_aligned = FALSE;

  orc_neon_emit_prologue (compiler);

  if (orc_compiler_has_float (compiler)) {
    set_fpscr = TRUE;
    ORC_ASM_CODE (compiler,"  vmrs %s, fpscr\n", orc_arm_reg_name (compiler->gp_tmpreg));
    orc_arm_emit (compiler, 0xeef10a10 | ((compiler->gp_tmpreg&0xf)<<12));
    ORC_ASM_CODE (compiler,"  push %s\n", orc_arm_reg_name (compiler->gp_tmpreg));
    orc_arm_emit (compiler, 0xe52d0004 | ((compiler->gp_tmpreg&0xf)<<12));

    orc_arm_emit_load_imm (compiler, compiler->gp_tmpreg, 1<<24);
    ORC_ASM_CODE (compiler,"  vmsr fpscr, %s\n", orc_arm_reg_name (compiler->gp_tmpreg));
    orc_arm_emit (compiler, 0xeee10a10 | ((compiler->gp_tmpreg&0xf)<<12));
  }

  orc_neon_load_constants_outer (compiler);

  if (compiler->program->is_2d) {
    if (compiler->program->constant_m > 0) {
      orc_arm_emit_load_imm (compiler, ORC_ARM_A3, compiler->program->constant_m);
      orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]));
    } else {
      orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A1]));
      orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]));
    }

    orc_arm_emit_label (compiler, LABEL_OUTER_LOOP);
  }

  if (compiler->loop_shift > 0 && compiler->n_insns < 5) {
    orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));

    orc_arm_emit_cmp_imm (compiler, ORC_ARM_A3, ORC_NEON_ALIGNED_DEST_CUTOFF);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_GT, LABEL_REGION0_SKIP);

    orc_arm_emit_asr_imm (compiler, ORC_ARM_A2, ORC_ARM_A3,
        compiler->loop_shift);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A2, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    orc_arm_emit_and_imm (compiler, ORC_ARM_A3, ORC_ARM_A3,
        (1<<compiler->loop_shift)-1);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_neon_load_constants_inner (compiler);
    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SKIP);

    compiler->size_region = 0;
    orc_arm_emit_label (compiler, LABEL_REGION0_LOOP);

    orc_neon_emit_loop (compiler, -1);
    orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION0_LOOP);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);
    orc_arm_emit_label (compiler, LABEL_REGION0_SKIP);
  }

  if (compiler->loop_shift > 0) {
    orc_arm_emit_load_imm (compiler, ORC_ARM_IP, 1<<align_shift);

    orc_arm_emit_load_reg (compiler, ORC_ARM_A2, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[align_var]));
    orc_arm_emit_sub (compiler, ORC_ARM_IP, ORC_ARM_IP, ORC_ARM_A2);
    orc_arm_emit_and_imm (compiler, ORC_ARM_IP, ORC_ARM_IP,
        (1<<align_shift)-1);
    if (var_size_shift > 0) {
      orc_arm_emit_asr_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, var_size_shift);
    }

    orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));
    orc_arm_emit_cmp (compiler, ORC_ARM_A3, ORC_ARM_IP);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_LE, LABEL_ONE_REGION);

    orc_arm_emit_store_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));
    orc_arm_emit_sub (compiler, ORC_ARM_A2, ORC_ARM_A3, ORC_ARM_IP);

    orc_arm_emit_asr_imm (compiler, ORC_ARM_A3, ORC_ARM_A2,
        compiler->loop_shift + compiler->unroll_shift);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    orc_arm_emit_and_imm (compiler, ORC_ARM_A3, ORC_ARM_A2,
        (1<<(compiler->loop_shift + compiler->unroll_shift))-1);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_ONE_REGION_AFTER);
    orc_arm_emit_label (compiler, LABEL_ONE_REGION);

    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_arm_emit_load_imm (compiler, ORC_ARM_A3, 0);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_label (compiler, LABEL_ONE_REGION_AFTER);
  }

  orc_neon_load_constants_inner (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1));

    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION1_SKIP);

    orc_arm_emit_label (compiler, LABEL_REGION1_LOOP);
    orc_neon_emit_loop (compiler, -1);
    orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION1_LOOP);
    orc_arm_emit_label (compiler, LABEL_REGION1_SKIP);

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
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SKIP);

  orc_arm_emit_asr_imm (compiler, compiler->gp_tmpreg, ORC_ARM_IP,
      17 + var_size_shift - compiler->loop_shift - compiler->unroll_shift);
  orc_arm_emit_cmp_imm (compiler, compiler->gp_tmpreg, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_MEDIUM);

  /* N is larger than L2 cache size */
  compiler->size_region = 3;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_LARGE);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_LARGE);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);

  orc_arm_emit_label (compiler, LABEL_REGION2_MEDIUM);
  orc_arm_emit_asr_imm (compiler, compiler->gp_tmpreg, ORC_ARM_IP,
      13 + var_size_shift - compiler->loop_shift - compiler->unroll_shift);
  orc_arm_emit_cmp_imm (compiler, compiler->gp_tmpreg, 0);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION2_SMALL);

  /* N is smaller than L2 cache size */
  compiler->size_region = 2;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_MEDIUM);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_MEDIUM);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, LABEL_REGION2_SKIP);

  orc_arm_emit_label (compiler, LABEL_REGION2_SMALL);
  /* N is smaller than L2 cache size */
  compiler->size_region = 1;
  orc_arm_emit_label (compiler, LABEL_REGION2_LOOP_SMALL);
  for(i=0;i<(1<<compiler->unroll_shift);i++){
    orc_neon_emit_loop (compiler, i);
  }
  orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
  orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION2_LOOP_SMALL);

  orc_arm_emit_label (compiler, LABEL_REGION2_SKIP);

  if (compiler->loop_shift > 0) {
    int save_loop_shift = compiler->loop_shift;

    compiler->loop_shift = 0;

    compiler->vars[align_var].is_aligned = FALSE;

    orc_arm_emit_load_reg (compiler, ORC_ARM_IP, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3));

    orc_arm_emit_cmp_imm (compiler, ORC_ARM_IP, 0);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_EQ, LABEL_REGION3_SKIP);

    orc_arm_emit_label (compiler, LABEL_REGION3_LOOP);
    orc_neon_emit_loop (compiler, -1);
    orc_arm_emit_sub_imm (compiler, ORC_ARM_IP, ORC_ARM_IP, 1, TRUE);
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION3_LOOP);
    orc_arm_emit_label (compiler, LABEL_REGION3_SKIP);

    compiler->loop_shift = save_loop_shift;
  }

  if (compiler->program->is_2d) {
    neon_add_strides (compiler);

    orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A2]));
    orc_arm_emit_sub_imm (compiler, ORC_ARM_A3, ORC_ARM_A3, 1, TRUE);
    orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]));
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_OUTER_LOOP);
  }

  orc_neon_save_accumulators (compiler);

  if (set_fpscr) {
    ORC_ASM_CODE (compiler,"  pop %s\n", orc_arm_reg_name (compiler->gp_tmpreg));
    orc_arm_emit (compiler, 0xe49d0004 | ((compiler->gp_tmpreg&0xf)<<12));

    ORC_ASM_CODE (compiler,"  vmsr fpscr, %s\n", orc_arm_reg_name (compiler->gp_tmpreg));
    orc_arm_emit (compiler, 0xeee10a10 | ((compiler->gp_tmpreg&0xf)<<12));
  }

  orc_neon_emit_epilogue (compiler);

  orc_arm_emit_align (compiler, 4);

  orc_arm_emit_label (compiler, 20);
  orc_arm_emit_data (compiler, 0x07060706);
  orc_arm_emit_data (compiler, 0x07060706);
  orc_arm_emit_data (compiler, 0x0f0e0f0e);
  orc_arm_emit_data (compiler, 0x0f0e0f0e);

  orc_arm_do_fixups (compiler);
}

static void
orc_compiler_neon_assemble_64 (OrcCompiler *compiler)
{
  const enum RegionFlags region_flags = orc_compiler_neon_calc_regions (compiler);
  const orc_bool region1 = region_flags & FLAG_REGION1;
  const orc_bool region2 = region_flags & FLAG_REGION2;
  const orc_bool region3 = region_flags & FLAG_REGION3;
  const orc_bool nunroll = region_flags & FLAG_NUNROLL;
  int align_var;

  align_var = get_align_var (compiler);
  if (compiler->error) return;

  ORC_DEBUG ("Neon compiler regions = [ %s %s %s %s ]"
      , region1 ? "region1":"-------"
      , region2 ? "region2":"-------"
      , region3 ? "region3":"-------"
      , nunroll ? "nunroll":"-------"
  );

  compiler->vars[align_var].is_aligned = FALSE;

  orc_neon_emit_prologue (compiler);

  orc_neon_load_constants_outer (compiler);

  if (compiler->program->is_2d) {
    if (compiler->program->constant_m > 0) {
      orc_arm64_emit_mov_imm (compiler, 32, ORC_ARM64_IP1, compiler->program->constant_m);
      orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]));
    } else {
      orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A1]));
      orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A2]));
    }

    orc_arm_emit_label (compiler, LABEL_OUTER_LOOP);
  }

  if (!region1 && nunroll) {
    const int constant_n = compiler->program->constant_n;
    const int num_loops = constant_n >> compiler->loop_shift;
    const int remaining = constant_n & ((1 << compiler->loop_shift) - 1);
    ORC_DEBUG("unrolled loops     = %d", num_loops);
    ORC_DEBUG("remaining elements = %d", remaining);

    orc_neon_load_constants_inner (compiler);

    for (int i = 0; i < num_loops; i++) {
      orc_neon_emit_loop (compiler, i);
    }

    if (region3) {
      orc_arm64_emit_mov_imm (compiler, ORC_ARM64_REG_32, ORC_ARM64_IP0, remaining);
      orc_arm_emit_label (compiler, LABEL_REGION3_LOOP);
      orc_arm64_emit_subs_imm (compiler, ORC_ARM64_REG_32, ORC_ARM64_IP0, ORC_ARM64_IP0, 1);

      int save_loop_shift = compiler->loop_shift; // Ugly, but...uff
      compiler->loop_shift = 0;
      orc_neon_emit_loop (compiler, -1);
      compiler->loop_shift = save_loop_shift;

      orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_REGION3_LOOP);
    }
  } else if (compiler->loop_shift > 0) {
    if (compiler->n_insns < ORC_NEON_LONG_PROGRAM_CUTOFF)
      orc_neon64_short_unaligned_loop (compiler);

    orc_neon64_set_region_counters (compiler);

    orc_neon_load_constants_inner (compiler);

    orc_neon64_loop_shift_remainder (compiler, counter1,
        LABEL_REGION1_LOOP, LABEL_REGION1_SKIP);
    compiler->vars[align_var].is_aligned = TRUE;

    orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP0, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2));

    orc_neon64_loop_caches (compiler);

    compiler->vars[align_var].is_aligned = FALSE;
    orc_neon64_loop_shift_remainder (compiler, counter3,
        LABEL_REGION3_LOOP, LABEL_REGION3_SKIP);
  } else {
    orc_neon_load_constants_inner (compiler);

    orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP0, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n));

    orc_neon64_loop_caches (compiler);
  }

  if (compiler->program->is_2d) {
    neon_add_strides (compiler);

    orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[ORC_VAR_A2]));
    orc_arm64_emit_subs_imm (compiler, 32, ORC_ARM64_IP1, ORC_ARM64_IP1, 1);
    orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A2]));
    orc_arm_emit_branch (compiler, ORC_ARM_COND_NE, LABEL_OUTER_LOOP);
  }

  orc_neon_save_accumulators (compiler);

  orc_neon_emit_epilogue (compiler);

  orc_arm_emit_align (compiler, 4);

  orc_arm_do_fixups (compiler);
}

#define NEON_BINARY(code,a,b,c) \
  ((code) | \
   (((a)&0xf)<<12) | \
   ((((a)>>4)&0x1)<<22) | \
   (((b)&0xf)<<16) | \
   ((((b)>>4)&0x1)<<7) | \
   (((c)&0xf)<<0) | \
   ((((c)>>4)&0x1)<<5))

static void
orc_neon_save_accumulators (OrcCompiler *compiler)
{
  int i;
  int src;
  unsigned int code;

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        src = compiler->vars[i].alloc;

        if (compiler->is_64bit) {
          orc_arm64_emit_add_imm (compiler, 64, compiler->gp_tmpreg,
	    compiler->exec_reg,
	    ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]));
	} else {
          orc_arm_emit_load_imm (compiler, compiler->gp_tmpreg,
              ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]));
          orc_arm_emit_add (compiler, compiler->gp_tmpreg,
              compiler->gp_tmpreg, compiler->exec_reg);
        }
        switch (var->size) {
          case 2:
            if (compiler->loop_shift > 0) {
              if (compiler->is_64bit) {
                ORC_ASM_CODE(compiler,"  addv %s, %s, %s\n",
                    orc_neon64_reg_name_vector (src, 8, 0),
                    orc_neon64_reg_name_vector (src, 8, 0),
                    orc_neon64_reg_name_vector (src, 8, 0));
                code = 0x0e71b800;
                code |= (src&0x1f)<<5;
                code |= (src&0x1f);
                orc_arm_emit (compiler, code);
              } else {
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
            }

            if (compiler->is_64bit) {
              ORC_ASM_CODE(compiler,"  st1 %s, [%s]\n",
                  orc_neon64_reg_name_vector (src, 8, 0),
                  orc_arm64_reg_name (compiler->gp_tmpreg, 64));
              code = 0x0d004000;
              code |= (compiler->gp_tmpreg&0x1f) << 5;
              code |= (src&0x1f) << 0;
              orc_arm_emit (compiler, code);
            } else {
              ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s]\n",
                  orc_neon_reg_name (src), 0,
                  orc_arm_reg_name (compiler->gp_tmpreg));
              code = 0xf480040f;
              code |= (compiler->gp_tmpreg&0xf) << 16;
              code |= (src&0xf) << 12;
              code |= ((src>>4)&0x1) << 22;
              orc_arm_emit (compiler, code);
            }
            break;
          case 4:
            if (compiler->loop_shift > 0) {
              if (compiler->is_64bit) {
                ORC_ASM_CODE(compiler,"  addp %s, %s, %s\n",
                    orc_neon64_reg_name_vector (src, 8, 0),
                    orc_neon64_reg_name_vector (src, 8, 0),
                    orc_neon64_reg_name_vector (src, 8, 0));
                code = 0x0ea0bc00;
                code |= (src&0x1f)<<16;
                code |= (src&0x1f)<<5;
                code |= (src&0x1f);
                orc_arm_emit (compiler, code);
              } else {
                ORC_ASM_CODE(compiler,"  vpadd.u32 %s, %s, %s\n",
                    orc_neon_reg_name (src),
                    orc_neon_reg_name (src),
                    orc_neon_reg_name (src));
                code = NEON_BINARY(0xf2200b10, src, src, src);
                orc_arm_emit (compiler, code);
              }
            }

            if (compiler->is_64bit) {
              ORC_ASM_CODE(compiler,"  st1 {%s}[0], [%s]\n",
                  orc_neon64_reg_name_vector (src, 8, 0),
                  orc_arm64_reg_name (compiler->gp_tmpreg, 64));
              code = 0x0d008000;
              code |= (compiler->gp_tmpreg&0x1f) << 5;
              code |= (src&0x1f) << 0;
              orc_arm_emit (compiler, code);
            } else {
              ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s]\n",
                  orc_neon_reg_name (src), 0,
                  orc_arm_reg_name (compiler->gp_tmpreg));
              code = 0xf480080f;
              code |= (compiler->gp_tmpreg&0xf) << 16;
              code |= (src&0xf) << 12;
              code |= ((src>>4)&0x1) << 22;
              orc_arm_emit (compiler, code);
            }
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

void
neon_add_strides (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->is_64bit) {
          orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
          orc_arm64_emit_load_reg (compiler, 32, ORC_ARM64_R18, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]));
          orc_arm64_emit_add (compiler, 32, ORC_ARM64_IP1, ORC_ARM64_IP1, ORC_ARM64_R18);
          orc_arm64_emit_store_reg (compiler, 32, ORC_ARM64_IP1, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor,arrays[i]));
        } else {
          orc_arm_emit_load_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
          orc_arm_emit_load_reg (compiler, ORC_ARM_A2, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]));
          orc_arm_emit_add (compiler, ORC_ARM_A3, ORC_ARM_A3, ORC_ARM_A2);
          orc_arm_emit_store_reg (compiler, ORC_ARM_A3, compiler->exec_reg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
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

