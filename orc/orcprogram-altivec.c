
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcpowerpc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


void orc_compiler_powerpc_init (OrcCompiler *compiler);
unsigned int orc_compiler_powerpc_get_default_flags (void);
void orc_compiler_powerpc_assemble (OrcCompiler *compiler);
void orc_compiler_powerpc_register_rules (OrcTarget *target);


void
powerpc_emit_prologue (OrcCompiler *compiler)
{
  int i;

  ORC_ASM_CODE (compiler, ".global %s\n", compiler->program->name);
  ORC_ASM_CODE (compiler, "%s:\n", compiler->program->name);

  powerpc_emit_stwu (compiler, POWERPC_R1, POWERPC_R1, -16);

  for(i=POWERPC_R13;i<=POWERPC_R31;i++){
    if (compiler->used_regs[i]) {
      //powerpc_emit_push (compiler, 4, i);
    }
  }
}

void
powerpc_emit_epilogue (OrcCompiler *compiler)
{
  int i;

  for(i=POWERPC_R31;i>=POWERPC_R31;i--){
    if (compiler->used_regs[i]) {
      //powerpc_emit_pop (compiler, 4, i);
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
  orc_compiler_powerpc_assemble
};

void
orc_powerpc_init (void)
{
  orc_target_register (&altivec_target);

  orc_compiler_powerpc_register_rules (&altivec_target);
}

unsigned int
orc_compiler_powerpc_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_powerpc_init (OrcCompiler *compiler)
{
  int i;

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
  compiler->gp_tmpreg = POWERPC_R0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  for(i=14;i<32;i++){
    compiler->save_regs[POWERPC_R0 + i] = 1;
  }
  for(i=20;i<32;i++){
    compiler->save_regs[POWERPC_V0 + i] = 1;
  }

  compiler->loop_shift = 0;
}

#if 0
static void
powerpc_load_constant (OrcCompiler *p, int i, int reg)
{
  int j;
  int value = p->constants[i].value;
  int greg = POWERPC_R31;
  int label_skip, label_data;

#if 0
  switch (p->constants[i].type) {
    case ORC_CONST_ZERO:
      powerpc_emit_VX_2(p, "vxor", 0x100004c4, reg, reg, reg);
      break;
    case ORC_CONST_SPLAT_B:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltisb %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000020c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_W:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltish %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000024c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    case ORC_CONST_SPLAT_L:
      if (value < 16 && value >= -16) {
        ORC_ASM_CODE(p,"  vspltisw %s, %d\n",
            powerpc_get_regname(reg), value&0x1f);
        powerpc_emit_VX(p, 0x1000028c,
            powerpc_regnum(reg), value & 0x1f, 0);
      } else {
        ORC_COMPILER_ERROR(p,"can't load constant");
      }
      break;
    default:
      ORC_COMPILER_ERROR(p,"unhandled");
      break;
  }
#endif

  switch (p->constants[i].type) {
    case ORC_CONST_ZERO:
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = 0;
      }
      break;
    case ORC_CONST_SPLAT_B:
      value &= 0xff;
      value |= (value<<8);
      value |= (value<<16);
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    case ORC_CONST_SPLAT_W:
      value &= 0xffff;
      value |= (value<<16);
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    case ORC_CONST_SPLAT_L:
      for(j=0;j<4;j++){
        p->constants[i].full_value[j] = value;
      }
      break;
    default:
      break;
  }

  label_skip = orc_compiler_label_new (p);
  label_data = orc_compiler_label_new (p);

  powerpc_emit_b (p, label_skip);

  while ((p->codeptr - p->program->code) & 0xf) {
    ORC_ASM_CODE(p,"  .long 0x00000000\n");
    powerpc_emit (p, 0x00000000);
  }

  powerpc_emit_label (p, label_data);
  for(j=0;j<4;j++){
    ORC_ASM_CODE(p,"  .long 0x%08x\n", p->constants[i].full_value[j]);
    powerpc_emit (p, p->constants[i].full_value[j]);
  }

  powerpc_emit_label (p, label_skip);
  powerpc_emit_lwz (p,
      greg,
      POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, program));
  powerpc_emit_lwz (p,
      greg, greg,
      (int)ORC_STRUCT_OFFSET(OrcProgram, code_exec));

  powerpc_add_fixup (p, 1, p->codeptr, label_data);
  {
    unsigned int insn;

    ORC_ASM_CODE(p,"  addi %s, %s, %db - %s\n",
        powerpc_get_regname(greg),
        powerpc_get_regname(greg), label_data, p->program->name);
    insn = (14<<26) | (powerpc_regnum (greg)<<21) | (powerpc_regnum (greg)<<16);
    insn |= 0;

    powerpc_emit (p, insn);
  }

  ORC_ASM_CODE(p,"  lvx %s, 0, %s\n",
      powerpc_get_regname(reg),
      powerpc_get_regname(greg));
  powerpc_emit_X (p, 0x7c0000ce, reg, 0, greg);

}

