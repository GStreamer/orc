
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcinternal.h>
#include <orc/orcarm.h>
#include <orc/orcdebug.h>

#include <orc/orcneon.h>

static void orc_neon_emit_loadib (OrcCompiler *compiler, OrcVariable *dest, int param);
static void orc_neon_emit_loadiw (OrcCompiler *compiler, OrcVariable *dest, int param);
static void orc_neon_emit_loadiq (OrcCompiler *compiler, OrcVariable *dest, long long param);
static void orc_neon_emit_loadpq (OrcCompiler *compiler, int dest, int param);

const char *orc_neon_reg_name (int reg)
{
  static const char *vec_regs[] = {
    "d0", "d1", "d2", "d3",
    "d4", "d5", "d6", "d7",
    "d8", "d9", "d10", "d11",
    "d12", "d13", "d14", "d15",
    "d16", "d17", "d18", "d19",
    "d20", "d21", "d22", "d23",
    "d24", "d25", "d26", "d27",
    "d28", "d29", "d30", "d31",
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  return vec_regs[reg&0x1f];
}

const char *orc_neon_reg_name_quad (int reg)
{
  static const char *vec_regs[] = {
    "q0", "ERROR", "q1", "ERROR",
    "q2", "ERROR", "q3", "ERROR",
    "q4", "ERROR", "q5", "ERROR",
    "q6", "ERROR", "q7", "ERROR",
    "q8", "ERROR", "q9", "ERROR",
    "q10", "ERROR", "q11", "ERROR",
    "q12", "ERROR", "q13", "ERROR",
    "q14", "ERROR", "q15", "ERROR",
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  return vec_regs[reg&0x1f];
}

/** the names of the SIMD registers when used in a scalar way */
const char *orc_neon64_reg_name_scalar (int reg, int size)
{
  static const char *vec_regs[5][32] = {
    { /** 8-bit */
      "b0", "b1", "b2", "b3",
      "b4", "b5", "b6", "b7",
      "b8", "b9", "b10", "b11",
      "b12", "b13", "b14", "b15",
      "b16", "b17", "b18", "b19",
      "b20", "b21", "b22", "b23",
      "b24", "b25", "b26", "b27",
      "b28", "b29", "b30", "b31"
    },
    { /** 16-bit */
      "h0", "h1", "h2", "h3",
      "h4", "h5", "h6", "h7",
      "h8", "h9", "h10", "h11",
      "h12", "h13", "h14", "h15",
      "h16", "h17", "h18", "h19",
      "h20", "h21", "h22", "h23",
      "h24", "h25", "h26", "h27",
      "h28", "h29", "h30", "h31"
    },
    { /** 32-bit */
      "s0", "s1", "s2", "s3",
      "s4", "s5", "s6", "s7",
      "s8", "s9", "s10", "s11",
      "s12", "s13", "s14", "s15",
      "s16", "s17", "s18", "s19",
      "s20", "s21", "s22", "s23",
      "s24", "s25", "s26", "s27",
      "s28", "s29", "s30", "s31"
    },
    { /** 64-bit */
      "d0", "d1", "d2", "d3",
      "d4", "d5", "d6", "d7",
      "d8", "d9", "d10", "d11",
      "d12", "d13", "d14", "d15",
      "d16", "d17", "d18", "d19",
      "d20", "d21", "d22", "d23",
      "d24", "d25", "d26", "d27",
      "d28", "d29", "d30", "d31"
    },
    { /** 128-bit */
      "q0", "q1", "q2", "q3",
      "q4", "q5", "q6", "q7",
      "q8", "q9", "q10", "q11",
      "q12", "q13", "q14", "q15",
      "q16", "q17", "q18", "q19",
      "q20", "q21", "q22", "q23",
      "q24", "q25", "q26", "q27",
      "q28", "q29", "q30", "q31"
    }
  };
  int size_idx;

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  size_idx = -1;
  while (size) {
    size_idx++;
    size >>= 1;
  }

  if (size_idx < 0 || size_idx >= 5) {
    return "ERROR";
  }

  return vec_regs[size_idx][reg&0x1f];
}

/** the names of the SIMD vector registers when used for vectorization */
const char *orc_neon64_reg_name_vector (int reg, int size, int quad)
{
  static const char *vec_regs[8][32] = {
    {
      "v0.8b", "v1.8b", "v2.8b", "v3.8b",
      "v4.8b", "v5.8b", "v6.8b", "v7.8b",
      "v8.8b", "v9.8b", "v10.8b", "v11.8b",
      "v12.8b", "v13.8b", "v14.8b", "v15.8b",
      "v16.8b", "v17.8b", "v18.8b", "v19.8b",
      "v20.8b", "v21.8b", "v22.8b", "v23.8b",
      "v24.8b", "v25.8b", "v26.8b", "v27.8b",
      "v28.8b", "v29.8b", "v30.8b", "v31.8b"
    },
    {
      "v0.16b", "v1.16b", "v2.16b", "v3.16b",
      "v4.16b", "v5.16b", "v6.16b", "v7.16b",
      "v8.16b", "v9.16b", "v10.16b", "v11.16b",
      "v12.16b", "v13.16b", "v14.16b", "v15.16b",
      "v16.16b", "v17.16b", "v18.16b", "v19.16b",
      "v20.16b", "v21.16b", "v22.16b", "v23.16b",
      "v24.16b", "v25.16b", "v26.16b", "v27.16b",
      "v28.16b", "v29.16b", "v30.16b", "v31.16b"
    },
    {
      "v0.4h", "v1.4h", "v2.4h", "v3.4h",
      "v4.4h", "v5.4h", "v6.4h", "v7.4h",
      "v8.4h", "v9.4h", "v10.4h", "v11.4h",
      "v12.4h", "v13.4h", "v14.4h", "v15.4h",
      "v16.4h", "v17.4h", "v18.4h", "v19.4h",
      "v20.4h", "v21.4h", "v22.4h", "v23.4h",
      "v24.4h", "v25.4h", "v26.4h", "v27.4h",
      "v28.4h", "v29.4h", "v30.4h", "v31.4h"
    },
    {
      "v0.8h", "v1.8h", "v2.8h", "v3.8h",
      "v4.8h", "v5.8h", "v6.8h", "v7.8h",
      "v8.8h", "v9.8h", "v10.8h", "v11.8h",
      "v12.8h", "v13.8h", "v14.8h", "v15.8h",
      "v16.8h", "v17.8h", "v18.8h", "v19.8h",
      "v20.8h", "v21.8h", "v22.8h", "v23.8h",
      "v24.8h", "v25.8h", "v26.8h", "v27.8h",
      "v28.8h", "v29.8h", "v30.8h", "v31.8h"
    },
    {
      "v0.2s", "v1.2s", "v2.2s", "v3.2s",
      "v4.2s", "v5.2s", "v6.2s", "v7.2s",
      "v8.2s", "v9.2s", "v10.2s", "v11.2s",
      "v12.2s", "v13.2s", "v14.2s", "v15.2s",
      "v16.2s", "v17.2s", "v18.2s", "v19.2s",
      "v20.2s", "v21.2s", "v22.2s", "v23.2s",
      "v24.2s", "v25.2s", "v26.2s", "v27.2s",
      "v28.2s", "v29.2s", "v30.2s", "v31.2s"
    },
    {
      "v0.4s", "v1.4s", "v2.4s", "v3.4s",
      "v4.4s", "v5.4s", "v6.4s", "v7.4s",
      "v8.4s", "v9.4s", "v10.4s", "v11.4s",
      "v12.4s", "v13.4s", "v14.4s", "v15.4s",
      "v16.4s", "v17.4s", "v18.4s", "v19.4s",
      "v20.4s", "v21.4s", "v22.4s", "v23.4s",
      "v24.4s", "v25.4s", "v26.4s", "v27.4s",
      "v28.4s", "v29.4s", "v30.4s", "v31.4s"
    },
    {
      "v0.1d", "v1.1d", "v2.1d", "v3.1d",
      "v4.1d", "v5.1d", "v6.1d", "v7.1d",
      "v8.1d", "v9.1d", "v10.1d", "v11.1d",
      "v12.1d", "v13.1d", "v14.1d", "v15.1d",
      "v16.1d", "v17.1d", "v18.1d", "v19.1d",
      "v20.1d", "v21.1d", "v22.1d", "v23.1d",
      "v24.1d", "v25.1d", "v26.1d", "v27.1d",
      "v28.1d", "v29.1d", "v30.1d", "v31.1d"
    },
    {
      "v0.2d", "v1.2d", "v2.2d", "v3.2d",
      "v4.2d", "v5.2d", "v6.2d", "v7.2d",
      "v8.2d", "v9.2d", "v10.2d", "v11.2d",
      "v12.2d", "v13.2d", "v14.2d", "v15.2d",
      "v16.2d", "v17.2d", "v18.2d", "v19.2d",
      "v20.2d", "v21.2d", "v22.2d", "v23.2d",
      "v24.2d", "v25.2d", "v26.2d", "v27.2d",
      "v28.2d", "v29.2d", "v30.2d", "v31.2d"
    }
  };
  int size_idx;

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  size_idx = -1;
  while (size) {
    size_idx++;
    size >>= 1;
  }

  if (size_idx < 0 || size_idx >= 4) {
    return "ERROR";
  }

  if (quad != 0 && quad != 1) {
    return "ERROR";
  }

  return vec_regs[size_idx*2+quad][reg&0x1f];
}

/** a single element from a SIMD vector register as a scalar operand */
const char *orc_neon64_reg_name_vector_single (int reg, int size)
{
  static const char *vec_regs[4][32] = {
    {
      "v0.b", "v1.b", "v2.b", "v3.b",
      "v4.b", "v5.b", "v6.b", "v7.b",
      "v8.b", "v9.b", "v10.b", "v11.b",
      "v12.b", "v13.b", "v14.b", "v15.b",
      "v16.b", "v17.b", "v18.b", "v19.b",
      "v20.b", "v21.b", "v22.b", "v23.b",
      "v24.b", "v25.b", "v26.b", "v27.b",
      "v28.b", "v29.b", "v30.b", "v31.b"
    },
    {
      "v0.h", "v1.h", "v2.h", "v3.h",
      "v4.h", "v5.h", "v6.h", "v7.h",
      "v8.h", "v9.h", "v10.h", "v11.h",
      "v12.h", "v13.h", "v14.h", "v15.h",
      "v16.h", "v17.h", "v18.h", "v19.h",
      "v20.h", "v21.h", "v22.h", "v23.h",
      "v24.h", "v25.h", "v26.h", "v27.h",
      "v28.h", "v29.h", "v30.h", "v31.h"
    },
    {
      "v0.s", "v1.s", "v2.s", "v3.s",
      "v4.s", "v5.s", "v6.s", "v7.s",
      "v8.s", "v9.s", "v10.s", "v11.s",
      "v12.s", "v13.s", "v14.s", "v15.s",
      "v16.s", "v17.s", "v18.s", "v19.s",
      "v20.s", "v21.s", "v22.s", "v23.s",
      "v24.s", "v25.s", "v26.s", "v27.s",
      "v28.s", "v29.s", "v30.s", "v31.s"
    },
    {
      "v0.d", "v1.d", "v2.d", "v3.d",
      "v4.d", "v5.d", "v6.d", "v7.d",
      "v8.d", "v9.d", "v10.d", "v11.d",
      "v12.d", "v13.d", "v14.d", "v15.d",
      "v16.d", "v17.d", "v18.d", "v19.d",
      "v20.d", "v21.d", "v22.d", "v23.d",
      "v24.d", "v25.d", "v26.d", "v27.d",
      "v28.d", "v29.d", "v30.d", "v31.d"
    },
  };

  int size_idx;

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  size_idx = -1;
  while (size) {
    size_idx++;
    size >>= 1;
  }

  if (size_idx < 0 || size_idx >= 4) {
    return "ERROR";
  }

  return vec_regs[size_idx][reg&0x1f];
}

static void
orc_neon_emit_binary (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name (src1),
      orc_neon_reg_name (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon64_emit_binary (OrcCompiler *p, const char *name, unsigned int code,
    OrcVariable dest, OrcVariable src1, OrcVariable src2, int vec_shift)
{
  int is_quad = 0;

  if (p->insn_shift == vec_shift + 1) {
    is_quad = 1;
  } else if (p->insn_shift > vec_shift + 1) {
    ORC_COMPILER_ERROR(p, "out-of-shift");
    return;
  }

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon64_reg_name_vector (dest.alloc, dest.size, is_quad),
      orc_neon64_reg_name_vector (src1.alloc, src1.size, is_quad),
      orc_neon64_reg_name_vector (src2.alloc, src2.size, is_quad));
  code |= (is_quad&0x1)<<30;
  code |= (src2.alloc&0x1f)<<16;
  code |= (src1.alloc&0x1f)<<5;
  code |= (dest.alloc&0x1f);
  orc_arm_emit (p, code);
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
orc_neon_emit_binary_long (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name (src1),
      orc_neon_reg_name (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

#if 0
static void
orc_neon_emit_binary_narrow (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name_quad (src1),
      orc_neon_reg_name_quad (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}
#endif

static void
orc_neon_emit_binary_quad (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name_quad (src1),
      orc_neon_reg_name_quad (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  code |= 0x40;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon64_emit_unary (OrcCompiler *p, const char *name, unsigned int code,
    OrcVariable dest, OrcVariable src1, int vec_shift)
{
  int is_quad = 0;

  if (p->insn_shift == vec_shift + 1) {
    is_quad = 1;
  } else if (p->insn_shift > vec_shift + 1) {
    ORC_COMPILER_ERROR(p, "out-of-shift");
    return;
  }

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon64_reg_name_vector (dest.alloc, dest.size, is_quad),
      orc_neon64_reg_name_vector (src1.alloc, src1.size, is_quad));
  code |= (is_quad&0x1)<<30;
  code |= (src1.alloc&0x1f)<<5;
  code |= (dest.alloc&0x1f);
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary_long (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary_narrow (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name_quad (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary_quad (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name_quad (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  code |= 0x40;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_mov (OrcCompiler *compiler, OrcVariable dest, OrcVariable src)
{
  if (compiler->is_64bit) {
      orc_neon64_emit_binary (compiler, "orr", 0x0ea01c00,
          dest, src, src, compiler->insn_shift);
  } else {
      orc_neon_emit_binary (compiler, "vorr", 0xf2200110,
          dest.alloc, src.alloc, src.alloc);
  }
}

static void
orc_neon_emit_mov_quad (OrcCompiler *compiler, OrcVariable dest, OrcVariable src)
{
  if (compiler->is_64bit) {
      orc_neon64_emit_binary (compiler, "orr", 0x0ea01c00,
          dest, src, src, compiler->insn_shift - 1);
  } else {
      orc_neon_emit_binary_quad (compiler, "vorr", 0xf2200110,
          dest.alloc, src.alloc, src.alloc);
  }
}

void
orc_neon_preload (OrcCompiler *compiler, OrcVariable *var, int write,
    int offset)
{
  orc_uint32 code;

  /* Don't use multiprocessing extensions */
  write = FALSE;

  ORC_ASM_CODE(compiler,"  pld%s [%s, #%d]\n",
      write ? "w" : "",
      orc_arm_reg_name (var->ptr_register), offset);
  code = 0xf510f000;
  if (!write) code |= (1<<22);
  code |= (var->ptr_register&0xf) << 16;
  if (offset < 0) {
    code |= ((-offset)&0xfff) << 0;
  } else {
    code |= (offset&0xfff) << 0;
    code |= (1<<23);
  }
  orc_arm_emit (compiler, code);
}

#if 0
void
orc_neon_load_halfvec_aligned (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.32 %s[0], [%s]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf4a0080d;
  code |= (var->ptr_register&0xf) << 16;
  code |= (var->alloc&0xf) << 12;
  code |= ((var->alloc>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_load_vec_aligned (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf42007cd;
  code |= (var->ptr_register&0xf) << 16;
  code |= (var->alloc&0xf) << 12;
  code |= ((var->alloc>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_load_vec_unaligned (OrcCompiler *compiler, OrcVariable *var,
    int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.8 %s, [%s]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf420070d;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->alloc)&0xf) << 12;
  code |= (((var->alloc)>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
#if 0
  /* used with need_mask_regs */
  ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
      orc_neon_reg_name (var->aligned_data + 1),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf42007cd;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->aligned_data+1)&0xf) << 12;
  code |= (((var->aligned_data+1)>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);

  ORC_ASM_CODE(compiler,"  vtbl.8 %s, {%s,%s}, %s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->aligned_data),
      orc_neon_reg_name (var->aligned_data+1),
      orc_neon_reg_name (var->mask_alloc));
  code = NEON_BINARY(0xf3b00900, var->alloc, var->aligned_data,
      var->mask_alloc);
  orc_arm_emit (compiler, code);
/* orc_neon_emit_mov (compiler, var->alloc, var->mask_alloc); */

  orc_neon_emit_mov (compiler, var->aligned_data, var->aligned_data + 1);
#endif
}

void
orc_neon_load_halfvec_unaligned (OrcCompiler *compiler, OrcVariable *var,
    int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.8 %s, [%s]\n",
      orc_neon_reg_name (var->alloc),
      orc_arm_reg_name (var->ptr_register));
  code = 0xf420070d;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->alloc)&0xf) << 12;
  code |= (((var->alloc)>>4)&0x1) << 22;
  /* code |= (!update) << 1; */
  code |= (1) << 1;
  orc_arm_emit (compiler, code);

  if (update) {
    orc_arm_emit_add_imm (compiler, var->ptr_register,
        var->ptr_register, 4);
  }
#if 0
  /* used with need_mask_regs */
  ORC_ASM_CODE(compiler,"  vld1.32 %s[1], [%s]%s\n",
      orc_neon_reg_name (var->aligned_data),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf4a0088d;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->aligned_data)&0xf) << 12;
  code |= (((var->aligned_data)>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);

  ORC_ASM_CODE(compiler,"  vtbl.8 %s, {%s,%s}, %s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->aligned_data),
      orc_neon_reg_name (var->aligned_data + 1),
      orc_neon_reg_name (var->mask_alloc));
  code = NEON_BINARY(0xf3b00900, var->alloc, var->aligned_data, var->mask_alloc);
  orc_arm_emit (compiler, code);

  orc_neon_emit_unary (compiler, "vrev64.i32", 0xf3b80000,
      var->aligned_data, var->aligned_data);
#endif
}

void
orc_neon_load_fourvec_aligned (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s, %s, %s }, [%s,:256]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->alloc + 1),
      orc_neon_reg_name (var->alloc + 2),
      orc_neon_reg_name (var->alloc + 3),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf42002dd;
  code |= (var->ptr_register&0xf) << 16;
  code |= (var->alloc&0xf) << 12;
  code |= ((var->alloc>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_load_fourvec_unaligned (OrcCompiler *compiler, OrcVariable *var,
    int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s, %s, %s }, [%s]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->alloc + 1),
      orc_neon_reg_name (var->alloc + 2),
      orc_neon_reg_name (var->alloc + 3),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf420020d;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->alloc)&0xf) << 12;
  code |= (((var->alloc)>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_load_twovec_aligned (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s }, [%s,:128]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->alloc + 1),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf4200aed;
  code |= (var->ptr_register&0xf) << 16;
  code |= (var->alloc&0xf) << 12;
  code |= ((var->alloc>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_load_twovec_unaligned (OrcCompiler *compiler, OrcVariable *var,
    int update)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s }, [%s]%s\n",
      orc_neon_reg_name (var->alloc),
      orc_neon_reg_name (var->alloc + 1),
      orc_arm_reg_name (var->ptr_register),
      update ? "!" : "");
  code = 0xf4200a0d;
  code |= (var->ptr_register&0xf) << 16;
  code |= ((var->alloc)&0xf) << 12;
  code |= (((var->alloc)>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_loadb (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;
  int i;

  if (var->is_aligned && compiler->insn_shift == 5) {
    orc_neon_load_fourvec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 4) {
    orc_neon_load_twovec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 3) {
    orc_neon_load_vec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 2) {
    orc_neon_load_halfvec_aligned (compiler, var, update);
  } else if (compiler->insn_shift == 5) {
    orc_neon_load_fourvec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 4) {
    orc_neon_load_twovec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 3) {
    orc_neon_load_vec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 2) {
    orc_neon_load_halfvec_unaligned (compiler, var, update);
  } else {
    if (compiler->insn_shift > 1) {
      ORC_ERROR("slow load");
    }
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.8 %s[%d], [%s]%s\n",
          orc_neon_reg_name (var->alloc + (i>>3)), i&7,
          orc_arm_reg_name (var->ptr_register),
          update ? "!" : "");
      code = NEON_BINARY(0xf4a0000d, var->alloc + (i>>3),
          var->ptr_register, 0);
      code |= (i&7) << 5;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadw (OrcCompiler *compiler, OrcVariable *var, int update)
{
  if (var->is_aligned && compiler->insn_shift == 3) {
    orc_neon_load_twovec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 2) {
    orc_neon_load_vec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 1) {
    orc_neon_load_halfvec_aligned (compiler, var, update);
  } else if (compiler->insn_shift == 3) {
    orc_neon_load_twovec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 2) {
    orc_neon_load_vec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 1) {
    orc_neon_load_halfvec_unaligned (compiler, var, update);
  } else {
    orc_uint32 code;
    int i;

    if (compiler->insn_shift == 2) {
      orc_neon_load_vec_aligned (compiler, var, update);
      return;
    } else if (compiler->insn_shift == 1) {
      orc_neon_load_halfvec_aligned (compiler, var, update);
      return;
    }
    if (compiler->insn_shift > 1) {
      ORC_ERROR("slow load");
    }
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.16 %s[%d], [%s]%s\n",
          orc_neon_reg_name (var->alloc + (i>>2)), i&3,
          orc_arm_reg_name (var->ptr_register),
          update ? "!" : "");
      code = NEON_BINARY(0xf4a0040d, var->alloc + (i>>2),
          var->ptr_register, 0);
      code |= (i&3) << 6;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadl (OrcCompiler *compiler, OrcVariable *var, int update)
{
  orc_uint32 code;
  int i;

  if (var->is_aligned && compiler->insn_shift == 2) {
    orc_neon_load_twovec_aligned (compiler, var, update);
  } else if (var->is_aligned && compiler->insn_shift == 1) {
    orc_neon_load_vec_aligned (compiler, var, update);
  } else if (compiler->insn_shift == 2) {
    orc_neon_load_twovec_unaligned (compiler, var, update);
  } else if (compiler->insn_shift == 1) {
    orc_neon_load_vec_unaligned (compiler, var, update);
  } else {
    if (compiler->insn_shift > 0) {
      /* ORC_ERROR("slow load"); */
    }
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.32 %s[%d], [%s]%s\n",
          orc_neon_reg_name (var->alloc + (i>>1)), i & 1,
          orc_arm_reg_name (var->ptr_register),
          update ? "!" : "");
      code = NEON_BINARY(0xf4a0080d, var->alloc + (i>>1),
          var->ptr_register, 0);
      code |= (i&1)<<7;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadq (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
      orc_neon_reg_name (dest),
      orc_arm_reg_name (src1),
      update ? "!" : "");
  code = 0xf42007cd;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= ((dest>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}


void
orc_neon_storeb (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  orc_uint32 code;
  int i;

  if (is_aligned && compiler->insn_shift == 5) {
    ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s, %s, %s }, [%s,:256]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1+1),
        orc_neon_reg_name (src1+2),
        orc_neon_reg_name (src1+3),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf400023d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (compiler->insn_shift == 5) {
    ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s, %s, %s }, [%s]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1+1),
        orc_neon_reg_name (src1+2),
        orc_neon_reg_name (src1+3),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf400020d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (is_aligned && compiler->insn_shift == 4) {
    ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s }, [%s,:128]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1+1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf4000a2d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (compiler->insn_shift == 4) {
    ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s }, [%s]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1+1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf4000a0d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (is_aligned && compiler->insn_shift == 3) {
    ORC_ASM_CODE(compiler,"  vst1.8 %s, [%s,:64]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf400071d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.8 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1 + (i>>3)), i&7,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480000d;
      code |= (dest&0xf) << 16;
      code |= ((src1 + (i>>3))&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= (i&7)<<5;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storew (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  orc_uint32 code;
  int i;

  if (is_aligned && compiler->insn_shift == 3) {
    ORC_ASM_CODE(compiler,"  vst1.16 { %s, %s }, [%s,:128]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1 + 1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf4000a6d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (is_aligned && compiler->insn_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.16 %s, [%s,:64]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf400075d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1 + (i>>2)), i&3,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480040d;
      code |= (dest&0xf) << 16;
      code |= ((src1 + (i>>2))&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= (i&3)<<6;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storel (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  orc_uint32 code;
  int i;

  if (is_aligned && compiler->insn_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.32 { %s, %s }, [%s,:128]%s\n",
        orc_neon_reg_name (src1),
        orc_neon_reg_name (src1 + 1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf4000aad;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (is_aligned && compiler->insn_shift == 1) {
    ORC_ASM_CODE(compiler,"  vst1.32 %s, [%s,:64]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf400079d;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->insn_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1 + (i>>1)), i&1,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480080d;
      code |= (dest&0xf) << 16;
      code |= ((src1 + (i>>1))&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= (i&1)<<7;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storeq (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
      orc_neon_reg_name (src1),
      orc_arm_reg_name (dest),
      update ? "!" : "");
  code = 0xf40007cd;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= ((src1>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}
#endif

static void
neon_rule_loadupdb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  unsigned int code = 0;
  int size = src->size << compiler->insn_shift;
  ORC_ASSERT(src->ptr_register);	/* can ptr_register be 0 ? */
  int ptr_reg;

  /* FIXME this should be fixed at a higher level */
  if (src->vartype != ORC_VAR_TYPE_SRC && src->vartype != ORC_VAR_TYPE_DEST) {
    ORC_COMPILER_ERROR(compiler, "loadX used with non src/dest");
    return;
  }

  if (compiler->is_64bit) {
    if (src->ptr_offset) {
      ptr_reg = compiler->gp_tmpreg;
      orc_arm64_emit_add_lsr(compiler, 64, ptr_reg, src->ptr_register, src->ptr_offset, 1);
    } else {
      ptr_reg = src->ptr_register;
    }

    int opcode, flag;

    if (size >= 16) {
      /** load multiple single-element structures to one, two, three, or four registers */
      char vt_str[64];

      memset(vt_str, '\x00', 64);

      if (size == 64) {
        snprintf(vt_str, 64, "%s, %s, %s, %s",
            orc_neon64_reg_name_vector (compiler->tmpreg, 1, 1),
            orc_neon64_reg_name_vector (compiler->tmpreg + 1, 1, 1),
            orc_neon64_reg_name_vector (compiler->tmpreg + 2, 1, 1),
            orc_neon64_reg_name_vector (compiler->tmpreg + 3, 1, 1));
        opcode = 0x2;
      } else if (size == 32) {
        snprintf(vt_str, 64, "%s, %s",
            orc_neon64_reg_name_vector (compiler->tmpreg, 1, 1),
            orc_neon64_reg_name_vector (compiler->tmpreg + 1, 1, 1));
        opcode = 0xa;
      } else if (size == 16) {
        snprintf(vt_str, 64, "%s",
            orc_neon64_reg_name_vector (compiler->tmpreg, 1, 1));
        opcode = 0x7;
      } else {
        ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
            src->size << compiler->insn_shift);
        return;
      }
      flag = 0; /* Bytes */

      ORC_ASM_CODE(compiler,"  ld1 { %s }, [%s]\n",
          vt_str, orc_arm64_reg_name (ptr_reg, 64));
      code = 0x0c400000;
      code |= 0 << 30; /* Q-bit */
      code |= (flag&0x3) << 10;
      code |= (opcode&0xf) << 12;
    } else {
      /** load one single-element structure to one lane of one register */
      flag = 0;
      if (size == 8) {
        opcode = 4;
        flag = 1; /* size==01 */
      } else if (size == 4) {
        opcode = 4;
      } else if (size == 2) {
        opcode = 2;
      } else if (size == 1) {
        opcode = 0;
      } else {
        ORC_COMPILER_ERROR(compiler,"bad unaligned load size %d",
            src->size << compiler->insn_shift);
        return;
      }
      ORC_ASM_CODE(compiler,"  ld1 { %s }[0], [%s]\n",
          orc_neon64_reg_name_vector_single (compiler->tmpreg, size),
          orc_arm64_reg_name (ptr_reg, 64));
      code = 0x0d400000;
      code |= (opcode&0x7) << 13;
      code |= (flag&0x3) << 10;
    }

    code |= (ptr_reg&0x1f) << 5;
    code |= (compiler->tmpreg&0x1f);

    orc_arm_emit (compiler, code);

    OrcVariable tmpreg = { .alloc = compiler->tmpreg, .size = compiler->vars[insn->src_args[0]].size };

    switch (src->size) {
      case 1:
        orc_neon64_emit_binary (compiler, "zip1", 0x0e003800,
            compiler->vars[insn->dest_args[0]],
            tmpreg,
            tmpreg, compiler->insn_shift - 1);
        break;
      case 2:
        orc_neon64_emit_binary (compiler, "zip1", 0x0e403800,
            compiler->vars[insn->dest_args[0]],
            tmpreg,
            tmpreg, compiler->insn_shift - 1);
        break;
      case 4:
        orc_neon64_emit_binary (compiler, "zip1", 0x0e803800,
            compiler->vars[insn->dest_args[0]],
            tmpreg,
            tmpreg, compiler->insn_shift - 1);
        break;
    }
  } else {
    if (src->ptr_offset) {
      ptr_reg = compiler->gp_tmpreg;
      orc_arm_emit_add_rsi(compiler, ORC_ARM_COND_AL, 0,
                         ptr_reg, src->ptr_register,
                         src->ptr_offset, ORC_ARM_LSR, 1);
    } else {
      ptr_reg = src->ptr_register;
    }
    if (size >= 8) {
      if (src->is_aligned) {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s, %s, %s }, [%s,:256]\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_neon_reg_name (dest->alloc + 2),
              orc_neon_reg_name (dest->alloc + 3),
              orc_arm_reg_name (ptr_reg));
          code = 0xf42002dd;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s }, [%s,:128]\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_arm_reg_name (ptr_reg));
          code = 0xf4200aed;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]\n",
              orc_neon_reg_name (dest->alloc),
              orc_arm_reg_name (ptr_reg));
          code = 0xf42007cd;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
        }
      } else {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s, %s, %s }, [%s]\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_neon_reg_name (dest->alloc + 2),
              orc_neon_reg_name (dest->alloc + 3),
              orc_arm_reg_name (ptr_reg));
          code = 0xf420020d;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s }, [%s]\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_arm_reg_name (ptr_reg));
          code = 0xf4200a0d;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vld1.8 %s, [%s]\n",
              orc_neon_reg_name (dest->alloc),
              orc_arm_reg_name (ptr_reg));
          code = 0xf420070d;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad unaligned load size %d",
              src->size << compiler->insn_shift);
        }
      }
    } else {
      int shift;
      if (size == 4) {
        shift = 2;
      } else if (size == 2) {
        shift = 1;
      } else {
        shift = 0;
      }
      ORC_ASM_CODE(compiler,"  vld1.%d %s[0], [%s]\n",
          8<<shift,
          orc_neon_reg_name (dest->alloc),
          orc_arm_reg_name (ptr_reg));
      code = 0xf4a0000d;
      code |= shift<<10;
      code |= (0&7)<<5;
    }
    code |= (ptr_reg&0xf) << 16;
    code |= (dest->alloc&0xf) << 12;
    code |= ((dest->alloc>>4)&0x1) << 22;
    code |= 1 << 1;
    orc_arm_emit (compiler, code);

    switch (src->size) {
      case 1:
        orc_neon_emit_binary (compiler, "vorr", 0xf2200110,
          compiler->vars[insn->dest_args[0]].alloc + 1,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc);
        orc_neon_emit_unary (compiler, "vzip.8", 0xf3b20180,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc + 1);

        if (compiler->loop_shift == 1) {
          /* When the loop_shift is 1, it is possible that one iteration of shift 0
          has already been performed if the destination array is 8-byte aligned
          (but not 16-byte aligned).
          In this case, the output offset has been incremented by 1, and we need to
          shift the outputs of loadupdb.*/

          // set temp reg to 0
          orc_arm_emit_eor_r(compiler, ORC_ARM_COND_AL, 0,
            compiler->gp_tmpreg, compiler->gp_tmpreg, compiler->gp_tmpreg);
          // test if input offset is odd
          orc_arm_emit_tst_i(compiler, ORC_ARM_COND_AL, src->ptr_offset, 0x1);
          // if yes, tmpreg = 0xff
          orc_arm_emit_mov_i(compiler, ORC_ARM_COND_NE, 0, compiler->gp_tmpreg, 0xff);

          // fill a simd reg with value of tmpreg (0xff or 0x0)
          ORC_ASM_CODE(compiler,"  %s %s, %s\n", "vdup.8",
            orc_neon_reg_name (dest->alloc+3), orc_arm_reg_name (compiler->gp_tmpreg));
          code = 0xeec00b10;
          code |= ((compiler->vars[insn->dest_args[0]].alloc+3)&0xf)<<16; // Vd
          code |= (compiler->gp_tmpreg&0xf) << 12; // Rt
          code |= (((compiler->vars[insn->dest_args[0]].alloc+3)>>4)&0x1) << 7; // D
          orc_arm_emit (compiler, code);

          // vext.8 with #imm=1 to create shifted output
          orc_neon_emit_binary (compiler, "vext.8", 0xf2b00100,
            compiler->vars[insn->dest_args[0]].alloc+1,
            compiler->vars[insn->dest_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc+1);

          // select shifted output or not
          orc_neon_emit_binary(compiler, "vbit.8", 0xf3200110,
            compiler->vars[insn->dest_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc+1,
            compiler->vars[insn->dest_args[0]].alloc+3);
        }
        break;
      case 2:
        orc_neon_emit_binary (compiler, "vorr", 0xf2200110,
          compiler->vars[insn->dest_args[0]].alloc + 1,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc);
        orc_neon_emit_unary (compiler, "vzip.16", 0xf3b60180,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc + 1);
        break;
      case 4:
        orc_neon_emit_binary (compiler, "vorr", 0xf2200110,
          compiler->vars[insn->dest_args[0]].alloc + 1,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc);
        orc_neon_emit_unary_quad (compiler, "vzip.32", 0xf3ba0180,
          compiler->vars[insn->dest_args[0]].alloc,
          compiler->vars[insn->dest_args[0]].alloc + 1);
        break;
    }
  }

  src->update_type = 1;
}

static void
neon_rule_loadpX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    if (size == 1) {
      orc_neon_emit_loadib (compiler, dest, src->value.i);
    } else if (size == 2) {
      orc_neon_emit_loadiw (compiler, dest, src->value.i);
    } else if (size == 4) {
      orc_neon_emit_loadil (compiler, dest, src->value.i);
    } else if (size == 8) {
      if (src->size == 8 && !compiler->is_64bit) {
        ORC_COMPILER_ERROR(compiler,"64-bit constants not implemented");
      }
      orc_neon_emit_loadiq (compiler, dest, src->value.i);
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  } else {
    if (size == 1) {
      orc_neon_emit_loadpb (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 2) {
      orc_neon_emit_loadpw (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 4) {
      orc_neon_emit_loadpl (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 8) {
      orc_neon_emit_loadpq (compiler, dest->alloc, insn->src_args[0]);
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  }
}

static void
neon_rule_loadX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int update = FALSE;
  unsigned int code = 0;
  int size = src->size << compiler->insn_shift;
  int type = ORC_PTR_TO_INT(user);
  int ptr_register;
  int is_aligned = src->is_aligned;

  /* FIXME this should be fixed at a higher level */
  if (src->vartype != ORC_VAR_TYPE_SRC && src->vartype != ORC_VAR_TYPE_DEST) {
    ORC_COMPILER_ERROR(compiler, "loadX used with non src/dest");
    return;
  }

  if (src->vartype == ORC_VAR_TYPE_DEST) update = FALSE;

  if (type == 1) {
    OrcVariable *src2 = compiler->vars + insn->src_args[1];

    if (src2->vartype != ORC_VAR_TYPE_CONST) {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
      return;
    }

    ptr_register = compiler->gp_tmpreg;
    if (compiler->is_64bit) {
      if (src2->value.i < 0) {
        orc_arm64_emit_sub_imm (compiler, 64, ptr_register,
            src->ptr_register,
            src2->value.i * src->size * -1);
      }
      else
      {
        orc_arm64_emit_add_imm (compiler, 64, ptr_register,
            src->ptr_register,
            src2->value.i * src->size);
      }
    } else {
      if (src2->value.i < 0) {
        orc_arm_emit_sub_imm (compiler, ptr_register,
            src->ptr_register,
            src2->value.i * src->size * -1, TRUE);
      }
      else
      {
        orc_arm_emit_add_imm (compiler, ptr_register,
            src->ptr_register,
            src2->value.i * src->size);
      }
    }

    update = FALSE;
    is_aligned = FALSE;
  } else {
    ptr_register = src->ptr_register;
  }

  if (compiler->is_64bit) {
    int opcode, flag;

    if (size >= 16) {
      /** load multiple single-element structures to one, two, three, or four registers */
      char vt_str[64];

      memset(vt_str, '\x00', 64);

      if (is_aligned) {
        if (size == 64) {
          snprintf(vt_str, 64, "%s, %s, %s, %s",
              orc_neon64_reg_name_vector (dest->alloc, 8, 1),
              orc_neon64_reg_name_vector (dest->alloc + 1, 8, 1),
              orc_neon64_reg_name_vector (dest->alloc + 2, 8, 1),
              orc_neon64_reg_name_vector (dest->alloc + 3, 8, 1));
          opcode = 2;
        } else if (size == 32) {
          snprintf(vt_str, 64, "%s, %s",
              orc_neon64_reg_name_vector (dest->alloc, 8, 1),
              orc_neon64_reg_name_vector (dest->alloc + 1, 8, 1));
          opcode = 10;
        } else if (size == 16) {
          snprintf(vt_str, 64, "%s",
              orc_neon64_reg_name_vector (dest->alloc, 8, 1));
          opcode = 7;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
          return;
        }
        flag = 7;
      } else {
        if (size == 64) {
          snprintf(vt_str, 64, "%s, %s, %s, %s",
              orc_neon64_reg_name_vector (dest->alloc, 1, 1),
              orc_neon64_reg_name_vector (dest->alloc + 1, 1, 1),
              orc_neon64_reg_name_vector (dest->alloc + 2, 1, 1),
              orc_neon64_reg_name_vector (dest->alloc + 3, 1, 1));
          opcode = 2;
        } else if (size == 32) {
          snprintf(vt_str, 64, "%s, %s",
              orc_neon64_reg_name_vector (dest->alloc, 1, 1),
              orc_neon64_reg_name_vector (dest->alloc + 1, 1, 1));
          opcode = 10;
        } else if (size == 16) {
          snprintf(vt_str, 64, "%s",
              orc_neon64_reg_name_vector (dest->alloc, 1, 1));
          opcode = 7;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
          return;
        }
        flag = 1;
      }
      ORC_ASM_CODE(compiler,"  ld1 { %s }, [%s]\n",
          vt_str, orc_arm64_reg_name (ptr_register, 64));
      code = 0x0c400000;
      code |= (flag&0x1) << 30;
      code |= (flag&0x3) << 10;
      code |= (opcode&0xf) << 12;
    } else {
      /** load one single-element structure to one lane of one register */
      flag = 0;
      if (size == 8) {
        opcode = 4;
        flag = 1;
      } else if (size == 4) {
        opcode = 4;
      } else if (size == 2) {
        opcode = 2;
      } else if (size == 1) {
        opcode = 0;
      } else {
        ORC_COMPILER_ERROR(compiler,"bad unaligned load size %d",
            src->size << compiler->insn_shift);
        return;
      }
      ORC_ASM_CODE(compiler,"  ld1 { %s }[0], [%s]\n",
          orc_neon64_reg_name_vector_single (dest->alloc, size),
          orc_arm64_reg_name (ptr_register, 64));
      code = 0x0d400000;
      code |= (opcode&0x7) << 13;
      code |= (flag&0x3) << 10;
    }

    code |= (ptr_register&0x1f) << 5;
    code |= (dest->alloc&0x1f);

    orc_arm_emit (compiler, code);
  } else {
    if (size >= 8) {
      if (is_aligned) {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s, %s, %s }, [%s,:256]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_neon_reg_name (dest->alloc + 2),
              orc_neon_reg_name (dest->alloc + 3),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf42002dd;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vld1.64 { %s, %s }, [%s,:128]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf4200aed;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf42007cd;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
        }
      } else {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s, %s, %s }, [%s]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_neon_reg_name (dest->alloc + 2),
              orc_neon_reg_name (dest->alloc + 3),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf420020d;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vld1.8 { %s, %s }, [%s]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_neon_reg_name (dest->alloc + 1),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf4200a0d;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vld1.8 %s, [%s]%s\n",
              orc_neon_reg_name (dest->alloc),
              orc_arm_reg_name (ptr_register),
              update ? "!" : "");
          code = 0xf420070d;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad unaligned load size %d",
              src->size << compiler->insn_shift);
        }
      }
    } else {
      int shift;
      if (size == 4) {
        shift = 2;
      } else if (size == 2) {
        shift = 1;
      } else {
        shift = 0;
      }
      ORC_ASM_CODE(compiler,"  vld1.%d %s[0], [%s]%s\n",
          8<<shift,
          orc_neon_reg_name (dest->alloc),
          orc_arm_reg_name (ptr_register),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= shift<<10;
      code |= (0&7)<<5;
    }
    code |= (ptr_register&0xf) << 16;
    code |= (dest->alloc&0xf) << 12;
    code |= ((dest->alloc>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  }
}

static void
neon_rule_storeX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int update = FALSE;
  unsigned int code = 0;
  int size = dest->size << compiler->insn_shift;

  if (compiler->is_64bit) {
    int opcode, flag;

    if (size >= 16) {
      /** store multiple single-element structures to one, two, three, or four registers */
      char vt_str[64];

      memset(vt_str, '\x00', 64);

      if (dest->is_aligned) {
        if (size == 64) {
          snprintf(vt_str, 64, "%s, %s, %s, %s",
              orc_neon64_reg_name_vector (src->alloc, 8, 1),
              orc_neon64_reg_name_vector (src->alloc + 1, 8, 1),
              orc_neon64_reg_name_vector (src->alloc + 2, 8, 1),
              orc_neon64_reg_name_vector (src->alloc + 3, 8, 1));
          opcode = 0x2;
        } else if (size == 32) {
          snprintf(vt_str, 64, "%s, %s",
              orc_neon64_reg_name_vector (src->alloc, 8, 1),
              orc_neon64_reg_name_vector (src->alloc + 1, 8, 1));
          opcode = 0xa;
        } else if (size == 16) {
          snprintf(vt_str, 64, "%s",
              orc_neon64_reg_name_vector (src->alloc, 8, 1));
          opcode = 0x7;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
          return;
        }
      } else {
        if (size == 64) {
          snprintf(vt_str, 64, "%s, %s, %s, %s",
              orc_neon64_reg_name_vector (src->alloc, 1, 1),
              orc_neon64_reg_name_vector (src->alloc + 1, 1, 1),
              orc_neon64_reg_name_vector (src->alloc + 2, 1, 1),
              orc_neon64_reg_name_vector (src->alloc + 3, 1, 1));
          opcode = 0x2;
        } else if (size == 32) {
          snprintf(vt_str, 64, "%s, %s",
              orc_neon64_reg_name_vector (src->alloc, 1, 1),
              orc_neon64_reg_name_vector (src->alloc + 1, 1, 1));
          opcode = 0xa;
        } else if (size == 16) {
          snprintf(vt_str, 64, "%s",
              orc_neon64_reg_name_vector (src->alloc, 1, 1));
          opcode = 0x7;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned load size %d",
              src->size << compiler->insn_shift);
          return;
        }
      }
      ORC_ASM_CODE(compiler,"  st1 { %s }, [%s]\n",
          vt_str, orc_arm64_reg_name (dest->ptr_register, 64));
      code = 0x0c000000;
      code |= 1 << 30;
      code |= (opcode&0xf) << 12;
    } else {
      /** store one single-element structure to one lane of one register */
      flag = 0;
      if (size == 8) {
        opcode = 4;
        flag = 1;
      } else if (size == 4) {
        opcode = 4;
      } else if (size == 2) {
        opcode = 2;
      } else if (size == 1) {
        opcode = 0;
      } else {
        ORC_COMPILER_ERROR(compiler,"bad unaligned load size %d",
            src->size << compiler->insn_shift);
        return;
      }
      ORC_ASM_CODE(compiler,"  st1 { %s }[0], [%s]\n",
          orc_neon64_reg_name_vector_single (src->alloc, size),
          orc_arm64_reg_name (dest->ptr_register, 64));
      code = 0x0d000000;
      code |= (opcode&0x7) << 13;
      code |= (flag&0x3) << 10;
    }

    code |= (dest->ptr_register&0x1f) << 5;
    code |= (src->alloc&0x1f);

    orc_arm_emit (compiler, code);
  } else {
    if (size >= 8) {
      if (dest->is_aligned) {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vst1.64 { %s, %s, %s, %s }, [%s,:256]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_neon_reg_name (src->alloc + 1),
              orc_neon_reg_name (src->alloc + 2),
              orc_neon_reg_name (src->alloc + 3),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf40002dd;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vst1.64 { %s, %s }, [%s,:128]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_neon_reg_name (src->alloc + 1),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf4000aed;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf40007cd;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned store size %d", size);
        }
      } else {
        if (size == 32) {
          ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s, %s, %s }, [%s]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_neon_reg_name (src->alloc + 1),
              orc_neon_reg_name (src->alloc + 2),
              orc_neon_reg_name (src->alloc + 3),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf400020d;
        } else if (size == 16) {
          ORC_ASM_CODE(compiler,"  vst1.8 { %s, %s }, [%s]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_neon_reg_name (src->alloc + 1),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf4000a0d;
        } else if (size == 8) {
          ORC_ASM_CODE(compiler,"  vst1.8 %s, [%s]%s\n",
              orc_neon_reg_name (src->alloc),
              orc_arm_reg_name (dest->ptr_register),
              update ? "!" : "");
          code = 0xf400070d;
        } else {
          ORC_COMPILER_ERROR(compiler,"bad aligned store size %d", size);
        }
      }
    } else {
      int shift;
      if (size == 4) {
        shift = 2;
      } else if (size == 2) {
        shift = 1;
      } else {
        shift = 0;
      }
      ORC_ASM_CODE(compiler,"  vst1.%d %s[0], [%s]%s\n",
          8<<shift,
          orc_neon_reg_name (src->alloc),
          orc_arm_reg_name (dest->ptr_register),
          update ? "!" : "");
      code = 0xf480000d;
      code |= shift<<10;
      code |= (0&7)<<5;
    }
    code |= (dest->ptr_register&0xf) << 16;
    code |= (src->alloc&0xf) << 12;
    code |= ((src->alloc>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  }
}

#if 0
static int
orc_neon_get_const_shift (unsigned int value)
{
  int shift = 0;

  while((value & 0xff) != value) {
    shift++;
    value >>= 1;
  }
  return shift;
}
#endif

void
orc_neon_emit_loadib (OrcCompiler *compiler, OrcVariable *dest, int value)
{
  int reg = dest->alloc;
  orc_uint32 code;

  if (compiler->is_64bit) {
    if (value == 0) {
      orc_neon64_emit_binary (compiler, "eor", 0x2e201c00,
          *dest, *dest, *dest, compiler->insn_shift - 1);
      return;
    }

    value &= 0xff;
    ORC_ASM_CODE(compiler,"  movi %s, #%d\n",
        orc_neon64_reg_name_vector (reg, 16, 0), value);
    code = 0x0f00e400; /* 8-bit (op==0 && cmode==1110) */
    code |= (reg&0x1f) << 0;
    code |= (value&0x1f) << 5;
    code |= (value&0xe0) << 11;
    code |= 1 << 30;
    orc_arm_emit (compiler, code);
  } else {
    if (value == 0) {
      orc_neon_emit_binary_quad (compiler, "veor", 0xf3000110, reg, reg, reg);
      return;
    }

    value &= 0xff;
    ORC_ASM_CODE(compiler,"  vmov.i8 %s, #%d\n",
        orc_neon_reg_name_quad (reg), value);
    code = 0xf2800e10;
    code |= (reg&0xf) << 12;
    code |= ((reg>>4)&0x1) << 22;
    code |= (value&0xf) << 0;
    code |= (value&0x70) << 12;
    code |= (value&0x80) << 17;
    code |= 0x40;
    orc_arm_emit (compiler, code);
  }
}

void
orc_neon_emit_loadiw (OrcCompiler *compiler, OrcVariable *dest, int value)
{
  int reg = dest->alloc;
  orc_uint32 code;

  if (compiler->is_64bit) {
    if (value == 0) {
      orc_neon64_emit_binary (compiler, "eor", 0x2e201c00,
          *dest, *dest, *dest, compiler->insn_shift - 1);
      return;
    }

    ORC_ASM_CODE(compiler, "  movi %s, #0x%02x\n",
                 orc_neon64_reg_name_vector(reg, 2, 1), value & 0xff);
    code = 0x0f008400; /* 16-bit (op==0 && cmode==10x0), x=0 is LSL #0 */
    code |= (reg&0x1f) << 0;
    code |= (value&0x1f) << 5;
    code |= (value&0xe0) << 11;
    code |= 1 << 30;
    orc_arm_emit (compiler, code);

    value >>= 8;
    if (value) {
      ORC_ASM_CODE(compiler, "  orr %s, #0x%02x, lsl #8\n",
                   orc_neon64_reg_name_vector(reg, 2, 1), value & 0xff);
      code = 0x0f00b400; /* 16-bit (cmode==10x1), x=1 is LSL #8 */
      code |= (reg&0x1f) << 0;
      code |= (value&0x1f) << 5;
      code |= (value&0xe0) << 11;
      code |= 1 << 30;
      orc_arm_emit (compiler, code);
    }
  } else {
    if (value == 0) {
      orc_neon_emit_binary_quad (compiler, "veor", 0xf3000110, reg, reg, reg);
      return;
    }

    ORC_ASM_CODE(compiler,"  vmov.i16 %s, #0x%04x\n",
        orc_neon_reg_name_quad (reg), value & 0xff);
    code = 0xf2800810;
    code |= (reg&0xf) << 12;
    code |= ((reg>>4)&0x1) << 22;
    code |= (value&0xf) << 0;
    code |= (value&0x70) << 12;
    code |= (value&0x80) << 17;
    code |= 0x40;
    orc_arm_emit (compiler, code);

    value >>= 8;
    if (value) {
      ORC_ASM_CODE(compiler,"  vorr.i16 %s, #0x%04x\n",
          orc_neon_reg_name_quad (reg), (value & 0xff) << 8);
      code = 0xf2800b10;

      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (value&0xf) << 0;
      code |= (value&0x70) << 12;
      code |= (value&0x80) << 17;
      code |= 0x40;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_emit_loadil (OrcCompiler *compiler, OrcVariable *dest, int value)
{
  int reg = dest->alloc;
  orc_uint32 code;

  if (compiler->is_64bit) {
    if (value == 0) {
      orc_neon64_emit_binary (compiler, "eor", 0x2e201c00,
          *dest, *dest, *dest, compiler->insn_shift - 1);
      return;
    }

    ORC_ASM_CODE(compiler,"  movi %s, #0x%02x\n",
        orc_neon64_reg_name_vector (reg, 16, 0), value & 0xff);
    code = 0x0f000400; /* 32-bit (op==0 && cmode==0xx0), xx=0 is LSL #0 */
    code |= (reg&0x1f) << 0;
    code |= (value&0x1f) << 5;
    code |= (value&0xe0) << 11;
    code |= 1 << 30;
    orc_arm_emit (compiler, code);

    value >>= 8;
    if (value) {
      ORC_ASM_CODE(compiler,"  orr %s, #0x%02x, lsl #8\n",
          orc_neon64_reg_name_vector (reg, 16, 0), value & 0xff);
      code = 0x0f003400; /* 32-bit (cmode==0xx1), xx=01 is LSL #8 */
      code |= (reg&0x1f) << 0;
      code |= (value&0x1f) << 5;
      code |= (value&0xe0) << 11;
      code |= 1 << 30;
      orc_arm_emit (compiler, code);
    }
    value >>= 8;
    if (value) {
      ORC_ASM_CODE(compiler,"  orr %s, #0x%02x, lsl #16\n",
          orc_neon64_reg_name_vector (reg, 16, 0), value & 0xff);
      code = 0x0f005400; /* 32-bit (cmode==0xx1), xx=10 is LSL #16 */
      code |= (reg&0x1f) << 0;
      code |= (value&0x1f) << 5;
      code |= (value&0xe0) << 11;
      code |= 1 << 30;
      orc_arm_emit (compiler, code);
    }
    value >>= 8;
    if (value) {
      ORC_ASM_CODE(compiler,"  orr %s, #0x%02x, lsl #8\n",
          orc_neon64_reg_name_vector (reg, 16, 0), value & 0xff);
      code = 0x0f007400; /* 32-bit (cmode==0xx1), xx=11 is LSL #24 */
      code |= (reg&0x1f) << 0;
      code |= (value&0x1f) << 5;
      code |= (value&0xe0) << 11;
      code |= 1 << 30;
      orc_arm_emit (compiler, code);
    }
  } else {
    if (value == 0) {
      orc_neon_emit_binary_quad (compiler, "veor", 0xf3000110, reg, reg, reg);
      return;
    }

    ORC_ASM_CODE(compiler,"  vmov.i32 %s, #0x%08x\n",
        orc_neon_reg_name_quad (reg), value & 0xff);
    code = 0xf2800010;

    code |= (reg&0xf) << 12;
    code |= ((reg>>4)&0x1) << 22;
    code |= (value&0xf) << 0;
    code |= (value&0x70) << 12;
    code |= (value&0x80) << 17;
    code |= 0x40;
    orc_arm_emit (compiler, code);

    value >>= 8;
    if (value & 0xff) {
      ORC_ASM_CODE(compiler,"  vorr.i32 %s, #0x%08x\n",
          orc_neon_reg_name_quad (reg), (value & 0xff) << 8);
      code = 0xf2800310;

      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (value&0xf) << 0;
      code |= (value&0x70) << 12;
      code |= (value&0x80) << 17;
      code |= 0x40;
      orc_arm_emit (compiler, code);
    }
    value >>= 8;
    if (value & 0xff) {
      ORC_ASM_CODE(compiler,"  vorr.i32 %s, #0x%08x\n",
          orc_neon_reg_name_quad (reg), (value & 0xff) << 16);
      code = 0xf2800510;

      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (value&0xf) << 0;
      code |= (value&0x70) << 12;
      code |= (value&0x80) << 17;
      code |= 0x40;
      orc_arm_emit (compiler, code);
    }
    value >>= 8;
    if (value & 0xff) {
      ORC_ASM_CODE(compiler,"  vorr.i32 %s, #0x%08x\n",
          orc_neon_reg_name_quad (reg), (value & 0xff) << 24);
      code = 0xf2800710;

      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (value&0xf) << 0;
      code |= (value&0x70) << 12;
      code |= (value&0x80) << 17;
      code |= 0x40;
      orc_arm_emit (compiler, code);
    }
  }
}

static void
orc_neon_emit_loadiq (OrcCompiler *compiler, OrcVariable *dest, long long value)
{
  int reg = dest->alloc;
  /* orc_uint32 code; */
  /* int shift; */
  /* int neg = FALSE; */

  if (compiler->is_64bit) {
    if (value == 0) {
      orc_neon64_emit_binary (compiler, "eor", 0x2e201c00,
          *dest, *dest, *dest, compiler->insn_shift - 1);
      return;
    }

    /*
     * NOTE: This could be optimized further. The code below is 5 instructions
     * long. By locating 8-bit "islands" of bits in the value itself (8-bit is
     * the limit of IMM field in MOVI/ORR opcode), it may be possible for some
     * sparse constants (with fewer than 5 such islands) to generate far more
     * optimal (shorter than 5 instructions) load using MOVI and ORR opcodes
     * instead. However, such optimization might also be premature, since the
     * constant is usually loaded only once when the program starts, hence it
     * is not implemented below for now.
     */
    ORC_ASM_CODE(compiler,"  ldr %s, L30\n",
        orc_neon64_reg_name_vector (reg, 8, 0));
    orc_arm_emit (compiler, 0x5c000040 | (reg & 0x1f));

    orc_arm_emit_branch (compiler, ORC_ARM_COND_AL, 30);
    orc_arm_emit (compiler, value & 0xffffffffULL);
    orc_arm_emit (compiler, value >> 32ULL);
    orc_arm_emit_label (compiler, 30);

    orc_neon64_emit_binary (compiler, "trn1", 0x0ec02800,
        *dest, *dest, *dest, compiler->insn_shift - 1);

      return;
  } else {
    if (value == 0) {
      orc_neon_emit_binary_quad (compiler, "veor", 0xf3000110, reg, reg, reg);
      return;
    }

    if (value < 0) {
      /* neg = TRUE; */
      value = ~value;
    }
#if 0
    shift = orc_neon_get_const_shift (value);
    if ((value & (0xff<<shift)) == value) {
      value >>= shift;
      if (neg) {
        ORC_ASM_CODE(compiler,"  vmvn.i64 %s, #%d\n",
            orc_neon_reg_name_quad (reg), value);
        code = 0xf2800030;
      } else {
        ORC_ASM_CODE(compiler,"  vmov.i64 %s, #%d\n",
            orc_neon_reg_name_quad (reg), value);
        code = 0xf2800010;
      }
      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (value&0xf) << 0;
      code |= (value&0x70) << 12;
      code |= (value&0x80) << 17;
      code |= 0x40;
      orc_arm_emit (compiler, code);

      if (shift > 0) {
        ORC_ASM_CODE(compiler,"  vshl.i64 %s, %s, #%d\n",
            orc_neon_reg_name_quad (reg), orc_neon_reg_name_quad (reg), shift);
        code = 0xf2a00510;
        code |= (reg&0xf) << 12;
        code |= ((reg>>4)&0x1) << 22;
        code |= (reg&0xf) << 0;
        code |= ((reg>>4)&0x1) << 5;
        code |= (shift&0xf) << 16;
        code |= 0x40;
        orc_arm_emit (compiler, code);
      }

      return;
    }
#endif
  }
  ORC_COMPILER_ERROR(compiler, "unimplemented load of constant %d", value);
}

void
orc_neon_emit_loadpb (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;

  if (compiler->is_64bit) {
      orc_arm64_emit_add_imm (compiler, 64, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  ld1r {%s, %s}, [%s]\n",
          orc_neon64_reg_name_vector (dest, 1, 0),
          orc_neon64_reg_name_vector (dest+1, 1, 0),
          orc_arm64_reg_name (compiler->gp_tmpreg, 64));
      code = 0x0d40c000;
      code |= 1 << 30; /* Q-bit */
      code |= (compiler->gp_tmpreg&0x1f) << 5;
      code |= (dest&0x1f) << 0;
  } else {
      orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  vld1.8 {%s[],%s[]}, [%s]\n",
          orc_neon_reg_name (dest), orc_neon_reg_name (dest+1),
          orc_arm_reg_name (compiler->gp_tmpreg));
      code = 0xf4a00c2f;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
  }
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_loadpw (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;

  if (compiler->is_64bit) {
      orc_arm64_emit_add_imm (compiler, 64, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  ld1r {%s, %s}, [%s]\n",
          orc_neon64_reg_name_vector (dest, 2, 0),
          orc_neon64_reg_name_vector (dest+1, 2, 0),
          orc_arm64_reg_name (compiler->gp_tmpreg, 64));
      code = 0x0d40c400;
      code |= 1 << 30; /* Q-bit */
      code |= (compiler->gp_tmpreg&0x1f) << 5;
      code |= (dest&0x1f) << 0;
  } else {
      orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  vld1.16 {%s[],%s[]}, [%s]\n",
          orc_neon_reg_name (dest), orc_neon_reg_name (dest+1),
          orc_arm_reg_name (compiler->gp_tmpreg));
      code = 0xf4a00c6f;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
  }
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_loadpl (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;

  if (compiler->is_64bit) {
      orc_arm64_emit_add_imm (compiler, 64, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  ld1r {%s, %s}, [%s]\n",
          orc_neon64_reg_name_vector (dest, 4, 0),
          orc_neon64_reg_name_vector (dest+1, 4, 0),
          orc_arm64_reg_name (compiler->gp_tmpreg, 64));
      code = 0x0d40c800;
      code |= 1 << 30; /* Q-bit */
      code |= (compiler->gp_tmpreg&0x1f) << 5;
      code |= (dest&0x1f) << 0;
  } else {
      orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  vld1.32 {%s[],%s[]}, [%s]\n",
          orc_neon_reg_name (dest), orc_neon_reg_name (dest+1),
          orc_arm_reg_name (compiler->gp_tmpreg));
      code = 0xf4a00caf;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
  }
  orc_arm_emit (compiler, code);
}

static void
orc_neon_emit_loadpq (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;
  int update = FALSE;

  if (compiler->is_64bit) {
      orc_arm64_emit_add_imm (compiler, 64, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      /*
       * This here is a bit more complex, as the top 32 bits of the Tx are
       * stored at an offset sizeof(params) * (ORC_N_PARAMS) from
       * bottom 32 bits Px, so we do interleaved load using LD3, where the
       * (v0.4s)[0] is Px and (v2.4s)[2] is Tx, because they are exactly
       * 256 bits apart = 32 bytes = sizeof(params)*(ORC_N_PARAMS).
       *
       * The way all the LD1..LD4R opcodes work may be inobvious from the
       * ARM A64 ISA documentation. See the following article:
       * https://community.arm.com/developer/ip-products/processors/b/processors-ip-blog/posts/coding-for-neon---part-1-load-and-stores
       *
       * Specifically, LD3.32 with Q-bit set (128-bit operation) works this
       * way. Assume array of 32bit types with 12 entries:
       *
       *  uint32_t x0[12];
       *  ld3 {v0.4s, v1.4d, v2.4s}, [x0]          .--- LSB (address 0)
       *  results in:                              v
       *  v0.4s[127:0] = { x0[9],  x0[6], x0[3], x0[0] };
       *  v1.4s[127:0] = { x0[10], x0[7], x0[4], x0[1] };
       *  v2.4s[127:0] = { x0[11], x0[8], x0[5], x0[2] };
       *
       * To obtain the correct final result of loadpq, two MOV instructions
       * are necessary to generate v0.4s = { x0[8], x0[0], x0[8], x0[0] };
       * Note that there might be a better way to perform the mixing with
       * some TRN/ZIP/UZP instruction.
       */
      ORC_ASSERT((ORC_N_PARAMS) == 8);
      ORC_ASM_CODE(compiler,"  ld3 {%s - %s}, [%s]\n",
          orc_neon64_reg_name_vector (dest, 8, 0),
          orc_neon64_reg_name_vector (dest+2, 8, 0),
          orc_arm64_reg_name (compiler->gp_tmpreg, 64));
      code = 0x0c404800;
      code |= 1 << 30; /* Q-bit */
      code |= (compiler->gp_tmpreg&0x1f) << 5;
      code |= (dest&0x1f) << 0;
      orc_arm_emit (compiler, code);

      ORC_ASM_CODE(compiler,"  mov %s[1], %s[2]\n",
          orc_neon64_reg_name_vector (dest, 4, 0),
          orc_neon64_reg_name_vector (dest+2, 4, 0));
      code = 0x6e0c4400;
      code |= ((dest+2)&0x1f) << 5;
      code |= (dest&0x1f) << 0;
      orc_arm_emit (compiler, code);

      ORC_ASM_CODE(compiler,"  mov %s[1], %s[0]\n",
          orc_neon64_reg_name_vector (dest, 8, 0),
          orc_neon64_reg_name_vector (dest, 8, 0));
      code = 0x6e180400;
      code |= (dest&0x1f) << 5;
      code |= (dest&0x1f) << 0;
      orc_arm_emit (compiler, code);
  } else {
      orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

      ORC_ASM_CODE(compiler,"  vld1.32 %s[0], [%s]%s\n",
          orc_neon_reg_name (dest),
          orc_arm_reg_name (compiler->gp_tmpreg),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= 2<<10;
      code |= (0)<<7;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);

      ORC_ASM_CODE(compiler,"  vld1.32 %s[0], [%s]%s\n",
          orc_neon_reg_name (dest+1),
          orc_arm_reg_name (compiler->gp_tmpreg),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= 2<<10;
      code |= (0)<<7;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= ((dest+1)&0xf) << 12;
      code |= (((dest+1)>>4)&0x1) << 22;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);

      orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
          compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor,
            params[param + (ORC_N_PARAMS)]));

      ORC_ASM_CODE(compiler,"  vld1.32 %s[1], [%s]%s\n",
          orc_neon_reg_name (dest),
          orc_arm_reg_name (compiler->gp_tmpreg),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= 2<<10;
      code |= (1)<<7;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);

      ORC_ASM_CODE(compiler,"  vld1.32 %s[1], [%s]%s\n",
          orc_neon_reg_name (dest+1),
          orc_arm_reg_name (compiler->gp_tmpreg),
          update ? "!" : "");
      code = 0xf4a0000d;
      code |= 2<<10;
      code |= (1)<<7;
      code |= (compiler->gp_tmpreg&0xf) << 16;
      code |= ((dest+1)&0xf) << 12;
      code |= (((dest+1)>>4)&0x1) << 22;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
  }
}

#define UNARY(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_unary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
      orc_neon_emit_unary (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else if (p->insn_shift == vec_shift + 1) { \
      orc_neon_emit_unary_quad (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define UNARY_LONG(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_unary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
      orc_neon_emit_unary_long (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define UNARY_NARROW(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_unary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
      orc_neon_emit_unary_narrow (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define BINARY(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_binary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], \
          p->vars[insn->src_args[1]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet"); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
      orc_neon_emit_binary (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[1]].alloc); \
    } else if (p->insn_shift == vec_shift + 1) { \
      orc_neon_emit_binary_quad (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[1]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define BINARY_LONG(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_binary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], \
          p->vars[insn->src_args[1]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
    orc_neon_emit_binary_long (p, insn_name, code, \
        p->vars[insn->dest_args[0]].alloc, \
        p->vars[insn->src_args[0]].alloc, \
        p->vars[insn->src_args[1]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define BINARY_NARROW(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_binary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], \
          p->vars[insn->src_args[1]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
    orc_neon_emit_binary_narrow (p, insn_name, code, \
        p->vars[insn->dest_args[0]].alloc, \
        p->vars[insn->src_args[0]].alloc, \
        p->vars[insn->src_args[1]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define MOVE(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->vars[insn->dest_args[0]].alloc == p->vars[insn->src_args[0]].alloc) { \
    return; \
  } \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_binary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], \
          p->vars[insn->src_args[0]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    if (p->insn_shift <= vec_shift) { \
      orc_neon_emit_binary (p, "vorr", 0xf2200110, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else if (p->insn_shift == vec_shift + 1) { \
      orc_neon_emit_binary_quad (p, "vorr", 0xf2200110, \
          p->vars[insn->dest_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[0]].alloc); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

typedef struct {
  orc_uint32 code;
  char *name;
  orc_uint32 code64;
  char *name64;
  int negate;
  int bits;
  int vec_shift;
} ShiftInfo;
ShiftInfo immshift_info[] = {
  { 0xf2880510, "vshl.i8", 0x0f085400, "shl", FALSE, 8, 3 }, /* shlb */
  { 0xf2880010, "vshr.s8", 0x0f080400, "sshr", TRUE, 8, 3 }, /* shrsb */
  { 0xf3880010, "vshr.u8", 0x2f080400, "ushr", TRUE, 8, 3 }, /* shrub */
  { 0xf2900510, "vshl.i16", 0x0f105400, "shl", FALSE, 16, 2 },
  { 0xf2900010, "vshr.s16", 0x0f100400, "sshr", TRUE, 16, 2 },
  { 0xf3900010, "vshr.u16", 0x2f100400, "ushr", TRUE, 16, 2 },
  { 0xf2a00510, "vshl.i32", 0x0f205400, "shl", FALSE, 32, 1 },
  { 0xf2a00010, "vshr.s32", 0x0f200400, "sshr", TRUE, 32, 1 },
  { 0xf3a00010, "vshr.u32", 0x2f200400, "ushr", TRUE, 32, 1 }
};
ShiftInfo regshift_info[] = {
  { 0xf3000400, "vshl.u8", 0x2e204400, "ushl", FALSE, 0, 3 }, /* shlb */
  { 0xf2000400, "vshl.s8", 0x0e204400, "sshl", TRUE, 0, 3 }, /* shrsb */
  { 0xf3000400, "vshl.u8", 0x2e204400, "ushl", TRUE, 0, 3 }, /* shrub */
  { 0xf3100400, "vshl.u16", 0x2e604400, "ushl", FALSE, 0, 2 },
  { 0xf2100400, "vshl.s16", 0x0e604400, "sshl", TRUE, 0, 2 },
  { 0xf3100400, "vshl.u16", 0x2e604400, "ushl", TRUE, 0, 2 },
  { 0xf3200400, "vshl.u32", 0x2ea04400, "ushl", FALSE, 0, 1 },
  { 0xf2200400, "vshl.s32", 0x0ea04400, "sshl", TRUE, 0, 1 },
  { 0xf3200400, "vshl.u32", 0x2ea04400, "ushl", TRUE, 0, 1 }
};

static void
orc_neon_emit_shift(OrcCompiler *const p, int type,
                    const OrcVariable *const dest,
                    const OrcVariable *const src, int shift,
                    int is_quad)
{
  orc_uint32 code = 0;
  if (shift < 0) {
    ORC_COMPILER_ERROR(p, "shift negative");
    return;
  }
  if (shift >= immshift_info[type].bits) {
    ORC_COMPILER_ERROR(p, "shift too large");
    return;
  }
  if (p->is_64bit) {
    code = immshift_info[type].code64;
    ORC_ASM_CODE(p, "  %s %s, %s, #%d\n", immshift_info[type].name64,
                 orc_neon64_reg_name_vector(dest->alloc, dest->size, is_quad),
                 orc_neon64_reg_name_vector(src->alloc, src->size, is_quad),
                 shift);
    if (is_quad) {
      code |= 1 << 30;
    }
    code |= (dest->alloc & 0x1f) << 0;
    code |= (src->alloc & 0x1f) << 5;
  } else {
    code = immshift_info[type].code;
    if (is_quad == 0) {
      ORC_ASM_CODE(p, "  %s %s, %s, #%d\n", immshift_info[type].name,
                   orc_neon_reg_name(dest->alloc),
                   orc_neon_reg_name(src->alloc), shift);
    } else {
      ORC_ASM_CODE(p, "  %s %s, %s, #%d\n", immshift_info[type].name,
                   orc_neon_reg_name_quad(dest->alloc),
                   orc_neon_reg_name_quad(src->alloc), shift);
      code |= 0x40;
    }
    code |= (dest->alloc & 0xf) << 12;
    code |= ((dest->alloc >> 4) & 0x1) << 22;
    code |= (src->alloc & 0xf) << 0;
    code |= ((src->alloc >> 4) & 0x1) << 5;
  }
  if (immshift_info[type].negate) {
    shift = immshift_info[type].bits - shift;
  }
  code |= shift << 16;
  orc_arm_emit(p, code);
}

static void
orc_neon_rule_shift(OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = ORC_PTR_TO_INT(user);
  orc_uint32 code;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_neon_emit_shift(p, type, p->vars + insn->dest_args[0],
                        p->vars + insn->src_args[0],
                        (int)p->vars[insn->src_args[1]].value.i,
                        p->insn_shift > immshift_info[type].vec_shift);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };
    orc_neon_emit_loadpb (p, p->tmpreg, insn->src_args[1]);
    if (regshift_info[type].negate) {
      if (p->is_64bit)
        orc_neon64_emit_unary (p, "neg", 0x2e20b800, tmpreg, tmpreg, p->insn_shift - 1);
      else
        orc_neon_emit_unary_quad (p, "vneg.s8", 0xf3b10380, p->tmpreg, p->tmpreg);
    }

    if (p->is_64bit) {
      orc_neon64_emit_binary (p, regshift_info[type].name64,
          regshift_info[type].code64,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          tmpreg,
	  p->insn_shift - !!(p->insn_shift > regshift_info[type].vec_shift));
    } else {
      code = regshift_info[type].code;
      if (p->insn_shift <= regshift_info[type].vec_shift) {
        ORC_ASM_CODE(p,"  %s %s, %s, %s\n",
            regshift_info[type].name,
            orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
            orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
            orc_neon_reg_name (p->tmpreg));
      } else {
        ORC_ASM_CODE(p,"  %s %s, %s, %s\n",
            regshift_info[type].name,
            orc_neon_reg_name_quad (p->vars[insn->dest_args[0]].alloc),
            orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc),
            orc_neon_reg_name_quad (p->tmpreg));
        code |= 0x40;
      }
      code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
      code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
      code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
      code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
      code |= (p->tmpreg&0xf)<<16;
      code |= ((p->tmpreg>>4)&0x1)<<7;
      orc_arm_emit (p, code);
    }
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}

#if 0
static void
orc_neon_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_uint32 code;
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    code = 0xf2900010;
    ORC_ASM_CODE(p,"  vshr.s16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        p->vars[insn->src_args[1]].value);
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= ((16 - p->vars[insn->src_args[1]].value)&0xf)<<16;
    orc_arm_emit (p, code);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    code = 0xf2100400;
    ORC_ASM_CODE(p,"  vshl.s16 %s, %s, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= (p->vars[insn->src_args[1]].alloc&0xf)<<16;
    code |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7;
    orc_arm_emit (p, code);
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}

static void
orc_neon_rule_shrsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_uint32 code;
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    code = 0xf2900010;
    ORC_ASM_CODE(p,"  vshr.s32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        p->vars[insn->src_args[1]].value);
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= ((16 - p->vars[insn->src_args[1]].value)&0xf)<<16;
    orc_arm_emit (p, code);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    code = 0xf2100400;
    ORC_ASM_CODE(p,"  vshl.s32 %s, %s, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= (p->vars[insn->src_args[1]].alloc&0xf)<<16;
    code |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7;
    orc_arm_emit (p, code);
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}
#endif


static void
orc_neon_rule_andn (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int max_shift = ORC_PTR_TO_INT(user);

  if (p->is_64bit) {
      orc_neon64_emit_binary (p, "bic", 0x0e601c00,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[1]],
          p->vars[insn->src_args[0]],
	  p->insn_shift - (p->insn_shift > max_shift));
  } else {
    /* this is special because the operand order is reversed */
    if (p->insn_shift <= max_shift) {
      orc_neon_emit_binary (p, "vbic", 0xf2100110,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->src_args[1]].alloc,
          p->vars[insn->src_args[0]].alloc);
    } else {
      orc_neon_emit_binary_quad (p, "vbic", 0xf2100110,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->src_args[1]].alloc,
          p->vars[insn->src_args[0]].alloc);
    }
  }
}



UNARY(absb,"vabs.s8",0xf3b10300, "abs", 0x0e20b800, 3)
BINARY(addb,"vadd.i8",0xf2000800, "add", 0x0e208400, 3)
BINARY(addssb,"vqadd.s8",0xf2000010, "sqadd", 0x0e200c00, 3)
BINARY(addusb,"vqadd.u8",0xf3000010, "uqadd", 0x2e200c00, 3)
BINARY(andb,"vand",0xf2000110, "and", 0x0e201c00, 3)
/* BINARY(andnb,"vbic",0xf2100110, NULL, 0, 3) */
BINARY(avgsb,"vrhadd.s8",0xf2000100, "srhadd", 0x0e201400, 3)
BINARY(avgub,"vrhadd.u8",0xf3000100, "urhadd", 0x2e201400, 3)
BINARY(cmpeqb,"vceq.i8",0xf3000810, "cmeq", 0x2e208c00, 3)
BINARY(cmpgtsb,"vcgt.s8",0xf2000300, "cmgt", 0x0e203400, 3)
MOVE(copyb,"vmov",0xf2200110, "mov", 0x0ea01c00, 3)
BINARY(maxsb,"vmax.s8",0xf2000600, "smax", 0x0e206400, 3)
BINARY(maxub,"vmax.u8",0xf3000600, "umax", 0x2e206400, 3)
BINARY(minsb,"vmin.s8",0xf2000610, "smin", 0x0e206c00, 3)
BINARY(minub,"vmin.u8",0xf3000610, "umin", 0x2e206c00, 3)
BINARY(mullb,"vmul.i8",0xf2000910, "mul", 0x0e209c00, 3)
BINARY(orb,"vorr",0xf2200110, "orr", 0x0ea01c00, 3)
/* LSHIFT(shlb,"vshl.i8",0xf2880510, NULL, 0, 3) */
/* RSHIFT(shrsb,"vshr.s8",0xf2880010,8, NULL, 0, 3) */
/* RSHIFT(shrub,"vshr.u8",0xf3880010,8, NULL, 0, 3) */
BINARY(subb,"vsub.i8",0xf3000800, "sub", 0x2e208400, 3)
BINARY(subssb,"vqsub.s8",0xf2000210, "sqsub", 0x0e202c00, 3)
BINARY(subusb,"vqsub.u8",0xf3000210, "uqsub", 0x2e202c00, 3)
BINARY(xorb,"veor",0xf3000110, "eor", 0x2e201c00, 3)

UNARY(absw,"vabs.s16",0xf3b50300, "abs", 0x0e60b800, 2)
BINARY(addw,"vadd.i16",0xf2100800, "add", 0x0e608400, 2)
BINARY(addssw,"vqadd.s16",0xf2100010, "sqadd", 0x0e600c00, 2)
BINARY(addusw,"vqadd.u16",0xf3100010, "uqadd", 0x2e600c00, 2)
BINARY(andw,"vand",0xf2000110, "and", 0x0e201c00, 2)
/* BINARY(andnw,"vbic",0xf2100110, NULL, 0, 2) */
BINARY(avgsw,"vrhadd.s16",0xf2100100, "srhadd", 0x0e601400, 2)
BINARY(avguw,"vrhadd.u16",0xf3100100, "urhadd", 0x2e601400, 2)
BINARY(cmpeqw,"vceq.i16",0xf3100810, "cmeq", 0x2e608c00, 2)
BINARY(cmpgtsw,"vcgt.s16",0xf2100300, "cmgt", 0x0e603400, 2)
MOVE(copyw,"vmov",0xf2200110, "mov", 0x0ea01c00, 2)
BINARY(maxsw,"vmax.s16",0xf2100600, "smax", 0x0e606400, 2)
BINARY(maxuw,"vmax.u16",0xf3100600, "umax", 0x2e606400, 2)
BINARY(minsw,"vmin.s16",0xf2100610, "smin", 0x0e606c00, 2)
BINARY(minuw,"vmin.u16",0xf3100610, "umin", 0x2e606c00, 2)
BINARY(mullw,"vmul.i16",0xf2100910, "mul", 0x0e609c00, 2)
BINARY(orw,"vorr",0xf2200110, "orr", 0x0ea01c00, 2)
/* LSHIFT(shlw,"vshl.i16",0xf2900510, NULL, 0, 2) */
/* RSHIFT(shrsw,"vshr.s16",0xf2900010,16, NULL, 0, 2) */
/* RSHIFT(shruw,"vshr.u16",0xf3900010,16, NULL, 0, 2) */
BINARY(subw,"vsub.i16",0xf3100800, "sub", 0x2e608400, 2)
BINARY(subssw,"vqsub.s16",0xf2100210, "sqsub", 0x0e602c00, 2)
BINARY(subusw,"vqsub.u16",0xf3100210, "uqsub", 0x2e602c00, 2)
BINARY(xorw,"veor",0xf3000110, "eor", 0x2e201c00, 2)

UNARY(absl,"vabs.s32",0xf3b90300, "abs", 0x0ea0b800, 1)
BINARY(addl,"vadd.i32",0xf2200800, "add", 0x0ea08400, 1)
BINARY(addssl,"vqadd.s32",0xf2200010, "sqadd", 0x0ea00c00, 1)
BINARY(addusl,"vqadd.u32",0xf3200010, "uqadd", 0x2ea00c00, 1)
BINARY(andl,"vand",0xf2000110, "and", 0x0e201c00, 1)
/* BINARY(andnl,"vbic",0xf2100110, NULL, 0, 1) */
BINARY(avgsl,"vrhadd.s32",0xf2200100, "srhadd", 0x0ea01400, 1)
BINARY(avgul,"vrhadd.u32",0xf3200100, "urhadd", 0x2ea01400, 1)
BINARY(cmpeql,"vceq.i32",0xf3200810, "cmeq", 0x2ea08c00, 1)
BINARY(cmpgtsl,"vcgt.s32",0xf2200300, "cmgt", 0x0ea03400, 1)
MOVE(copyl,"vmov",0xf2200110, "mov", 0x0ea01c00, 1)
BINARY(maxsl,"vmax.s32",0xf2200600, "smax", 0x0ea06400, 1)
BINARY(maxul,"vmax.u32",0xf3200600, "umax", 0x2ea06400, 1)
BINARY(minsl,"vmin.s32",0xf2200610, "smin", 0x0ea06c00, 1)
BINARY(minul,"vmin.u32",0xf3200610, "umin", 0x2ea06c00, 1)
BINARY(mulll,"vmul.i32",0xf2200910, "mul", 0x0ea09c00, 1)
BINARY(orl,"vorr",0xf2200110, "orr", 0x0ea01c00, 1)
/* LSHIFT(shll,"vshl.i32",0xf2a00510, NULL, 0, 1) */
/* RSHIFT(shrsl,"vshr.s32",0xf2a00010,32, NULL, 0, 1) */
/* RSHIFT(shrul,"vshr.u32",0xf3a00010,32, NULL, 0, 1) */
BINARY(subl,"vsub.i32",0xf3200800, "sub", 0x2ea08400, 1)
BINARY(subssl,"vqsub.s32",0xf2200210, "sqsub", 0x0ea02c00, 1)
BINARY(subusl,"vqsub.u32",0xf3200210, "uqsub", 0x2ea02c00, 1)
BINARY(xorl,"veor",0xf3000110, "eor", 0x2e201c00, 1)

/* UNARY(absq,"vabs.s64",0xf3b10300, "abs", 0xee0b800, 0) */
BINARY(addq,"vadd.i64",0xf2300800, "add", 0x4ee08400, 0)
/* BINARY(addssq,"vqadd.s64",0xf2000010, "sqadd", 0x0ee00c00, 0) */
/* BINARY(addusq,"vqadd.u64",0xf3000010, "uqadd", 0x2ee00c00, 0) */
BINARY(andq,"vand",0xf2000110, "and", 0x0e201c00, 0)
/* BINARY(avgsq,"vrhadd.s64",0xf2000100, "srhadd", 0x0ee01400, 0) */
/* BINARY(avguq,"vrhadd.u64",0xf3000100, "urhadd", 0x2ee01400, 0) */
/* BINARY(cmpeqq,"vceq.i64",0xf3000810, "cmeq", 0x2ee08c00, 0) */
/* BINARY(cmpgtsq,"vcgt.s64",0xf2000300, "cmgt", 0x0ee03400, 0) */
MOVE(copyq,"vmov",0xf2200110, "mov", 0x0ea01c00, 0)
/* BINARY(maxsq,"vmax.s64",0xf2000600, "smax", 0x0ee06400, 0) */
/* BINARY(maxuq,"vmax.u64",0xf3000600, "umax", 0x2ee06400, 0) */
/* BINARY(minsq,"vmin.s64",0xf2000610, "smin", 0x0ee06c00, 0) */
/* BINARY(minuq,"vmin.u64",0xf3000610, "umin", 0x2ee06c00, 0) */
/* BINARY(mullq,"vmul.i64",0xf2000910, "mul", 0x0ee09c00, 0) */
BINARY(orq,"vorr",0xf2200110, "orr", 0x0ea01c00, 0)
BINARY(subq,"vsub.i64",0xf3300800, "sub", 0x6ee08400, 0)
/* BINARY(subssq,"vqsub.s64",0xf2000210, "sqsub", 0x0ee00c00, 0) */
/* BINARY(subusq,"vqsub.u64",0xf3000210, "uqsub", 0x2ee00c00, 0) */
BINARY(xorq,"veor",0xf3000110, "eor", 0x2e201c00, 0)

UNARY_LONG(convsbw,"vmovl.s8",0xf2880a10, "sshll", 0x0f08a400, 3)
UNARY_LONG(convubw,"vmovl.u8",0xf3880a10, "ushll", 0x2f08a400, 3)
UNARY_LONG(convswl,"vmovl.s16",0xf2900a10, "sshll", 0x0f10a400, 2)
UNARY_LONG(convuwl,"vmovl.u16",0xf3900a10, "ushll", 0x2f10a400, 2)
UNARY_LONG(convslq,"vmovl.s32",0xf2a00a10, "sshll", 0x0f20a400, 1)
UNARY_LONG(convulq,"vmovl.u32",0xf3a00a10, "ushll", 0x2f20a400, 1)
UNARY_NARROW(convwb,"vmovn.i16",0xf3b20200, "xtn", 0x0e212800, 3)
UNARY_NARROW(convssswb,"vqmovn.s16",0xf3b20280, "sqxtn", 0x0e214800, 3)
UNARY_NARROW(convsuswb,"vqmovun.s16",0xf3b20240, "sqxtun", 0x2e212800, 3)
UNARY_NARROW(convuuswb,"vqmovn.u16",0xf3b202c0, "uqxtn", 0x2e214800, 3)
UNARY_NARROW(convlw,"vmovn.i32",0xf3b60200, "xtn", 0x0e612800, 2)
UNARY_NARROW(convql,"vmovn.i64",0xf3ba0200, "xtn", 0x0ea12800, 1)
UNARY_NARROW(convssslw,"vqmovn.s32",0xf3b60280, "sqxtn", 0x0e614800, 2)
UNARY_NARROW(convsuslw,"vqmovun.s32",0xf3b60240, "sqxtun", 0x2e612800, 2)
UNARY_NARROW(convuuslw,"vqmovn.u32",0xf3b602c0, "uqxtn", 0x2e614800, 2)
UNARY_NARROW(convsssql,"vqmovn.s64",0xf3ba0280, "sqxtn", 0x0ea14800, 1)
UNARY_NARROW(convsusql,"vqmovun.s64",0xf3ba0240, "sqxtun", 0x2ea12800, 1)
UNARY_NARROW(convuusql,"vqmovn.u64",0xf3ba02c0, "uqxtn", 0x2ea14800, 1)

BINARY_LONG(mulsbw,"vmull.s8",0xf2800c00, "smull", 0x0e20c000, 3)
BINARY_LONG(mulubw,"vmull.u8",0xf3800c00, "umull", 0x2e20c000, 3)
BINARY_LONG(mulswl,"vmull.s16",0xf2900c00, "smull", 0x0e60c000, 2)
BINARY_LONG(muluwl,"vmull.u16",0xf3900c00, "umull", 0x2e60c000, 2)

UNARY(swapw,"vrev16.i8",0xf3b00100, "rev16", 0x0e201800, 2)
UNARY(swapl,"vrev32.i8",0xf3b00080, "rev32", 0x2e200800, 1)
UNARY(swapq,"vrev64.i8",0xf3b00000, "rev64", 0x0e200800, 0)
UNARY(swapwl,"vrev32.i16",0xf3b40080, "rev32", 0x2e600800, 1)
UNARY(swaplq,"vrev64.i32",0xf3b80000, "rev64", 0x0ea00800, 0)

UNARY_NARROW(select0ql,"vmovn.i64",0xf3ba0200, "xtn", 0x0ea12800, 1)
UNARY_NARROW(select0lw,"vmovn.i32",0xf3b60200, "xtn", 0x0e612800, 2)
UNARY_NARROW(select0wb,"vmovn.i16",0xf3b20200, "xtn", 0x0e212800, 3)

BINARY(addf,"vadd.f32",0xf2000d00, "fadd", 0x0e20d400, 1)
BINARY(subf,"vsub.f32",0xf2200d00, "fsub", 0x0ea0d400, 1)
BINARY(mulf,"vmul.f32",0xf3000d10, "fmul", 0x2e20dc00, 1)
BINARY(maxf,"vmax.f32",0xf2000f00, "fmax", 0x0e20f400, 1)
BINARY(minf,"vmin.f32",0xf2200f00, "fmin", 0x0ea0f400, 1)
BINARY(cmpeqf,"vceq.f32",0xf2000e00, "fcmeq", 0x5e20e400, 1)
/* BINARY_R(cmpltf,"vclt.f32",0xf3200e00, "fcmlt", 0x5ef8e800, 1) */
/* BINARY_R(cmplef,"vcle.f32",0xf3000e00, "fcmle", 0x7ef8d800, 1) */
UNARY(convfl,"vcvt.s32.f32",0xf3bb0700, "fcvtzs", 0x0ea1b800, 1)
UNARY(convlf,"vcvt.f32.s32",0xf3bb0600, "scvtf", 0x0e21d800, 1)

#define UNARY_VFP(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_unary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], vec_shift - 1); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    orc_neon_emit_unary (p, insn_name, code, \
        p->vars[insn->dest_args[0]].alloc, \
        p->vars[insn->src_args[0]].alloc); \
    if (p->insn_shift == vec_shift + 1) { \
      orc_neon_emit_unary (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc + 1, \
          p->vars[insn->src_args[0]].alloc + 1); \
    } else { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

#define BINARY_VFP(opcode,insn_name,code,insn_name64,code64,vec_shift) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->is_64bit) { \
    if (insn_name64) { \
      orc_neon64_emit_binary (p, insn_name64, code64, \
          p->vars[insn->dest_args[0]], \
          p->vars[insn->src_args[0]], \
          p->vars[insn->src_args[1]], vec_shift); \
    } else { \
      ORC_COMPILER_ERROR(p, "not supported in AArch64 yet [%s %x]", (insn_name64), (code64)); \
    } \
  } else { \
    orc_neon_emit_binary (p, insn_name, code, \
        p->vars[insn->dest_args[0]].alloc, \
        p->vars[insn->src_args[0]].alloc, \
        p->vars[insn->src_args[1]].alloc); \
    if (p->insn_shift == vec_shift + 1) { \
      orc_neon_emit_binary (p, insn_name, code, \
          p->vars[insn->dest_args[0]].alloc+1, \
          p->vars[insn->src_args[0]].alloc+1, \
          p->vars[insn->src_args[1]].alloc+1); \
    } else if (p->insn_shift > vec_shift + 1) { \
      ORC_COMPILER_ERROR(p, "shift too large"); \
    } \
  } \
}

BINARY_VFP(addd,"vadd.f64",0xee300b00, "fadd", 0x4e60d400, 0)
BINARY_VFP(subd,"vsub.f64",0xee300b40, "fsub", 0x4ee0d400, 0)
BINARY_VFP(muld,"vmul.f64",0xee200b00, "fmul", 0x6e60dc00, 0)
BINARY_VFP(divd,"vdiv.f64",0xee800b00, "fdiv", 0x6e60fc00, 0)
BINARY_VFP(divf,"vdiv.f32",0xee800a00, "fdiv", 0x6e20fc00, 0)
UNARY_VFP(sqrtd,"vsqrt.f64",0xeeb10b00, "fsqrt", 0x6ee1f800, 0)
UNARY_VFP(sqrtf,"vsqrt.f32",0xeeb10ac0, "fsqrt", 0x6ea1f800, 0)
/* BINARY_VFP(cmpeqd,"vcmpe.f64",0xee000000, NULL, 0, 0) */
UNARY_VFP(convdf,"vcvt.f64.f32",0xee200b00, "fcvtzs", 0x4ee1b800, 0)
UNARY_VFP(convfd,"vcvt.f32.f64",0xee200b00, "scvtf", 0x4e61d800, 0)

static void
orc_neon_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };
  unsigned int code;

  if (p->insn_shift < 2) {
    if (p->is_64bit) {
      orc_neon64_emit_unary (p, "shl",
          0x0f405400 | (48 << 16),
          tmpreg, p->vars[insn->src_args[0]],
          p->insn_shift - 1);
      orc_neon64_emit_binary (p, "add", 0x0ee08400,
          p->vars[insn->dest_args[0]],
          p->vars[insn->dest_args[0]],
          tmpreg, p->insn_shift - 1);
    } else {
      ORC_ASM_CODE(p,"  vshl.i64 %s, %s, #%d\n",
          orc_neon_reg_name (p->tmpreg),
          orc_neon_reg_name (p->vars[insn->src_args[0]].alloc), 48);
      code = NEON_BINARY(0xf2a00590, p->tmpreg, 0,
          p->vars[insn->src_args[0]].alloc);
      code |= (48) << 16;
      orc_arm_emit (p, code);

      orc_neon_emit_binary (p, "vadd.i16", 0xf2100800,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "add", 0x0e608400,
          p->vars[insn->dest_args[0]],
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift);
    } else {
      orc_neon_emit_binary (p, "vadd.i16", 0xf2100800,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->src_args[0]].alloc);
    }
  }
}

static void
orc_neon_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };
  unsigned int code;

  if (p->insn_shift < 1) {
    if (p->is_64bit) {
      orc_neon64_emit_unary (p, "shl",
          0x0f405400 | (32 << 16),
          tmpreg, p->vars[insn->src_args[0]],
          p->insn_shift - 1);
      orc_neon64_emit_binary (p, "add", 0x0ee08400,
          p->vars[insn->dest_args[0]],
          p->vars[insn->dest_args[0]],
          tmpreg, p->insn_shift - 1);
    } else {
      ORC_ASM_CODE(p,"  vshl.i64 %s, %s, #%d\n",
          orc_neon_reg_name (p->tmpreg),
          orc_neon_reg_name (p->vars[insn->src_args[0]].alloc), 32);
      code = NEON_BINARY(0xf2a00590, p->tmpreg, 0,
          p->vars[insn->src_args[0]].alloc);
      code |= (32) << 16;
      orc_arm_emit (p, code);

      orc_neon_emit_binary (p, "vadd.i32", 0xf2200800,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "add", 0x0ea08400,
          p->vars[insn->dest_args[0]],
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift);
    } else {
      orc_neon_emit_binary (p, "vadd.i32", 0xf2200800,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc,
          p->vars[insn->src_args[0]].alloc);
    }
  }
}

static void
orc_neon_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    ORC_ASM_CODE(p,"  shrn %s, %s, #%d\n",
        orc_neon64_reg_name_vector (p->vars[insn->dest_args[0]].alloc, 8, 0),
        orc_neon64_reg_name_vector (p->vars[insn->src_args[0]].alloc, 8, 1), 8);
    orc_neon64_emit_unary (p, "shrn", 0x0f088400,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift);
  } else {
    ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc), 8);
    code = NEON_BINARY (0xf2880810,
        p->vars[insn->dest_args[0]].alloc,
        0, p->vars[insn->src_args[0]].alloc);
    orc_arm_emit (p, code);
  }
}

static void
orc_neon_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    ORC_ASM_CODE(p,"  shrn %s, %s, #%d\n",
        orc_neon64_reg_name_vector (p->vars[insn->dest_args[0]].alloc, 8, 0),
        orc_neon64_reg_name_vector (p->vars[insn->src_args[0]].alloc, 8, 1), 16);
    orc_neon64_emit_unary (p, "shrn", 0x0f108400,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift);
  } else {
    ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc), 16);
    code = NEON_BINARY (0xf2900810,
        p->vars[insn->dest_args[0]].alloc,
        0, p->vars[insn->src_args[0]].alloc);
    orc_arm_emit (p, code);
  }
}

static void
orc_neon_rule_select1ql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    ORC_ASM_CODE(p,"  shrn %s, %s, #%d\n",
        orc_neon64_reg_name_vector (p->vars[insn->dest_args[0]].alloc, 8, 0),
        orc_neon64_reg_name_vector (p->vars[insn->src_args[0]].alloc, 8, 1), 32);
    orc_neon64_emit_unary (p, "shrn", 0x0f208400,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift);
  } else {
    ORC_ASM_CODE(p,"  vtrn.32 %s, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc));
    code = NEON_BINARY (0xf2a00810,
        p->vars[insn->dest_args[0]].alloc,
        0, p->vars[insn->src_args[0]].alloc);
    orc_arm_emit (p, code);
  }
}

static void
orc_neon_rule_convhwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    ORC_ASM_CODE(p,"  shrn %s, %s, #%d\n",
        orc_neon64_reg_name_vector (p->vars[insn->dest_args[0]].alloc, 8, 0),
        orc_neon64_reg_name_vector (p->vars[insn->src_args[0]].alloc, 8, 1), 8);
    orc_neon64_emit_unary (p, "shrn", 0x0f088400,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift);
  } else {
    ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc), 8);
    code = NEON_BINARY (0xf2880810,
        p->vars[insn->dest_args[0]].alloc,
        0, p->vars[insn->src_args[0]].alloc);
    orc_arm_emit (p, code);
  }
}

static void
orc_neon_rule_convhlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    ORC_ASM_CODE(p,"  shrn %s, %s\n",
        orc_neon64_reg_name_vector (p->vars[insn->dest_args[0]].alloc, 8, 0),
        orc_neon64_reg_name_vector (p->vars[insn->src_args[0]].alloc, 8, 1));
    orc_neon64_emit_unary (p, "shrn", 0x0f108400,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift);
  } else {
    ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->vars[insn->src_args[0]].alloc), 16);
    code = NEON_BINARY (0xf2900810,
        p->vars[insn->dest_args[0]].alloc,
        0, p->vars[insn->src_args[0]].alloc);
    orc_arm_emit (p, code);
  }
}

static void
orc_neon_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[1]].size };

  if (p->insn_shift <= 2) {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "zip1", 0x0e003800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      if (p->vars[insn->src_args[1]].last_use != p->insn_index ||
          p->vars[insn->src_args[1]].alloc == p->vars[insn->dest_args[0]].alloc) {
        orc_neon_emit_mov (p, tmpreg, p->vars[insn->src_args[1]]);
        orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
            p->vars[insn->dest_args[0]].alloc,
            p->tmpreg);
      } else {
        orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
            p->vars[insn->dest_args[0]].alloc,
            p->vars[insn->src_args[1]].alloc);
      }
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "zip1", 0x0e003800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift - 1);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      orc_neon_emit_mov_quad (p, tmpreg, p->vars[insn->src_args[1]]);
      orc_neon_emit_unary_quad (p, "vzip.8", 0xf3b20180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  }
}

static void
orc_neon_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[1]].size };

  if (p->insn_shift <= 1) {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "zip1", 0x0e403800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      if (p->vars[insn->src_args[1]].last_use != p->insn_index ||
          p->vars[insn->src_args[1]].alloc == p->vars[insn->dest_args[0]].alloc) {
        orc_neon_emit_mov (p, tmpreg, p->vars[insn->src_args[1]]);
        orc_neon_emit_unary (p, "vzip.16", 0xf3b60180,
            p->vars[insn->dest_args[0]].alloc,
            p->tmpreg);
      } else {
        orc_neon_emit_unary (p, "vzip.16", 0xf3b60180,
            p->vars[insn->dest_args[0]].alloc,
            p->vars[insn->src_args[1]].alloc);
      }
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "zip1", 0x0e403800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift - 1);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      if (p->vars[insn->src_args[1]].last_use != p->insn_index ||
          p->vars[insn->src_args[1]].alloc == p->vars[insn->dest_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, tmpreg, p->vars[insn->src_args[1]]);
        orc_neon_emit_unary_quad (p, "vzip.16", 0xf3b60180,
            p->vars[insn->dest_args[0]].alloc,
            p->tmpreg);
      } else {
        orc_neon_emit_unary_quad (p, "vzip.16", 0xf3b60180,
            p->vars[insn->dest_args[0]].alloc,
            p->vars[insn->src_args[1]].alloc);
      }
    }
  }
}

