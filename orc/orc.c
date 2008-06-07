
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


void
orc_init (void)
{
  orc_opcode_init();
  orc_x86_init();
  orc_powerpc_init();
  orc_c_init();
}

