#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <orc/orcprogram.h>
#include <orc/orcx86.h>
#include <orc/orcinternal.h>

#define ORC_X86_ALIGNED_DEST_CUTOFF 64
#define LABEL_REGION1_SKIP 1
#define LABEL_INNER_LOOP_START 2
#define LABEL_REGION2_SKIP 3
#define LABEL_OUTER_LOOP 4
#define LABEL_OUTER_LOOP_SKIP 5
// XXX: For AVX-512 onwards, check that this range doesn't overlap
// with the region 1 labels (LABEL_STEP_UP)
#define LABEL_STEP_DOWN(x) (8 + (x))
#define LABEL_STEP_UP(x) (t->label_step_up + (x))

static void
orc_x86_validate_registers (OrcX86Target *t, OrcCompiler *c)
{
  t->validate_registers (c->valid_regs, c->is_64bit);
}

#ifdef HAVE_OS_WIN32
static void
orc_x86_saveable_registers (OrcX86Target *t, OrcCompiler *c)
{
  t->saveable_registers (c->save_regs, c->is_64bit);
}
#endif

static void
orc_x86_is_64bit (OrcX86Target *t, OrcCompiler *c)
{
  c->is_64bit = t->is_64bit (c->target_flags);
}

static void
orc_x86_use_frame_pointer (OrcX86Target *t, OrcCompiler *c)
{
  c->use_frame_pointer = t->use_frame_pointer (c->target_flags);
}

static void
orc_x86_use_long_jumps (OrcX86Target *t, OrcCompiler *c)
{
  c->long_jumps = t->use_long_jumps (c->target_flags);
}

static void
orc_x86_compiler_max_loop_shift (OrcX86Target *t, OrcCompiler *c)
{
  int i;
  int n = 2;

  for (i = 1; i; i++) {
    if ((t->register_size / c->max_var_size) == n)
      break;
    n *= 2;
  } 
  c->loop_shift = i;
}

static void
orc_x86_compiler_init (OrcCompiler *c)
{
  OrcX86Target *t;
  int i;

  t = c->target->target_data;
  orc_x86_is_64bit (t, c);
  orc_x86_use_frame_pointer (t, c);
  orc_x86_use_long_jumps (t, c);

  if (c->is_64bit) {
    for (i = ORC_GP_REG_BASE; i < ORC_GP_REG_BASE + 16; i++) {
      c->valid_regs[i] = 1;
    }
    c->valid_regs[X86_ESP] = 0;

    orc_x86_validate_registers (t, c);

    c->save_regs[X86_EBX] = 1;
    c->save_regs[X86_EBP] = 1;
    c->save_regs[X86_R12] = 1;
    c->save_regs[X86_R13] = 1;
    c->save_regs[X86_R14] = 1;
    c->save_regs[X86_R15] = 1;
#ifdef HAVE_OS_WIN32
    c->save_regs[X86_EDI] = 1;
    c->save_regs[X86_ESI] = 1;
    // When present, the upper portions of YMM0-YMM15 and ZMM0-ZMM15 are also
    // volatile
    // https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170#callercallee-saved-registers
    orc_x86_saveable_registers (t, c);
#endif
  } else {
    for (i = ORC_GP_REG_BASE; i < ORC_GP_REG_BASE + 8; i++) {
      c->valid_regs[i] = 1;
    }
    c->valid_regs[X86_ESP] = 0;
    if (c->use_frame_pointer) {
      c->valid_regs[X86_EBP] = 0;
    }
    orc_x86_validate_registers (t, c);

    c->save_regs[X86_EBX] = 1;
    c->save_regs[X86_EDI] = 1;
    c->save_regs[X86_EBP] = 1;
  }
  for (i = 0; i < 128; i++) {
    c->alloc_regs[i] = 0;
    c->used_regs[i] = 0;
  }

  if (c->is_64bit) {
#ifdef HAVE_OS_WIN32
    c->exec_reg = X86_ECX;
    c->gp_tmpreg = X86_EDX;
#else
    c->exec_reg = X86_EDI;
    c->gp_tmpreg = X86_ECX;
#endif
  } else {
    c->gp_tmpreg = X86_ECX;
    if (c->use_frame_pointer) {
      c->exec_reg = X86_EBX;
    } else {
      c->exec_reg = X86_EBP;
    }
  }
  c->valid_regs[c->gp_tmpreg] = 0;
  c->valid_regs[c->exec_reg] = 0;

  orc_x86_compiler_max_loop_shift (t, c);

  /* This limit is arbitrary, but some large functions run slightly
     slower when unrolled (ginger Core2 6,15,6), and only some small
     functions run faster when unrolled.  Most are the same speed. */
  /* Also don't enable unrolling with loop_shift == 0, this enables
     double reading in the hot loop. */
  if (c->n_insns <= 10 && c->loop_shift > 0) {
    c->unroll_shift = 1;
  }
  if (!c->long_jumps) {
    c->unroll_shift = 0;
  }
  c->alloc_loop_counter = TRUE;
  c->allow_gp_on_stack = TRUE;

  /* FIXME ldreslinb, ldreslinl, ldresnearb, ldresnearl
   * are special opcodes that require more initialization
   * but their flags are shared among more opcodes. These
   * opcodes should have specific flags to proceed accordingly
   */
  {
    for (i = 0; i < c->n_insns; i++) {
      OrcInstruction *insn = c->insns + i;
      OrcStaticOpcode *opcode = insn->opcode;

      if (strcmp (opcode->name, "ldreslinb") == 0
          || strcmp (opcode->name, "ldreslinl") == 0
          || strcmp (opcode->name, "ldresnearb") == 0
          || strcmp (opcode->name, "ldresnearl") == 0) {
        c->vars[insn->src_args[0]].need_offset_reg = TRUE;
      }
    }
  }
}