static void
orc_neon_rule_mergelq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[1]].size };

  if (p->insn_shift <= 0) {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "trn1", 0x0e802800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      if (p->vars[insn->src_args[1]].last_use != p->insn_index ||
          p->vars[insn->src_args[1]].alloc == p->vars[insn->dest_args[0]].alloc) {
        orc_neon_emit_mov (p, tmpreg, p->vars[insn->src_args[1]]);
        orc_neon_emit_unary (p, "vtrn.32", 0xf3ba0080,
            p->vars[insn->dest_args[0]].alloc,
            p->tmpreg);
      } else {
        orc_neon_emit_unary (p, "vtrn.32", 0xf3ba0080,
            p->vars[insn->dest_args[0]].alloc,
            p->vars[insn->src_args[1]].alloc);
      }
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "zip1", 0x0e803800,
          p->vars[insn->dest_args[0]],
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift - 1);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      if (p->vars[insn->src_args[1]].last_use != p->insn_index ||
          p->vars[insn->src_args[1]].alloc == p->vars[insn->dest_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, tmpreg, p->vars[insn->src_args[1]]);
        orc_neon_emit_unary_quad (p, "vzip.32", 0xf3ba0180,
            p->vars[insn->dest_args[0]].alloc,
            p->tmpreg);
      } else {
        orc_neon_emit_unary_quad (p, "vzip.32", 0xf3ba0180,
            p->vars[insn->dest_args[0]].alloc,
            p->vars[insn->src_args[1]].alloc);
      }
    }
  }
}