void
powerpc_load_constants (OrcCompiler *compiler)
{
  OrcVariable *var;
  int i;
  int j;
  int greg = POWERPC_R0;

  for(i=0;i<ORC_N_VARIABLES;i++){
    var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        j = compiler->n_constants;
        compiler->n_constants++;
        if (compiler->vars[i].size == 1) {
          compiler->constants[j].type = ORC_CONST_SPLAT_B;
        } else if (compiler->vars[i].size == 2) {
          compiler->constants[j].type = ORC_CONST_SPLAT_W;
        } else {
          compiler->constants[j].type = ORC_CONST_SPLAT_L;
        }
        compiler->constants[j].value = compiler->vars[i].value;
        compiler->constants[j].alloc_reg = compiler->vars[i].alloc;
        break;
      case ORC_VAR_TYPE_PARAM:
        powerpc_emit_addi (compiler,
            greg, POWERPC_R3,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, params[i]));
        ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n", 
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (greg));
        powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(var->alloc),
            0, powerpc_regnum(greg));

        ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n", 
            powerpc_get_regname (POWERPC_V0),
            powerpc_get_regname (greg));
        powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
            0, powerpc_regnum(greg));

        ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (var->alloc),
            powerpc_get_regname (POWERPC_V0));
        powerpc_emit_VA (compiler, 4,
            powerpc_regnum(var->alloc),
            powerpc_regnum(var->alloc),
            powerpc_regnum(var->alloc),
            powerpc_regnum(POWERPC_V0), 43);
        switch (var->size) {
          case 1:
            ORC_ASM_CODE(compiler,"  vspltb %s, %s, 3\n",
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000020c,
                powerpc_regnum(var->alloc), 3, powerpc_regnum(var->alloc));
            break;
          case 2:
            ORC_ASM_CODE(compiler,"  vsplth %s, %s, 1\n", 
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000024c,
                powerpc_regnum(var->alloc), 1, powerpc_regnum(var->alloc));
            break;
          case 4:
            ORC_ASM_CODE(compiler,"  vspltw %s, %s, 0\n", 
                powerpc_get_regname (var->alloc),
                powerpc_get_regname (var->alloc));
            powerpc_emit_VX (compiler, 0x1000028c,
                powerpc_regnum(var->alloc), 0, powerpc_regnum(var->alloc));
            break;
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          powerpc_emit_lwz (compiler,
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

  for(i=0;i<compiler->n_constants;i++){
    if (compiler->constants[i].alloc_reg > 0) {
      powerpc_load_constant (compiler, i, compiler->constants[i].alloc_reg);
    }
  }
}

void
powerpc_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (compiler->loop_shift) {
    case 0:
      switch (var->size) {
        case 1:
          ORC_ASM_CODE(compiler,"  lvebx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00000e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 2:
          ORC_ASM_CODE(compiler,"  lvehx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00004e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 4:
          ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
      }
      ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg));
      ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (compiler, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      break;
    default:
      ORC_ASM_CODE(compiler,"ERROR\n");
  }
}

void
powerpc_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  ptr_reg = var->ptr_register;

  switch (compiler->loop_shift) {
    case 0:
      ORC_ASM_CODE(compiler,"  lvsr %s, 0, %s\n", 
          powerpc_get_regname (POWERPC_V0),
          powerpc_get_regname (ptr_reg));
      powerpc_emit_X (compiler, 0x7c00004c, powerpc_regnum(POWERPC_V0),
          0, powerpc_regnum(ptr_reg));
      ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (var->alloc),
          powerpc_get_regname (POWERPC_V0));
      powerpc_emit_VA (compiler, 4,
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(var->alloc),
          powerpc_regnum(POWERPC_V0), 43);
      switch (var->size) {
        case 1:
          ORC_ASM_CODE(compiler,"  stvebx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00010e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 2:
          ORC_ASM_CODE(compiler,"  stvehx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00014e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
        case 4:
          ORC_ASM_CODE(compiler,"  stvewx %s, 0, %s\n", 
              powerpc_get_regname (var->alloc),
              powerpc_get_regname (ptr_reg));
          powerpc_emit_X (compiler, 0x7c00018e,
              powerpc_regnum(var->alloc),
              0, powerpc_regnum(ptr_reg));
          break;
      }
      break;
    default:
      ORC_ASM_CODE(compiler,"ERROR\n");
  }
}
#endif

void
powerpc_load_inner_constants (OrcCompiler *compiler)
{
  OrcVariable *var;
  int i;

  for(i=0;i<ORC_N_VARIABLES;i++){
    var = compiler->vars + i;

    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          powerpc_emit_lwz (compiler,
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

void
orc_compiler_powerpc_assemble (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  //OrcVariable *args[10];
  OrcRule *rule;
  int label_outer_loop_start;
  int label_loop_start;
  int label_leave;

  label_outer_loop_start = orc_compiler_label_new (compiler);
  label_loop_start = orc_compiler_label_new (compiler);
  label_leave = orc_compiler_label_new (compiler);

  powerpc_emit_prologue (compiler);

  if (compiler->program->is_2d) {
    powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m));
    powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
        compiler->loop_shift, 1);
    powerpc_emit_beq (compiler, label_leave);
    powerpc_emit_stw (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));
  }

  //powerpc_load_constants (compiler);
  powerpc_load_inner_constants (compiler);

  for(k=0;k<4;k++){
    OrcVariable *var = &compiler->vars[ORC_VAR_A1 + k];

    if (compiler->vars[ORC_VAR_A1 + k].name == NULL) continue;

      //powerpc_emit_VX_2(p, "vxor", 0x100004c4, reg, reg, reg);
    powerpc_emit_vxor (compiler, var->alloc, var->alloc, var->alloc);
  }

  powerpc_emit_label (compiler, label_outer_loop_start);

  powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, n));
  powerpc_emit_srawi (compiler, POWERPC_R0, POWERPC_R0,
      compiler->loop_shift, 1);

  powerpc_emit_beq (compiler, label_leave);

  powerpc_emit (compiler, 0x7c0903a6);
  ORC_ASM_CODE (compiler, "  mtctr %s\n", powerpc_get_regname(POWERPC_R0));

  powerpc_emit_label (compiler, label_loop_start);

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
          //powerpc_emit_load_src (compiler, var);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }

    rule = insn->rule;
    if (rule) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_ASM_CODE(compiler,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      OrcVariable *var = compiler->vars + insn->dest_args[k];

      if (opcode->dest_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_DEST:
          //powerpc_emit_store_dest (compiler, var);
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
        powerpc_emit_addi (compiler,
            compiler->vars[k].ptr_register,
            compiler->vars[k].ptr_register,
            compiler->vars[k].size << compiler->loop_shift);
      } else {
        ORC_ASM_CODE(compiler,"ERROR\n");
      }
    }
  }

  powerpc_emit_bne (compiler, label_loop_start);

  if (compiler->program->is_2d) {
    powerpc_emit_lwz (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));
    powerpc_emit_addi_rec (compiler, POWERPC_R0, POWERPC_R0, -1);
    powerpc_emit_beq (compiler, label_leave);

    powerpc_emit_stw (compiler, POWERPC_R0, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutorAlt, m_index));

    for(k=0;k<ORC_N_VARIABLES;k++){
      if (compiler->vars[k].name == NULL) continue;
      if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
          compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
        if (compiler->vars[k].ptr_register) {
          powerpc_emit_lwz (compiler,
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
          powerpc_emit_stw (compiler,
              compiler->vars[k].ptr_register,
              POWERPC_R3,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]));
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
      powerpc_emit_vxor (compiler,
          POWERPC_V0, POWERPC_V0, POWERPC_V0);
      powerpc_emit_VX_2 (compiler, "vsum4shs", 0x10000648,
          POWERPC_V0, var->alloc, POWERPC_V0);
      powerpc_emit_vor (compiler, var->alloc, POWERPC_V0, POWERPC_V0);
    }

    ORC_ASM_CODE(compiler,"  lvsr %s, 0, %s\n", 
        powerpc_get_regname (POWERPC_V0),
        powerpc_get_regname (POWERPC_R0));
    powerpc_emit_X (compiler, 0x7c00004c, powerpc_regnum(POWERPC_V0),
        0, powerpc_regnum(POWERPC_R0));

    ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n", 
        powerpc_get_regname (var->alloc),
        powerpc_get_regname (var->alloc),
        powerpc_get_regname (var->alloc),
        powerpc_get_regname (POWERPC_V0));
    powerpc_emit_VA (compiler, 4,
        powerpc_regnum(var->alloc),
        powerpc_regnum(var->alloc),
        powerpc_regnum(var->alloc),
        powerpc_regnum(POWERPC_V0), 43);

    ORC_ASM_CODE(compiler,"  stvewx %s, 0, %s\n", 
        powerpc_get_regname (var->alloc),
        powerpc_get_regname (POWERPC_R0));
    powerpc_emit_X (compiler, 0x7c00018e,
        powerpc_regnum(var->alloc),
        0, powerpc_regnum(POWERPC_R0));
  }

  powerpc_emit_epilogue (compiler);

  powerpc_do_fixups (compiler);

  powerpc_flush (compiler);
}

