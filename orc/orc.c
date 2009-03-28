
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


void
orc_init (void)
{
  static int _inited = 0;
  if (_inited) return;

  _inited = 1;

  orc_opcode_init();
  orc_c_init();
  orc_mmx_init();
  orc_sse_init();
  orc_powerpc_init();
  orc_arm_init();
}

