
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <processenv.h>
#endif

#include <orc/orc.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcutils-private.h>
#include <orc/orconce.h>

#include "orcinternal.h"

/**
 * SECTION:orc
 * @title: Orc
 * @short_description: Library Initialization
 */

void _orc_debug_init(void);
void _orc_once_init(void);
void _orc_compiler_init(void);

/**
 * orc_init:
 * 
 * This function initializes the Orc library, and
 * should be called before using any other Orc function.
 * Subsequent calls to this function have no effect.
 */
void
orc_init (void)
{
  static int inited = FALSE;

  if (!inited) {
    orc_global_mutex_lock ();
    if (!inited) {
      /* Validate extensions for API/ABI compatibility */
      ORC_ASSERT(sizeof(OrcExecutor) == sizeof(OrcExecutorAlt));
      _orc_debug_init();
      _orc_compiler_init();
      orc_opcode_init();
      orc_c_init();
#ifdef ENABLE_TARGET_C64X
      orc_c64x_c_init();
#endif
#ifdef ENABLE_TARGET_MMX
      orc_mmx_init();
#endif
#ifdef ENABLE_TARGET_SSE
      orc_sse_init();
#endif
#ifdef ENABLE_TARGET_AVX
      orc_avx_init();
#endif
#ifdef ENABLE_TARGET_ALTIVEC
      orc_powerpc_init();
#endif
#ifdef ENABLE_TARGET_ARM
      orc_arm_init();
#endif
#ifdef ENABLE_TARGET_NEON
      orc_neon_init();
#endif
#ifdef ENABLE_TARGET_MIPS
      orc_mips_init();
#endif
#ifdef ENABLE_TARGET_RISCV
      orc_riscv_init();
#endif
#ifdef ENABLE_TARGET_LSX
      orc_lsx_init();
#endif
#ifdef ENABLE_TARGET_LASX
      orc_lasx_init();
#endif

      inited = TRUE;
    }
    orc_global_mutex_unlock ();
  }
}

/**
 * orc_version_string:
 *
 * Returns the orc version as a string. This will be either a triplet like
 * "0.4.25" or with an additional nano number like "0.4.25.1".
 *
 * Since: 0.4.25
 */
const char *
orc_version_string (void)
{
  return (const char *) VERSION;
}

/* getenv() is deprecated on Windows and always returns NULL on UWP */
#ifdef _WIN32
char*
_orc_getenv (const char *key)
{
  int len;
  char check[1], *value;

  /* Get the len */
  len = GetEnvironmentVariableA (key, check, 1);
  if (len == 0)
    /* env var is not set or is "" (empty string) */
    return NULL;

  /* max size of len is 32767, cannot overflow */
  value = orc_malloc (sizeof (value) * len);

  if (GetEnvironmentVariableA (key, value, len) != (len - 1)) {
    free (value);
    return NULL;
  }

  return value;
}
#else
char*
_orc_getenv (const char *key)
{
  char *value = getenv (key);

  if (value)
    value = strdup (value);

  return value;
}
#endif
