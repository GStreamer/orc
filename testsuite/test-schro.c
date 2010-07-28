
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <orc-test/orctest.h>


int error = FALSE;


OrcProgram *
get_program (int type)
{
  OrcProgram *p;

  switch (type) {
  case 0:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_constant (p, 2, 2, "c1");
    orc_program_add_constant (p, 2, 2, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "s2");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "addw", "d1", "d1", "t1");
    break;
  case 1:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_constant (p, 2, 2, "c1");
    orc_program_add_constant (p, 2, 2, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "s2");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "subw", "d1", "d1", "t1");
    break;
  case 2:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_constant (p, 2, 1, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "s2");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "addw", "d1", "d1", "t1");
    break;
  case 3:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s1");
    orc_program_add_source (p, 2, "s2");
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_constant (p, 2, 1, "c2");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "s2");
    orc_program_append_str (p, "addw", "t1", "t1", "c1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "c2");
    orc_program_append_str (p, "subw", "d1", "d1", "t1");
    break;
  case 4:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_constant (p, 2, 1, "c1");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_str (p, "addw", "t1", "s1", "c1");
    orc_program_append_str (p, "shrsw", "d1", "t1", "c1");
    break;
  case 5:
    p = orc_program_new_dss (2,2,2);

    orc_program_append_str (p, "addw", "d1", "s1", "s2");
    break;
  case 6:
    p = orc_program_new_ds (2,2);
    orc_program_add_constant (p, 2, 1, "c1");

    orc_program_append_str (p, "shlw", "d1", "s1", "c1");
    break;
  case 7:
    p = orc_program_new_ds (2,2);
    orc_program_add_constant (p, 2, 2, "c1");

    orc_program_append_str (p, "shlw", "d1", "s1", "c1");
    break;
  case 8:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 4, "t2");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 4, "p2");
    orc_program_add_parameter (p, 4, "p3");

    orc_program_append_str (p, "addw", "t1", "s1", "s2");
    orc_program_append_str (p, "mulswl", "t2", "t1", "p1");
    orc_program_append_str (p, "addl", "t2", "t2", "p2");
    orc_program_append_str (p, "shll", "t2", "t2", "p3");
    orc_program_append_ds_str (p, "convlw", "t1", "t2");
    orc_program_append_str (p, "addw", "d1", "d1", "t1");
    break;
  case 9:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s20");
    orc_program_add_source (p, 2, "s21");
    orc_program_add_source (p, 2, "s22");
    orc_program_add_source (p, 2, "s23");
    orc_program_add_constant (p, 2, 9, "c1");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 2, "p2");
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");

    orc_program_append_str (p, "addw", "t1", "s21", "s22");
    orc_program_append_str (p, "mullw", "t1", "t1", "c1");
    orc_program_append_str (p, "addw", "t2", "s20", "s23");
    orc_program_append_str (p, "subw", "t1", "t1", "t2");
    orc_program_append_str (p, "addw", "t1", "t1", "p1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "p2");
    orc_program_append_str (p, "addw", "d1", "d1", "t1");
    break;
  case 10:
    p = orc_program_new ();
    orc_program_add_destination (p, 2, "d1");
    orc_program_add_source (p, 2, "s20");
    orc_program_add_source (p, 2, "s21");
    orc_program_add_source (p, 2, "s22");
    orc_program_add_source (p, 2, "s23");
    orc_program_add_constant (p, 2, 9, "c1");
    orc_program_add_parameter (p, 2, "p1");
    orc_program_add_parameter (p, 2, "p2");
    orc_program_add_temporary (p, 2, "t1");
    orc_program_add_temporary (p, 2, "t2");

    orc_program_append_str (p, "addw", "t1", "s21", "s22");
    orc_program_append_str (p, "mullw", "t1", "t1", "c1");
    orc_program_append_str (p, "addw", "t2", "s20", "s23");
    orc_program_append_str (p, "subw", "t1", "t1", "t2");
    orc_program_append_str (p, "addw", "t1", "t1", "p1");
    orc_program_append_str (p, "shrsw", "t1", "t1", "p2");
    orc_program_append_str (p, "subw", "d1", "d1", "t1");
    break;
  case 11:
    p = orc_program_new_dss (2,2,2);

    orc_program_append_str (p, "subw", "d1", "s1", "s2");
    break;
  case 12:
    p = orc_program_new_ds (1,1);
    orc_program_append_ds_str (p, "copyb", "d1", "s1");
    break;
  case 13:
    p = orc_program_new_dss (2,2,1);
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s2");
    orc_program_append_str (p, "addw", "d1", "t1", "s1");
    break;
  case 14:
    p = orc_program_new_ds (2,1);

    orc_program_append_ds_str (p, "convubw", "d1", "s1");
    break;
  case 15:
    p = orc_program_new_ds (1,2);

    orc_program_append_ds_str (p, "convsuswb", "d1", "s1");
    break;
  case 16:
    p = orc_program_new_dss (2,2,1);
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s2");
    orc_program_append_str (p, "subw", "d1", "s1", "t1");
    break;
  case 17:
    p = orc_program_new_dss (2,2,2);
    orc_program_add_source (p, 1, "s3");
    orc_program_add_temporary (p, 2, "t1");

    orc_program_append_ds_str (p, "convubw", "t1", "s3");
    orc_program_append_str (p, "mullw", "t1", "t1", "s2");
    orc_program_append_str (p, "addw", "d1", "s1", "t1");
    break;
  default:
    return NULL;
  }

  return p;
}



void
test_program (int type)
{
  OrcProgram *p;
  char s[40];
  OrcTestResult ret;

  p = get_program(type);

  sprintf(s, "test_schro_%d", type);
  orc_program_set_name (p, s);

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }
}


int
main (int argc, char *argv[])
{
  int i;

  orc_init();
  orc_test_init();

  for(i=0;i<18;i++){
    //printf("/* %d */\n", i);
    test_program (i);
  }

  if (error) return 1;
  return 0;
}

