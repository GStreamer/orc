
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>

#include <orc/orcavx.h>
#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcsse.h>
#include <orc/orcx86insn.h>


/**
 * SECTION:orcavx
 * @title: AVX
 * @short_description: code generation for AVX/AVX2
 */

const char *
orc_x86_get_regname_avx (int i, OrcX86OpcodePrefix prefix)
{
  static const char *x86_regs[] = { "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
    "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13",
    "ymm14", "ymm15" };

  if (i >= X86_YMM0 && i <= X86_YMM15) {
    if (prefix == ORC_X86_AVX_VEX128_PREFIX) {
      return orc_x86_get_regname_sse (X86_XMM0 + (i - X86_YMM0));
    }
    return x86_regs[i - X86_YMM0];
  }
  return orc_x86_get_regname_sse (i);
}

void
orc_x86_emit_mov_memoffset_avx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    // AVX does not provide loads except vmov(u*, dq*, ap*)
    case 1:
      orc_avx_emit_pxor (compiler, reg2, reg2, reg2);
      orc_avx_sse_emit_pinsrb_memoffset (compiler, 0, offset, reg2, reg1, reg2);
      break;
    case 2:
      orc_avx_emit_pxor (compiler, reg2, reg2, reg2);
      orc_avx_sse_emit_pinsrw_memoffset (compiler, 0, offset, reg2, reg1, reg2);
      break;
    case 4:
      orc_avx_sse_emit_movd_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_avx_sse_emit_movq_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (is_aligned) {
        orc_avx_sse_emit_movdqa_load_memoffset (compiler, offset, reg1, reg2);
      } else {
        orc_avx_sse_emit_movdqu_load_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    case 32:
      if (is_aligned) {
        orc_avx_emit_movdqa_load_memoffset (compiler, offset, reg1, reg2);
      } else {
        orc_avx_emit_movdqu_load_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "bad load size %d", size);
      break;
  }
}

void
orc_x86_emit_mov_memindex_avx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    // VEX instructions zeroupper the high lane
    case 4:
      orc_avx_sse_emit_movd_load_memindex (compiler, offset, reg1, regindex,
          shift, reg2);
      break;
    case 8:
      orc_avx_sse_emit_movq_load_memindex (compiler, offset, reg1, regindex,
          shift, reg2);
      break;
    case 16:
      if (is_aligned) {
        orc_avx_sse_emit_movdqa_load_memindex (compiler, offset, reg1, regindex,
            shift, reg2);
      } else {
        orc_avx_sse_emit_movdqu_load_memindex (compiler, offset, reg1, regindex,
            shift, reg2);
      }
      break;
    case 32:
      if (is_aligned) {
        orc_avx_emit_movdqa_load_memindex (compiler, offset, reg1, regindex,
            shift, reg2);
      } else {
        orc_avx_emit_movdqu_load_memindex (compiler, offset, reg1, regindex,
            shift, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "bad load size %d", size);
      break;
  }
}

void
orc_x86_emit_mov_avx_memoffset (OrcCompiler *compiler, int size, int reg1,
    int offset, int reg2, int aligned, int uncached)
{
  switch (size) {
    case 1:
      orc_avx_sse_emit_pextrb_memoffset (compiler, 0, offset, reg1, reg2);
      break;
    case 2:
      orc_avx_sse_emit_pextrw_memoffset (compiler, 0, offset, reg1, reg2);
      break;
    case 4:
      orc_avx_sse_emit_movd_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_avx_sse_emit_movq_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (aligned) {
        if (uncached) {
          orc_avx_sse_emit_movntdq_store_memoffset (compiler, offset, reg1,
              reg2);
        } else {
          orc_avx_sse_emit_movdqa_store_memoffset (compiler, offset, reg1,
              reg2);
        }
      } else {
        orc_avx_sse_emit_movdqu_store_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    case 32:
      if (aligned) {
        if (uncached) {
          orc_avx_emit_movntdq_store_memoffset (compiler, offset, reg1, reg2);
        } else {
          orc_avx_emit_movdqa_store_memoffset (compiler, offset, reg1, reg2);
        }
      } else {
        orc_avx_emit_movdqu_store_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "bad store size %d", size);
      break;
  }
}

void
orc_avx_emit_broadcast (OrcCompiler *const compiler, const int s1, const int d,
    const int size)
{
  switch (size) {
    case 1:
      orc_avx_emit_pbroadcastb (compiler, s1, d);
      break;
    case 2:
      orc_avx_emit_pbroadcastw (compiler, s1, d);
      break;
    case 4:
      orc_avx_emit_pbroadcastd (compiler, s1, d);
      break;
    case 8:
      orc_avx_emit_pbroadcastq (compiler, s1, d);
      break;
    case 16:
      orc_avx_emit_permute2i128 (compiler, ORC_AVX_PERMUTE (0, 0), s1, s1, d);
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "this variable size cannot be broadcast");
      break;
  }
}

void
orc_avx_set_mxcsr (OrcCompiler *compiler)
{
  orc_vex_emit_cpuinsn_load_memoffset (compiler, ORC_X86_stmxcsr, 4, 0,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      compiler->exec_reg, 0, 0, ORC_X86_AVX_VEX128_PREFIX);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      compiler->exec_reg, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_C1]),
      compiler->exec_reg);

  orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_or_imm32_rm, 4, 0x8040,
      compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      compiler->exec_reg);

  orc_vex_emit_cpuinsn_load_memoffset (compiler, ORC_X86_ldmxcsr, 4, 0,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      compiler->exec_reg, 0, 0, ORC_X86_AVX_VEX128_PREFIX);
}

void
orc_avx_restore_mxcsr (OrcCompiler *compiler)
{
  orc_vex_emit_cpuinsn_load_memoffset (compiler, ORC_X86_ldmxcsr, 4, 0,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      compiler->exec_reg, 0, 0, ORC_X86_AVX_VEX128_PREFIX);
}