static void
orc_neon_rule_splatbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };

  if (p->is_64bit) {
    orc_neon64_emit_binary (p, "zip1", 0x0e003800,
        p->vars[insn->dest_args[0]],
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift > 2));
  } else {
    if (p->insn_shift <= 2) {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      orc_neon_emit_mov (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      orc_neon_emit_mov_quad (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary_quad (p, "vzip.8", 0xf3b20180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  }
}

static void
orc_neon_rule_splatbl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };

  if (p->is_64bit) {
    orc_neon64_emit_binary (p, "zip1", 0x0e003800,
        tmpreg,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift > 1));
    orc_neon64_emit_binary (p, "zip1", 0x0e403800,
        p->vars[insn->dest_args[0]],
        tmpreg,
        tmpreg, p->insn_shift - (p->insn_shift > 1));
  } else {
    if (p->insn_shift <= 1) {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      orc_neon_emit_mov (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
      orc_neon_emit_mov (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary (p, "vzip.16", 0xf3b60180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    } else {
      if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]],
            p->vars[insn->src_args[0]]);
      }

      orc_neon_emit_mov (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary_quad (p, "vzip.8", 0xf3b20180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
      orc_neon_emit_mov (p, tmpreg, p->vars[insn->dest_args[0]]);
      orc_neon_emit_unary_quad (p, "vzip.16", 0xf3b60180,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  }
}

static void
orc_neon_rule_splatw3q (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_uint32 code;
  int offset = 0;
  int label = 20;

  if (p->is_64bit) {
      OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
      orc_neon64_emit_binary (p, "trn2", 0x0e406800,
          tmpreg,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift > 0));
      orc_neon64_emit_binary (p, "trn2", 0x0e806800,
          p->vars[insn->dest_args[0]],
          tmpreg,
          tmpreg, p->insn_shift - (p->insn_shift > 0));
  } else {
    orc_arm_add_fixup (p, label, 1);
    ORC_ASM_CODE(p,"  vldr %s, .L%d+%d\n",
        orc_neon_reg_name (p->tmpreg), label, offset);
    code = 0xed9f0b00;
    code |= (p->tmpreg&0xf) << 12;
    code |= ((p->tmpreg>>4)&0x1) << 22;
    code |= ((offset - 8) >> 2)&0xff;
    orc_arm_emit (p, code);

    ORC_ASM_CODE(p,"  vtbl.8 %s, { %s, %s }, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc + 1),
        orc_neon_reg_name (p->tmpreg));
    code = NEON_BINARY(0xf3b00900,
        p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[0]].alloc,
        p->tmpreg);
    orc_arm_emit (p, code);

    if (p->insn_shift > 0) {
      ORC_ASM_CODE(p,"  vtbl.8 %s, { %s }, %s\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc+1),
          orc_neon_reg_name (p->vars[insn->src_args[0]].alloc+1),
          orc_neon_reg_name (p->tmpreg));
      code = NEON_BINARY(0xf3b00800,
          p->vars[insn->dest_args[0]].alloc+1,
          p->vars[insn->src_args[0]].alloc+1,
          p->tmpreg);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };
  orc_uint32 x;
  unsigned int code;

  if (p->insn_shift < 2) {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "uabdl", 0x2e207000,
          tmpreg,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift);
      orc_neon64_emit_unary (p, "shl",
          0x0f405400 | ((64 - (16<<p->insn_shift)) << 16),
          tmpreg, tmpreg,
	  p->insn_shift - 1);
      orc_neon64_emit_unary (p, "uadalp", 0x2e606800,
          p->vars[insn->dest_args[0]],
          tmpreg, p->insn_shift);
    } else {
      x = 0xf3800700;
      ORC_ASM_CODE(p,"  vabdl.u8 %s, %s, %s\n",
          orc_neon_reg_name_quad (p->tmpreg),
          orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
          orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
      x |= (p->tmpreg&0xf)<<12;
      x |= ((p->tmpreg>>4)&0x1)<<22;
      x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
      x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
      x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0;
      x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5;
      orc_arm_emit (p, x);

      ORC_ASM_CODE(p,"  vshl.i64 %s, %s, #%d\n",
          orc_neon_reg_name (p->tmpreg),
          orc_neon_reg_name (p->tmpreg), 64 - (16<<p->insn_shift));
      code = NEON_BINARY(0xf2a00590, p->tmpreg, 0, p->tmpreg);
      code |= (64 - (16<<p->insn_shift)) << 16;
      orc_arm_emit (p, code);

      orc_neon_emit_unary (p, "vpadal.u16", 0xf3b40680,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  } else {
    if (p->is_64bit) {
      orc_neon64_emit_binary (p, "uabdl", 0x2e207000,
          tmpreg,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]], p->insn_shift);
      orc_neon64_emit_unary (p, "uadalp", 0x2e606800,
          p->vars[insn->dest_args[0]],
          tmpreg, p->insn_shift);
    } else {
      x = 0xf3800700;
      ORC_ASM_CODE(p,"  vabdl.u8 %s, %s, %s\n",
          orc_neon_reg_name_quad (p->tmpreg),
          orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
          orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
      x |= (p->tmpreg&0xf)<<12;
      x |= ((p->tmpreg>>4)&0x1)<<22;
      x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
      x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
      x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0;
      x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5;
      orc_arm_emit (p, x);

      orc_neon_emit_unary (p, "vpadal.u16", 0xf3b40680,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg);
    }
  }
}

static void
orc_neon_rule_signw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };

  /* slow */

  orc_neon_emit_loadiw (p, &tmpreg, 1);
  if (p->insn_shift < 3) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0e606c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmin.s16", 0xf2100610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0e606c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmin.s16", 0xf2100610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  }
  orc_neon_emit_loadiw (p, &tmpreg, -1);
  if (p->insn_shift < 3) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0e606400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmax.s16", 0xf2100600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0e606400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmax.s16", 0xf2100600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  }
}

