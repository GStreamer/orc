
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcpowerpc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>


void orc_compiler_powerpc_register_rules (OrcTarget *target);
static void orc_compiler_powerpc_init (OrcCompiler *compiler);
static unsigned int orc_compiler_powerpc_get_default_flags (void);
static void orc_compiler_powerpc_assemble (OrcCompiler *compiler);


static void
powerpc_emit_prologue (OrcCompiler *compiler)
{
  int i;

  ORC_ASM_CODE (compiler, ".global %s\n", compiler->program->name);
  ORC_ASM_CODE (compiler, "%s:\n", compiler->program->name);

  if (compiler->is_64bit) {
#if !defined(_CALL_ELF) || _CALL_ELF == 1
    ORC_ASM_CODE (compiler, " .quad .%s,.TOC.@tocbase,0\n",
                  compiler->program->name);
    ORC_ASM_CODE (compiler, ".%s:\n", compiler->program->name);
    powerpc_emit (compiler, 0); powerpc_emit (compiler, 0);
    powerpc_emit (compiler, 0); powerpc_emit (compiler, 0);
    powerpc_emit (compiler, 0); powerpc_emit (compiler, 0);
#endif
    powerpc_emit_stdu (compiler, POWERPC_R1, POWERPC_R1, -16);
  } else {
    powerpc_emit_stwu (compiler, POWERPC_R1, POWERPC_R1, -16);
  }

  for(i=POWERPC_R13;i<=POWERPC_R31;i++){
    if (compiler->used_regs[i]) {
      /* powerpc_emit_push (compiler, 4, i); */
    }
  }
}

static void
powerpc_emit_epilogue (OrcCompiler *compiler)
{
  int i;

  for(i=POWERPC_R31;i>=POWERPC_R31;i--){
    if (compiler->used_regs[i]) {
      /* powerpc_emit_pop (compiler, 4, i); */
    }
  }

  powerpc_emit_addi (compiler, POWERPC_R1, POWERPC_R1, 16);
  ORC_ASM_CODE(compiler,"  blr\n");
  powerpc_emit(compiler, 0x4e800020);
}

static OrcTarget altivec_target = {
  "altivec",
#ifdef HAVE_POWERPC
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_powerpc_get_default_flags,
  orc_compiler_powerpc_init,
  orc_compiler_powerpc_assemble,
  { { 0 } },
  0,
  NULL,
  NULL,
  NULL,
  orc_powerpc_flush_cache

};

void
orc_powerpc_init (void)
{
  orc_target_register (&altivec_target);

  orc_compiler_powerpc_register_rules (&altivec_target);
}

static unsigned int
orc_compiler_powerpc_get_default_flags (void)
{
  unsigned int flags = 0;

#ifdef __powerpc64__
  flags |= ORC_TARGET_POWERPC_64BIT;
#endif
#if defined(__LITTLE_ENDIAN__)
  flags |= ORC_TARGET_POWERPC_LE;
#endif

  return flags;
}

static void
orc_compiler_powerpc_init (OrcCompiler *compiler)
{
  int i;

  if (compiler->target_flags & ORC_TARGET_POWERPC_64BIT) {
    compiler->is_64bit = TRUE;
  }

  for(i=0;i<32;i++){
    compiler->valid_regs[POWERPC_R0+i] = 1;
    compiler->valid_regs[POWERPC_V0+i] = 1;
  }
  compiler->valid_regs[POWERPC_R0] = 0; /* used for temp space */
  compiler->valid_regs[POWERPC_R1] = 0; /* stack pointer */
  compiler->valid_regs[POWERPC_R2] = 0; /* TOC pointer */
  compiler->valid_regs[POWERPC_R3] = 0; /* pointer to OrcExecutor */
  compiler->valid_regs[POWERPC_R13] = 0; /* reserved */

  compiler->tmpreg = POWERPC_V0;
  compiler->gp_tmpreg = POWERPC_R4;
  compiler->valid_regs[compiler->tmpreg] = 0;
  compiler->valid_regs[compiler->gp_tmpreg] = 0;

  for(i=14;i<32;i++){
    compiler->save_regs[POWERPC_R0 + i] = 1;
  }
  for(i=20;i<32;i++){
    compiler->save_regs[POWERPC_V0 + i] = 1;
  }

  compiler->loop_shift = 0;
  compiler->load_params = TRUE;
}

