
#include "config.h"

#include <orc-test/orctest.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>


void
orc_test_init (void)
{
  orc_init ();

  setvbuf (stdout, NULL, _IONBF, 0);

  //srand (time(NULL));
}


OrcTestResult
orc_test_gcc_compile (OrcProgram *p)
{
  char cmd[200];
  char *base;
  char source_filename[100];
  char obj_filename[100];
  char dis_filename[100];
  char dump_filename[100];
  char dump_dis_filename[100];
  int ret;
  FILE *file;
  OrcCompileResult result;
  OrcTarget *target;
  unsigned int flags;

  base = "temp-orc-test";

  sprintf(source_filename, "%s-source.s", base);
  sprintf(obj_filename, "%s.o", base);
  sprintf(dis_filename, "%s-source.dis", base);
  sprintf(dump_filename, "%s-dump.bin", base);
  sprintf(dump_dis_filename, "%s-dump.dis", base);

  target = orc_target_get_default ();
  flags = orc_target_get_default_flags (target);
  if (strcmp (orc_target_get_name (target), "sse") == 0) {
    flags |= ORC_TARGET_SSE_SHORT_JUMPS;
  }

  result = orc_program_compile_full (p, target, flags);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    return ORC_TEST_INDETERMINATE;
  }

  fflush (stdout);

  file = fopen (source_filename, "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen (dump_filename, "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

#if defined(HAVE_POWERPC)
  sprintf (cmd, "gcc -Wa,-mregnames -Wall -c %s -o %s", source_filename,
      obj_filename);
#else
  sprintf (cmd, "gcc -Wall -c %s -o %s", source_filename,
      obj_filename);
#endif
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("gcc failed");
    printf("%s\n", orc_program_get_asm_code (p));
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, "objdump -dr %s >%s", obj_filename, dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("objdump failed");
    return ORC_TEST_FAILED;
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
      "--redefine-sym _binary_temp_orc_test_dump_bin_start=%s "
      "%s %s", p->name, dump_filename, obj_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, "objdump -Dr %s >%s", obj_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objdump failed\n");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, "diff -u %s %s", dis_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("diff failed\n");
    return ORC_TEST_FAILED;
  }

  remove (source_filename);
  remove (obj_filename);
  remove (dis_filename);
  remove (dump_filename);
  remove (dump_dis_filename);

  return ORC_TEST_OK;
}


#define PREFIX "/opt/arm-2008q3/bin/arm-none-linux-gnueabi-"

OrcTestResult
orc_test_gcc_compile_neon (OrcProgram *p)
{
  char cmd[300];
  char *base;
  char source_filename[100];
  char obj_filename[100];
  char dis_filename[100];
  char dump_filename[100];
  char dump_dis_filename[100];
  int ret;
  FILE *file;
  OrcCompileResult result;
  OrcTarget *target;
  unsigned int flags;

  base = "temp-orc-test";

  sprintf(source_filename, "%s-source.s", base);
  sprintf(obj_filename, "%s.o", base);
  sprintf(dis_filename, "%s-source.dis", base);
  sprintf(dump_filename, "%s-dump.bin", base);
  sprintf(dump_dis_filename, "%s-dump.dis", base);

  target = orc_target_get_by_name ("neon");
  flags = orc_target_get_default_flags (target);
  flags |= ORC_TARGET_NEON_CLEAN_COMPILE;

  result = orc_program_compile_full (p, target, flags);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    return ORC_TEST_INDETERMINATE;
  }

  fflush (stdout);

  file = fopen (source_filename, "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen (dump_filename, "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

  sprintf (cmd, PREFIX "gcc -march=armv6t2 -mcpu=cortex-a8 -mfpu=neon -Wall "
      "-c %s -o %s", source_filename, obj_filename);
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("gcc failed");
    printf("%s\n", orc_program_get_asm_code (p));
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, PREFIX "objdump -dr %s >%s", obj_filename, dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("objdump failed");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, PREFIX "objcopy -I binary "
      "-O elf32-littlearm -B arm "
      "--rename-section .data=.text "
      "--redefine-sym _binary_temp_orc_test_dump_bin_start=%s "
      "%s %s", p->name, dump_filename, obj_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, PREFIX "objdump -Dr %s >%s", obj_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objdump failed\n");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, "diff -u %s %s", dis_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("diff failed\n");
    return ORC_TEST_FAILED;
  }

  remove (source_filename);
  remove (obj_filename);
  remove (dis_filename);
  remove (dump_filename);
  remove (dump_dis_filename);

  return ORC_TEST_OK;
}

#define C64X_PREFIX "/opt/TI/cg6x_6_1_7/bin/"

OrcTestResult
orc_test_gcc_compile_c64x (OrcProgram *p)
{
  char cmd[300];
  char *base;
  char source_filename[100];
  char obj_filename[100];
  char dis_filename[100];
  char dump_filename[100];
  char dump_dis_filename[100];
  int ret;
  FILE *file;
  OrcCompileResult result;
  OrcTarget *target;
  unsigned int flags;

  base = "temp-orc-test";

  sprintf(source_filename, "%s-source.s", base);
  sprintf(obj_filename, "%s-source.obj", base);
  sprintf(dis_filename, "%s-source.dis", base);
  sprintf(dump_filename, "%s-dump.bin", base);
  sprintf(dump_dis_filename, "%s-dump.dis", base);

  target = orc_target_get_by_name ("c64x");
  flags = orc_target_get_default_flags (target);

  result = orc_program_compile_full (p, target, flags);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    return ORC_TEST_INDETERMINATE;
  }

  fflush (stdout);

  file = fopen (source_filename, "w");
  fprintf(file, "%s", orc_program_get_asm_code (p));
  fclose (file);

  file = fopen (dump_filename, "w");
  ret = fwrite(p->code, p->code_size, 1, file);
  fclose (file);

  sprintf (cmd, C64X_PREFIX "cl6x -mv=6400+ "
      "-c %s", source_filename);
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("compiler failed");
    //printf("%s\n", orc_program_get_asm_code (p));
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, C64X_PREFIX "dis6x %s >%s", obj_filename, dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    ORC_ERROR ("objdump failed");
    return ORC_TEST_FAILED;
  }

