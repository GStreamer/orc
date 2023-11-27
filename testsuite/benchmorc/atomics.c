#include <stdio.h>
#include <orc/orc.h>
#include <orc-test/orcprofile.h>

#define TIMESTAMP "u:%02u:%02u.%09u"
#define IS_VALID_TIME(t) (orc_uint64)t != (orc_uint64)-1
#define SECOND (orc_int64)1000000000
#define TIME(t) IS_VALID_TIME (t) ? \
        (orc_uint32) (((orc_uint64)(t)) / (SECOND * 60 * 60)) : 99, \
        IS_VALID_TIME (t) ? \
        (orc_uint32) ((((orc_uint64)(t)) / (SECOND * 60)) % 60) : 99, \
        IS_VALID_TIME (t) ? \
        (orc_uint32) ((((orc_uint64)(t)) / SECOND) % 60) : 99, \
        IS_VALID_TIME (t) ? \
        (orc_uint32) (((orc_uint64)(t)) % SECOND) : 999999999

int
main (int argc, char ** argv)
{
  unsigned long start, end, diff;
  const orc_uint64 num_iter = 5000000000;
  OrcOnce once = ORC_ONCE_INIT;
  void *my_val = NULL;

  start = orc_profile_stamp();

  for (orc_uint64 i = 0; i < num_iter; i++) {
    if (!orc_once_enter (&once, &my_val)) {
      printf ("called\n");
      orc_once_leave (&once, (void*) (1));
    }
  }

  end = orc_profile_stamp ();

  diff = end - start;
  printf ("val %p, taken %" TIMESTAMP " (%lu)\n", (my_val), TIME(diff), diff);

  return 0;
}
