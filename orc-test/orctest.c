
#include <orc-test/orctest.h>

#include <stdio.h>
#include <stdlib.h>


void
orc_test_init (void)
{
  orc_init ();

}




int
orc_test_gcc_compile (OrcProgram *p)
{
  char cmd[200];
  int ret;
  FILE *file;

  ret = orc_program_compile (p);
  if (!ret) {
    return FALSE;
  }

  fflush (stdout);

  file = fopen ("tmp.s", "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen ("dump", "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

  ret = system ("gcc -Wall -c tmp.s");
  if (ret != 0) {
    printf("gcc failed\n");
    return FALSE;
  }

  ret = system ("objdump -dr tmp.o >tmp.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    return FALSE;
  }

  sprintf (cmd, "objcopy -I binary -O elf32-i386 -B i386 "
      "--rename-section .data=.text "
      "--redefine-sym _binary_dump_start=%s "
      "dump tmp.o", p->name);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    return FALSE;
  }

  ret = system ("objdump -Dr tmp.o >tmp-dump.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    return FALSE;
  }

  ret = system ("diff -u tmp.dis tmp-dump.dis");
  if (ret != 0) {
    printf("diff failed\n");
    return FALSE;
  }

  return TRUE;
}


