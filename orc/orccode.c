
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcutils-private.h>
#include <orc/orcinternal.h>

#ifdef _WIN64
#include <windows.h>
#endif

OrcCode *
orc_code_new (void)
{
  OrcCode *code;
  code = orc_malloc(sizeof(OrcCode));
  memset (code, 0, sizeof(OrcCode));
  return code;
}

void
orc_code_free (OrcCode *code)
{
  if (code->insns) {
    free (code->insns);
    code->insns = NULL;
  }
  if (code->vars) {
    free (code->vars);
    code->vars = NULL;
  }
  if (code->chunk) {
#if defined(_WIN64) && defined(ORC_SUPPORTS_BACKTRACE_FROM_JIT)
  DWORD64 dyn_base = 0;
  PRUNTIME_FUNCTION p =
      RtlLookupFunctionEntry((DWORD64)code->code, &dyn_base, NULL);
  if (p != NULL) {
    RtlDeleteFunctionTable((PRUNTIME_FUNCTION)((DWORD64)code->code | 0x3));
  }
#endif
    orc_code_chunk_free (code->chunk);
    code->chunk = NULL;
  }

  free (code);
}



