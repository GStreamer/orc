#include "config.h"

#include <string.h>
#include <orc/orcdebug.h>
#include <orc/orcconstant.h>

orc_bool
orc_constant_resolve (const OrcConstant *c, OrcConstant *r, int reg_size)
{
  /* The number of 64-bits values that fit in the register size */
  int count_64 = reg_size / 8;
  int i;

  if (!count_64 || reg_size > sizeof (r->v)) {
    ORC_ERROR ("Register size %d does not fit on a constant", reg_size);
    return FALSE;
  }

  if (c->type == ORC_CONST_FULL) {
    memcpy (r, c, sizeof (OrcConstant));
    return TRUE;
  }

  r->type = ORC_CONST_FULL;
  r->alloc_reg = 0;
  r->label = 0;
  r->use_count = 0;
  memset (r->v, 0, sizeof (r->v));
  switch (c->type) {
    case ORC_CONST_ZERO:
      break;
    case ORC_CONST_SPLAT_B:
      memset (r->v, c->v[0].x8[0], reg_size);
      break;
    case ORC_CONST_SPLAT_W:
      for (i = 0; i < count_64; i++) {
        r->v[i].x4[0] = c->v[0].x4[0];
        r->v[i].x4[1] = c->v[0].x4[0];
        r->v[i].x4[2] = c->v[0].x4[0];
        r->v[i].x4[3] = c->v[0].x4[0];
      }
      break;
    case ORC_CONST_SPLAT_L:
      for (i = 0; i < count_64; i++) {
        r->v[i].x2[0] = c->v[0].x2[0];
        r->v[i].x2[1] = c->v[0].x2[0];
      }
      break;
    case ORC_CONST_SPLAT_Q:
      for (i = 0; i < count_64; i++) {
        r->v[i].i = c->v[0].i;
      }
      break;

    case ORC_CONST_SPLAT_DQ:
      for (i = 0; i < count_64; i+=2) {
        r->v[i].i = c->v[0].i;
        r->v[i+1].i = c->v[1].i;
      }
      break;

    case ORC_CONST_SPLAT_QQ:
      for (i = 0; i < count_64; i+=4) {
        r->v[i].i = c->v[0].i;
        r->v[i+1].i = c->v[1].i;
        r->v[i+2].i = c->v[2].i;
        r->v[i+3].i = c->v[3].i;
      }
      break;

     default:
       ORC_ERROR ("Impossible to resolve %d", c->type);
       return FALSE;
  }
  return TRUE;
}

orc_bool
orc_constant_is_equal (const OrcConstant *c1, const OrcConstant *c2,
    int reg_size)
{
  OrcConstant r1, r2;

  if (!orc_constant_resolve (c1, &r1, reg_size))
    return FALSE;
  if (!orc_constant_resolve (c2, &r2, reg_size))
    return FALSE;

  if (memcmp (&r1.v, &r2.v, sizeof (r1.v)))
    return FALSE;
  else
    return TRUE;
}
