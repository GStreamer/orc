
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <orc-test/orctest.h>

#define PREFIX "/opt/arm-2008q3/bin/arm-none-linux-gnueabi-"

int error = FALSE;


OrcProgram *
get_program (int type)
{
  OrcProgram *p;
  int d1;

  switch (type) {
  case 0:
    p = orc_program_new ();
    d1 = orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_source (p, 2, "s3");
    orc_program_add_constant (p, 2, 2, "c1");
    orc_program_add_constant (p, 2, 2, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s2", "s3");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "addw", "d1", "s1", "t1");
    break;
  case 1:
    p = orc_program_new ();
    d1 = orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_source (p, 2, "s3");
    orc_program_add_constant (p, 2, 2, "c1");
    orc_program_add_constant (p, 2, 2, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s2", "s3");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "subw", "d1", "s1", "t1");
    break;
  case 2:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_source (p, 2, "s3");
#if 1
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_constant (p, 2, 1, "c2");
#else
    orc_program_add_constant (p, 2, 0x8000, "c1");
#endif
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");
    orc_program_add_temporary (p, 2, "t3");

#if 1
    orc_program_append_str (p, "addw", "t1", "s2", "s3");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
#else
    orc_program_append_str (p, "xorw", "t1", "c1", "s2");
    orc_program_append_str (p, "xorw", "t2", "c1", "s3");
    orc_program_append_str (p, "avguw", "t3", "t1", "t2");
    orc_program_append_str (p, "xorw", "t1", "c1", "t3");
#endif
    orc_program_append_str (p, "addw", "d1", "s1", "t1");
    break;
  case 3:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_source (p, 2, "s3");
#if 1
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_constant (p, 2, 1, "c2");
#else
    orc_program_add_constant (p, 2, 0x8000, "c1");
#endif
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");
    orc_program_add_temporary (p, 2, "t3");

#if 1
    orc_program_append_str (p, "addw", "t1", "s2", "s3");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
#else
    orc_program_append_str (p, "xorw", "t1", "c1", "s2");
    orc_program_append_str (p, "xorw", "t2", "c1", "s3");
    orc_program_append_str (p, "avguw", "t3", "t1", "t2");
    orc_program_append_str (p, "xorw", "t1", "c1", "t3");
#endif
    orc_program_append_str (p, "subw", "d1", "s1", "t1");
    break;
  case 4:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "c1");
    orc_program_append_str (p, "shrsw", "d1", "t1", "c1");
    break;
  case 5:
    p = orc_program_new_dss (2,2,2);

    orc_program_append_str (p, "addw", "d1", "s1", "s2");
    break;
  case 6:
    p = orc_program_new_ds (2,2);
    orc_program_add_constant (p, 2, 1, "c1");

    orc_program_append_str (p, "shlw", "d1", "s1", "c1");
    break;
  case 7:
    p = orc_program_new_ds (2,2);
    orc_program_add_constant (p, 2, 2, "c1");

    orc_program_append_str (p, "shlw", "d1", "s1", "c1");
    break;
  case 8:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_source (p, 2, "s3");
    orc_program_add_temporary (p, 4, "t1");
    orc_program_add_temporary (p, 2, "t2");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 4, "p2");
    orc_program_add_parameter (p, 2, "p3");

    orc_program_append_str (p, "addw", "t1", "s2", "s3");
    orc_program_append_str (p, "mulswl", "t1", "t1", "p1");
    orc_program_append_str (p, "addl", "t1", "t1", "p2");
    orc_program_append_str (p, "shll", "t1", "s1", "p3");
    orc_program_append_ds_str (p, "convlw", "t2", "t1");
    orc_program_append_str (p, "addl", "d1", "t2", "s1");
    break;
  case 9:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s20");
    orc_program_add_source (p, 2, "s21");
    orc_program_add_source (p, 2, "s22");
    orc_program_add_source (p, 2, "s23");
    orc_program_add_constant (p, 2, 9, "c1");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 2, "p2");
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");

    orc_program_append_str (p, "addw", "t1", "s21", "s22");
    orc_program_append_str (p, "mullw", "t1", "t1", "c1");
    orc_program_append_str (p, "addw", "t2", "s20", "s23");
    orc_program_append_str (p, "subw", "t1", "t1", "t2");
    orc_program_append_str (p, "addw", "t1", "t1", "p1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "p2");
    orc_program_append_str (p, "addw", "d1", "s1", "t1");
    break;
  case 10:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s20");
    orc_program_add_source (p, 2, "s21");
    orc_program_add_source (p, 2, "s22");
    orc_program_add_source (p, 2, "s23");
    orc_program_add_constant (p, 2, 9, "c1");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 2, "p2");
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");

    orc_program_append_str (p, "addw", "t1", "s21", "s22");
    orc_program_append_str (p, "mullw", "t1", "t1", "c1");
    orc_program_append_str (p, "addw", "t2", "s20", "s23");
    orc_program_append_str (p, "subw", "t1", "t1", "t2");
    orc_program_append_str (p, "addw", "t1", "t1", "p1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "p2");
    orc_program_append_str (p, "subw", "d1", "s1", "t1");
    break;
  case 11:
    p = orc_program_new_dss (2,2,2);

    orc_program_append_str (p, "subw", "d1", "s1", "s2");
    break;
  case 12:
    p = orc_program_new_ds (1,1);
    orc_program_append_ds_str (p, "copyb", "d1", "s1");
    break;
  case 13:
    p = orc_program_new_dss (2,2,1);
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s2");
    orc_program_append_str (p, "addw", "d1", "t1", "s1");
    break;
  case 14:
    p = orc_program_new_ds (2,1);

    orc_program_append_ds_str (p, "convubw", "d1", "s1");
    break;
  case 15:
    p = orc_program_new_ds (1,2);

    orc_program_append_ds_str (p, "convsuswb", "d1", "s1");
    break;
  case 16:
    p = orc_program_new_dss (2,2,1);
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s2");
    orc_program_append_str (p, "subw", "d1", "s1", "t1");
    break;
  case 17:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_source (p, 1, "s3");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s3");
    orc_program_append_str (p, "mullw", "t1", "t1", "s2");
    orc_program_append_str (p, "addw", "d1", "s1", "t1");
    break;
  default:
    return NULL;
  }

  return p;
}



