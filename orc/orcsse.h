
#ifndef _ORC_SSE_H_
#define _ORC_SSE_H_

#include <orc/orc.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcsseinsn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  X86_XMM0 = ORC_VEC_REG_BASE + 16,
  X86_XMM1,
  X86_XMM2,
  X86_XMM3,
  X86_XMM4,
  X86_XMM5,
  X86_XMM6,
  X86_XMM7,
  X86_XMM8,
  X86_XMM9,
  X86_XMM10,
  X86_XMM11,
  X86_XMM12,
  X86_XMM13,
  X86_XMM14,
  X86_XMM15
} OrcSSERegister;

ORC_API const char * orc_x86_get_regname_sse(int i);
#endif

ORC_API unsigned int orc_sse_get_cpu_flags (void);

ORC_END_DECLS

#endif

