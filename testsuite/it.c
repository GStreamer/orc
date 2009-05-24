
void it(int *a, int *b)
{
  int i;
  i = 10;
  while(i) {
    *a++ = *b++;
    i--;
  }
}

