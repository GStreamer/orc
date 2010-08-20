#include <stdio.h>
#include "example3orc.h"

#define N 10

unsigned char input_y[640*480];
unsigned char input_u[320*240];
unsigned char input_v[320*240];

unsigned int output[640*480];

int
main (int argc, char *argv[])
{

  /* Call a function that uses Orc */
  convert_I420_AYUV (output, 1280*4, output + 640, 1280 * 4,
      input_y, 1280, input_y + 640, 1280,
      input_u, 320, input_v, 320,
      320, 240);

  return 0;
}