static void
orc_x86_save_accumulators (OrcX86Target *t, OrcCompiler *c)
{
  int i;

  for (i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    OrcVariable *var = c->vars + i;

    if (var->name == NULL)
      continue;

    if (var->vartype != ORC_VAR_TYPE_ACCUMULATOR)
      continue;

    t->reduce_accumulator (c, i, var);
  }
}

static void
orc_x86_init_accumulators (OrcX86Target *t, OrcCompiler *c)
{
  int i;

  for (i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    OrcVariable *var = c->vars + i;

    if (var->name == NULL)
      continue;

    if (var->vartype != ORC_VAR_TYPE_ACCUMULATOR)
      continue;

    t->init_accumulator (c, var);
  }
}

static void
orc_x86_load_constant_uint64 (OrcX86Target *t, OrcCompiler *c, int reg, int size,
    orc_uint64 value)
{
  t->load_constant (c, reg, size, value);
}

static void
orc_x86_load_constant (OrcCompiler *c, int reg, int size, int value)
{
  OrcX86Target *t;

  t = c->target->target_data;
  orc_x86_load_constant_uint64 (t, c, reg, size, value);
}

static void
orc_x86_load_constant_long (OrcX86Target *t, OrcCompiler *c, int reg,
    OrcConstant *constant)
{
  t->load_constant_long (c, reg, constant);
}

static void
orc_x86_init_constants (OrcX86Target *t, OrcCompiler *c)
{
  int i;

  for (i = 0; i < c->n_constants; i++) {
    c->constants[i].alloc_reg = orc_compiler_get_constant_reg (c);
    if (!c->constants[i].alloc_reg)
      continue;

    if (c->constants[i].is_long) {
      orc_x86_load_constant_long (t, c, c->constants[i].alloc_reg,
          c->constants + i);
    } else {
      orc_x86_load_constant_uint64 (t, c, c->constants[i].alloc_reg, 4,
          c->constants[i].value);
    }
  }
}

static void
orc_x86_load_constants_outer (OrcX86Target *t, OrcCompiler *c)
{
  orc_x86_init_accumulators (t, c);
  orc_compiler_emit_invariants (c);
  orc_x86_init_constants (t, c);

  /* FIXME ldreslinb, ldreslinl, ldresnearb, ldresnearl
   * are special opcodes that require more initialization
   * but their flags are shared among more opcodes. These
   * opcodes should have specific flags to proceed accordingly
   */

  {
    for (int i = 0; i < c->n_insns; i++) {
      OrcInstruction *insn = c->insns + i;
      OrcStaticOpcode *opcode = insn->opcode;

      if (strcmp (opcode->name, "ldreslinb") == 0
          || strcmp (opcode->name, "ldreslinl") == 0
          || strcmp (opcode->name, "ldresnearb") == 0
          || strcmp (opcode->name, "ldresnearl") == 0) {
        if (c->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
          orc_x86_emit_mov_memoffset_reg (c, 4,
              (int)ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[1]]),
              c->exec_reg, c->vars[insn->src_args[0]].ptr_offset);
        } else {
          orc_x86_emit_mov_imm_reg (c, 4,
              c->vars[insn->src_args[1]].value.i,
              c->vars[insn->src_args[0]].ptr_offset);
        }
      }
    }
  }
}

