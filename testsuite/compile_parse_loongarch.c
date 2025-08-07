/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: jinbo, jinbo@loongson.cn
  Author: hecai yuan, yuanhecai@loongson.cn

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>

static char *read_file (const char *filename);
void output_code (OrcProgram * p, FILE * output);
void output_code_header (OrcProgram * p, FILE * output);
void output_code_test (OrcProgram * p, FILE * output);

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
    printf ("compile_parse_test <file.orc>\n");
    exit (1);
  }

  n = orc_parse (code, &programs);

  for (i = 0; i < n; i++) {
    OrcTestResult ret;
    ret = orc_test_gcc_compile_loongarch (programs[i]);
    if (ret == ORC_TEST_FAILED) {
      error = TRUE;
    }
  }

  if (error)
    return 1;
  return 0;
}


static char *
read_file (const char *filename)
{
  FILE *file = NULL;
  char *contents = NULL;
  long size;
  int ret;

  file = fopen (filename, "rb");
  if (file == NULL)
    return NULL;

  ret = fseek (file, 0, SEEK_END);
  if (ret < 0)
    goto bail;

  size = ftell (file);
  if (size < 0)
    goto bail;

  ret = fseek (file, 0, SEEK_SET);
  if (ret < 0)
    goto bail;

  contents = malloc (size + 1);
  if (contents == NULL)
    goto bail;

  ret = fread (contents, size, 1, file);
  if (ret < 0)
    goto bail;

  contents[size] = 0;

  fclose (file);

  return contents;
bail:
  /* something failed */
  if (file)
    fclose (file);
  if (contents)
    free (contents);

  return NULL;
}