static void
powerpc_load_constants_outer (OrcCompiler *compiler)
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
        powerpc_emit_vxor(compiler, compiler->vars[i].alloc,
            compiler->vars[i].alloc, compiler->vars[i].alloc);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error(compiler,"bad vartype");
        break;
    }
  }

  orc_compiler_emit_invariants (compiler);

  for(i=0;i<compiler->n_constants;i++) {
    if (compiler->constants[i].is_long &&
        !compiler->constants[i].alloc_reg) {
      compiler->constants[i].alloc_reg =
          orc_compiler_get_constant_reg (compiler);
      if (compiler->constants[i].alloc_reg > 0) {
        powerpc_load_constant (compiler, i,
            compiler->constants[i].alloc_reg);
      }
    }
  }
}

static void
powerpc_load_inner_constants (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          powerpc_emit_load_address (compiler,
              compiler->vars[i].ptr_register,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
        } else {
          /* FIXME */
          ORC_ASM_CODE(compiler,"ERROR");
        }
        break;
      default:
        break;
    }
  }
}

static void
orc_powerpc_emit_loop (OrcCompiler* compiler, int update)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    compiler->insn_index = j;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT) continue;

    ORC_ASM_CODE(compiler,"# %d: %s\n", j, insn->opcode->name);

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
    for(k=0;k<ORC_N_COMPILER_VARIABLES;k++){
      if (compiler->vars[k].name == NULL) continue;
      if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
          compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
        if (compiler->vars[k].ptr_register) {
          powerpc_emit_addi (compiler,
              compiler->vars[k].ptr_register,
              compiler->vars[k].ptr_register,
              compiler->vars[k].size << compiler->loop_shift);
        } else {
            ORC_ASM_CODE(compiler,"ERROR\n");
        }
      }
    }
  }
}