static void
orc_x86_load_constants_inner (OrcCompiler *c)
{
  int i;

  for (i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].name == NULL)
      continue;
    switch (c->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (c->vars[i].ptr_register) {
          orc_x86_emit_mov_memoffset_reg (c, c->is_64bit ? 8 : 4,
              (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]),
              c->exec_reg, c->vars[i].ptr_register);
        }
        break;
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (c, "bad vartype");
        break;
    }
  }
}

static void
orc_x86_add_strides (OrcCompiler *c)
{
  int i;

  for (i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].name == NULL)
      continue;
    switch (c->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_x86_emit_mov_memoffset_reg (c, 4,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, params[i]), c->exec_reg,
            c->gp_tmpreg);
        orc_x86_emit_add_reg_memoffset (c, c->is_64bit ? 8 : 4,
            c->gp_tmpreg,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]),
            c->exec_reg);

        if (c->vars[i].ptr_register == 0) {
          orc_compiler_error (c,
              "unimplemented: stride on pointer stored in memory");
        }
        break;
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (c, "bad vartype");
        break;
    }
  }
}

static void
orc_x86_save_registers (OrcX86Target *t, OrcCompiler *c)
{
  int i;
  int saved = 0;

  for (i = 0; i < t->n_registers; ++i) {
    if (c->save_regs[t->register_start + i] == 1) {
      ++saved;
    }
  }

  if (saved > 0) {
    orc_x86_emit_mov_imm_reg (c, 4, t->register_size * saved, c->gp_tmpreg);
    orc_x86_emit_sub_reg_reg (c, c->is_64bit ? 8 : 4,
        c->gp_tmpreg, X86_ESP);
    saved = 0;
    for (i = 0; i < t->n_registers; ++i) {
      if (c->save_regs[t->register_start + i] == 1) {
        t->move_register_to_memoffset (c, t->register_size, t->register_start + i,
            saved * t->register_size, X86_ESP, FALSE, FALSE);
        ++saved;
      }
    }
  }
}

static void
orc_x86_restore_registers (OrcX86Target *t, OrcCompiler *c)
{
  int i;
  int saved = 0;

  for (i = 0; i < t->n_registers; ++i) {
    if (c->save_regs[t->register_start + i] == 1) {
      t->move_memoffset_to_register (c, t->register_size, saved * t->register_size, X86_ESP,
          t->register_start + i, FALSE);
      ++saved;
    }
  }
  if (saved > 0) {
    orc_x86_emit_mov_imm_reg (c, 4, t->register_size * saved, c->gp_tmpreg);
    orc_x86_emit_add_reg_reg (c, c->is_64bit ? 8 : 4,
        c->gp_tmpreg, X86_ESP);
  }
}

static int
orc_x86_get_max_alignment_var (OrcX86Target *t, OrcCompiler *c)
{
  int s;
  int i;

  /* Iterate over register size to 8 in halves: 32, 16, 8, etc */
  for (s = t->register_size; s >= 8; s >>= 2) {
    for (i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
      if (c->vars[i].size == 0)
        continue;
      if ((c->vars[i].size << c->loop_shift) >= s) {
        return i;
      }
    }
  }

  /* Last case */
  for (i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (c->vars[i].size == 0)
      continue;
    return i;
  }

  orc_compiler_error (c, "could not find alignment variable");
  return -1;
}


static int
orc_x86_get_shift (OrcX86Target *t, int size)
{
  /* Get n for 2^n, taking into account the register size */
  /* FIXME missing the get_shift code generalization, the SSE 
   * case can handle until 8, but AVX until 32? */
  return t->get_shift(size);
}

