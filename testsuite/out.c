
/* begin Orc C target preamble */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#define ORC_RESTRICT restrict
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
#define ORC_SWAP_W(x) ((((x)&0xff)<<8) | (((x)&0xff00)>>8))
#define ORC_SWAP_L(x) ((((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)<<24))
/* end Orc C target preamble */

/* absb 1,1,0 0xb7ef6a90 */
void
test_absb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: absb */
    var0[i] = ORC_ABS(var4[i]);
  }
}

/* addb 1,1,1 0xb7ef6ab0 */
void
test_addb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addb */
    var0[i] = var4[i] + var5[i];
  }
}

/* addssb 1,1,1 0xb7ef6ad0 */
void
test_addssb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssb */
    var0[i] = ORC_CLAMP_SB(var4[i] + var5[i]);
  }
}

/* addusb 1,1,1 0xb7ef6b10 */
void
test_addusb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusb */
    var0[i] = ORC_CLAMP_UB(var4[i] + var5[i]);
  }
}

/* andb 1,1,1 0xb7ef6b40 */
void
test_andb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andb */
    var0[i] = var4[i] & var5[i];
  }
}

/* andnb 1,1,1 0xb7ef6b60 */
void
test_andnb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnb */
    var0[i] = var4[i] & (~var5[i]);
  }
}

/* avgsb 1,1,1 0xb7ef6b80 */
void
test_avgsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsb */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* avgub 1,1,1 0xb7ef6ba0 */
void
test_avgub (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgub */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* cmpeqb 1,1,1 0xb7ef6bc0 */
void
test_cmpeqb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeqb */
    var0[i] = (var4[i] == var5[i]) ? (~0) : 0;
  }
}

/* cmpgtsb 1,1,1 0xb7ef6be0 */
void
test_cmpgtsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsb */
    var0[i] = (var4[i] > var5[i]) ? (~0) : 0;
  }
}

/* copyb 1,1,0 0xb7ef6c00 */
void
test_copyb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyb */
    var0[i] = var4[i];
  }
}

/* maxsb 1,1,1 0xb7ef6c10 */
void
test_maxsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsb */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* maxub 1,1,1 0xb7ef6c30 */
void
test_maxub (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxub */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* minsb 1,1,1 0xb7ef6c50 */
void
test_minsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsb */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* minub 1,1,1 0xb7ef6c70 */
void
test_minub (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minub */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* mullb 1,1,1 0xb7ef6c90 */
void
test_mullb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mullb */
    var0[i] = (var4[i] * var5[i]) & 0xff;
  }
}

/* mulhsb 1,1,1 0xb7ef6cb0 */
void
test_mulhsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsb */
    var0[i] = (var4[i] * var5[i]) >> 8;
  }
}

/* mulhub 1,1,1 0xb7ef6cd0 */
void
test_mulhub (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhub */
    var0[i] = (var4[i] * var5[i]) >> 8;
  }
}

/* orb 1,1,1 0xb7ef6cf0 */
void
test_orb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: orb */
    var0[i] = var4[i] | var5[i];
  }
}

/* shlb 1,1,1 0xb7ef6d10 */
void
test_shlb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shlb */
    var0[i] = var4[i] << var5[i];
  }
}

/* shrsb 1,1,1 0xb7ef6d30 */
void
test_shrsb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsb */
    var0[i] = var4[i] >> var5[i];
  }
}

/* shrub 1,1,1 0xb7ef6d50 */
void
test_shrub (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrub */
    var0[i] = ((uint8_t)var4[i]) >> var5[i];
  }
}

/* signb 1,1,0 0xb7ef6d70 */
void
test_signb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: signb */
    var0[i] = ORC_CLAMP(var4[i],-1,1);
  }
}

/* subb 1,1,1 0xb7ef6da0 */
void
test_subb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subb */
    var0[i] = var4[i] - var5[i];
  }
}

/* subssb 1,1,1 0xb7ef6dc0 */
void
test_subssb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssb */
    var0[i] = ORC_CLAMP_SB(var4[i] - var5[i]);
  }
}

/* subusb 1,1,1 0xb7ef6df0 */
void
test_subusb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusb */
    var0[i] = ORC_CLAMP_UB((uint8_t)var4[i] - (uint8_t)var5[i]);
  }
}

/* xorb 1,1,1 0xb7ef6e30 */
void
test_xorb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorb */
    var0[i] = var4[i] ^ var5[i];
  }
}

/* absw 2,2,0 0xb7ef6e50 */
void
test_absw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: absw */
    var0[i] = ORC_ABS(var4[i]);
  }
}

/* addw 2,2,2 0xb7ef6e70 */
void
test_addw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addw */
    var0[i] = var4[i] + var5[i];
  }
}

