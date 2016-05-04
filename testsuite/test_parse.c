/* Test orc_parse() to make sure it handles both \r\n and \n line endings */
#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>

const char txt_unix[] = ".function orc_add_s16\n"
    ".dest 2 d1 orc_int16\n.source 2 s1 orc_int16\n.source 2 s2 orc_int16\n"
    "\naddw d1, s1, s2\n";

const char txt_win32[] = ".function orc_add_s16\r\n"
    ".dest 2 d1 orc_int16\r\n.source 2 s1 orc_int16\r\n.source 2 s2 orc_int16\r\n"
    "\r\naddw d1, s1, s2\r\n";

void output_code (OrcProgram * p, FILE * output);
void output_code_header (OrcProgram * p, FILE * output);
void output_code_test (OrcProgram * p, FILE * output);

int verbose = FALSE;
int error = FALSE;

int
main (int argc, char *argv[])
{
  OrcProgram **programs;
  OrcCompileResult cres;
  int n, i;

  orc_init ();
  orc_test_init ();

  /* 1 - unix */
  n = orc_parse (txt_unix, &programs);
  for (i = 0; i < n; i++) {
    if (verbose)
      printf ("%s\n", programs[i]->name);
    orc_test_compare_output_full (programs[i], 0);
    cres = orc_program_compile (programs[i]);
    if (ORC_COMPILE_RESULT_IS_FATAL (cres)) {
      fprintf (stderr, "compile error: %d\n", cres);
      error = TRUE;
    }
    orc_program_free (programs[i]);
  }

  if (error || n == 0)
    return 1;
 
  /* 2 - windows */
  n = orc_parse (txt_win32, &programs);
  for (i = 0; i < n; i++) {
    if (verbose)
      printf ("%s\n", programs[i]->name);
    orc_test_compare_output_full (programs[i], 0);
    cres = orc_program_compile (programs[i]);
    if (ORC_COMPILE_RESULT_IS_FATAL (cres)) {
      fprintf (stderr, "compile error: %d\n", cres);
      error = TRUE;
    }
    orc_program_free (programs[i]);
  }

  if (error || n == 0)
    return 1;

  return 0;
}
