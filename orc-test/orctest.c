
#include "config.h"

#include <orc-test/orctest.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void
orc_test_init (void)
{
  orc_init ();

  setvbuf (stdout, NULL, _IONBF, 0);
}




OrcTestResult
orc_test_gcc_compile (OrcProgram *p)
{
  char cmd[200];
  int ret;
  FILE *file;
  OrcCompileResult result;

  result = orc_program_compile (p);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    return ORC_TEST_INDETERMINATE;
  }

  fflush (stdout);

  file = fopen ("tmp.s", "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen ("dump", "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

#if defined(HAVE_POWERPC)
  ret = system ("gcc -Wa,-mregnames -Wall -c tmp.s");
#else
  ret = system ("gcc -Wall -c tmp.s");
#endif
  if (ret != 0) {
    printf("gcc failed\n");
    return FALSE;
  }

  ret = system ("objdump -dr tmp.o >tmp.dis");
  if (ret != 0) {
    printf("objdump failed\n");
    return FALSE;
  }

  sprintf (cmd, "objcopy -I binary "
#ifdef HAVE_I386
      "-O elf32-i386 -B i386 "
#elif defined(HAVE_AMD64)
      "-O elf64-x86-64 -B i386 "
#elif defined(HAVE_POWERPC)
      "-O elf32-powerpc -B powerpc "
#else
      /* FIXME */
#endif
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
#if 1
  uint8_t *d = data;
  int i;
  for(i=0;i<n_bytes;i++){
    d[i] = rand();
  }
#endif
#if 0
  float *d = data;
  int i;
  for(i=0;i<n_bytes/4;i++){
    d[i] = ((rand() & 0xffff)-32768)*0.01;
  }
#endif
}

int
print_array_val_signed (void *array, int size, int i)
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
    case 8:
      {
        int64_t *a = array;
#ifdef HAVE_AMD64
        printf(" %20ld", a[i]);
#else
        printf(" %20lld", a[i]);
#endif
        return a[i];
      }
      break;
    default:
      return -1;
  }
}

int
print_array_val_unsigned (void *array, int size, int i)
{
  switch (size) {
    case 1:
      {
        uint8_t *a = array;
        printf(" %4u", a[i]);
        return a[i];
      }
      break;
    case 2:
      {
        uint16_t *a = array;
        printf(" %5u", a[i]);
        return a[i];
      }
      break;
    case 4:
      {
        uint32_t *a = array;
        printf(" %10u", a[i]);
        return a[i];
      }
      break;
    case 8:
      {
        uint64_t *a = array;
#ifdef HAVE_AMD64
        printf(" %20lu", a[i]);
#else
        printf(" %20llu", a[i]);
#endif
        return a[i];
      }
      break;
    default:
      return -1;
  }
}

int
print_array_val_hex (void *array, int size, int i)
{
  switch (size) {
    case 1:
      {
        uint8_t *a = array;
        printf(" %2x", a[i]);
        return a[i];
      }
      break;
    case 2:
      {
        uint16_t *a = array;
        printf(" %4x", a[i]);
        return a[i];
      }
      break;
    case 4:
      {
        uint32_t *a = array;
        printf(" %8x", a[i]);
        return a[i];
      }
      break;
    case 8:
      {
        uint64_t *a = array;
#ifdef HAVE_AMD64
        printf(" %20lx", a[i]);
#else
        printf(" %16llx", a[i]);
#endif
        return a[i];
      }
      break;
    default:
      return -1;
  }
}

float
print_array_val_float (void *array, int size, int i)
{
  switch (size) {
    case 4:
      {
        float *a = array;
        printf(" %g", a[i]);
        return a[i];
      }
      break;
    case 8:
      {
        double *a = array;
        printf(" %g", a[i]);
        return a[i];
      }
      break;
    default:
      printf(" ERROR");
      return -1;
  }
}

int delign_index = 1;

void *
alloc_array (int n, int size, void **m_ptr)
{
  unsigned char *ptr = malloc (n*size+256*2);
  memset (ptr, 0xa5, n*size+256*2);
  if (m_ptr) *m_ptr = ptr;

  delign_index++;
  delign_index &= 0xf;

  return (ptr + 256 + size*delign_index);
}

