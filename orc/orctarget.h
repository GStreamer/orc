
#ifndef _ORC_TARGET_H_
#define _ORC_TARGET_H_

#include <orc/orclimits.h>
#include <orc/orcrule.h>

ORC_BEGIN_DECLS


enum {
  ORC_TARGET_C_C99 = (1<<0),
  ORC_TARGET_C_BARE = (1<<1),
  ORC_TARGET_C_NOEXEC = (1<<2),
  ORC_TARGET_C_OPCODE = (1<<3),
  ORC_TARGET_CLEAN_COMPILE = (1<<29),
  ORC_TARGET_FAST_NAN = (1<<30),
  ORC_TARGET_FAST_DENORMAL = (1<<31)
};

typedef enum {
  ORC_TARGET_POWERPC_64BIT = (1<<0),
  ORC_TARGET_POWERPC_LE = (1<<1),
  ORC_TARGET_POWERPC_ALTIVEC = (1<<2),
  ORC_TARGET_POWERPC_VSX = (1<<3),
  ORC_TARGET_POWERPC_V207 = (1<<4)
} OrcTargetPowerPCFlags;

typedef enum {
  ORC_TARGET_RISCV_64BIT = (1<<0),
  ORC_TARGET_RISCV_C = (1<<1),
  ORC_TARGET_RISCV_V = (1<<2),
  ORC_TARGET_RISCV_ZVKB = (1<<3),
  ORC_TARGET_RISCV_ZVBB = (1<<4) | ORC_TARGET_RISCV_ZVKB,
  ORC_TARGET_RISCV_ZVKN = (1<<5) | ORC_TARGET_RISCV_ZVKB,
  ORC_TARGET_RISCV_ZVKS = (1<<6) | ORC_TARGET_RISCV_ZVKB,
} OrcTargetRiscvFlags;

enum {
  ORC_TARGET_NEON_CLEAN_COMPILE = (1<<0),
  ORC_TARGET_NEON_NEON = (1<<1),
  ORC_TARGET_NEON_EDSP = (1<<2),
  ORC_TARGET_NEON_64BIT = (1<<3)
};

enum {
  ORC_TARGET_ARM_EDSP = (1<<2),
  ORC_TARGET_ARM_ARM6 = (1<<3)
};

typedef enum {
  ORC_TARGET_MMX_MMX = (1<<0),
  ORC_TARGET_MMX_MMXEXT = (1<<1),
  ORC_TARGET_MMX_3DNOW = (1<<2),
  ORC_TARGET_MMX_3DNOWEXT = (1<<3),
  ORC_TARGET_MMX_SSSE3 = (1<<4),
  ORC_TARGET_MMX_SSE4_1 = (1<<5),
  ORC_TARGET_MMX_SSE4_2 = (1<<6),
  ORC_TARGET_MMX_FRAME_POINTER = (1<<7),
  ORC_TARGET_MMX_SHORT_JUMPS = (1<<8),
  ORC_TARGET_MMX_64BIT = (1<<9),
  ORC_TARGET_MMX_SSE2 = (1<<10),
  ORC_TARGET_MMX_SSE3 = (1<<11),
} OrcTargetMMXFlags;

typedef enum {
  ORC_TARGET_SSE_SSE2 = (1<<0),
  ORC_TARGET_SSE_SSE3 = (1<<1),
  ORC_TARGET_SSE_SSSE3 = (1<<2),
  ORC_TARGET_SSE_SSE4_1 = (1<<3),
  ORC_TARGET_SSE_SSE4_2 = (1<<4),
  ORC_TARGET_SSE_SSE4A = (1<<5),
  ORC_TARGET_SSE_SSE5 = (1<<6),
  ORC_TARGET_SSE_FRAME_POINTER = (1<<7),
  ORC_TARGET_SSE_SHORT_JUMPS = (1<<8),
  ORC_TARGET_SSE_64BIT = (1<<9),
  ORC_TARGET_SSE_SSE = (1<<12), /* 10 and 11 were used for AVX */
} OrcTargetSSEFlags;

typedef enum _OrcTargetAVXFlags {
  /* This was shared with SSE, keep the API */
  ORC_TARGET_AVX_FRAME_POINTER = (1<<7),
  ORC_TARGET_AVX_SHORT_JUMPS = (1<<8),
  ORC_TARGET_AVX_64BIT = (1<<9),
  ORC_TARGET_AVX_AVX = (1<<10),
  ORC_TARGET_AVX_AVX2 = (1<<11),
} OrcTargetAVXFlags;

typedef enum {
  ORC_TARGET_LOONGARCH_64BIT = (1<<0),
  ORC_TARGET_LOONGARCH_LSX = (1<<1),
  ORC_TARGET_LOONGARCH_FRAME_POINTER = (1<<2),
} OrcTargetLSXFlags;

typedef enum {
  ORC_TARGET_LOONGARCH_LASX = (1<<3),
} OrcTargetLASXFlags;


ORC_API OrcRule *    orc_target_get_rule (OrcTarget *target,
                                          OrcStaticOpcode *opcode,
                                          unsigned int target_flags);

ORC_API OrcTarget *  orc_target_get_default (void);
ORC_API unsigned int orc_target_get_default_flags (OrcTarget *target);
ORC_API const char * orc_target_get_name (OrcTarget *target);
ORC_API const char * orc_target_get_flag_name (OrcTarget *target, int shift);

ORC_API const char * orc_target_get_asm_preamble (const char *target);
ORC_API const char * orc_target_get_preamble (OrcTarget *target);
ORC_API const char * orc_target_c_get_typedefs (void);

ORC_API void         orc_target_register (OrcTarget *target);
ORC_API OrcTarget *  orc_target_get_by_name (const char *target_name);

#ifdef ORC_ENABLE_UNSTABLE_API
ORC_API OrcRuleSet * orc_target_add_rule_set (OrcTarget *target,
    OrcOpcodeSet *opcode_set, unsigned int required_flags);
#endif

ORC_END_DECLS

#endif

