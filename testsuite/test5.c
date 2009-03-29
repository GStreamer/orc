
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>


int error = FALSE;

void test_opcode (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s %d,%d,%d %p */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1],
        opcode_set->opcodes[i].emulate);
    test_opcode (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  char cmd[200];
  int ret;
  FILE *file;

  if (opcode->src_size[1] == 0) {
    p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  } else {
    p = orc_program_new_dss (opcode->dest_size[0], opcode->src_size[0],
        opcode->src_size[1]);
  }

  sprintf(s, "test_%s", opcode->name);
  orc_program_set_name (p, s);
  //orc_program_add_constant (p, 2, 1, "c1");

  orc_program_append_str (p, opcode->name, "d1", "s1", "s2");

  ret = orc_program_compile (p);
  if (!ret) {
    error = TRUE;
    goto out;
  }

  fflush (stdout);

  file = fopen ("tmp-test5.s", "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen ("dump", "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

  ret = system ("gcc -Wall -c tmp-test5.s");
  if (ret != 0) {
    printf("gcc failed\n");
    error = TRUE;
  }

  ret = system ("objdump -dr tmp-test5.o >tmp-test5.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    error = TRUE;
    goto out;
  }

  sprintf (cmd, "objcopy -I binary -O elf32-i386 -B i386 "
      "--rename-section .data=.text "
      "--redefine-sym _binary_dump_start=%s "
      "dump tmp-test5.o", s);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    error = TRUE;
    goto out;
  }

  ret = system ("objdump -Dr tmp-test5.o >tmp-test5-dump.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    error = TRUE;
    goto out;
  }

  ret = system ("diff -u tmp-test5.dis tmp-test5-dump.dis");
  if (ret != 0) {
    printf("diff failed\n");
    error = TRUE;
    goto out;
  }

out:
  orc_program_free (p);
}