static void
orc_x86_emit_split_3_regions (OrcX86Target *t, OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  int var_size_shift;

  align_var = orc_x86_get_max_alignment_var (t, compiler);
  if (align_var < 0)
    return;
  var_size_shift = orc_x86_get_shift (t, compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  /* determine how many iterations until align array is aligned (n1) */
  orc_x86_emit_mov_imm_reg (compiler, 4, 32, X86_EAX);
  // Get the address of the array in question
  // and eax <- eax - addressof(alignment variable)
  orc_x86_emit_sub_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[align_var]),
      compiler->exec_reg, X86_EAX);
  // How many bytes are needed for alignment? (mask wise)
  orc_x86_emit_and_imm_reg (compiler, 4, (1 << align_shift) - 1, X86_EAX);
  // Undo the shift to determine number of ELEMENTS
  orc_x86_emit_sar_imm_reg (compiler, 4, var_size_shift, X86_EAX);

  /* check if n1 is greater than n. */
  orc_x86_emit_cmp_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg);

  orc_x86_emit_jle (compiler, 6);

  /* If so, we have a standard 3-region split. */
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);

  /* Calculate n2 */
  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
      compiler->gp_tmpreg);
  orc_x86_emit_sub_reg_reg (compiler, 4, X86_EAX, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_reg (compiler, 4, compiler->gp_tmpreg, X86_EAX);

  orc_x86_emit_sar_imm_reg (compiler, 4,
      compiler->loop_shift + compiler->unroll_shift, compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);

  /* Calculate n3 */
  orc_x86_emit_and_imm_reg (compiler, 4,
      (1 << (compiler->loop_shift + compiler->unroll_shift)) - 1, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);

  orc_x86_emit_jmp (compiler, 7);

  /* else, iterations are all unaligned: n1=n, n2=0, n3=0 */
  orc_x86_emit_label (compiler, 6);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);
  orc_x86_emit_mov_imm_reg (compiler, 4, 0, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);

  orc_x86_emit_label (compiler, 7);
}