/* addssw 2,2,2 0xb7ef6e90 */
void
test_addssw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssw */
    var0[i] = ORC_CLAMP_SW(var4[i] + var5[i]);
  }
}

/* addusw 2,2,2 0xb7ef6ed0 */
void
test_addusw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusw */
    var0[i] = ORC_CLAMP_UW(var4[i] + var5[i]);
  }
}

/* andw 2,2,2 0xb7ef6f00 */
void
test_andw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andw */
    var0[i] = var4[i] & var5[i];
  }
}

/* andnw 2,2,2 0xb7ef6f20 */
void
test_andnw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnw */
    var0[i] = var4[i] & (~var5[i]);
  }
}

/* avgsw 2,2,2 0xb7ef6f40 */
void
test_avgsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsw */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* avguw 2,2,2 0xb7ef6f60 */
void
test_avguw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avguw */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* cmpeqw 2,2,2 0xb7ef6f80 */
void
test_cmpeqw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeqw */
    var0[i] = (var4[i] == var5[i]) ? (~0) : 0;
  }
}

/* cmpgtsw 2,2,2 0xb7ef6fa0 */
void
test_cmpgtsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsw */
    var0[i] = (var4[i] > var5[i]) ? (~0) : 0;
  }
}

/* copyw 2,2,0 0xb7ef6fc0 */
void
test_copyw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyw */
    var0[i] = var4[i];
  }
}

/* maxsw 2,2,2 0xb7ef6fd0 */
void
test_maxsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsw */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* maxuw 2,2,2 0xb7ef6ff0 */
void
test_maxuw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxuw */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* minsw 2,2,2 0xb7ef7010 */
void
test_minsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsw */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* minuw 2,2,2 0xb7ef7030 */
void
test_minuw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minuw */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* mullw 2,2,2 0xb7ef7050 */
void
test_mullw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mullw */
    var0[i] = (var4[i] * var5[i]) & 0xffff;
  }
}

/* mulhsw 2,2,2 0xb7ef7070 */
void
test_mulhsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsw */
    var0[i] = (var4[i] * var5[i]) >> 16;
  }
}

/* mulhuw 2,2,2 0xb7ef7090 */
void
test_mulhuw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhuw */
    var0[i] = (var4[i] * var5[i]) >> 16;
  }
}

/* orw 2,2,2 0xb7ef70b0 */
void
test_orw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: orw */
    var0[i] = var4[i] | var5[i];
  }
}

/* shlw 2,2,2 0xb7ef70d0 */
void
test_shlw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shlw */
    var0[i] = var4[i] << var5[i];
  }
}

/* shrsw 2,2,2 0xb7ef70f0 */
void
test_shrsw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsw */
    var0[i] = var4[i] >> var5[i];
  }
}

/* shruw 2,2,2 0xb7ef7110 */
void
test_shruw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shruw */
    var0[i] = var4[i] >> var5[i];
  }
}

/* signw 2,2,0 0xb7ef7130 */
void
test_signw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: signw */
    var0[i] = ORC_CLAMP(var4[i],-1,1);
  }
}

/* subw 2,2,2 0xb7ef7160 */
void
test_subw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subw */
    var0[i] = var4[i] - var5[i];
  }
}

/* subssw 2,2,2 0xb7ef7180 */
void
test_subssw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssw */
    var0[i] = ORC_CLAMP_SW(var4[i] - var5[i]);
  }
}

/* subusw 2,2,2 0xb7ef71c0 */
void
test_subusw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusw */
    var0[i] = ORC_CLAMP_UW(var4[i] - var5[i]);
  }
}

/* xorw 2,2,2 0xb7ef7200 */
void
test_xorw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorw */
    var0[i] = var4[i] ^ var5[i];
  }
}

/* absl 4,4,0 0xb7ef7220 */
void
test_absl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: absl */
    var0[i] = ORC_ABS(var4[i]);
  }
}

/* addl 4,4,4 0xb7ef7240 */
void
test_addl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addl */
    var0[i] = var4[i] + var5[i];
  }
}

/* addssl 4,4,4 0xb7ef7250 */
void
test_addssl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addssl */
    var0[i] = ORC_CLAMP_SL((int64_t)var4[i] + (int64_t)var5[i]);
  }
}

/* addusl 4,4,4 0xb7ef72d0 */
void
test_addusl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: addusl */
    var0[i] = ORC_CLAMP_UL((uint64_t)var4[i] + (uint64_t)var5[i]);
  }
}

/* andl 4,4,4 0xb7ef7310 */
void
test_andl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andl */
    var0[i] = var4[i] & var5[i];
  }
}

/* andnl 4,4,4 0xb7ef7320 */
void
test_andnl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: andnl */
    var0[i] = var4[i] & (~var5[i]);
  }
}