static void
orc_compiler_powerpc_assemble (OrcCompiler *compiler)
{
  int k;
  int label_outer_loop_start;
  int label_loop_start;
  int label_leave;
  int set_vscr = FALSE;

  label_outer_loop_start = orc_compiler_label_new (compiler);
  label_loop_start = orc_compiler_label_new (compiler);
  label_leave = orc_compiler_label_new (compiler);

  {
    int i;

    orc_powerpc_emit_loop (compiler, 0);

    compiler->codeptr = compiler->code;
    free (compiler->asm_code);
    compiler->asm_code = NULL;
    compiler->asm_code_len = 0;
    memset (compiler->labels, 0, sizeof (compiler->labels));
    memset (compiler->labels_int, 0, sizeof (compiler->labels_int));
    compiler->n_fixups = 0;
    compiler->n_output_insns = 0;

    for(i=0;i<compiler->n_constants;i++) {
      compiler->constants[i].label = 0;
    }
  }

  if (compiler->error) return;

  powerpc_emit_prologue (compiler);

  if (orc_program_has_float (compiler)) {
    int tmp = POWERPC_V0;

    set_vscr = TRUE;

    ORC_ASM_CODE(compiler,"  vspltish %s, %d\n",
        powerpc_get_regname(tmp), 1);
    powerpc_emit_VX(compiler, 0x1000034c,
        powerpc_regnum(tmp), 1, 0);

    powerpc_emit_VX_b(compiler, "mtvscr", 0x10000644, tmp);
  }

  powerpc_load_constants_outer (compiler);

  if (compiler->program->is_2d) {
    powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m));
    powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
        compiler->loop_shift, 1);
    powerpc_emit_beq (compiler, label_leave);
    powerpc_emit_stw (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));
  }

  powerpc_load_inner_constants (compiler);

  powerpc_emit_label (compiler, label_outer_loop_start);

  powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
  powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
      compiler->loop_shift, 1);

  powerpc_emit_beq (compiler, label_leave);

  powerpc_emit (compiler, 0x7c0903a6);
  ORC_ASM_CODE (compiler, "  mtctr %s\n", powerpc_get_regname(POWERPC_R0));

  powerpc_emit_label (compiler, label_loop_start);

  orc_powerpc_emit_loop (compiler, 1);

  powerpc_emit_bne (compiler, label_loop_start);

  if (compiler->program->is_2d) {
    powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));
    powerpc_emit_addi_rec (compiler, POWERPC_R0, POWERPC_R0, -1);
    powerpc_emit_beq (compiler, label_leave);

    powerpc_emit_stw (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));

    for(k=0;k<ORC_N_COMPILER_VARIABLES;k++){
      if (compiler->vars[k].name == NULL) continue;
      if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
          compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
        if (compiler->vars[k].ptr_register) {
          powerpc_emit_load_address (compiler,
              compiler->vars[k].ptr_register,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]));
          powerpc_emit_lwz (compiler,
              POWERPC_R0,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, strides[k]));
          powerpc_emit_add (compiler,
              compiler->vars[k].ptr_register,
              compiler->vars[k].ptr_register,
              POWERPC_R0);
          if (compiler->is_64bit) {
            powerpc_emit_std (compiler,
                compiler->vars[k].ptr_register,
                POWERPC_R3,
                (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]));
          } else {
            powerpc_emit_stw (compiler,
                compiler->vars[k].ptr_register,
                POWERPC_R3,
                (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]));
          }
        } else {
          ORC_ASM_CODE(compiler,"ERROR\n");
        }
      }
    }

    powerpc_emit_b (compiler, label_outer_loop_start);
  }

  powerpc_emit_label (compiler, label_leave);

  for(k=0;k<4;k++){
    OrcVariable *var = &compiler->vars[ORC_VAR_A1 + k];

    if (compiler->vars[ORC_VAR_A1 + k].name == NULL) continue;

    powerpc_emit_addi (compiler,
        POWERPC_R0,
        POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[k]));

    if (var->size == 2) {
      powerpc_emit_vxor (compiler, POWERPC_V0, POWERPC_V0, POWERPC_V0);
      if (IS_POWERPC_BE (compiler)) {
        powerpc_emit_vmrghh (compiler, var->alloc, POWERPC_V0, var->alloc);
      } else {
        powerpc_emit_vmrglh (compiler, var->alloc, POWERPC_V0, var->alloc);
      }
    }

    if (IS_POWERPC_BE (compiler)) {
      ORC_ASM_CODE(compiler,"  lvsr %s, 0, %s\n",
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (POWERPC_R0));
      powerpc_emit_X (compiler, 0x7c00004c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(POWERPC_R0));
    } else {
      ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n",
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (POWERPC_R0));
      powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(POWERPC_R0));
    }

    powerpc_emit_vperm (compiler, var->alloc, var->alloc, var->alloc,
        POWERPC_V0);

    ORC_ASM_CODE(compiler,"  stvewx %s, 0, %s\n", 
      powerpc_get_regname (var->alloc),
      powerpc_get_regname (POWERPC_R0));
    powerpc_emit_X (compiler, 0x7c00018e,
        powerpc_regnum(var->alloc),
        0, powerpc_regnum(POWERPC_R0));
  }

  if (set_vscr) {
    int tmp = POWERPC_V0;

    ORC_ASM_CODE(compiler,"  vspltisw %s, %d\n",
        powerpc_get_regname(tmp), 0);
    powerpc_emit_VX(compiler, 0x1000038c,
        powerpc_regnum(tmp), 0, 0);

    powerpc_emit_VX_b(compiler, "mtvscr", 0x10000644, tmp);
  }
  powerpc_emit_epilogue (compiler);

  powerpc_emit_full_constants (compiler);

  powerpc_do_fixups (compiler);
}

