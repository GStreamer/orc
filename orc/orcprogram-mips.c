#include <orc/orcmips.h>
#include <orc/orcdebug.h>
#include <stdlib.h>
#include "config.h"

unsigned int orc_compiler_orc_mips_get_default_flags (void);

void orc_compiler_orc_mips_init (OrcCompiler *compiler);

void orc_compiler_orc_mips_assemble (OrcCompiler *compiler);

const char * orc_compiler_orc_mips_get_asm_preamble (void);

/* in orcrules-mips.c */
void orc_compiler_orc_mips_register_rules (OrcTarget *target);

/* ORC_STRUCT_OFFSET doesn't work for cross-compiling, so we use that */

#define ORC_MIPS_EXECUTOR_OFFSET_PROGRAM 0
#define ORC_MIPS_EXECUTOR_OFFSET_N 4
#define ORC_MIPS_EXECUTOR_OFFSET_COUNTER1 8
#define ORC_MIPS_EXECUTOR_OFFSET_COUNTER2 12
#define ORC_MIPS_EXECUTOR_OFFSET_COUNTER3 16
#define ORC_MIPS_EXECUTOR_OFFSET_ARRAYS(i) (20 + 4 * i)
#define ORC_MIPS_EXECUTOR_OFFSET_PARAMS(i) (276 + 4 * i)
#define ORC_MIPS_EXECUTOR_OFFSET_ACCUMULATORS(i) (532 + 4 * i)

static OrcTarget orc_mips_target = {
  "mips",
#ifdef HAVE_MIPSEL
  TRUE,
#else
  FALSE,
#endif
  ORC_GP_REG_BASE,
  orc_compiler_orc_mips_get_default_flags,
  orc_compiler_orc_mips_init,
  orc_compiler_orc_mips_assemble,
  { { 0 } },
  0,
  orc_compiler_orc_mips_get_asm_preamble,
};

enum {
  LABEL_REGION0_LOOP = 1,
  LABEL_REGION1,
  LABEL_REGION1_LOOP,
  LABEL_REGION2,
  LABEL_REGION2_LOOP,
  LABEL_END
};

void
orc_mips_init (void)
{
  orc_target_register (&orc_mips_target);

  orc_compiler_orc_mips_register_rules (&orc_mips_target);
}

unsigned int
orc_compiler_orc_mips_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_orc_mips_init (OrcCompiler *compiler)
{
  int i;

  for (i=ORC_GP_REG_BASE; i<ORC_GP_REG_BASE+32; i++)
    compiler->valid_regs[i] = 1;

  compiler->valid_regs[ORC_MIPS_ZERO] = 0; /* always 0 */
  compiler->valid_regs[ORC_MIPS_AT] = 0; /* we shouldn't touch that (assembler
                                            temporary) */
  compiler->exec_reg = ORC_MIPS_A0;
  compiler->valid_regs[ORC_MIPS_A0] = 0; /* first (and in our case only)
                                            function argument */
  compiler->valid_regs[ORC_MIPS_T0] = 0; /* $t0, $t1 and $t2 are used as loop */
  compiler->valid_regs[ORC_MIPS_T1] = 0; /* counters */
  compiler->valid_regs[ORC_MIPS_T2] = 0;
  compiler->valid_regs[ORC_MIPS_T3] = 0; /* used for unaligned load/store of 16
                                            bit values */
  compiler->valid_regs[ORC_MIPS_K0] = 0; /* for kernel/interupts */
  compiler->valid_regs[ORC_MIPS_K1] = 0; /* for kernel/interupts */
  compiler->valid_regs[ORC_MIPS_GP] = 0; /* global pointer */
  compiler->valid_regs[ORC_MIPS_SP] = 0; /* stack pointer */
  compiler->valid_regs[ORC_MIPS_FP] = 0; /* frame pointer */
  compiler->valid_regs[ORC_MIPS_RA] = 0; /* return address */

  for (i=0;i<ORC_N_REGS;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
    compiler->save_regs[i] = 0;
  }

  compiler->save_regs[ORC_MIPS_V0] = 1;
  compiler->save_regs[ORC_MIPS_V1] = 1;
  compiler->save_regs[ORC_MIPS_A1] = 1;
  compiler->save_regs[ORC_MIPS_A2] = 1;
  compiler->save_regs[ORC_MIPS_A3] = 1;
  for (i=ORC_MIPS_S0; i<= ORC_MIPS_S7; i++)
    compiler->save_regs[i] = 1;

  /* what's compiler->gp_tmpreg? and ->tmpreg? */

  switch (compiler->max_var_size) {
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
    ORC_ERROR("unhandled variable size %d", compiler->max_var_size);
  }
}

const char *
orc_compiler_orc_mips_get_asm_preamble (void)
{
  return "\n"
      "/* begin Orc MIPS target preamble */\n"
      ".abicalls\n" /* not exactly sure what this is, but linker complains
                       without it  */
      ".set noreorder\n"
      "/* end Orc MIPS target preamble */\n\n";
}