/* avgsl 4,4,4 0xb7ef7340 */
void
test_avgsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgsl */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* avgul 4,4,4 0xb7ef7390 */
void
test_avgul (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: avgul */
    var0[i] = (var4[i] + var5[i] + 1)>>1;
  }
}

/* cmpeql 4,4,4 0xb7ef73d0 */
void
test_cmpeql (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpeql */
    var0[i] = (var4[i] == var5[i]) ? (~0) : 0;
  }
}

/* cmpgtsl 4,4,4 0xb7ef73f0 */
void
test_cmpgtsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: cmpgtsl */
    var0[i] = (var4[i] > var5[i]) ? (~0) : 0;
  }
}

/* copyl 4,4,0 0xb7ef7410 */
void
test_copyl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: copyl */
    var0[i] = var4[i];
  }
}

/* maxsl 4,4,4 0xb7ef7420 */
void
test_maxsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxsl */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* maxul 4,4,4 0xb7ef7440 */
void
test_maxul (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: maxul */
    var0[i] = ORC_MAX(var4[i], var5[i]);
  }
}

/* minsl 4,4,4 0xb7ef7460 */
void
test_minsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minsl */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* minul 4,4,4 0xb7ef7480 */
void
test_minul (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: minul */
    var0[i] = ORC_MIN(var4[i], var5[i]);
  }
}

/* mulll 4,4,4 0xb7ef74a0 */
void
test_mulll (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulll */
    var0[i] = (var4[i] * var5[i]) & 0xffffffff;
  }
}

/* mulhsl 4,4,4 0xb7ef74c0 */
void
test_mulhsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhsl */
    var0[i] = ((int64_t)var4[i] * (int64_t)var5[i]) >> 32;
  }
}

/* mulhul 4,4,4 0xb7ef74e0 */
void
test_mulhul (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulhul */
    var0[i] = ((uint64_t)var4[i] * (uint64_t)var5[i]) >> 32;
  }
}

/* orl 4,4,4 0xb7ef74f0 */
void
test_orl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: orl */
    var0[i] = var4[i] | var5[i];
  }
}

/* shll 4,4,4 0xb7ef7500 */
void
test_shll (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shll */
    var0[i] = var4[i] << var5[i];
  }
}

/* shrsl 4,4,4 0xb7ef7520 */
void
test_shrsl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrsl */
    var0[i] = var4[i] >> var5[i];
  }
}

/* shrul 4,4,4 0xb7ef7540 */
void
test_shrul (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: shrul */
    var0[i] = ((uint32_t)var4[i]) >> var5[i];
  }
}

/* signl 4,4,0 0xb7ef7560 */
void
test_signl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: signl */
    var0[i] = ORC_CLAMP(var4[i],-1,1);
  }
}

/* subl 4,4,4 0xb7ef7580 */
void
test_subl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subl */
    var0[i] = var4[i] - var5[i];
  }
}

/* subssl 4,4,4 0xb7ef7590 */
void
test_subssl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subssl */
    var0[i] = ORC_CLAMP_SL((int64_t)var4[i] - (int64_t)var5[i]);
  }
}

/* subusl 4,4,4 0xb7ef7610 */
void
test_subusl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: subusl */
    var0[i] = ORC_CLAMP_UL((uint64_t)var4[i] - (uint64_t)var5[i]);
  }
}

/* xorl 4,4,4 0xb7ef7630 */
void
test_xorl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: xorl */
    var0[i] = var4[i] ^ var5[i];
  }
}

/* convsbw 2,1,0 0xb7ef68e0 */
void
test_convsbw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsbw */
    var0[i] = var4[i];
  }
}

/* convubw 2,1,0 0xb7ef68f0 */
void
test_convubw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convubw */
    var0[i] = (uint8_t)var4[i];
  }
}

/* convswl 4,2,0 0xb7ef6900 */
void
test_convswl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convswl */
    var0[i] = var4[i];
  }
}

/* convuwl 4,2,0 0xb7ef6910 */
void
test_convuwl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuwl */
    var0[i] = (uint16_t)var4[i];
  }
}

/* convwb 1,2,0 0xb7ef6920 */
void
test_convwb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convwb */
    var0[i] = var4[i];
  }
}

/* convssswb 1,2,0 0xb7ef6930 */
void
test_convssswb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convssswb */
    var0[i] = ORC_CLAMP_SB(var4[i]);
  }
}

/* convsuswb 1,2,0 0xb7ef6970 */
void
test_convsuswb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsuswb */
    var0[i] = ORC_CLAMP_UB(var4[i]);
  }
}