int
check_bounds (void *ptr, int n, int size)
{
  unsigned char *data = ptr;
  int i;

  for(i=0;i<100;i++){
    if (data[-1-i] != 0xa5) {
      ORC_ERROR("early bounds failure at %d", i);
      return FALSE;
    }
    if (data[n*size+i] != 0xa5) {
      ORC_ERROR("late bounds failure at %d", i);
      return FALSE;
    }
  }

  return TRUE;
}

OrcTestResult
orc_test_compare_output (OrcProgram *program)
{
  OrcExecutor *ex;
  int n = 64;
  int dest_index;
  void *dest_exec;
  void *dest_emul;
  void *ptr_exec;
  void *ptr_emul;
  int i;
  OrcCompileResult result;

  result = orc_program_compile (program);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    return ORC_TEST_INDETERMINATE;
  }

  ex = orc_executor_new (program);
  orc_executor_set_n (ex, n);

  dest_index = -1;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name == NULL) continue;

    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      uint8_t *data;
      data = alloc_array (n,program->vars[i].size, NULL);
      orc_test_random_bits (data, n*program->vars[i].size);
      orc_executor_set_array (ex, i, data);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      dest_index = i;
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_PARAM) {
      orc_executor_set_param (ex, i, 2);
    }
  }
  if (dest_index == -1) {
    return FALSE;
  }

  dest_exec = alloc_array (n, program->vars[dest_index].size, &ptr_exec);
  dest_emul = alloc_array (n, program->vars[dest_index].size, &ptr_emul);

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
          print_array_val_signed (ex->arrays[j], program->vars[j].size, i);
        }
      }

      printf(" ->");
      a = print_array_val_signed (dest_emul, program->vars[dest_index].size, i);
      b = print_array_val_signed (dest_exec, program->vars[dest_index].size, i);

      if (a != b) {
        printf(" *");
      }

      printf("\n");
    }

    printf("%s", orc_program_get_asm_code (program));

    return FALSE;
  }
  if (!check_bounds (dest_exec, n, program->vars[dest_index].size)) {
    printf("out of bounds failure\n");

    return FALSE;
  }

  free (ptr_exec);
  free (ptr_emul);
  orc_executor_free (ex);

  return TRUE;
}

OrcProgram *
orc_test_get_program_for_opcode (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    if (opcode->src_size[1] == 0) {
      p = orc_program_new_as (opcode->dest_size[0], opcode->src_size[0]);
    } else {
      p = orc_program_new_ass (opcode->dest_size[0], opcode->src_size[0],
          opcode->src_size[1]);
    }
  } else {
    if (opcode->src_size[1] == 0) {
      p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
    } else {
      p = orc_program_new_dss (opcode->dest_size[0], opcode->src_size[0],
          opcode->src_size[1]);
    }
  }

  sprintf(s, "test_%s", opcode->name);
  orc_program_set_name (p, s);

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_append_str (p, opcode->name, "a1", "s1", "s2");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  return p;
}

OrcProgram *
orc_test_get_program_for_opcode_const (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];

  if (opcode->src_size[1] == 0) {
    return NULL;
  }

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    p = orc_program_new_as (opcode->dest_size[0], opcode->src_size[0]);
  } else {
    p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  }
  orc_program_add_constant (p, opcode->src_size[1], 1, "c1");

  sprintf(s, "test_const_%s", opcode->name);
  orc_program_set_name (p, s);

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_append_str (p, opcode->name, "a1", "s1", "c1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "c1");
  }

  return p;
}

OrcProgram *
orc_test_get_program_for_opcode_param (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];

  if (opcode->src_size[1] == 0) {
    return NULL;
  }

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    p = orc_program_new_as (opcode->dest_size[0], opcode->src_size[0]);
  } else {
    p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  }
  orc_program_add_parameter (p, opcode->src_size[1], "p1");

  sprintf(s, "test_const_%s", opcode->name);
  orc_program_set_name (p, s);

  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_append_str (p, opcode->name, "a1", "s1", "p1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "p1");
  }

  return p;
}