int
orc_mips_emit_prologue (OrcCompiler *compiler)
{
  int i, stack_size = 0;

  orc_compiler_append_code(compiler,".globl %s\n", compiler->program->name);
  orc_compiler_append_code(compiler,"%s:\n", compiler->program->name);

  /* push registers we need to save */
  for(i=0; i<32; i++)
    if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
        compiler->save_regs[ORC_GP_REG_BASE + i])
      stack_size += 4;

  if (stack_size) {
    unsigned int stack_increment = 0;

    orc_mips_emit_addiu (compiler, ORC_MIPS_SP, ORC_MIPS_SP, -stack_size);

    for(i=0; i<32; i++){
      if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
          compiler->save_regs[ORC_GP_REG_BASE + i]) {
        orc_mips_emit_sw (compiler, ORC_GP_REG_BASE+i,
                          ORC_MIPS_SP, stack_increment);
            stack_increment +=4;
      }
    }
  }

  return stack_size;
}

void orc_mips_emit_epilogue (OrcCompiler *compiler, int stack_size)
{
  int i;

  /* pop saved registers */
  if (stack_size) {
    unsigned int stack_increment = 0;
    for(i=0; i<32; i++){
      if (compiler->used_regs[ORC_GP_REG_BASE + i] &&
          compiler->save_regs[ORC_GP_REG_BASE + i]) {
        orc_mips_emit_lw (compiler, ORC_GP_REG_BASE+i,
                          ORC_MIPS_SP, stack_increment);
            stack_increment +=4;
      }
    }
    orc_mips_emit_addiu (compiler, ORC_MIPS_SP, ORC_MIPS_SP, stack_size);
  }

  orc_mips_emit_jr (compiler, ORC_MIPS_RA);
  orc_mips_emit_nop (compiler);
  if (compiler->target_flags & ORC_TARGET_CLEAN_COMPILE) {
    /* we emit some padding nops at the end to align to 16 bytes because that's
     * what gnu as does (not sure why) and we want to generate the same code
     * for testing purposes */
    orc_mips_emit_align (compiler, 4);
  }
}

void
orc_mips_load_constants_inner (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_mips_emit_lw (compiler,
            compiler->vars[i].ptr_register,
            compiler->exec_reg, ORC_MIPS_EXECUTOR_OFFSET_ARRAYS(i));
        break;
      default:
        break;
    }
  }
}

void
orc_mips_emit_loop (OrcCompiler *compiler)
{
  int i, j;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;
  ORC_DEBUG ("loop_shift=%d", compiler->loop_shift);

  for (i=0; i<compiler->n_insns; i++) {
    insn = compiler->insns + i;
    opcode = insn->opcode;
    if (insn->flags & ORC_INSN_FLAG_INVARIANT) continue;

    orc_compiler_append_code(compiler,"/* %d: %s */\n", i, insn->opcode->name);

    rule = insn->rule;
    if (rule && rule->emit) {
      compiler->insn_shift = compiler->loop_shift;
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_append_code (compiler, "No rule for %s\n", opcode->name);
    }
  }

  for (j=0; j<ORC_N_COMPILER_VARIABLES; j++) {
    if (compiler->vars[j].name == NULL) continue;
    if (compiler->vars[j].vartype == ORC_VAR_TYPE_SRC ||
        compiler->vars[j].vartype == ORC_VAR_TYPE_DEST) {
      if (compiler->vars[j].ptr_register) {
        orc_mips_emit_addiu (compiler,
            compiler->vars[j].ptr_register,
            compiler->vars[j].ptr_register,
            compiler->vars[j].size << compiler->loop_shift);
      }
    }
  }
}

static int
get_align_var (OrcCompiler *compiler)
{
  if (compiler->vars[ORC_VAR_D1].size) return ORC_VAR_D1;
  if (compiler->vars[ORC_VAR_S1].size) return ORC_VAR_S1;

  ORC_PROGRAM_ERROR(compiler, "could not find alignment variable");

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
    default:
      ORC_ERROR("bad size %d", size);
  }
  return -1;
}


