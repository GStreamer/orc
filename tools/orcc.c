
#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>

static char * read_file (const char *filename);

int
main (int argc, char *argv[])
{
  char *code;
  int n;
  int i;
  OrcProgram **programs;

  orc_init ();
  orc_test_init ();

  code = read_file ("test.orc");

  n = orc_parse (code, &programs);

#if 1
  for(i=0;i<n;i++){
    printf("%s\n", programs[i]->name);
    orc_test_gcc_compile (programs[i]);
  }
#endif
#if 0
  for(i=0;i<n;i++){
    orc_program_compile_for_target (programs[i], orc_target_get_by_name("c"));
    printf("%s", orc_program_get_asm_code (programs[i]));
  }
#endif

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

