
/* autogenerated from orcfunctions.orc */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _ORC_INTEGER_TYPEDEFS_
#define _ORC_INTEGER_TYPEDEFS_
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdint.h>
typedef int8_t orc_int8;
typedef int16_t orc_int16;
typedef int32_t orc_int32;
typedef int64_t orc_int64;
typedef uint8_t orc_uint8;
typedef uint16_t orc_uint16;
typedef uint32_t orc_uint32;
typedef uint64_t orc_uint64;
#define ORC_UINT64_C(x) UINT64_C(x)
#elif defined(_MSC_VER)
typedef signed __int8 orc_int8;
typedef signed __int16 orc_int16;
typedef signed __int32 orc_int32;
typedef signed __int64 orc_int64;
typedef unsigned __int8 orc_uint8;
typedef unsigned __int16 orc_uint16;
typedef unsigned __int32 orc_uint32;
typedef unsigned __int64 orc_uint64;
#define ORC_UINT64_C(x) (x##Ui64)
#define inline __inline
#else
#include <limits.h>
typedef signed char orc_int8;
typedef short orc_int16;
typedef int orc_int32;
typedef unsigned char orc_uint8;
typedef unsigned short orc_uint16;
typedef unsigned int orc_uint32;
#if INT_MAX == LONG_MAX
typedef long long orc_int64;
typedef unsigned long long orc_uint64;
#define ORC_UINT64_C(x) (x##ULL)
#else
typedef long orc_int64;
typedef unsigned long orc_uint64;
#define ORC_UINT64_C(x) (x##UL)
#endif
#endif
typedef union { orc_int16 i; orc_int8 x2[2]; } orc_union16;
typedef union { orc_int32 i; float f; orc_int16 x2[2]; orc_int8 x4[4]; } orc_union32;
typedef union { orc_int64 i; double f; orc_int32 x2[2]; float x2f[2]; orc_int16 x4[4]; orc_int8 x8[8]; } orc_union64;
#endif
#ifndef ORC_RESTRICT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define ORC_RESTRICT restrict
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ORC_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define ORC_RESTRICT __restrict
#else
#define ORC_RESTRICT
#endif
#endif

#ifndef ORC_INTERNAL
#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define ORC_INTERNAL __attribute__((visibility("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define ORC_INTERNAL __hidden
#elif defined (__GNUC__)
#define ORC_INTERNAL __attribute__((visibility("hidden")))
#else
#define ORC_INTERNAL
#endif
#endif


#ifndef DISABLE_ORC
#include <orc/orc.h>
#endif
void orc_memcpy (void * ORC_RESTRICT d1, const void * ORC_RESTRICT s1, int n);
void orc_memset (void * ORC_RESTRICT d1, int p1, int n);


/* begin Orc C target preamble */
#include <math.h>
#define ORC_CLAMP_U(x,a,b) (((x)>(b) ? (b) : (x)))
#define ORC_CLAMP_S(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define ORC_ABS(a) ((a)<0 ? -(a) : (a))
#define ORC_MIN(a,b) ((a)<(b) ? (a) : (b))
#define ORC_MAX(a,b) ((a)>(b) ? (a) : (b))
#define ORC_SB_MAX 127
#define ORC_SB_MIN (-1-ORC_SB_MAX)
#define ORC_UB_MAX (orc_uint8) 255
#define ORC_UB_MIN 0
#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)
#define ORC_UW_MAX (orc_uint16)65535
#define ORC_UW_MIN 0
#define ORC_SL_MAX 2147483647
#define ORC_SL_MIN (-1-ORC_SL_MAX)
#define ORC_UL_MAX 4294967295U
#define ORC_UL_MIN 0
#define ORC_CLAMP_SB(x) ORC_CLAMP_S(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) ORC_CLAMP_U(x,ORC_UB_MIN,ORC_UB_MAX)
#define ORC_CLAMP_SW(x) ORC_CLAMP_S(x,ORC_SW_MIN,ORC_SW_MAX)
#define ORC_CLAMP_UW(x) ORC_CLAMP_U(x,ORC_UW_MIN,ORC_UW_MAX)
#define ORC_CLAMP_SL(x) ORC_CLAMP_S(x,ORC_SL_MIN,ORC_SL_MAX)
#define ORC_CLAMP_UL(x) ORC_CLAMP_U(x,ORC_UL_MIN,ORC_UL_MAX)
#define ORC_SWAP_W(x) ((((x)&0xffU)<<8) | (((x)&0xff00U)>>8))
#define ORC_SWAP_L(x) ((((x)&0xffU)<<24) | (((x)&0xff00U)<<8) | (((x)&0xff0000U)>>8) | (((x)&0xff000000U)>>24))
#define ORC_SWAP_Q(x) ((((x)&ORC_UINT64_C(0xff))<<56) | (((x)&ORC_UINT64_C(0xff00))<<40) | (((x)&ORC_UINT64_C(0xff0000))<<24) | (((x)&ORC_UINT64_C(0xff000000))<<8) | (((x)&ORC_UINT64_C(0xff00000000))>>8) | (((x)&ORC_UINT64_C(0xff0000000000))>>24) | (((x)&ORC_UINT64_C(0xff000000000000))>>40) | (((x)&ORC_UINT64_C(0xff00000000000000))>>56))
#define ORC_PTR_OFFSET(ptr,offset) ((void *)(((unsigned char *)(ptr)) + (offset)))
#define ORC_DENORMAL(x) ((x) & ((((x)&0x7f800000) == 0) ? 0xff800000 : 0xffffffff))
#define ORC_ISNAN(x) ((((x)&0x7f800000) == 0x7f800000) && (((x)&0x007fffff) != 0))
#define ORC_DENORMAL_DOUBLE(x) ((x) & ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == 0) ? ORC_UINT64_C(0xfff0000000000000) : ORC_UINT64_C(0xffffffffffffffff)))
#define ORC_ISNAN_DOUBLE(x) ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == ORC_UINT64_C(0x7ff0000000000000)) && (((x)&ORC_UINT64_C(0x000fffffffffffff)) != 0))
#ifndef ORC_RESTRICT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define ORC_RESTRICT restrict
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ORC_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define ORC_RESTRICT __restrict
#else
#define ORC_RESTRICT
#endif
#endif
/* end Orc C target preamble */