static void
orc_neon_rule_signb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };

  /* slow */

  orc_neon_emit_loadib (p, &tmpreg, 1);
  if (p->insn_shift < 4) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0e206c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmin.s8", 0xf2000610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0e206c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmin.s8", 0xf2000610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  }
  orc_neon_emit_loadib (p, &tmpreg, -1);
  if (p->insn_shift < 4) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0e206400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmax.s8", 0xf2000600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0e206400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmax.s8", 0xf2000600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  }
}

static void
orc_neon_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  OrcVariable tmpreg = { .alloc = p->tmpreg, .size = p->vars[insn->src_args[0]].size };

  /* slow */

  orc_neon_emit_loadil (p, &tmpreg, 1);
  if (p->insn_shift < 2) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0ea06c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmin.s32", 0xf2200610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smin", 0x0ea06c00,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->src_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmin.s32", 0xf2200610,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc);
  }
  orc_neon_emit_loadil (p, &tmpreg, -1);
  if (p->insn_shift < 2) {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0ea06400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift);
    else
      orc_neon_emit_binary (p, "vmax.s32", 0xf2200600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  } else {
    if (p->is_64bit)
      orc_neon64_emit_binary (p, "smax", 0x0ea06400,
          p->vars[insn->dest_args[0]],
          tmpreg,
          p->vars[insn->dest_args[0]], p->insn_shift - 1);
    else
      orc_neon_emit_binary_quad (p, "vmax.s32", 0xf2200600,
          p->vars[insn->dest_args[0]].alloc,
          p->tmpreg,
          p->vars[insn->dest_args[0]].alloc);
  }
}

