
/* begin Orc C target preamble */
#include <stdint.h>
typedef struct _OrcProgram OrcProgram;
typedef struct _OrcExecutor OrcExecutor;
#define ORC_N_VARIABLES 20
#define ORC_N_REGISTERS 20
#define ORC_OPCODE_N_ARGS 4
struct _OrcExecutor {
  OrcProgram *program;
  int n;
  int counter1;
  int counter2;
  int counter3;
  void *arrays[ORC_N_VARIABLES];
  int params[ORC_N_VARIABLES];
  //OrcVariable vars[ORC_N_VARIABLES];
  //OrcVariable *args[ORC_OPCODE_N_ARGS];
};
#define ORC_CLAMP(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define ORC_ABS(a) ((a)<0 ? (-a) : (a))
#define ORC_MIN(a,b) ((a)<(b) ? (a) : (b))
#define ORC_MAX(a,b) ((a)>(b) ? (a) : (b))
#define ORC_SB_MAX 127
#define ORC_SB_MIN (-1-ORC_SB_MAX)
#define ORC_UB_MAX 255
#define ORC_UB_MIN 0
#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)
#define ORC_UW_MAX 65535
#define ORC_UW_MIN 0
#define ORC_SL_MAX 2147483647
#define ORC_SL_MIN (-1-ORC_SL_MAX)
#define ORC_UL_MAX 4294967295U
#define ORC_UL_MIN 0
#define ORC_CLAMP_SB(x) ORC_CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_SB_MAX)
#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_SB_MAX)
/* end Orc C target preamble */

/* absb 1,1,0 0xb7f12fd0 */
void
test_absb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: absb */
    var0[i] = ORC_ABS(var1[i]);
  }
}

/* addb 1,1,1 0xb7f12ff0 */
void
test_addb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addb */
    var0[i] = var1[i] + var2[i];
  }
}

/* addssb 1,1,1 0xb7f13010 */
void
test_addssb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssb */
    var0[i] = ORC_CLAMP_SB(var1[i] + var2[i]);
  }
}

/* addusb 1,1,1 0xb7f13020 */
void
test_addusb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusb */
    var0[i] = ORC_CLAMP_UB(var1[i] + var2[i]);
  }
}

/* andb 1,1,1 0xb7f13030 */
void
test_andb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andb */
    var0[i] = var1[i] & var2[i];
  }
}

/* andnb 1,1,1 0xb7f13050 */
void
test_andnb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnb */
    var0[i] = var1[i] & (~var2[i]);
  }
}

/* avgsb 1,1,1 0xb7f13070 */
void
test_avgsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsb */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* avgub 1,1,1 0xb7f13090 */
void
test_avgub (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgub */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* cmpeqb 1,1,1 0xb7f130b0 */
void
test_cmpeqb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeqb */
    var0[i] = (var1[i] == var2[i]) ? (~0) : 0;
  }
}

/* cmpgtsb 1,1,1 0xb7f130d0 */
void
test_cmpgtsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsb */
    var0[i] = (var1[i] > var2[i]) ? (~0) : 0;
  }
}

/* copyb 1,1,0 0xb7f130f0 */
void
test_copyb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyb */
    var0[i] = var1[i];
  }
}

/* maxsb 1,1,1 0xb7f13100 */
void
test_maxsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsb */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* maxub 1,1,1 0xb7f13120 */
void
test_maxub (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxub */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* minsb 1,1,1 0xb7f13140 */
void
test_minsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsb */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* minub 1,1,1 0xb7f13160 */
void
test_minub (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minub */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* mullb 1,1,1 0xb7f13180 */
void
test_mullb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mullb */
    var0[i] = (var1[i] * var2[i]) & 0xff;
  }
}

/* mulhsb 1,1,1 0xb7f131a0 */
void
test_mulhsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsb */
    var0[i] = (var1[i] * var2[i]) >> 8;
  }
}

/* mulhub 1,1,1 0xb7f131c0 */
void
test_mulhub (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhub */
    var0[i] = (var1[i] * var2[i]) >> 8;
  }
}

/* orb 1,1,1 0xb7f131e0 */
void
test_orb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: orb */
    var0[i] = var1[i] | var2[i];
  }
}

/* shlb 1,1,1 0xb7f13200 */
void
test_shlb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shlb */
    var0[i] = var1[i] << var2[i];
  }
}

