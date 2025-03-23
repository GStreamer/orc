#ifndef _ORC_MMX_H_
#define _ORC_MMX_H_

#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcmmxinsn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  X86_MM0 = ORC_VEC_REG_BASE,
  X86_MM1,
  X86_MM2,
  X86_MM3,
  X86_MM4,
  X86_MM5,
  X86_MM6,
  X86_MM7
} OrcMMXRegister;

#define ORC_MMX_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

ORC_API
const char * orc_x86_get_regname_mmx(int i);
ORC_API
const char *
orc_mmx_get_regname_by_size (int reg, int size);

ORC_API
void orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);

ORC_API
void orc_x86_emit_mov_memindex_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned);

ORC_API
void orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);

ORC_API unsigned int orc_mmx_get_cpu_flags (void);

#endif

ORC_END_DECLS

#endif

