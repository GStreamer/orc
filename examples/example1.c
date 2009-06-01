

#include <stdio.h>

#include <orc/orcprogram.h>

#define N 10

int16_t a[N];
int16_t b[N];
int16_t c[N];

void add_s16(int16_t *dest, int16_t *src1, int16_t *src2, int n);


int
main (int argc, char *argv[])
{
  int i;

  /* orc_init() must be called before any other Orc function */
  orc_init ();

  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    a[i] = i;
    b[i] = 100;
  }

  /* Call a function that uses Orc */
  add_s16 (c, a, b, N);

  /* Print the results */
  for(i=0;i<N;i++){
    printf("%d: %d %d -> %d\n", i, a[i], b[i], c[i]);
  }

  return 0;
}

void
add_s16(int16_t *dest, int16_t *src1, int16_t *src2, int n)
{
  static OrcProgram *p = NULL;
  OrcExecutor _ex;
  OrcExecutor *ex = &_ex;

  if (p == NULL) {
    /* First time through, create the program */

    /* Create a new program with two sources and one destination.
     * Size of the members of each array is 2.  */
    p = orc_program_new_dss (2, 2, 2);

    /* Append an instruction to add the 2-byte values s1 and s2 (which
     * are the source arrays created above), and place the result in
     * the destination array d1, which was also create above.  */
    orc_program_append_str (p, "addw", "d1", "s1", "s2");

    /* Compile the program.  Ignore the very important result. */
    orc_program_compile (p);
  }

  /* Set the values on the executor structure */
  orc_executor_set_program (ex, p);
  orc_executor_set_n (ex, n);
  orc_executor_set_array_str (ex, "s1", src1);
  orc_executor_set_array_str (ex, "s2", src2);
  orc_executor_set_array_str (ex, "d1", dest);

  /* Run the program.  This calls the code that was generated above,
   * or, if the compilation failed, will emulate the program. */
  orc_executor_run (ex);
}