/* shrsb 1,1,1 0xb7f13220 */
void
test_shrsb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsb */
    var0[i] = var1[i] >> var2[i];
  }
}

/* shrub 1,1,1 0xb7f13240 */
void
test_shrub (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrub */
    var0[i] = ((uint8_t)var1[i]) >> var2[i];
  }
}

/* signb 1,1,0 0xb7f13260 */
void
test_signb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: signb */
    var0[i] = ORC_CLAMP(var1[i],-1,1);
  }
}

/* subb 1,1,1 0xb7f13270 */
void
test_subb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subb */
    var0[i] = var1[i] - var2[i];
  }
}

/* subssb 1,1,1 0xb7f13290 */
void
test_subssb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssb */
    var0[i] = ORC_CLAMP_SB(var1[i] - var2[i]);
  }
}

/* subusb 1,1,1 0xb7f132a0 */
void
test_subusb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusb */
    var0[i] = ORC_CLAMP_UB((uint8_t)var1[i] - (uint8_t)var2[i]);
  }
}

/* xorb 1,1,1 0xb7f132b0 */
void
test_xorb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorb */
    var0[i] = var1[i] ^ var2[i];
  }
}

/* absw 2,2,0 0xb7f132d0 */
void
test_absw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: absw */
    var0[i] = ORC_ABS(var1[i]);
  }
}

/* addw 2,2,2 0xb7f132f0 */
void
test_addw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addw */
    var0[i] = var1[i] + var2[i];
  }
}

/* addssw 2,2,2 0xb7f13310 */
void
test_addssw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssw */
    var0[i] = ORC_CLAMP_SW(var1[i] + var2[i]);
  }
}

/* addusw 2,2,2 0xb7f13320 */
void
test_addusw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusw */
    var0[i] = ORC_CLAMP_UW(var1[i] + var2[i]);
  }
}

/* andw 2,2,2 0xb7f13330 */
void
test_andw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andw */
    var0[i] = var1[i] & var2[i];
  }
}

/* andnw 2,2,2 0xb7f13350 */
void
test_andnw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnw */
    var0[i] = var1[i] & (~var2[i]);
  }
}

/* avgsw 2,2,2 0xb7f13370 */
void
test_avgsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsw */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* avguw 2,2,2 0xb7f13390 */
void
test_avguw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avguw */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* cmpeqw 2,2,2 0xb7f133b0 */
void
test_cmpeqw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeqw */
    var0[i] = (var1[i] == var2[i]) ? (~0) : 0;
  }
}

/* cmpgtsw 2,2,2 0xb7f133d0 */
void
test_cmpgtsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsw */
    var0[i] = (var1[i] > var2[i]) ? (~0) : 0;
  }
}

/* copyw 2,2,0 0xb7f133f0 */
void
test_copyw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyw */
    var0[i] = var1[i];
  }
}

/* maxsw 2,2,2 0xb7f13400 */
void
test_maxsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsw */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* maxuw 2,2,2 0xb7f13420 */
void
test_maxuw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxuw */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* minsw 2,2,2 0xb7f13440 */
void
test_minsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsw */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* minuw 2,2,2 0xb7f13460 */
void
test_minuw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minuw */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* mullw 2,2,2 0xb7f13480 */
void
test_mullw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mullw */
    var0[i] = (var1[i] * var2[i]) & 0xffff;
  }
}

/* mulhsw 2,2,2 0xb7f134a0 */
void
test_mulhsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsw */
    var0[i] = (var1[i] * var2[i]) >> 16;
  }
}

/* mulhuw 2,2,2 0xb7f134c0 */
void
test_mulhuw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhuw */
    var0[i] = (var1[i] * var2[i]) >> 16;
  }
}

/* orw 2,2,2 0xb7f134e0 */
void
test_orw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: orw */
    var0[i] = var1[i] | var2[i];
  }
}

/* shlw 2,2,2 0xb7f13500 */
void
test_shlw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shlw */
    var0[i] = var1[i] << var2[i];
  }
}

/* shrsw 2,2,2 0xb7f13520 */
void
test_shrsw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsw */
    var0[i] = var1[i] >> var2[i];
  }
}

/* shruw 2,2,2 0xb7f13540 */
void
test_shruw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shruw */
    var0[i] = var1[i] >> var2[i];
  }
}