void
orc_compiler_orc_mips_assemble (OrcCompiler *compiler)
{
  int stack_size;
  int align_shift = 2; /* this wouldn't work on mips64 */
  int align_var = get_align_var (compiler);
  int var_size_shift;
  int saved_loop_shift;

  var_size_shift = get_shift (compiler->vars[align_var].size);

  stack_size = orc_mips_emit_prologue (compiler);

  /* FIXME: load constants and params */
#if 0
  for (i=0; i<ORC_N_COMPILER_VARIABLES; i++) {
    if (compiler->vars[i].name == NULL)
      ORC_PROGRAM_ERROR (compiler, "unimplemented");
  }
#endif

  /* FIXME */
  if (compiler->program->is_2d)
    ORC_PROGRAM_ERROR (compiler, "unimplemented");

  orc_mips_emit_lw (compiler, ORC_MIPS_T2, compiler->exec_reg,
                    ORC_MIPS_EXECUTOR_OFFSET_N);
  orc_mips_emit_blez (compiler, ORC_MIPS_T2, LABEL_END);

  /* Note: in all these counter calculations ($t0, $t1 and $t2), we assume that
   * variables of k bytes are k-bytes aligned. */

  /* $t0 = number of iterations in region0 (before alignment) */
  /* = number of bytes to get to alignment / var_size
     = ((alignment - data_address) % alignment) / var_size
     = (((1 << align_shift) - data_address) % (1 << align_shift)) / var_size
     = (((1 << align_shift) - data_address) & ((1 << align_shfit) - 1)) >> var_size_shift
   */
  /* FIXME: we load this twice. We should call orc_mips_load_constants_inner
   * first and use the register it uses */
  orc_mips_load_constants_inner (compiler);

  orc_mips_emit_addiu (compiler, ORC_MIPS_T0, ORC_MIPS_ZERO, 1 << align_shift);
  orc_mips_emit_sub (compiler, ORC_MIPS_T0, ORC_MIPS_T0,
                     compiler->vars[align_var].ptr_register);
  orc_mips_emit_andi (compiler, ORC_MIPS_T0, ORC_MIPS_T0,
                      (1 << align_shift) - 1);
  if (var_size_shift > 0)
    orc_mips_emit_srl (compiler, ORC_MIPS_T0, ORC_MIPS_T0, var_size_shift);

  /* FIXME handle case n < $t0 */

  /* $t1 = number of iterations in region1 (aligned)
         = (n - $t0) / loop_size
         = (n - $t0) >> loop_shift
   */
  orc_mips_emit_sub (compiler, ORC_MIPS_T2, ORC_MIPS_T2, ORC_MIPS_T0);
  if (compiler->loop_shift> 0)
    orc_mips_emit_srl (compiler, ORC_MIPS_T1, ORC_MIPS_T2,
                       compiler->loop_shift);
  else
    orc_mips_emit_move (compiler, ORC_MIPS_T1, ORC_MIPS_T2);


  /* if ($t0 == 0) goto REGION1 */
  orc_mips_emit_beqz (compiler, ORC_MIPS_T0, LABEL_REGION1);

  /* $t2 = number of iterations in region2 (after aligned region)
         = (n - $t0) % loop_size
         = (previous $t2) % loop_size
         = $t2 & ((1 << loop_shift) - 1)
   */
     /* note that this instruction is in the branch delay slot */
  if (compiler->loop_shift > 0)
    orc_mips_emit_andi (compiler, ORC_MIPS_T2, ORC_MIPS_T2,
                        (1 << compiler->loop_shift) - 1);
  else
    /* loop_shift==0: $t2 should be 0 because we can handle all our data in region 1*/
    orc_mips_emit_move (compiler, ORC_MIPS_T2, ORC_MIPS_ZERO);

  /* FIXME: when loop_shift == 0, we only need to emit region1 */

  orc_mips_emit_label (compiler, LABEL_REGION0_LOOP);
  saved_loop_shift = compiler->loop_shift;
  compiler->loop_shift = 0;
  orc_mips_emit_loop (compiler);
  compiler->loop_shift = saved_loop_shift;
  orc_mips_emit_addi (compiler, ORC_MIPS_T0, ORC_MIPS_T0, -1);
  orc_mips_emit_bnez (compiler, ORC_MIPS_T0, LABEL_REGION0_LOOP);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, LABEL_REGION1);
  orc_mips_emit_beqz (compiler, ORC_MIPS_T1, LABEL_REGION2);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, LABEL_REGION1_LOOP);
  orc_mips_emit_loop (compiler);
  orc_mips_emit_addi (compiler, ORC_MIPS_T1, ORC_MIPS_T1, -1);
  orc_mips_emit_bnez (compiler, ORC_MIPS_T1, LABEL_REGION1_LOOP);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, LABEL_REGION2);
  orc_mips_emit_beqz (compiler, ORC_MIPS_T2, LABEL_END);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, LABEL_REGION2_LOOP);
  saved_loop_shift = compiler->loop_shift;
  compiler->loop_shift = 0;
  orc_mips_emit_loop (compiler);
  compiler->loop_shift = saved_loop_shift;
  orc_mips_emit_addi (compiler, ORC_MIPS_T2, ORC_MIPS_T2, -1);
  orc_mips_emit_bnez (compiler, ORC_MIPS_T2, LABEL_REGION2_LOOP);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, LABEL_END);

  orc_mips_do_fixups (compiler);

  orc_mips_emit_epilogue (compiler, stack_size);
}

