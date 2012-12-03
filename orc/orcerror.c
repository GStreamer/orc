
#include "config.h"

#include <orc/orcerror.h>

void
orc_error_free (OrcError * error)
{
  if (error) {
    free (error->message);
    free (error);
  }
}