#if 0
  sprintf (cmd, C64X_PREFIX "objcopy -I binary "
      "-O elf32-littlearm -B arm "
      "--rename-section .data=.text "
      "--redefine-sym _binary_temp_orc_test_dump_bin_start=%s "
      "%s %s", p->name, dump_filename, obj_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objcopy failed\n");
    return ORC_TEST_FAILED;
  }
#endif

#if 0
  sprintf (cmd, C64X_PREFIX "dis6x %s >%s", dump_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("objdump failed\n");
    return ORC_TEST_FAILED;
  }

  sprintf (cmd, "diff -u %s %s", dis_filename, dump_dis_filename);
  ret = system (cmd);
  if (ret != 0) {
    printf("diff failed\n");
    return ORC_TEST_FAILED;
  }
#endif

  remove (source_filename);
  remove (obj_filename);
  remove (dis_filename);
  remove (dump_filename);
  remove (dump_dis_filename);

  return ORC_TEST_OK;
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

static OrcTestResult orc_test_compare_output_full (OrcProgram *program,
    int backup);

OrcTestResult
orc_test_compare_output (OrcProgram *program)
{
  return orc_test_compare_output_full (program, 0);
}

OrcTestResult
orc_test_compare_output_backup (OrcProgram *program)
{
  return orc_test_compare_output_full (program, 1);
}


OrcTestResult
orc_test_compare_output_full (OrcProgram *program, int backup)
{
  OrcExecutor *ex;
  int n = 64 + (rand()&0xf);
  void *dest_exec[4] = { NULL, NULL, NULL, NULL };
  void *dest_emul[4] = { NULL, NULL, NULL, NULL };
  void *ptr_exec[4];
  void *ptr_emul[4];
  int i;
  int k;
  int have_dest = FALSE;
  OrcCompileResult result;
  int have_acc = FALSE;
  int acc_exec = 0, acc_emul = 0;
  int ret = ORC_TEST_OK;

  if (!backup) {
    result = orc_program_compile (program);
    if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
      return ORC_TEST_INDETERMINATE;
    }
  }

  ex = orc_executor_new (program);
  orc_executor_set_n (ex, n);

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name == NULL) continue;

    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      uint8_t *data;
      data = alloc_array (n,program->vars[i].size, NULL);
      orc_test_random_bits (data, n*program->vars[i].size);
      orc_executor_set_array (ex, i, data);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      dest_exec[i] = alloc_array (n, program->vars[i].size, &ptr_exec[i]);
      dest_emul[i] = alloc_array (n, program->vars[i].size, &ptr_emul[i]);
      memset (dest_exec[i], 0xa5, n*program->vars[i].size);
      memset (dest_emul[i], 0xa5, n*program->vars[i].size);

      orc_executor_set_array (ex, i, dest_exec[i]);
      have_dest = TRUE;
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_PARAM) {
      orc_executor_set_param (ex, i, 2);
    }
  }

  orc_executor_run (ex);

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      orc_executor_set_array (ex, i, dest_emul[i]);
    }
    if (program->vars[i].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      acc_exec = ex->accumulators[0];
      have_acc = TRUE;
    }
  }
  orc_executor_emulate (ex);

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (program->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (memcmp (dest_exec[k], dest_emul[k], n*program->vars[k].size) != 0) {
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
          a = print_array_val_signed (dest_emul[k], program->vars[k].size, i);
          b = print_array_val_signed (dest_exec[k], program->vars[k].size, i);

          if (a != b) {
            printf(" *");
          }

          printf("\n");
        }

        ret = ORC_TEST_FAILED;
      }
      if (!check_bounds (dest_exec[k], n, program->vars[k].size)) {
        printf("out of bounds failure\n");

        ret = ORC_TEST_FAILED;
      }

      free (ptr_exec[k]);
      free (ptr_emul[k]);
    }
    if (program->vars[k].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      acc_emul = ex->accumulators[0];
    }
  }

  if (have_acc) {
    if (acc_emul != acc_exec) {
      for(i=0;i<n;i++){
        int j;

        printf("%2d:", i);

        for(j=0;j<ORC_N_VARIABLES;j++){
          if (program->vars[j].name == NULL) continue;
          if (program->vars[j].vartype == ORC_VAR_TYPE_SRC &&
              program->vars[j].size > 0) {
            print_array_val_signed (ex->arrays[j], program->vars[j].size, i);
          }
        }

        printf(" -> acc\n");
      }
      printf("acc %d %d\n", acc_emul, acc_exec);
      ret = ORC_TEST_FAILED;
    }
  }

  if (ret == ORC_TEST_FAILED) {
    printf("%s", orc_program_get_asm_code (program));
  }

  orc_executor_free (ex);

  return ret;
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