/* convusswb 1,2,0 0xb7ef69a0 */
void
test_convusswb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convusswb */
    var0[i] = ORC_CLAMP_SB(var4[i]);
  }
}

/* convuuswb 1,2,0 0xb7ef69c0 */
void
test_convuuswb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuuswb */
    var0[i] = ORC_CLAMP_UB(var4[i]);
  }
}

/* convlw 2,4,0 0xb7ef69e0 */
void
test_convlw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convlw */
    var0[i] = var4[i];
  }
}

/* convssslw 2,4,0 0xb7ef69f0 */
void
test_convssslw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convssslw */
    var0[i] = ORC_CLAMP_SW(var4[i]);
  }
}

/* convsuslw 2,4,0 0xb7ef6a20 */
void
test_convsuslw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convsuslw */
    var0[i] = ORC_CLAMP_UW(var4[i]);
  }
}

/* convusslw 2,4,0 0xb7ef6a50 */
void
test_convusslw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convusslw */
    var0[i] = ORC_CLAMP_SW(var4[i]);
  }
}

/* convuuslw 2,4,0 0xb7ef6a70 */
void
test_convuuslw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: convuuslw */
    var0[i] = ORC_CLAMP_UW(var4[i]);
  }
}

/* mulsbw 2,1,1 0xb7ef7640 */
void
test_mulsbw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulsbw */
    var0[i] = var4[i] * var5[i];
  }
}

/* mulubw 2,1,1 0xb7ef7660 */
void
test_mulubw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulubw */
    var0[i] = var4[i] * var5[i];
  }
}

/* mulswl 4,2,2 0xb7ef7680 */
void
test_mulswl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mulswl */
    var0[i] = var4[i] * var5[i];
  }
}

/* muluwl 4,2,2 0xb7ef76a0 */
void
test_muluwl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: muluwl */
    var0[i] = var4[i] * var5[i];
  }
}

/* accw 2,2,0 0xb7ef76c0 */
void
test_accw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var12 = ex->arrays[12];

  for (i = 0; i < ex->n; i++) {
    /* 0: accw */
    var12[i] = var12[i] + var4[i];
  }
}

/* accl 4,4,0 0xb7ef76d0 */
void
test_accl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var4 = ex->arrays[4];
  int32_t * restrict var12 = ex->arrays[12];

  for (i = 0; i < ex->n; i++) {
    /* 0: accl */
    var12[i] = var12[i] + var4[i];
  }
}

/* accsadubl 4,1,1 0xb7ef77c0 */
void
test_accsadubl (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];
  int32_t * restrict var12 = ex->arrays[12];

  for (i = 0; i < ex->n; i++) {
    /* 0: accsadubl */
    var12[i] = var12[i] + ORC_ABS(var4[i] - var5[i]);
  }
}

/* swapw 2,2,0 0xb7ef76e0 */
void
test_swapw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: swapw */
    var0[i] = ORC_SWAP_W(var4[i]);
  }
}

/* swapl 4,4,0 0xb7ef7700 */
void
test_swapl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: swapl */
    var0[i] = ORC_SWAP_L(var4[i]);
  }
}

/* select0wb 1,2,0 0xb7ef7760 */
void
test_select0wb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: select0wb */
    var0[i] = (var4[i] >> 8)&0xff;
  }
}

/* select1wb 1,2,0 0xb7ef7770 */
void
test_select1wb (OrcExecutor *ex)
{
  int i;
  int8_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: select1wb */
    var0[i] = var4[i] & 0xff;
  }
}

/* select0lw 2,4,0 0xb7ef7740 */
void
test_select0lw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: select0lw */
    var0[i] = (var4[i] >> 16)&0xffff;
  }
}

/* select1lw 2,4,0 0xb7ef7750 */
void
test_select1lw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int32_t * restrict var4 = ex->arrays[4];

  for (i = 0; i < ex->n; i++) {
    /* 0: select1lw */
    var0[i] = var4[i] & 0xffff;
  }
}

/* mergewl 4,2,2 0xb7ef7780 */
void
test_mergewl (OrcExecutor *ex)
{
  int i;
  int32_t * restrict var0 = ex->arrays[0];
  int16_t * restrict var4 = ex->arrays[4];
  int16_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mergewl */
    var0[i] = (var4[i] << 16) | (var5[i]);
  }
}

/* mergebw 2,1,1 0xb7ef77a0 */
void
test_mergebw (OrcExecutor *ex)
{
  int i;
  int16_t * restrict var0 = ex->arrays[0];
  int8_t * restrict var4 = ex->arrays[4];
  int8_t * restrict var5 = ex->arrays[5];

  for (i = 0; i < ex->n; i++) {
    /* 0: mergebw */
    var0[i] = (var4[i] << 8) | (var5[i]);
  }
}

