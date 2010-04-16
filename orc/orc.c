
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orc
 * @title: Orc
 * @short_description: Library Initialization
 */

void _orc_debug_init(void);
void _orc_once_init(void);

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
  static int _inited = 0;
  if (_inited) return;

  _inited = 1;

  ORC_ASSERT(sizeof(OrcExecutor) == sizeof(OrcExecutorAlt));

  _orc_debug_init();
  _orc_once_init();
  orc_opcode_init();
  orc_c_init();
  orc_c64x_c_init();
  orc_mmx_init();
  orc_sse_init();
  orc_powerpc_init();
  orc_arm_init();
  orc_neon_init();
}

