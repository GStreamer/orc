
#include "config.h"

#include <orc-test/orctest.h>
#include <orc-test/orcarray.h>
#include <orc-test/orcrandom.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


OrcRandom rand_context;

void
orc_test_init (void)
{
  orc_init ();

  setvbuf (stdout, NULL, _IONBF, 0);

  orc_random_init (&rand_context, 0x12345678);
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


int
print_array_val_signed (OrcArray *array, int i, int j)
{
  void *ptr = ORC_PTR_OFFSET (array->data,
      i*array->element_size + j*array->stride);

  switch (array->element_size) {
    case 1:
      printf(" %4d", *(int8_t *)ptr);
      return *(int8_t *)ptr;
    case 2:
      printf(" %5d", *(int16_t *)ptr);
      return *(int16_t *)ptr;
    case 4:
      printf(" %10d", *(int32_t *)ptr);
      return *(int32_t *)ptr;
    case 8:
#ifdef HAVE_AMD64
      printf(" %20ld", *(int64_t *)ptr);
#else
      printf(" %20lld", *(int64_t *)ptr);
#endif
      return *(int64_t *)ptr;
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

int
print_array_val_float (OrcArray *array, int i, int j)
{
  void *ptr = ORC_PTR_OFFSET (array->data,
      i*array->element_size + j*array->stride);

  switch (array->element_size) {
    case 4:
      if (isnan(*(float *)ptr)) {
        printf(" nan %08x", *(uint32_t *)ptr);
        /* This is to get around signaling/non-signaling nans in the output */
        return (*(uint32_t *)ptr) & 0xffbfffff;
      } else {
        printf(" %12.5g", *(float *)ptr);
        return *(int32_t *)ptr;
      }
    case 8:
      printf(" %12.5g", *(double *)ptr);
      return *(int64_t *)ptr;
    default:
      printf(" ERROR");
      return -1;
  }
}

static OrcTestResult orc_test_compare_output_full (OrcProgram *program,
    int flags);

OrcTestResult
orc_test_compare_output (OrcProgram *program)
{
  return orc_test_compare_output_full (program, 0);
}

OrcTestResult
orc_test_compare_output_backup (OrcProgram *program)
{
  return orc_test_compare_output_full (program, ORC_TEST_FLAGS_BACKUP);
}


OrcTestResult
orc_test_compare_output_full (OrcProgram *program, int flags)
{
  OrcExecutor *ex;
  int n = 64 + (orc_random(&rand_context)&0xf);
  int m;
  OrcArray *dest_exec[4] = { NULL, NULL, NULL, NULL };
  OrcArray *dest_emul[4] = { NULL, NULL, NULL, NULL };
  OrcArray *src[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
  int i;
  int j;
  int k;
  int have_dest = FALSE;
  OrcCompileResult result;
  int have_acc = FALSE;
  int acc_exec = 0, acc_emul = 0;
  int ret = ORC_TEST_OK;

  ORC_DEBUG ("got here");

flags |= ORC_TEST_FLAGS_FLOAT;
  if (!(flags & ORC_TEST_FLAGS_BACKUP)) {
    OrcTarget *target;
    unsigned int flags;

    target = orc_target_get_default ();
    flags = orc_target_get_default_flags (target);

    result = orc_program_compile_full (program, target, flags);
    if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
      return ORC_TEST_INDETERMINATE;
    }
  }

  ex = orc_executor_new (program);
  orc_executor_set_n (ex, n);
  if (program->is_2d) {
    m = 8 + (orc_random(&rand_context)&0xf);
  } else {
    m = 1;
  }
  orc_executor_set_m (ex, m);
  ORC_DEBUG("size %d %d", ex->n, ex->params[ORC_VAR_A1]);

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name == NULL) continue;

    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      src[i-ORC_VAR_S1] = orc_array_new (n, m, program->vars[i].size);
      orc_array_set_random (src[i-ORC_VAR_S1], &rand_context);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      dest_exec[i] = orc_array_new (n, m, program->vars[i].size);
      orc_array_set_pattern (dest_exec[i], 0xa5);
      dest_emul[i] = orc_array_new (n, m, program->vars[i].size);
      orc_array_set_pattern (dest_emul[i], 0xa5);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_PARAM) {
      orc_executor_set_param (ex, i, 2);
    }
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      orc_executor_set_array (ex, i, dest_exec[i]->data);
      orc_executor_set_stride (ex, i, dest_exec[i]->stride);
      have_dest = TRUE;
    }
    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      orc_executor_set_array (ex, i, src[i-ORC_VAR_S1]->data);
      orc_executor_set_stride (ex, i, src[i-ORC_VAR_S1]->stride);
    }
  }
  ORC_DEBUG ("running");
  orc_executor_run (ex);
  ORC_DEBUG ("done running");
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      acc_exec = ex->accumulators[0];
      have_acc = TRUE;
    }
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      orc_executor_set_array (ex, i, dest_emul[i]->data);
      orc_executor_set_stride (ex, i, dest_emul[i]->stride);
    }
    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      ORC_DEBUG("setting array %p", src[i-ORC_VAR_S1]->data);
      orc_executor_set_array (ex, i, src[i-ORC_VAR_S1]->data);
      orc_executor_set_stride (ex, i, src[i-ORC_VAR_S1]->stride);
    }
  }
  orc_executor_emulate (ex);
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      acc_emul = ex->accumulators[0];
    }
  }

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (program->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (!orc_array_compare (dest_exec[k], dest_emul[k], flags)) {
        for(j=0;j<m;j++){
          for(i=0;i<n;i++){
            int a,b;
            int l;

            printf("%2d %2d:", i, j);

            for(l=0;l<ORC_N_VARIABLES;l++){
              if (program->vars[l].name == NULL) continue;
              if (program->vars[l].vartype == ORC_VAR_TYPE_SRC &&
                  program->vars[l].size > 0) {
                if (flags & ORC_TEST_FLAGS_FLOAT) {
                  print_array_val_float (src[l-ORC_VAR_S1], i, j);
                } else {
                  print_array_val_signed (src[l-ORC_VAR_S1], i, j);
                }
              }
            }

            printf(" ->");
            if (flags & ORC_TEST_FLAGS_FLOAT) {
              a = print_array_val_float (dest_emul[k], i, j);
              b = print_array_val_float (dest_exec[k], i, j);
            } else {
              a = print_array_val_signed (dest_emul[k], i, j);
              b = print_array_val_signed (dest_exec[k], i, j);
            }

            if (a != b) {
              printf(" *");
            }

            printf("\n");
          }
        }

        ret = ORC_TEST_FAILED;
      }
      if (!orc_array_check_out_of_bounds (dest_exec[k])) {
        printf("out of bounds failure\n");

        ret = ORC_TEST_FAILED;
      }
    }
  }

  if (have_acc) {
    if (acc_emul != acc_exec) {
      for(j=0;j<m;j++){
        for(i=0;i<n;i++){

          printf("%2d %2d:", i, j);

          for(k=0;k<ORC_N_VARIABLES;k++){
            if (program->vars[k].name == NULL) continue;
            if (program->vars[k].vartype == ORC_VAR_TYPE_SRC &&
                program->vars[k].size > 0) {
              if (flags & ORC_TEST_FLAGS_FLOAT) {
                print_array_val_float (src[k-ORC_VAR_S1], i, j);
              } else {
                print_array_val_signed (src[k-ORC_VAR_S1], i, j);
              }
            }
          }

          printf(" -> acc\n");
        }
      }
      printf("acc %d %d\n", acc_emul, acc_exec);
      ret = ORC_TEST_FAILED;
    }
  }

  if (ret == ORC_TEST_FAILED) {
    printf("%s", orc_program_get_asm_code (program));
  }

  for(i=0;i<4;i++){
    if (dest_exec[i]) orc_array_free (dest_exec[i]);
    if (dest_emul[i]) orc_array_free (dest_emul[i]);
  }
  for(i=0;i<8;i++){
    if (src[i]) orc_array_free (src[i]);
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
  orc_program_set_2d (p);

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