static void
orc_neon_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "umull", 0x2e20c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 4) {
      orc_neon64_emit_binary (p, "umull", 0x2e20c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f088400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 4) {
      orc_neon64_emit_unary (p, "shrn", 0x0f088400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.u8",0xf3800c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);

    ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 8);
    code = NEON_BINARY (0xf2880810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 4) {
      orc_neon_emit_binary_long (p, "vmull.u8",0xf3800c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 8);
      code = NEON_BINARY (0xf2880810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "smull", 0x0e20c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 4) {
      orc_neon64_emit_binary (p, "smull", 0x0e20c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f088400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 4) {
      orc_neon64_emit_unary (p, "shrn", 0x0f088400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.s8",0xf2800c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
    ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 8);
    code = NEON_BINARY (0xf2880810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 4) {
      orc_neon_emit_binary_long (p, "vmull.s8",0xf2800c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i16 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 8);
      code = NEON_BINARY (0xf2880810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_mulhuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "umull", 0x2e60c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 3) {
      orc_neon64_emit_binary (p, "umull", 0x2e60c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f108400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 3) {
      orc_neon64_emit_unary (p, "shrn", 0x0f108400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.u16",0xf3900c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
    ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 16);
    code = NEON_BINARY (0xf2900810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 3) {
      orc_neon_emit_binary_long (p, "vmull.u16",0xf3900c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 16);
      code = NEON_BINARY (0xf2900810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_mulhsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "smull", 0x0e60c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 3) {
      orc_neon64_emit_binary (p, "smull", 0x0e60c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f108400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 3) {
      orc_neon64_emit_unary (p, "shrn", 0x0f108400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.s16",0xf2900c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
    ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 16);
    code = NEON_BINARY (0xf2900810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 3) {
      orc_neon_emit_binary_long (p, "vmull.s16",0xf2900c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i32 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 16);
      code = NEON_BINARY (0xf2900810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_mulhul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "umull", 0x2ea0c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 2) {
      orc_neon64_emit_binary (p, "umull", 0x2ea0c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f208400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 2) {
      orc_neon64_emit_unary (p, "shrn", 0x0f208400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.u32",0xf3a00c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
    ORC_ASM_CODE(p,"  vshrn.i64 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 32);
    code = NEON_BINARY (0xf2a00810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 2) {
      orc_neon_emit_binary_long (p, "vmull.u32",0xf3a00c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i64 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 32);
      code = NEON_BINARY (0xf2a00810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_mulhsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  unsigned int code;

  if (p->is_64bit) {
    OrcVariable tmpreg1 = { .alloc = p->tmpreg, .size = p->vars[insn->dest_args[0]].size };
    OrcVariable tmpreg2 = { .alloc = p->tmpreg2, .size = p->vars[insn->dest_args[0]].size };
    orc_neon64_emit_binary (p, "smull", 0x0ea0c000,
        tmpreg1,
        p->vars[insn->src_args[0]],
        p->vars[insn->src_args[1]],
	p->insn_shift);
    if (p->insn_shift == 2) {
      orc_neon64_emit_binary (p, "smull", 0x0ea0c000,
          tmpreg2,
          p->vars[insn->src_args[0]],
          p->vars[insn->src_args[1]],
	  p->insn_shift - 1);
    }
    /*
     * WARNING:
     *   Be careful here, SHRN (without Q-bit set) will write bottom 64 bits
     *   of the $dest register with data and top 64 bits with zeroes! SHRN2
     *   (with Q-bit set) will write top 64 bits of $dest register with data
     *   and will retain bottom 64 bits content. If $dest==$src{1 or 2}, then
     *   using SHRN will lead to corruption of source data!
     */
    orc_neon64_emit_unary (p, "shrn", 0x0f208400,
        p->vars[insn->dest_args[0]],
        tmpreg1, p->insn_shift);
    if (p->insn_shift == 2) {
      orc_neon64_emit_unary (p, "shrn", 0x0f208400,
          p->vars[insn->dest_args[0]],
          tmpreg2, p->insn_shift - 1);
    }
  } else {
    orc_neon_emit_binary_long (p, "vmull.s32",0xf2a00c00,
        p->tmpreg,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
    ORC_ASM_CODE(p,"  vshrn.i64 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name_quad (p->tmpreg), 32);
    code = NEON_BINARY (0xf2a00810,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg, 0);
    orc_arm_emit (p, code);

    if (p->insn_shift == 2) {
      orc_neon_emit_binary_long (p, "vmull.s32",0xf2a00c00,
          p->tmpreg,
          p->vars[insn->src_args[0]].alloc + 1,
          p->vars[insn->src_args[1]].alloc + 1);
      ORC_ASM_CODE(p,"  vshrn.i64 %s, %s, #%d\n",
          orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc + 1),
          orc_neon_reg_name_quad (p->tmpreg), 32);
      code = NEON_BINARY (0xf2a00810,
          p->vars[insn->dest_args[0]].alloc + 1,
          p->tmpreg, 0);
      orc_arm_emit (p, code);
    }
  }
}

static void
orc_neon_rule_splitql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int dest0 = p->vars[insn->dest_args[0]].alloc;
  int dest1 = p->vars[insn->dest_args[1]].alloc;
  int src = p->vars[insn->src_args[0]].alloc;

  if (p->is_64bit) {
    if (src != dest0) {
      orc_neon64_emit_binary (p, "uzp2", 0x0e805800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 1));
      orc_neon64_emit_binary (p, "uzp1", 0x0e801800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 1));
    } else {
      orc_neon64_emit_binary (p, "uzp1", 0x0e801800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 1));
      orc_neon64_emit_binary (p, "uzp2", 0x0e805800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 1));
    }
  } else {
    if (p->insn_shift < 1) {
      if (src != dest0) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary (p, "vtrn.32", 0xf3ba0080, dest1, dest0);
    } else {
      if (src != dest0) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary_quad (p, "vuzp.32", 0xf3ba0140, dest1, dest0);
    }
  }
}

static void
orc_neon_rule_splitlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int dest0 = p->vars[insn->dest_args[0]].alloc;
  int dest1 = p->vars[insn->dest_args[1]].alloc;
  int src = p->vars[insn->src_args[0]].alloc;

  if (p->is_64bit) {
    if (src != dest0) {
      orc_neon64_emit_binary (p, "uzp2", 0x0e405800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
      orc_neon64_emit_binary (p, "uzp1", 0x0e401800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
    } else {
      orc_neon64_emit_binary (p, "uzp1", 0x0e401800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
      orc_neon64_emit_binary (p, "uzp2", 0x0e405800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
    }
  } else {
    if (p->insn_shift < 2) {
      if (src != dest0) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary (p, "vuzp.16", 0xf3b60100, dest1, dest0);
    } else {
      if (src != dest0) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary_quad (p, "vuzp.16", 0xf3b60140, dest1, dest0);
    }
  }
}

static void
orc_neon_rule_splitwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int dest0 = p->vars[insn->dest_args[0]].alloc;
  int dest1 = p->vars[insn->dest_args[1]].alloc;
  int src = p->vars[insn->src_args[0]].alloc;

  if (p->is_64bit) {
    if (src != dest0) {
      orc_neon64_emit_binary (p, "uzp2", 0x0e005800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
      orc_neon64_emit_binary (p, "uzp1", 0x0e001800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
    } else {
      orc_neon64_emit_binary (p, "uzp1", 0x0e001800,
          p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
      orc_neon64_emit_binary (p, "uzp2", 0x0e005800,
          p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]],
          p->vars[insn->src_args[0]], p->insn_shift - (p->insn_shift >= 2));
    }
  } else {
    if (p->insn_shift < 2) {
      if (src != dest0) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary (p, "vuzp.8", 0xf3b20100, dest1, dest0);
    } else {
      if (src != dest0) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[0]], p->vars[insn->src_args[0]]);
      }
      if (src != dest1) {
        orc_neon_emit_mov_quad (p, p->vars[insn->dest_args[1]], p->vars[insn->src_args[0]]);
      }
      orc_neon_emit_unary_quad (p, "vuzp.8", 0xf3b20140, dest1, dest0);
    }
  }
}

static void
orc_neon_rule_div255w (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const OrcVariable dest = p->vars[insn->dest_args[0]];
  const OrcVariable src = p->vars[insn->src_args[0]];
  OrcVariable tmp1 = { .alloc = p->tmpreg2, .size = src.size * 2 };
  OrcVariable tmp2 = { .alloc = p->tmpreg, .size = src.size };
  orc_neon_emit_loadiw (p, &tmp2, 0x8081);

  if (p->is_64bit) {
    {
      // Unfortunately, this opcode requires quad for the destination
      // so we can't use orc_neon64_emit_binary
      ORC_ASM_CODE (p, "  %s %s, %s, %s\n", "umull",
          orc_neon64_reg_name_vector (tmp1.alloc, tmp1.size, 1),
          orc_neon64_reg_name_vector (src.alloc, src.size, 0),
          orc_neon64_reg_name_vector (tmp2.alloc, tmp2.size, 0));
      int code = 0x2e60c000;
      code |= (tmp2.alloc & 0x1f) << 16;
      code |= (src.alloc & 0x1f) << 5;
      code |= (tmp1.alloc & 0x1f);
      orc_arm_emit (p, code);
    }
    {
      // vreinterpret dest here, it'll be fixed by uzp2
      const OrcVariable dest_i32 = { .alloc = tmp2.alloc, .size = dest.size * 2 };
      orc_neon64_emit_binary (p, "umull2", 0x2e60c000, dest_i32, src, tmp2,
          p->insn_shift - 1);
    }
    {
      // vreinterpret src_2, it needs full quad width
      const OrcVariable tmp1_i64 = {.alloc = tmp1.alloc, .size = dest.size };
      const OrcVariable tmp2_i64 = {.alloc = tmp2.alloc, .size = dest.size };
      const OrcVariable dest_i64 = { .alloc = dest.alloc, .size = dest.size };
      orc_neon64_emit_binary (p, "uzp2", 0x0e405800, dest_i64, tmp1_i64, tmp2_i64,
          p->insn_shift - 1);
    }
    {
      ORC_ASM_CODE (p, "  %s %s, %s, #%d\n", immshift_info[5].name64,
          orc_neon64_reg_name_vector (dest.alloc, dest.size, 1),
          orc_neon64_reg_name_vector (dest.alloc, dest.size, 1), 7);
      int code = immshift_info[5].code64 | (1U << 30);
      code |= (dest.alloc & 0x1f) << 0;
      code |= (src.alloc & 0x1f) << 5;
      code |= ((immshift_info[5].bits - 7U) << 16);
      orc_arm_emit (p, code);
    }
  } else {
    // Multiply low
    orc_neon_emit_binary_long (p, "vmull.u16", 0xf3900c00, tmp1.alloc, src.alloc,
                              tmp2.alloc);
    // Multiply high
    orc_neon_emit_binary_long (p, "vmull.u16", 0xf3900c00, dest.alloc,
                              src.alloc + 1, tmp2.alloc);
    orc_neon_emit_unary_quad (p, "vuzp.16", 0xf3b60100, tmp1.alloc, dest.alloc);
    orc_neon_emit_shift (p, 5, &dest, &dest, 7, 1);
  }
}

void
orc_compiler_neon_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#define REG(x) \
    orc_rule_register (rule_set, #x , orc_neon_rule_ ## x, NULL)

  REG(absb);
  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  /* REG(andnb); */
  REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(copyb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  REG(mullb);
  REG(mulhsb);
  REG(mulhub);
  REG(orb);
  /* REG(shlb); */
  /* REG(shrsb); */
  /* REG(shrub); */
  REG(signb);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(absw);
  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  /* REG(andnw); */
  REG(avgsw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(copyw);
  REG(maxsw);
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);
  REG(orw);
  /* REG(shlw); */
  /* REG(shrsw); */
  /* REG(shruw); */
  REG(signw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(absl);
  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  /* REG(andnl); */
  REG(avgsl);
  REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(copyl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  REG(mulhsl);
  REG(mulhul);
  REG(orl);
  /* REG(shll); */
  /* REG(shrsl); */
  /* REG(shrul); */
  REG(signl);
  REG(subl);
  REG(subssl);
  REG(subusl);
  REG(xorl);

  REG(addq);
  REG(andq);
  REG(orq);
  REG(copyq);
  REG(subq);
  REG(xorq);

  REG(convsbw);
  REG(convubw);
  REG(convswl);
  REG(convuwl);
  REG(convslq);
  REG(convulq);
  REG(convlw);
  REG(convql);
  REG(convssslw);
  REG(convsuslw);
  REG(convuuslw);
  REG(convsssql);
  REG(convsusql);
  REG(convuusql);
  REG(convwb);
  REG(convhwb);
  REG(convhlw);
  REG(convssswb);
  REG(convsuswb);
  REG(convuuswb);

  REG(mulsbw);
  REG(mulubw);
  REG(mulswl);
  REG(muluwl);

  REG(accw);
  REG(accl);
  REG(accsadubl);
  REG(swapw);
  REG(swapl);
  REG(swapq);
  REG(swapwl);
  REG(swaplq);
  REG(select0wb);
  REG(select1wb);
  REG(select0lw);
  REG(select1lw);
  REG(select0ql);
  REG(select1ql);
  REG(mergebw);
  REG(mergewl);
  REG(mergelq);
  REG(splitql);
  REG(splitlw);
  REG(splitwb);

  REG(addf);
  REG(subf);
  REG(mulf);
  REG(divf);
  REG(sqrtf);
  REG(maxf);
  REG(minf);
  REG(cmpeqf);
  /* REG(cmpltf); */
  /* REG(cmplef); */
  REG(convfl);
  REG(convlf);

  REG(addd);
  REG(subd);
  REG(muld);
  REG(divd);
  REG(sqrtd);
  /* REG(cmpeqd); */
  REG(convdf);
  REG(convfd);

  REG(splatbw);
  REG(splatbl);
  REG(splatw3q);
  REG(div255w);

  orc_rule_register (rule_set, "loadpb", neon_rule_loadpX, (void *)1);
  orc_rule_register (rule_set, "loadpw", neon_rule_loadpX, (void *)2);
  orc_rule_register (rule_set, "loadpl", neon_rule_loadpX, (void *)4);
  orc_rule_register (rule_set, "loadpq", neon_rule_loadpX, (void *)8);
  orc_rule_register (rule_set, "loadupdb", neon_rule_loadupdb, (void *)0);
  orc_rule_register (rule_set, "loadb", neon_rule_loadX, (void *)0);
  orc_rule_register (rule_set, "loadw", neon_rule_loadX, (void *)0);
  orc_rule_register (rule_set, "loadl", neon_rule_loadX, (void *)0);
  orc_rule_register (rule_set, "loadq", neon_rule_loadX, (void *)0);
  orc_rule_register (rule_set, "loadoffb", neon_rule_loadX, (void *)1);
  orc_rule_register (rule_set, "loadoffw", neon_rule_loadX, (void *)1);
  orc_rule_register (rule_set, "loadoffl", neon_rule_loadX, (void *)1);
  orc_rule_register (rule_set, "storeb", neon_rule_storeX, (void *)0);
  orc_rule_register (rule_set, "storew", neon_rule_storeX, (void *)0);
  orc_rule_register (rule_set, "storel", neon_rule_storeX, (void *)0);
  orc_rule_register (rule_set, "storeq", neon_rule_storeX, (void *)0);

  orc_rule_register (rule_set, "shlb", orc_neon_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shrsb", orc_neon_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrub", orc_neon_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shlw", orc_neon_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrsw", orc_neon_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shruw", orc_neon_rule_shift, (void *)5);
  orc_rule_register (rule_set, "shll", orc_neon_rule_shift, (void *)6);
  orc_rule_register (rule_set, "shrsl", orc_neon_rule_shift, (void *)7);
  orc_rule_register (rule_set, "shrul", orc_neon_rule_shift, (void *)8);

  orc_rule_register (rule_set, "andnb", orc_neon_rule_andn, (void *)3);
  orc_rule_register (rule_set, "andnw", orc_neon_rule_andn, (void *)2);
  orc_rule_register (rule_set, "andnl", orc_neon_rule_andn, (void *)1);
  orc_rule_register (rule_set, "andnq", orc_neon_rule_andn, (void *)0);
}

