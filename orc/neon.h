
#ifndef _ORC_NEON_H_
#define _ORC_NEON_H_

#include <orc/orc.h>
#include <orc/arm.h>

void neon_loadb (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void neon_loadw (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void neon_loadl (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void neon_neg (OrcCompiler *compiler, int dest);
void neon_storeb (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void neon_storew (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void neon_storel (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void neon_emit_loadib (OrcCompiler *p, int reg, int value);
void neon_emit_loadiw (OrcCompiler *p, int reg, int value);
void neon_emit_loadil (OrcCompiler *p, int reg, int value);
void neon_emit_loadpb (OrcCompiler *p, int reg, int param);
void neon_emit_loadpw (OrcCompiler *p, int reg, int param);
void neon_emit_loadpl (OrcCompiler *p, int reg, int param);


#endif