/* signw 2,2,0 0xb7f13560 */
void
test_signw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: signw */
    var0[i] = ORC_CLAMP(var1[i],-1,1);
  }
}

/* subw 2,2,2 0xb7f13570 */
void
test_subw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subw */
    var0[i] = var1[i] - var2[i];
  }
}

/* subssw 2,2,2 0xb7f13590 */
void
test_subssw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssw */
    var0[i] = ORC_CLAMP_SW(var1[i] - var2[i]);
  }
}

/* subusw 2,2,2 0xb7f135a0 */
void
test_subusw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusw */
    var0[i] = ORC_CLAMP_UW(var1[i] - var2[i]);
  }
}

/* xorw 2,2,2 0xb7f135b0 */
void
test_xorw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorw */
    var0[i] = var1[i] ^ var2[i];
  }
}

/* absl 4,4,0 0xb7f135d0 */
void
test_absl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: absl */
    var0[i] = ORC_ABS(var1[i]);
  }
}

/* addl 4,4,4 0xb7f135f0 */
void
test_addl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addl */
    var0[i] = var1[i] + var2[i];
  }
}

/* addssl 4,4,4 0xb7f13600 */
void
test_addssl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssl */
    var0[i] = ORC_CLAMP_SL((int64_t)var1[i] + (int64_t)var2[i]);
  }
}

/* addusl 4,4,4 0xb7f13610 */
void
test_addusl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusl */
    var0[i] = ORC_CLAMP_UL((uint64_t)var1[i] + (uint64_t)var2[i]);
  }
}

/* andl 4,4,4 0xb7f13620 */
void
test_andl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andl */
    var0[i] = var1[i] & var2[i];
  }
}

/* andnl 4,4,4 0xb7f13630 */
void
test_andnl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnl */
    var0[i] = var1[i] & (~var2[i]);
  }
}

/* avgsl 4,4,4 0xb7f13650 */
void
test_avgsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsl */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* avgul 4,4,4 0xb7f13670 */
void
test_avgul (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgul */
    var0[i] = (var1[i] + var2[i] + 1)>>1;
  }
}

/* cmpeql 4,4,4 0xb7f13690 */
void
test_cmpeql (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeql */
    var0[i] = (var1[i] == var2[i]) ? (~0) : 0;
  }
}

/* cmpgtsl 4,4,4 0xb7f136b0 */
void
test_cmpgtsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsl */
    var0[i] = (var1[i] > var2[i]) ? (~0) : 0;
  }
}

/* copyl 4,4,0 0xb7f136d0 */
void
test_copyl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyl */
    var0[i] = var1[i];
  }
}

/* maxsl 4,4,4 0xb7f136e0 */
void
test_maxsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsl */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* maxul 4,4,4 0xb7f13700 */
void
test_maxul (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxul */
    var0[i] = ORC_MAX(var1[i], var2[i]);
  }
}

/* minsl 4,4,4 0xb7f13720 */
void
test_minsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsl */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* minul 4,4,4 0xb7f13740 */
void
test_minul (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: minul */
    var0[i] = ORC_MIN(var1[i], var2[i]);
  }
}

/* mulll 4,4,4 0xb7f13760 */
void
test_mulll (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulll */
    var0[i] = (var1[i] * var2[i]) & 0xffffffff;
  }
}

/* mulhsl 4,4,4 0xb7f13780 */
void
test_mulhsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsl */
    var0[i] = ((int64_t)var1[i] * (int64_t)var2[i]) >> 32;
  }
}

/* mulhul 4,4,4 0xb7f137a0 */
void
test_mulhul (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhul */
    var0[i] = ((uint64_t)var1[i] * (uint64_t)var2[i]) >> 32;
  }
}

/* orl 4,4,4 0xb7f137b0 */
void
test_orl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: orl */
    var0[i] = var1[i] | var2[i];
  }
}

/* shll 4,4,4 0xb7f137c0 */
void
test_shll (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shll */
    var0[i] = var1[i] << var2[i];
  }
}

/* shrsl 4,4,4 0xb7f137e0 */
void
test_shrsl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsl */
    var0[i] = var1[i] >> var2[i];
  }
}

/* shrul 4,4,4 0xb7f13800 */
void
test_shrul (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrul */
    var0[i] = var1[i] >> var2[i];
  }
}

