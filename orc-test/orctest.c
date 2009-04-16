
#include <orc-test/orctest.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void
orc_test_init (void)
{
  orc_init ();

  setvbuf (stdout, NULL, _IONBF, 0);
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


void
orc_test_random_bits (void *data, int n_bytes)
{
  uint8_t *d = data;
  int i;
  for(i=0;i<n_bytes;i++){
    d[i] = rand();
  }
}

static int
print_array_val (void *array, int size, int i)
{
  switch (size) {
    case 1:
      {
        int8_t *a = array;
        printf(" %4d", a[i]);
        return a[i];
      }
      break;
    case 2:
      {
        int16_t *a = array;
        printf(" %5d", a[i]);
        return a[i];
      }
      break;
    case 4:
      {
        int32_t *a = array;
        printf(" %10d", a[i]);
        return a[i];
      }
      break;
    default:
      return -1;
  }
}

int
orc_test_compare_output (OrcProgram *program)
{
  OrcExecutor *ex;
  int ret;
  int n = 64;
  int dest_index;
  void *dest_exec;
  void *dest_emul;
  int i;

  ret = orc_program_compile (program);
  if (!ret) {
    return TRUE;
  }

  ex = orc_executor_new (program);
  orc_executor_set_n (ex, n);

  dest_index = -1;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name == NULL) continue;

    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      uint8_t *data;
      data = malloc(n*program->vars[i].size);
      orc_test_random_bits (data, n*program->vars[i].size);
      orc_executor_set_array (ex, i, data);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      dest_index = i;
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_PARAM) {
      orc_executor_set_parameter (ex, i, 2);
    }
  }
  if (dest_index == -1) {
    return FALSE;
  }

  dest_exec = malloc(n*program->vars[dest_index].size);
  memset (dest_exec, 0xa5, n*program->vars[dest_index].size);
  dest_emul = malloc(n*program->vars[dest_index].size);
  memset (dest_emul, 0xa5, n*program->vars[dest_index].size);

  orc_executor_set_array (ex, dest_index, dest_exec);
  orc_executor_run (ex);

  orc_executor_set_array (ex, dest_index, dest_emul);
  orc_executor_emulate (ex);

  if (memcmp (dest_exec, dest_emul, n*program->vars[dest_index].size) != 0) {
    for(i=0;i<n;i++){
      int a,b;
      int j;

      printf("%2d:", i);

      for(j=0;j<ORC_N_VARIABLES;j++){
        if (program->vars[j].name == NULL) continue;
        if (program->vars[j].vartype == ORC_VAR_TYPE_SRC &&
            program->vars[j].size > 0) {
          print_array_val (ex->arrays[j], program->vars[j].size, i);
        }
      }

      printf(" ->");
      a = print_array_val (dest_emul, program->vars[dest_index].size, i);
      b = print_array_val (dest_exec, program->vars[dest_index].size, i);

      if (a != b) {
        printf(" *");
      }

      printf("\n");
    }
#if 0
    switch (program->vars[dest_index].size) {
      case 1:
        {
          uint8_t *a = dest_emul;
          uint8_t *b = dest_exec;
          for(i=0;i<n;i++){
            printf("%d: %d %d %c\n", i, a[i], b[i], (a[i]==b[i])?' ':'*');
          }
        }
        break;
      case 2:
        {
          uint16_t *a = dest_emul;
          uint16_t *b = dest_exec;
          for(i=0;i<n;i++){
            printf("%d: %d %d %c\n", i, a[i], b[i], (a[i]==b[i])?' ':'*');
          }
        }
        break;
      case 4:
        {
          uint32_t *a = dest_emul;
          uint32_t *b = dest_exec;
          for(i=0;i<n;i++){
            printf("%d: %d %d %c\n", i, a[i], b[i], (a[i]==b[i])?' ':'*');
          }
        }
        break;
      default:
        return FALSE;
    }
#endif

    printf("%s", orc_program_get_asm_code (program));

    return FALSE;
  }

  orc_executor_free (ex);

  return TRUE;
}


