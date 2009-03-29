
#ifndef _ORC_TEST_TEST_H_
#define _ORC_TEST_TEST_H_

#include <orc/orc.h>
#include <orc/orcutils.h>

ORC_BEGIN_DECLS

void orc_test_init (void);
int orc_test_gcc_compile (OrcProgram *p);
void orc_test_random_bits (void *data, int n_bytes);
int orc_test_compare_output (OrcProgram *program);


ORC_END_DECLS

#endif

