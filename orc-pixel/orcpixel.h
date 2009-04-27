
#ifndef _ORC_PIXEL_PIXEL_H_
#define _ORC_PIXEL_PIXEL_H_

#include <orc/orc.h>
#include <orc/orcutils.h>

ORC_BEGIN_DECLS

void orc_pixel_init (void);
int orc_pixel_gcc_compile (OrcProgram *p);
void orc_pixel_random_bits (void *data, int n_bytes);
int orc_pixel_compare_output (OrcProgram *program);


ORC_END_DECLS

#endif