static void
orc_x86_emit_split_2_regions (OrcX86Target *t, OrcCompiler *compiler)
{
  int align_var;
  int align_shift ORC_GNUC_UNUSED;
  int var_size_shift;

  align_var = orc_x86_get_max_alignment_var (t, compiler);
  if (align_var < 0)
    return;
  var_size_shift = orc_x86_get_shift (t, compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  /* Calculate n2 */
  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
      compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_reg (compiler, 4, compiler->gp_tmpreg, X86_EAX);
  orc_x86_emit_sar_imm_reg (compiler, 4,
      compiler->loop_shift + compiler->unroll_shift, compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);

  /* Calculate n3 */
  orc_x86_emit_and_imm_reg (compiler, 4,
      (1 << (compiler->loop_shift + compiler->unroll_shift)) - 1, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);
}

/*
 * The following code was ported from the MIPS backend,
 * and extended to allow for store reordering and the
 * multi-operand VEX syntax.
 */
static int
uses_in_destination_register (const OrcCompiler *const compiler,
               const OrcInstruction *const insn,
               int reg)
{
  for (int i=0; i<ORC_STATIC_OPCODE_N_DEST; i++) {
    const OrcVariable *const var = compiler->vars + insn->dest_args[i];
    if (var->alloc == reg || var->ptr_register == reg)
      return TRUE;
  }

  return FALSE;
}

static int uses_in_source_register(const OrcCompiler *const compiler,
               const OrcInstruction *const insn,
               int reg) {
  for (int i=0; i<ORC_STATIC_OPCODE_N_SRC; i++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[i];
    if (var->alloc == reg || var->ptr_register == reg)
      return TRUE;
  }

  return FALSE;
}

static void
do_swap (int *tab, int i, int j)
{
  int tmp = tab[i];
  tab[i] = tab[j];
  tab[j] = tmp;
}

/* Assumes that the instruction at indexes[i] is a load instruction */
static int
can_raise (const OrcCompiler *const compiler, const int *const indexes, int i)
{
  if (i==0)
    return FALSE;

  const OrcInstruction *const insn = compiler->insns + indexes[i];
  const OrcInstruction *const previous_insn = compiler->insns + indexes[i-1];

  /* Register where the load operation will put the data */
  const int reg = compiler->vars[insn->dest_args[0]].alloc;
  if (uses_in_source_register(compiler, previous_insn, reg) || uses_in_destination_register(compiler, previous_insn, reg)) {
    return FALSE;
  }

  for (int j = 0; j < ORC_STATIC_OPCODE_N_SRC; j++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[j];
    // If the previous instruction touches anything RIP
    if (uses_in_destination_register(compiler, previous_insn, var->alloc) || uses_in_destination_register(compiler, previous_insn, var->ptr_register))
      return FALSE;
  }

  return TRUE;
}

/* Recursive. */
static void
try_raise (OrcCompiler *compiler, int *indexes, int i)
{
  if (can_raise (compiler, indexes, i)) {
    do_swap (indexes, i-1, i);
    try_raise (compiler, indexes, i-1);
  }
}


/* Assumes that the instruction at indexes[i] is a load instruction */
static int
can_lower (const OrcCompiler *const compiler, const int *const indexes, int i)
{
  if (i >= compiler->n_insns - 1)
    return FALSE;

  const OrcInstruction *const insn = compiler->insns + indexes[i];
  const OrcInstruction *const next_insn = compiler->insns + indexes[i+1];

  /* Register where the store operation will put the data */
  const int reg = compiler->vars[insn->dest_args[0]].ptr_register;
  if (uses_in_source_register(compiler, next_insn, reg)) {
    return FALSE;
  }

  for (int j = 0; j < ORC_STATIC_OPCODE_N_SRC; j++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[j];
    // If the next instruction touches anything RIP
    if (uses_in_destination_register(compiler, next_insn, var->alloc) || uses_in_destination_register(compiler, next_insn, var->ptr_register))
      return FALSE;
  }

  return TRUE;
}

static void
try_lower (OrcCompiler *compiler, int *indexes, int i)
{
  if (can_lower (compiler, indexes, i)) {
    do_swap (indexes, i-1, i);
    try_lower (compiler, indexes, i+1);
  }
}

/*
 * Do a kind of bubble sort, though it might not exactly be a sort. It only
 * moves load instructions up until they reach an operation above which they
 * cannot go.
 */
static void
optimise_order (OrcCompiler *compiler, int *const indexes)
{
  for (int i=0; i<compiler->n_insns; i++) {
    const OrcInstruction *const insn = compiler->insns + indexes[i];
    if (insn->opcode->flags & ORC_STATIC_OPCODE_LOAD) {
      try_raise(compiler, indexes, i);
    }
    else if (insn->opcode->flags & ORC_STATIC_OPCODE_STORE) {
      try_lower(compiler, indexes, i);
    }
  }
}

static int *
get_optimised_instruction_order (OrcCompiler *compiler)
{
  if (compiler->n_insns == 0)
    return NULL;

  int *const instruction_idx = malloc (compiler->n_insns * sizeof(int));
  for (int i=0; i<compiler->n_insns; i++)
    instruction_idx[i] = i;

  optimise_order (compiler, instruction_idx);

  return instruction_idx;
}

static void
orc_x86_emit_loop (OrcCompiler *compiler, int offset, int update)
{
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;
  int j;
  int k;
  int *const insn_idx = get_optimised_instruction_order (compiler);

  for (j = 0; j < compiler->n_insns; j++) {
    insn = compiler->insns + insn_idx[j];
    opcode = insn->opcode;

    compiler->insn_index = j;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT)
      continue;

    ORC_ASM_CODE (compiler, "# %d: %s\n", j, insn->opcode->name);

    compiler->min_temp_reg = ORC_VEC_REG_BASE;

    compiler->insn_shift = compiler->loop_shift;
    if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
      compiler->insn_shift += 1;
    }
    if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
      compiler->insn_shift += 2;
    }

    rule = insn->rule;
    if (rule && rule->emit) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_error (compiler, "no code generation rule for %s",
          opcode->name);
    }
  }

  if (update) {
    for (k = 0; k < ORC_N_COMPILER_VARIABLES; k++) {
      OrcVariable *var = compiler->vars + k;

      if (var->name == NULL)
        continue;
      if (var->vartype == ORC_VAR_TYPE_SRC
          || var->vartype == ORC_VAR_TYPE_DEST) {
        int offset;
        if (var->update_type == 0) {
          offset = 0;
        } else if (var->update_type == 1) {
          offset = (var->size * update) >> 1;
        } else {
          offset = var->size * update;
        }

        if (offset != 0) {
          if (compiler->vars[k].ptr_register) {
            orc_x86_emit_add_imm_reg (compiler, compiler->is_64bit ? 8 : 4,
                offset, compiler->vars[k].ptr_register, FALSE);
          } else {
            orc_x86_emit_add_imm_memoffset (compiler,
                compiler->is_64bit ? 8 : 4, offset,
                (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[k]),
                compiler->exec_reg);
          }
        }
      }
    }
  }

  free (insn_idx);
}

