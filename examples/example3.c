#include <stdlib.h>
#include "example3orc.h"

#define N 10

int
main (int argc, char *argv[])
{
  unsigned char *input_y = calloc(640*480, sizeof(unsigned char));
  unsigned char *input_u = calloc(320*240, sizeof(unsigned char));
  unsigned char *input_v = calloc(320*240, sizeof(unsigned char));
  unsigned int *output = malloc(640*480*sizeof(unsigned int));

  /* Call a function that uses Orc */
  convert_I420_AYUV (output, 1280*4, output + 640, 1280 * 4,
      input_y, 1280, input_y + 640, 1280,
      input_u, 320, input_v, 320,
      320, 240);

  free(output);
  free(input_v);
  free(input_u);
  free(input_y);
  return 0;
}