/* orc_memcpy */
#ifdef DISABLE_ORC
void
orc_memcpy (void * ORC_RESTRICT d1, const void * ORC_RESTRICT s1, int n){
  int i;
  orc_int8 * ORC_RESTRICT ptr0;
  const orc_int8 * ORC_RESTRICT ptr4;
  orc_int8 var32;
  orc_int8 var33;

  ptr0 = (orc_int8 *)d1;
  ptr4 = (orc_int8 *)s1;


  for (i = 0; i < n; i++) {
    /* 0: loadb */
    var32 = ptr4[i];
    /* 1: copyb */
    var33 = var32;
    /* 2: storeb */
    ptr0[i] = var33;
  }

}

#else
static void
_backup_orc_memcpy (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_int8 * ORC_RESTRICT ptr0;
  const orc_int8 * ORC_RESTRICT ptr4;
  orc_int8 var32;
  orc_int8 var33;

  ptr0 = (orc_int8 *)ex->arrays[0];
  ptr4 = (orc_int8 *)ex->arrays[4];


  for (i = 0; i < n; i++) {
    /* 0: loadb */
    var32 = ptr4[i];
    /* 1: copyb */
    var33 = var32;
    /* 2: storeb */
    ptr0[i] = var33;
  }

}

void
orc_memcpy (void * ORC_RESTRICT d1, const void * ORC_RESTRICT s1, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static OrcOnce once = ORC_ONCE_INIT;
  OrcCode *c;
  OrcExecutorFunc func = NULL;

  if (!orc_once_enter (&once, (void **) &c)) {
    OrcProgram *p;

#if 1
    static const orc_uint8 bc[] = {
      1, 9, 10, 111, 114, 99, 95, 109, 101, 109, 99, 112, 121, 11, 1, 1, 
      12, 1, 1, 42, 0, 4, 2, 0, 
    };
    p = orc_program_new_from_static_bytecode (bc);
    orc_program_set_backup_function (p, _backup_orc_memcpy);
#else
    p = orc_program_new ();
    orc_program_set_name (p, "orc_memcpy");
    orc_program_set_backup_function (p, _backup_orc_memcpy);
    orc_program_add_destination (p, 1, "d1");
    orc_program_add_source (p, 1, "s1");

    orc_program_append_2 (p, "copyb", 0, ORC_VAR_D1, ORC_VAR_S1, ORC_VAR_D1, ORC_VAR_D1);
#endif

    orc_program_compile (p);
    c = orc_program_take_code (p);
    orc_program_free (p);
    orc_once_leave (&once, c);
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->arrays[ORC_VAR_S1] = (void *)s1;

  func = c->exec;
  func (ex);
}
#endif


/* orc_memset */
#ifdef DISABLE_ORC
void
orc_memset (void * ORC_RESTRICT d1, int p1, int n){
  int i;
  orc_int8 * ORC_RESTRICT ptr0;
  orc_int8 var32;
  orc_int8 var33;

  ptr0 = (orc_int8 *)d1;

    /* 0: loadpb */
    var32 = p1;

  for (i = 0; i < n; i++) {
    /* 1: copyb */
    var33 = var32;
    /* 2: storeb */
    ptr0[i] = var33;
  }

}

#else
static void
_backup_orc_memset (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_int8 * ORC_RESTRICT ptr0;
  orc_int8 var32;
  orc_int8 var33;

  ptr0 = (orc_int8 *)ex->arrays[0];

    /* 0: loadpb */
    var32 = ex->params[24];

  for (i = 0; i < n; i++) {
    /* 1: copyb */
    var33 = var32;
    /* 2: storeb */
    ptr0[i] = var33;
  }

}

void
orc_memset (void * ORC_RESTRICT d1, int p1, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static OrcOnce once = ORC_ONCE_INIT;
  OrcCode *c;
  OrcExecutorFunc func = NULL;

  if (!orc_once_enter (&once, (void **) &c)) {
    OrcProgram *p;

#if 1
    static const orc_uint8 bc[] = {
      1, 9, 10, 111, 114, 99, 95, 109, 101, 109, 115, 101, 116, 11, 1, 1, 
      16, 1, 42, 0, 24, 2, 0, 
    };
    p = orc_program_new_from_static_bytecode (bc);
    orc_program_set_backup_function (p, _backup_orc_memset);
#else
    p = orc_program_new ();
    orc_program_set_name (p, "orc_memset");
    orc_program_set_backup_function (p, _backup_orc_memset);
    orc_program_add_destination (p, 1, "d1");
    orc_program_add_parameter (p, 1, "p1");

    orc_program_append_2 (p, "copyb", 0, ORC_VAR_D1, ORC_VAR_P1, ORC_VAR_D1, ORC_VAR_D1);
#endif

    orc_program_compile (p);
    c = orc_program_take_code (p);
    orc_program_free (p);
    orc_once_leave (&once, c);
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->params[ORC_VAR_P1] = p1;

  func = c->exec;
  func (ex);
}
#endif