/* signl 4,4,0 0xb7f13820 */
void
test_signl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: signl */
    var0[i] = ORC_CLAMP(var1[i],-1,1);
  }
}

/* subl 4,4,4 0xb7f13830 */
void
test_subl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subl */
    var0[i] = var1[i] - var2[i];
  }
}

/* subssl 4,4,4 0xb7f13840 */
void
test_subssl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssl */
    var0[i] = ORC_CLAMP_SL((int64_t)var1[i] - (int64_t)var2[i]);
  }
}

/* subusl 4,4,4 0xb7f13850 */
void
test_subusl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusl */
    var0[i] = ORC_CLAMP_UL((uint64_t)var1[i] - (uint64_t)var2[i]);
  }
}

/* xorl 4,4,4 0xb7f13860 */
void
test_xorl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int32_t *var1 = ex->arrays[1];
  int32_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorl */
    var0[i] = var1[i] ^ var2[i];
  }
}

/* convsbw 2,1,0 0xb7f12e20 */
void
test_convsbw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsbw */
    var0[i] = var1[i];
  }
}

/* convubw 2,1,0 0xb7f12e30 */
void
test_convubw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convubw */
    var0[i] = (uint8_t)var1[i];
  }
}

/* convswl 4,2,0 0xb7f12e40 */
void
test_convswl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convswl */
    var0[i] = var1[i];
  }
}

/* convuwl 4,2,0 0xb7f12e50 */
void
test_convuwl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuwl */
    var0[i] = (uint16_t)var1[i];
  }
}

/* convwb 1,2,0 0xb7f12e60 */
void
test_convwb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convwb */
    var0[i] = var1[i];
  }
}

/* convssswb 1,2,0 0xb7f12e70 */
void
test_convssswb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convssswb */
    var0[i] = ORC_CLAMP_SB(var1[i]);
  }
}

/* convsuswb 1,2,0 0xb7f12eb0 */
void
test_convsuswb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsuswb */
    var0[i] = ORC_CLAMP_UB(var1[i]);
  }
}

/* convusswb 1,2,0 0xb7f12ee0 */
void
test_convusswb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convusswb */
    var0[i] = ORC_CLAMP_SB(var1[i]);
  }
}

/* convuuswb 1,2,0 0xb7f12f00 */
void
test_convuuswb (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuuswb */
    var0[i] = ORC_CLAMP_UB(var1[i]);
  }
}

/* convlw 1,2,0 0xb7f12f20 */
void
test_convlw (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convlw */
    var0[i] = var1[i];
  }
}

/* convssslw 1,2,0 0xb7f12f30 */
void
test_convssslw (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convssslw */
    var0[i] = ORC_CLAMP_SW(var1[i]);
  }
}

/* convsuslw 1,2,0 0xb7f12f60 */
void
test_convsuslw (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsuslw */
    var0[i] = ORC_CLAMP_UW(var1[i]);
  }
}

/* convusslw 1,2,0 0xb7f12f90 */
void
test_convusslw (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convusslw */
    var0[i] = ORC_CLAMP_SW(var1[i]);
  }
}

/* convuuslw 1,2,0 0xb7f12fb0 */
void
test_convuuslw (OrcExecutor *ex)
{
  int i;
  int8_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuuslw */
    var0[i] = ORC_CLAMP_UW(var1[i]);
  }
}

/* mulsbw 2,1,1 0xb7f13870 */
void
test_mulsbw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulsbw */
    var0[i] = var1[i] * var2[i];
  }
}

/* mulubw 2,1,1 0xb7f13890 */
void
test_mulubw (OrcExecutor *ex)
{
  int i;
  int16_t *var0 = ex->arrays[0];
  int8_t *var1 = ex->arrays[1];
  int8_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulubw */
    var0[i] = var1[i] * var2[i];
  }
}

/* mulswl 4,2,2 0xb7f138b0 */
void
test_mulswl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulswl */
    var0[i] = var1[i] * var2[i];
  }
}

/* muluwl 4,2,2 0xb7f138d0 */
void
test_muluwl (OrcExecutor *ex)
{
  int i;
  int32_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];

  for (i = 0; i < ex->n; i++) {
    /* 0: muluwl */
    var0[i] = var1[i] * var2[i];
  }
}

