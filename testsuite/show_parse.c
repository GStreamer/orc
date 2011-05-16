
#define ORC_ENABLE_UNSTABLE_API

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-test/orcarray.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef _MSC_VER
#define isnan(x) _isnan(x)
#endif

static char * read_file (const char *filename);
void output_code (OrcProgram *p, FILE *output);
void output_code_header (OrcProgram *p, FILE *output);
void output_code_test (OrcProgram *p, FILE *output);

void show (OrcProgram *p);

int error = FALSE;

enum {
  FORMAT_SIGNED,
  FORMAT_UNSIGNED,
  FORMAT_HEX,
  FORMAT_FLOAT
};

int format = FORMAT_SIGNED;
int array_n = 10;

int
main (int argc, char *argv[])
{
  char *code;
  int n = 0;
  int i;
  OrcProgram **programs;
  const char *filename = NULL;

  orc_init ();
  orc_test_init ();

  for(i=1;i<argc;i++){
    if (strcmp("-x", argv[i]) == 0) {
      format = FORMAT_HEX;
    } else if (strcmp("-s", argv[i]) == 0) {
      format = FORMAT_SIGNED;
    } else if (strcmp("-u", argv[i]) == 0) {
      format = FORMAT_UNSIGNED;
    } else if (strcmp("-f", argv[i]) == 0) {
      format = FORMAT_FLOAT;
    } else if (strcmp("-n", argv[i]) == 0) {
      if (i + 1 < argc) {
        array_n = strtol (argv[i+1], NULL, 0);
        i++;
      }
    } else {
      filename = argv[i];
    }
  }

  if (filename == NULL) {
    filename = getenv ("testfile");
  }
  if (filename == NULL) {
    filename = "test.orc";
  }
  code = read_file (filename);
  if (code) {
    n = orc_parse (code, &programs);
  } else {
    OrcStaticOpcode *opcode;

    opcode = orc_opcode_find_by_name (filename);
    if (opcode) {
      programs = malloc(sizeof(void *));
      programs[0] = orc_test_get_program_for_opcode (opcode);
      n = 1;
    } else {
      printf("show_parse [-fsux] (<file.orc>|opcode)\n");
      exit(1);
    }
  }

  for(i=0;i<n;i++){
    show (programs[i]);
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

int
print_array_val_signed (OrcArray *array, int i, int j)
{
  void *ptr = ORC_PTR_OFFSET (array->data,
      i*array->element_size + j*array->stride);

  switch (array->element_size) {
    case 1:
      printf(" %4d", *(orc_int8 *)ptr);
      return *(orc_int8 *)ptr;
    case 2:
      printf(" %5d", *(orc_int16 *)ptr);
      return *(orc_int16 *)ptr;
    case 4:
      printf(" %10d", *(orc_int32 *)ptr);
      return *(orc_int32 *)ptr;
    case 8:
      printf(" 0x%08x%08x", (orc_uint32)((*(orc_uint64 *)ptr)>>32),
          (orc_uint32)((*(orc_uint64 *)ptr)));
      return *(orc_int64 *)ptr;
    default:
      return -1;
  }
}

int
print_array_val_unsigned (OrcArray *array, int i, int j)
{
  void *ptr = ORC_PTR_OFFSET (array->data,
      i*array->element_size + j*array->stride);

  switch (array->element_size) {
    case 1:
      printf(" %4u", *(orc_uint8 *)ptr);
      return *(orc_int8 *)ptr;
    case 2:
      printf(" %5u", *(orc_uint16 *)ptr);
      return *(orc_int16 *)ptr;
    case 4:
      printf(" %10u", *(orc_uint32 *)ptr);
      return *(orc_int32 *)ptr;
    case 8:
      printf(" 0x%08x%08x", (orc_uint32)((*(orc_uint64 *)ptr)>>32),
          (orc_uint32)((*(orc_uint64 *)ptr)));
      return *(orc_int64 *)ptr;
    default:
      return -1;
  }
}

int
print_array_val_hex (OrcArray *array, int i, int j)
{
  void *ptr = ORC_PTR_OFFSET (array->data,
      i*array->element_size + j*array->stride);

  switch (array->element_size) {
    case 1:
      printf(" %02x", *(orc_uint8 *)ptr);
      return *(orc_int8 *)ptr;
    case 2:
      printf(" %04x", *(orc_uint16 *)ptr);
      return *(orc_int16 *)ptr;
    case 4:
      printf(" %08x", *(orc_uint32 *)ptr);
      return *(orc_int32 *)ptr;
    case 8:
      printf(" 0x%08x%08x", (orc_uint32)((*(orc_uint64 *)ptr)>>32),
          (orc_uint32)((*(orc_uint64 *)ptr)));
      return *(orc_int64 *)ptr;
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
        printf(" nan %08x", *(orc_uint32 *)ptr);
        /* This is to get around signaling/non-signaling nans in the output */
        return (*(orc_uint32 *)ptr) & 0xffbfffff;
      } else {
        printf(" %12.5g", *(float *)ptr);
        return *(orc_int32 *)ptr;
      }
    case 8:
      printf(" %12.5g", *(double *)ptr);
      return *(orc_int64 *)ptr;
    default:
      printf(" ERROR");
      return -1;
  }
}


void
show (OrcProgram *program)
{
  OrcCompileResult result;
  OrcTarget *target;
  const char *target_name;
  unsigned int target_flags;
  int n, m;
  OrcExecutor *ex;
  OrcArray *dest[4] = { NULL, NULL, NULL, NULL };
  OrcArray *src[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
  int i,j;
  OrcRandomContext rand_context = { 0 };


  target_name = NULL;
  target = orc_target_get_by_name (target_name);

  target_flags = orc_target_get_default_flags (target);

  result = orc_program_compile_full (program, target, target_flags);
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    printf("%s: compile failed\n", program->name);
    return;
  }

  printf("%s:\n", program->name);

  if (program->constant_n > 0) {
    n = program->constant_n;
  } else {
    n = array_n;
  }

  ex = orc_executor_new (program);
  orc_executor_set_n (ex, n);
  if (program->is_2d) {
    if (program->constant_m > 0) {
      m = program->constant_m;
    } else {
      m = 2;
    }
  } else {
    m = 1;
  }
  orc_executor_set_m (ex, m);

  for(i=0;i<ORC_N_VARIABLES;i++){
    if (program->vars[i].name == NULL) continue;

    if (program->vars[i].vartype == ORC_VAR_TYPE_SRC) {
      src[i-ORC_VAR_S1] = orc_array_new (n, m, program->vars[i].size, 0, 0);
      orc_array_set_random (src[i-ORC_VAR_S1], &rand_context);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_DEST) {
      dest[i-ORC_VAR_D1] = orc_array_new (n, m, program->vars[i].size, 0, 0);
      orc_array_set_pattern (dest[i], ORC_OOB_VALUE);
    } else if (program->vars[i].vartype == ORC_VAR_TYPE_PARAM) {
      switch (program->vars[i].param_type) {
        case ORC_PARAM_TYPE_INT:
          orc_executor_set_param (ex, i, 2);
          break;
        case ORC_PARAM_TYPE_FLOAT:
          orc_executor_set_param_float (ex, i, 2.0);
          break;
        case ORC_PARAM_TYPE_INT64:
          orc_executor_set_param_int64 (ex, i, 2);
          break;
        case ORC_PARAM_TYPE_DOUBLE:
          orc_executor_set_param_double (ex, i, 2.0);
          break;
        default:
          ORC_ASSERT(0);
      }
    }
  }

  orc_executor_set_n (ex, n);
  orc_executor_set_m (ex, m);
  for(j=0;j<ORC_N_VARIABLES;j++){
    if (program->vars[j].vartype == ORC_VAR_TYPE_DEST) {
      orc_executor_set_array (ex, j, dest[j-ORC_VAR_D1]->data);
      orc_executor_set_stride (ex, j, dest[j-ORC_VAR_D1]->stride);
    }
    if (program->vars[j].vartype == ORC_VAR_TYPE_SRC) {
      orc_executor_set_array (ex, j, src[j-ORC_VAR_S1]->data);
      orc_executor_set_stride (ex, j, src[j-ORC_VAR_S1]->stride);
    }
  }

  orc_executor_run (ex);

  {
    int i,j;

    for(j=0;j<m;j++){
      for(i=0;i<n;i++){
        int l;

        printf("%2d %2d:", i, j);

        for(l=ORC_VAR_S1;l<ORC_VAR_S1+8;l++){
          if (program->vars[l].size > 0) {
            switch (format) {
              case FORMAT_FLOAT:
                print_array_val_float (src[l-ORC_VAR_S1], i, j);
                break;
              case FORMAT_HEX:
                print_array_val_hex (src[l-ORC_VAR_S1], i, j);
                break;
              case FORMAT_SIGNED:
                print_array_val_signed (src[l-ORC_VAR_S1], i, j);
                break;
              case FORMAT_UNSIGNED:
                print_array_val_unsigned (src[l-ORC_VAR_S1], i, j);
                break;
            }
          }
        }

        printf(" ->");
        for(l=ORC_VAR_D1;l<ORC_VAR_D1+4;l++){
          if (program->vars[l].size > 0) {
            switch (format) {
              case FORMAT_FLOAT:
                print_array_val_float (dest[l-ORC_VAR_D1], i, j);
                break;
              case FORMAT_HEX:
                print_array_val_hex (dest[l-ORC_VAR_D1], i, j);
                break;
              case FORMAT_SIGNED:
                print_array_val_signed (dest[l-ORC_VAR_D1], i, j);
                break;
              case FORMAT_UNSIGNED:
                print_array_val_unsigned (dest[l-ORC_VAR_D1], i, j);
                break;
            }
          }
        }

        printf("\n");
      }
    }
  }



  for(i=0;i<4;i++){
    if (dest[i]) orc_array_free (dest[i]);
  }
  for(i=0;i<8;i++){
    if (src[i]) orc_array_free (src[i]);
  }

  orc_executor_free (ex);

}

