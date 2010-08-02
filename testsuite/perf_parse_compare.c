
#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>

static char * read_file (const char *filename);
void output_code (OrcProgram *p, FILE *output);
void output_code_header (OrcProgram *p, FILE *output);
void output_code_test (OrcProgram *p, FILE *output);

int error = FALSE;

int
main (int argc, char *argv[])
{
  char *code;
  int n;
  int i;
  OrcProgram **programs;
  const char *filename = NULL;

  orc_init ();
  orc_test_init ();

  if (argc >= 2) {
    filename = argv[1];
  }
  if (filename == NULL) {
    filename = getenv ("testfile");
  }
  if (filename == NULL) {
    filename = "test.orc";
  }
  code = read_file (filename);
  if (!code) {
    printf("perf_parse <file.orc>\n");
    exit(1);
  }

  n = orc_parse (code, &programs);

  for(i=0;i<n;i++){
    double perf_mmx;
    double perf_sse;
    perf_mmx = orc_test_performance_full (programs[i], 0, "mmx");
    perf_sse = orc_test_performance_full (programs[i], 0, "sse");
    printf("%g %g\n", perf_mmx, perf_sse);
  }

  if (error) return 1;
  return 0;
}


static char *
read_file (const char *filename)
{
  FILE *file = NULL;
  char *contents = NULL;
  long size;
  int ret;

  file = fopen (filename, "r");
  if (file == NULL) return NULL;

  ret = fseek (file, 0, SEEK_END);
  if (ret < 0) goto bail;

  size = ftell (file);
  if (size < 0) goto bail;

  ret = fseek (file, 0, SEEK_SET);
  if (ret < 0) goto bail;

  contents = malloc (size + 1);
  if (contents == NULL) goto bail;

  ret = fread (contents, size, 1, file);
  if (ret < 0) goto bail;

  contents[size] = 0;

  return contents;
bail:
  /* something failed */
  if (file) fclose (file);
  if (contents) free (contents);

  return NULL;
}

