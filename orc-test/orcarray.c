
#include <orc-test/orctest.h>
#include <orc-test/orcarray.h>
#include <orc-test/orcrandom.h>
#include <orc/orc.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define EXTEND_ROWS 16
#define EXTEND_STRIDE 256

#define OOB_VALUE 0xa5

OrcArray *
orc_array_new (int n, int m, int element_size)
{
  OrcArray *ar;

  ar = malloc (sizeof(OrcArray));
  memset (ar, 0, sizeof(OrcArray));

  ar->n = n;
  ar->m = m;
  ar->element_size = element_size;

  ar->stride = (n*element_size + EXTEND_STRIDE);
  ar->alloc_len = ar->stride * (m+2*EXTEND_ROWS);
  ar->alloc_data = malloc (ar->alloc_len);

  ar->data = ORC_PTR_OFFSET (ar->alloc_data, ar->stride * EXTEND_ROWS);
  
  return ar;
}

void
orc_array_free (OrcArray *array)
{
  free (array->alloc_data);
  free (array);
}

void
orc_array_set_pattern (OrcArray *array, int value)
{
  memset (array->alloc_data, value, array->alloc_len);
}

void
orc_array_set_random (OrcArray *array, OrcRandom *context)
{
  orc_random_bits (context, array->alloc_data, array->alloc_len);
}


int
orc_array_compare (OrcArray *array1, OrcArray *array2)
{
  if (memcmp (array1->alloc_data, array2->alloc_data,
        array1->alloc_len) == 0) {
    return TRUE;
  }

  return FALSE;
}

int
orc_array_check_out_of_bounds (OrcArray *array)
{
  int i;
  int j;
  unsigned char *data;
  
  data = array->alloc_data;
  for(i=0;i<array->stride * EXTEND_ROWS;i++){
    if (data[i] != OOB_VALUE) {
      printf("OOB check failed at start-%d\n", array->stride * EXTEND_ROWS - i);
      return FALSE;
    }
  }

  for(j=0;j<array->m;j++){
    data = array->data + array->stride * j;
    for(i=array->element_size * array->n;i<array->stride;i++){
      if (data[i] != OOB_VALUE) {
        printf("OOB check failed on row %d, end+%d\n", j,
            i - array->element_size * array->n);
        return FALSE;
      }
    }
  }

  data = array->data + array->stride * array->m;
  for(i=0;i<array->stride * EXTEND_ROWS;i++){
    if (data[i] != OOB_VALUE) {
      printf("OOB check failed at end+%d\n", i);
      return FALSE;
    }
  }

  return TRUE;
}

#if 0
void
orc_array_print_compare (OrcArray *array1, OrcArray *array2)
{

  for(j=0;j<array1->m;j++){
    for(i=0;i<array1->n;i++){
      int a,b;
      int j;

      printf("%2d %2d:", i, j);

      for(k=0;k<ORC_N_VARIABLES;k++){
        if (program->vars[k].name == NULL) continue;
        if (program->vars[k].vartype == ORC_VAR_TYPE_SRC &&
            program->vars[k].size > 0) {
          print_array_val_signed (ex->arrays[k], program->vars[k].size, i);
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
  }
}
#endif

