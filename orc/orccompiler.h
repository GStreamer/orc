
#ifndef _ORC_COMPILER_H_
#define _ORC_COMPILER_H_

#include <orc/orclimits.h>
#include <orc/orcexecutor.h>
#include <orc/orccode.h>
#include <orc/orctarget.h>
#include <orc/orcinstruction.h>
#include <orc/orcvariable.h>
#include <orc/orcconstant.h>

ORC_BEGIN_DECLS

#define ORC_ENABLE_ASM_CODE
#ifdef ORC_ENABLE_ASM_CODE
#define ORC_ASM_CODE(compiler,...) orc_compiler_append_code(compiler, __VA_ARGS__)
#else
#define ORC_ASM_CODE(compiler,...)
#endif


#define ORC_COMPILER_ERROR(compiler, fmt, ...) do { \
  orc_compiler_error (compiler, fmt, ##__VA_ARGS__); \
  ORC_WARNING (fmt, ##__VA_ARGS__); \
} while (0)

#if 0
/* FIXME in orcutils.h since it's needed in orccode.h */
typedef enum {
  ORC_COMPILE_RESULT_OK = 0,

  ORC_COMPILE_RESULT_UNKNOWN_COMPILE = 0x100,
  ORC_COMPILE_RESULT_MISSING_RULE = 0x101,

  ORC_COMPILE_RESULT_UNKNOWN_PARSE = 0x200,
  ORC_COMPILE_RESULT_PARSE = 0x201,
  ORC_COMPILE_RESULT_VARIABLE = 0x202

} OrcCompileResult;
#endif

#define ORC_COMPILE_RESULT_IS_SUCCESSFUL(x) ((x) < 0x100)
#define ORC_COMPILE_RESULT_IS_FATAL(x) ((x) >= 0x200)

ORC_API int orc_compiler_label_new (OrcCompiler *compiler);

ORC_API int orc_compiler_get_constant (OrcCompiler *compiler, int size, int value);

ORC_API int orc_compiler_get_constant_long (OrcCompiler *compiler, orc_uint32 a,
  orc_uint32 b, orc_uint32 c, orc_uint32 d);

ORC_API int orc_compiler_try_get_constant_long (OrcCompiler *compiler, orc_uint32 a,
  orc_uint32 b, orc_uint32 c, orc_uint32 d);

ORC_API int orc_compiler_get_temp_constant (OrcCompiler *compiler, int size, int value);

ORC_API int orc_compiler_get_temp_reg (OrcCompiler *compiler);

ORC_API int orc_compiler_get_constant_reg (OrcCompiler *compiler);

ORC_API void orc_compiler_error (OrcCompiler *compiler, const char *fmt, ...);

ORC_API void orc_compiler_append_code (OrcCompiler *p, const char *fmt, ...) ORC_GNU_PRINTF(2,3);
 
#ifdef ORC_ENABLE_UNSTABLE_API

/* We check if orcinternal.h was included before to avoid redefining
 * the following macros. You need to first include orccompiler.h and
 * then orcinternal.h, not the other way around
 */
#ifdef _ORC_INTERNAL_H_
#error "Include orccompiler.h before including orcinternal.h"
#endif

#define ORC_SRC_ARG(p,i,n) (orc_compiler_get_variable ((p), (i)->src_args[(n)])->alloc)
#define ORC_DEST_ARG(p,i,n) (orc_compiler_get_variable ((p), (i)->dest_args[(n)])->alloc)
#define ORC_SRC_TYPE(p,i,n) (orc_compiler_get_variable ((p), (i)->src_args[(n)])->vartype)
#define ORC_DEST_TYPE(p,i,n) (orc_compiler_get_variable ((p), (i)->dest_args[(n)])->vartype)
#define ORC_SRC_VAL(p,insn,n) (orc_compiler_get_variable ((p), (insn)->src_args[(n)])->value.i)
#define ORC_DEST_VAL(p,insn,n) (orc_compiler_get_variable ((p), (insn)->dest_args[(n)])->value.i)

ORC_API orc_bool orc_compiler_flag_check (const char *flag);
ORC_API OrcCompileResult orc_compiler_compile_program (OrcCompiler *compiler, OrcProgram *program, OrcTarget *target, unsigned int flags);

ORC_API void orc_compiler_release_temp_reg (OrcCompiler *compiler, int reg);
ORC_API void orc_compiler_reset_temp_regs (OrcCompiler *compiler, int start);
ORC_API int orc_compiler_get_temp_constant_full (OrcCompiler *c, OrcConstant *cnst);
ORC_API int orc_compiler_try_get_constant_full (OrcCompiler *c, OrcConstant *cnst);
ORC_API int orc_compiler_get_constant_full (OrcCompiler *c, OrcConstant *cnst);
ORC_API void orc_compiler_load_constant (OrcCompiler *c, int reg, OrcConstant *cnst);
ORC_API void  orc_compiler_load_constant_from_size_and_value (OrcCompiler *c,
    int reg, int size, orc_uint64 value);
ORC_API const OrcVariable * orc_compiler_get_variable (OrcCompiler *c, OrcVariableId idx);

#endif

ORC_END_DECLS

#endif