static void
orc_x86_set_mxcsr (OrcX86Target *t, OrcCompiler *c)
{
  t->set_mxcsr (c);
}

static void
orc_x86_restore_mxcsr (OrcX86Target *t, OrcCompiler *c)
{
  t->restore_mxcsr (c);
}

static void
orc_x86_clear_emms (OrcX86Target *t, OrcCompiler *c)
{
  if (t->clear_emms)
    t->clear_emms (c);
}


static void
orc_x86_adjust_alignment (OrcX86Target *t, OrcCompiler *compiler)
{
  int i;

  /* Adjust alignment of variables
   * We only care of array vars, as those require memory access
   */
  for (i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    if ((compiler->vars[i].alignment % t->register_size) == 0) {
      compiler->vars[i].is_aligned = TRUE;
    } else {
      compiler->vars[i].is_aligned = FALSE;
    }
  }
}

static void
orc_x86_compile (OrcCompiler *compiler)
{
  OrcX86Target *t;
  int set_mxcsr = FALSE;
  int align_var;
  int is_aligned;

  t = compiler->target->target_data;
  align_var = orc_x86_get_max_alignment_var (t, compiler);
  if (align_var < 0) {
    orc_x86_assemble_copy (compiler);
    return;
  }

  /* Align the compiler variables */
  orc_x86_adjust_alignment (t, compiler);

  is_aligned = compiler->vars[align_var].is_aligned;
  {
    orc_x86_emit_loop (compiler, 0, 0);

    compiler->codeptr = compiler->code;
    free (compiler->asm_code);
    compiler->asm_code = NULL;
    compiler->asm_code_len = 0;
    memset (compiler->labels, 0, sizeof (compiler->labels));
    memset (compiler->labels_int, 0, sizeof (compiler->labels_int));
    compiler->n_fixups = 0;
    compiler->n_output_insns = 0;
  }

  if (compiler->error)
    return;

  orc_x86_emit_prologue (compiler);

  orc_x86_save_registers (t, compiler);

  if (t->set_mxcsr && orc_program_has_float (compiler)) {
    set_mxcsr = TRUE;
    orc_x86_set_mxcsr (t, compiler);
  }

  orc_x86_load_constants_outer (t, compiler);

  if (compiler->program->is_2d) {
    if (compiler->program->constant_m > 0) {
      orc_x86_emit_mov_imm_reg (compiler, 4, compiler->program->constant_m,
          X86_EAX);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    } else {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A1]),
          compiler->exec_reg, X86_EAX);
      orc_x86_emit_test_reg_reg (compiler, 4, X86_EAX, X86_EAX);
      orc_x86_emit_jle (compiler, LABEL_OUTER_LOOP_SKIP);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    }

    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP);
  }

  if (compiler->program->constant_n > 0
      && compiler->program->constant_n <= ORC_X86_ALIGNED_DEST_CUTOFF) {
    /* don't need to load n */
  } else if (compiler->loop_shift > 0) {
    if (compiler->has_iterator_opcode || is_aligned) {
      orc_x86_emit_split_2_regions (t, compiler);
    } else {
      /* split n into three regions, with center region being aligned */
      orc_x86_emit_split_3_regions (t, compiler);
    }
  } else {
    /* loop shift is 0, no need to split */
    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
        compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
  }

  orc_x86_load_constants_inner (compiler);

  if (compiler->program->constant_n > 0
      && compiler->program->constant_n <= ORC_X86_ALIGNED_DEST_CUTOFF) {
    int n_left = compiler->program->constant_n;
    int save_loop_shift;
    int loop_shift;

    compiler->offset = 0;

    save_loop_shift = compiler->loop_shift;
    while (n_left >= (1 << compiler->loop_shift)) {
      ORC_ASM_CODE (compiler, "# AVX LOOP SHIFT %d\n", compiler->loop_shift);
      orc_x86_emit_loop (compiler, compiler->offset, 0);

      n_left -= 1 << compiler->loop_shift;
      compiler->offset += 1 << compiler->loop_shift;
    }
    for (loop_shift = compiler->loop_shift - 1; loop_shift >= 0; loop_shift--) {
      if (n_left >= (1 << loop_shift)) {
        compiler->loop_shift = loop_shift;
        ORC_ASM_CODE (compiler, "# AVX LOOP SHIFT %d\n", loop_shift);
        orc_x86_emit_loop (compiler, compiler->offset, 0);
        n_left -= 1 << loop_shift;
        compiler->offset += 1 << loop_shift;
      }
    }
    compiler->loop_shift = save_loop_shift;

  } else {
    int ui, ui_max;
    int emit_region1 = TRUE;
    int emit_region3 = TRUE;

    if (compiler->has_iterator_opcode || is_aligned) {
      emit_region1 = FALSE;
    }
    if (compiler->loop_shift == 0) {
      emit_region1 = FALSE;
      emit_region3 = FALSE;
    }

    if (emit_region1) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for (l = 0; l < save_loop_shift; l++) {
        compiler->loop_shift = l;
        ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1 << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_UP (compiler->loop_shift));
        orc_x86_emit_loop (compiler, 0, 1 << compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_UP (compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
      compiler->vars[align_var].is_aligned = TRUE;
    }

    orc_x86_emit_label (compiler, LABEL_REGION1_SKIP);

    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
    orc_x86_emit_je (compiler, LABEL_REGION2_SKIP);

    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg,
          compiler->loop_counter);
    }

    ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);
    // Instruction fetch windows are 16-byte aligned
    // https://easyperf.net/blog/2018/01/18/Code_alignment_issues
    orc_x86_emit_align (compiler, 4);
    orc_x86_emit_label (compiler, LABEL_INNER_LOOP_START);
    ui_max = 1 << compiler->unroll_shift;
    for (ui = 0; ui < ui_max; ui++) {
      compiler->offset = ui << compiler->loop_shift;
      orc_x86_emit_loop (compiler, compiler->offset,
          (ui == ui_max - 1)
              << (compiler->loop_shift + compiler->unroll_shift));
    }
    compiler->offset = 0;
    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_add_imm_reg (compiler, 4, -1, compiler->loop_counter, TRUE);
    } else {
      orc_x86_emit_dec_memoffset (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
    }
    orc_x86_emit_jne (compiler, LABEL_INNER_LOOP_START);
    orc_x86_emit_label (compiler, LABEL_REGION2_SKIP);

    if (emit_region3) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift + compiler->unroll_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for (l = save_loop_shift - 1; l >= 0; l--) {
        compiler->loop_shift = l;
        ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1 << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_DOWN (compiler->loop_shift));
        orc_x86_emit_loop (compiler, 0, 1 << compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_DOWN (compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
    }
  }

  if (compiler->program->is_2d && compiler->program->constant_m != 1) {
    orc_x86_add_strides (compiler);

    orc_x86_emit_add_imm_memoffset (compiler, 4, -1,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, LABEL_OUTER_LOOP);
    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP_SKIP);
  }

  orc_x86_save_accumulators (t, compiler);

  if (set_mxcsr) {
    orc_x86_restore_mxcsr (t, compiler);
  }
  orc_x86_clear_emms (t, compiler);

  orc_x86_restore_registers (t, compiler);

  orc_x86_emit_epilogue (compiler);

  orc_x86_calculate_offsets (compiler);
  orc_x86_output_insns (compiler);

  orc_x86_do_fixups (compiler);
}

OrcTarget *
orc_x86_register_target (OrcX86Target *x86t)
{
  OrcTarget *t;

  /* FIXME this needs to be freed */
  t = calloc (1, sizeof(OrcTarget));
  t->name = x86t->name;
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  t->executable = x86t->is_executable ();
#else
  t->executable = FALSE;
#endif
  t->data_register_offset = ORC_VEC_REG_BASE;
  t->get_default_flags = x86t->get_default_flags;
  t->compiler_init = orc_x86_compiler_init;
  t->compile = orc_x86_compile;
  t->load_constant = orc_x86_load_constant;
  t->get_flag_name = x86t->get_flag_name;
  t->load_constant_long = x86t->load_constant_long;
  t->target_data = x86t;
  orc_target_register (t);

  return t;
}
