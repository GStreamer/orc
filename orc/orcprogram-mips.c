#include <orc/orcmips.h>
#include <orc/orcdebug.h>
#include <stdlib.h>

unsigned int orc_compiler_orc_mips_get_default_flags (void);

void orc_compiler_orc_mips_init (OrcCompiler *compiler);

void orc_compiler_orc_mips_assemble (OrcCompiler *compiler);

const char * orc_compiler_orc_mips_get_asm_preamble (void);

/* in orcrules-mips.c */
void orc_compiler_orc_mips_register_rules (OrcTarget *target);

static OrcTarget orc_mips_target = {
  "mips",
#ifdef HAVE_MIPS
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
  compiler->valid_regs[ORC_MIPS_T0] = 0; /* We get the arg size here */
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
}

const char *
orc_compiler_orc_mips_get_asm_preamble (void)
{
  return "\n"
      "/* begin Orc MIPS target preamble */\n"
      ".abicalls\n" /* not exactly sure what this is, but linker complains
                       without it  */
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

        /* ORC_STRUCT_OFFSET is wrong when we run orc on a platform different
         * than the target, e.g. x86_64 vs Mips32. We replace it with a dirty
         * hand calculation for now */
        /*orc_mips_emit_lw (compiler,
            compiler->vars[i].ptr_register,
            compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, arrays[i])); */
        orc_mips_emit_lw (compiler,
            compiler->vars[i].ptr_register,
            compiler->exec_reg, 20+4*i);
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

  for (i=0; i<compiler->n_insns; i++) {
    insn = compiler->insns + i;
    opcode = insn->opcode;
    if (insn->flags & ORC_INSN_FLAG_INVARIANT) continue;

    orc_compiler_append_code(compiler,"/* %d: %s */\n", i, insn->opcode->name);

    rule = insn->rule;
    if (rule && rule->emit) {
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

void
orc_compiler_orc_mips_assemble (OrcCompiler *compiler)
{
  int stack_size;

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

  /*orc_mips_emit_lw (compiler, ORC_MIPS_T0, compiler->exec_reg,
                    (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
                    */
  /* the above breaks when orc runs on a 64 bit machine because the calculated
   * offset is not the same as on mips32 */
  /* On MIPS32, executor->n is at a 4 bytes offset */
  orc_mips_emit_lw (compiler, ORC_MIPS_T0, compiler->exec_reg, 4);
  orc_mips_emit_blez (compiler, ORC_MIPS_T0, 2);
  orc_mips_emit_nop (compiler);
  orc_mips_load_constants_inner (compiler);

  orc_mips_emit_label (compiler, 1);
  orc_mips_emit_loop (compiler);

  orc_mips_emit_addiu (compiler, ORC_MIPS_T0, ORC_MIPS_T0, -1);
  orc_mips_emit_bnez (compiler, ORC_MIPS_T0, 1);
  orc_mips_emit_nop (compiler);

  orc_mips_emit_label (compiler, 2);

  orc_mips_emit_epilogue (compiler, stack_size);
}

