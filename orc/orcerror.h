
#ifndef _ORC_ERROR_H_
#define _ORC_ERROR_H_

#include <orc/orcutils.h>
#include <stdlib.h>

ORC_BEGIN_DECLS

typedef struct _OrcError OrcError;

/**
 * OrcError:
 *
 * Structure that holds information about an error that occurs in the
 * Orc library.
 */
struct _OrcError {
  /*< private >*/
  char *message;
};

void orc_error_free (OrcError *error);

ORC_END_DECLS

#endif