void
test_program (int type)
{
  OrcProgram *p;
  char s[40];
  char cmd[200];
  int ret;
  FILE *file;

  p = get_program(type);

  sprintf(s, "test_schro_%d", type);
  orc_program_set_name (p, s);

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }
  return;

  ret = orc_program_compile_for_target (p, orc_target_get_by_name("neon"));
  if (!ret) {
    printf("/* %d failed to compile */\n", type);
    error = TRUE;
    goto out;
  }

  fflush (stdout);

  file = fopen ("tmp-test-schro.s", "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen ("dump", "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

  ret = system (PREFIX "gcc -mcpu=cortex-a8 -mfpu=neon -Wall -c tmp-test-schro.s");
  if (ret != 0) {
    printf("gcc failed\n");
    error = TRUE;
    goto out;
  }

  ret = system (PREFIX "objdump -dr tmp-test-schro.o >tmp-test-schro.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    error = TRUE;
    goto out;
  }

  sprintf (cmd, PREFIX "objcopy -I binary -O elf32-littlearm -B arm "
      "--rename-section .data=.text "
      "--redefine-sym _binary_dump_start=%s "
      "dump tmp-test-schro.o", s);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    error = TRUE;
    goto out;
  }

  ret = system (PREFIX "objdump -Dr tmp-test-schro.o >tmp-test-schro-dump.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    error = TRUE;
    goto out;
  }

  ret = system ("diff -u tmp-test-schro.dis tmp-test-schro-dump.dis");
  if (ret != 0) {
    printf("diff failed\n");
    error = TRUE;
    goto out;
  }

out:
  orc_program_free (p);
}


int
main (int argc, char *argv[])
{
  int i;

  orc_init();

  for(i=0;i<18;i++){
    printf("/* %d */\n", i);
    test_program (i);
  }

  if (error) return 1;
  return 0;
}

