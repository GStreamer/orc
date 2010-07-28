
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orcopcode
 * @title: OrcOpcode
 * @short_description: Operations
 */


static OrcOpcodeSet *opcode_sets;
static int n_opcode_sets;

static OrcTarget *targets[ORC_N_TARGETS];
static int n_targets;

static OrcTarget *default_target;

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
#define ORC_UL_MAX 4294967295ULL
#define ORC_UL_MIN 0

#define ORC_CLAMP_SB(x) ORC_CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_UB_MAX)
#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SW_MAX)
#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_UW_MAX)
#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SL_MAX)
#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_UL_MAX)


void
orc_target_register (OrcTarget *target)
{
  targets[n_targets] = target;
  n_targets++;

  if (target->executable) {
    default_target = target;
  }
}

OrcTarget *
orc_target_get_by_name (const char *name)
{
  int i;

  for(i=0;i<n_targets;i++){
    if (strcmp (name, targets[i]->name) == 0) {
      return targets[i];
    }
  }

  return NULL;
}

OrcTarget *
orc_target_get_default (void)
{
  return default_target;
}

const char *
orc_target_get_name (OrcTarget *target)
{
  if (target == NULL) return NULL;
  return target->name;
}

unsigned int
orc_target_get_default_flags (OrcTarget *target)
{
  if (target == NULL) return 0;
  return target->get_default_flags();
}

const char *
orc_target_get_preamble (OrcTarget *target)
{
  if (target->get_asm_preamble == NULL) return "";

  return target->get_asm_preamble ();
}

const char *
orc_target_get_asm_preamble (const char *target)
{
  OrcTarget *t;

  t = orc_target_get_by_name (target);
  if (t == NULL) return "";

  return orc_target_get_preamble (t);
}

const char *
orc_target_get_flag_name (OrcTarget *target, int shift)
{
  if (target->get_flag_name == NULL) return "";

  return target->get_flag_name (shift);
}

#if 0
int
orc_opcode_get_list (OrcOpcode **list)
{
  (*list) = opcode_list;
  return n_opcodes;
}
#endif

#if 0
void
orc_opcode_register (const char *name, int n_dest, int n_src,
    OrcOpcodeEmulateFunc emulate, void *user)
{
  OrcOpcode *opcode;

  if (n_opcodes == n_opcodes_alloc) {
    n_opcodes_alloc += 100;
    opcode_list = realloc(opcode_list, sizeof(OrcOpcode) * n_opcodes_alloc);
  }

  opcode = opcode_list + n_opcodes;

  opcode->name = strdup (name);
  opcode->n_src = n_src;
  opcode->n_dest = n_dest;
  opcode->emulate = emulate;
  opcode->emulate_user = user;

  n_opcodes++;
}
#endif

OrcRuleSet *
orc_rule_set_new (OrcOpcodeSet *opcode_set, OrcTarget *target,
    unsigned int required_flags)
{
  OrcRuleSet *rule_set;

  rule_set = target->rule_sets + target->n_rule_sets;
  target->n_rule_sets++;

  memset (rule_set, 0, sizeof(OrcRuleSet));

  rule_set->opcode_major = opcode_set->opcode_major;
  rule_set->required_target_flags = required_flags;

  rule_set->rules = malloc (sizeof(OrcRule) * opcode_set->n_opcodes);
  memset (rule_set->rules, 0, sizeof(OrcRule) * opcode_set->n_opcodes);

  return rule_set;
}

OrcRule *
orc_target_get_rule (OrcTarget *target, OrcStaticOpcode *opcode,
    unsigned int target_flags)
{
  OrcRule *rule;
  int i;
  int j;
  int k;

  for(k=0;k<n_opcode_sets;k++){
    j = opcode - opcode_sets[k].opcodes;

    if (j < 0 || j >= opcode_sets[k].n_opcodes) continue;
    if (opcode_sets[k].opcodes + j != opcode) continue;

    for(i=0;i<target->n_rule_sets;i++){
      if (target->rule_sets[i].opcode_major != opcode_sets[k].opcode_major) continue;
      if (target->rule_sets[i].required_target_flags & (~target_flags)) continue;

      rule = target->rule_sets[i].rules + j;
      if (rule->emit) return rule;
    }
  }

  return NULL;
}

int
orc_opcode_register_static (OrcStaticOpcode *sopcode, char *prefix)
{
  int n;
  int major;

  n = 0;
  while (sopcode[n].name[0]) {
    n++;
  }

  major = n_opcode_sets;

  n_opcode_sets++;
  opcode_sets = realloc (opcode_sets, sizeof(OrcOpcodeSet)*n_opcode_sets);
  
  memset (opcode_sets + major, 0, sizeof(OrcOpcodeSet));
  strncpy(opcode_sets[major].prefix, prefix, sizeof(opcode_sets[major].prefix)-1);
  opcode_sets[major].n_opcodes = n;
  opcode_sets[major].opcodes = sopcode;
  opcode_sets[major].opcode_major = major;

  return major;
}

OrcOpcodeSet *
orc_opcode_set_get (const char *name)
{
  int i;

  for(i=0;i<n_opcode_sets;i++){
    if (strcmp (opcode_sets[i].prefix, name) == 0) {
      return opcode_sets + i;
    }
  }

  return NULL;
}

OrcOpcodeSet *
orc_opcode_set_get_nth (int opcode_major)
{
  return opcode_sets + opcode_major;
}

int
orc_opcode_set_find_by_name (OrcOpcodeSet *opcode_set, const char *name)
{
  int j;

  for(j=0;j<opcode_set->n_opcodes;j++){
    if (strcmp (name, opcode_set->opcodes[j].name) == 0) {
      return j;
    }
  }

  return -1;
}

OrcStaticOpcode *
orc_opcode_find_by_name (const char *name)
{
  int i;
  int j;

  for(i=0;i<n_opcode_sets;i++){
    j = orc_opcode_set_find_by_name (opcode_sets + i, name);
    if (j >= 0) {
      return &opcode_sets[i].opcodes[j];
    }
  }

  return NULL;
}

static void
convsbw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_int8)(ex->src_values[0]);
}

static void
convubw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_uint8)(ex->src_values[0]);
}

static void
convswl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_int16)(ex->src_values[0]);
}

static void
convuwl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_uint16)(ex->src_values[0]);
}

static void
convwb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_int16)(ex->src_values[0]);
}

static void
convssswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_SB((orc_int16)(ex->src_values[0]));
}

static void
convsuswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_UB((orc_int16)(ex->src_values[0]));
}

static void
convusswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = MIN((int)(orc_uint16)(ex->src_values[0]), ORC_SB_MAX);
}

static void
convuuswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = MIN((int)(orc_uint16)(ex->src_values[0]), ORC_UB_MAX);
}

static void
convlw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (orc_int32)(ex->src_values[0]);
}

static void
convssslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_SW((orc_int32)(ex->src_values[0]));
}

static void
convsuslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_UW((orc_int32)(ex->src_values[0]));
}

static void
convusslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_SW((orc_uint32)(ex->src_values[0]));
}

static void
convuuslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_CLAMP_UW((orc_uint32)(ex->src_values[0]));
}

#define UNARY(name,type,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  int a = ex->src_values[0]; \
  ex->dest_values[0] = ( type )( code ); \
}

#define BINARY(name,type,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  int a = (type) ex->src_values[0]; \
  int b = (type) ex->src_values[1]; \
  ex->dest_values[0] = ( type )( code ); \
}

#define UNARY_SB(name,code) UNARY(name, orc_int8, code)
#define BINARY_SB(name,code) BINARY(name, orc_int8, code)
#define BINARY_UB(name,code) BINARY(name, orc_uint8, code)

#define UNARY_SW(name,code) UNARY(name, orc_int16, code)
#define BINARY_SW(name,code) BINARY(name, orc_int16, code)
#define BINARY_UW(name,code) BINARY(name, orc_uint16, code)

#define UNARY_SL(name,code) UNARY(name, orc_int32, code)
#define BINARY_SL(name,code) BINARY(name, orc_int32, code)
#define BINARY_UL(name,code) BINARY(name, orc_uint32, code)

UNARY_SB(absb, (a<0)?-a:a)
BINARY_SB(addb, a + b)
BINARY_SB(addssb, ORC_CLAMP_SB(a + b))
BINARY_UB(addusb, ORC_CLAMP_UB(a + b))
BINARY_SB(andb, a & b)
BINARY_SB(andnb, (~a) & b)
BINARY_SB(avgsb, (a + b + 1)>>1)
BINARY_UB(avgub, (a + b + 1)>>1)
BINARY_SB(cmpeqb, (a == b) ? (~0) : 0)
BINARY_SB(cmpgtsb, (a > b) ? (~0) : 0)
UNARY_SB(copyb, a)
BINARY_SB(maxsb, (a > b) ? a : b)
BINARY_UB(maxub, (a > b) ? a : b)
BINARY_SB(minsb, (a < b) ? a : b)
BINARY_UB(minub, (a < b) ? a : b)
BINARY_SB(mullb, (a * b) & 0xff)
BINARY_SB(mulhsb, (a * b) >> 8)
BINARY_UB(mulhub, (a * b) >> 8)
BINARY_SB(orb, a | b)
BINARY_SB(shlb, a << b)
BINARY_SB(shrsb, a >> b)
BINARY_UB(shrub, (a) >> b)
UNARY_SB(signb, ORC_CLAMP(a,-1,1))
BINARY_SB(subb, a - b)
BINARY_SB(subssb, ORC_CLAMP_SB(a - b))
BINARY_UB(subusb, ORC_CLAMP_UB(a - b))
BINARY_SB(xorb, a ^ b)

UNARY_SW(absw, (a<0)?-a:a)
BINARY_SW(addw, a + b)
BINARY_SW(addssw, ORC_CLAMP_SW(a + b))
BINARY_UW(addusw, ORC_CLAMP_UW(a + b))
BINARY_SW(andw, a & b)
BINARY_SW(andnw, (~a) & b)
BINARY_SW(avgsw, (a + b + 1)>>1)
BINARY_UW(avguw, (a + b + 1)>>1)
BINARY_SW(cmpeqw, (a == b) ? (~0) : 0)
BINARY_SW(cmpgtsw, (a > b) ? (~0) : 0)
UNARY_SW(copyw, a)
BINARY_SW(maxsw, (a > b) ? a : b)
BINARY_UW(maxuw, (a > b) ? a : b)
BINARY_SW(minsw, (a < b) ? a : b)
BINARY_UW(minuw, (a < b) ? a : b)
BINARY_SW(mullw, (a * b) & 0xffff)
BINARY_SW(mulhsw, (a * b) >> 16)
BINARY_UW(mulhuw, (a * b) >> 16)
BINARY_SW(orw, a | b)
BINARY_SW(shlw, a << b)
BINARY_SW(shrsw, a >> b)
BINARY_UW(shruw, a >> b)
UNARY_SW(signw, ORC_CLAMP(a,-1,1))
BINARY_SW(subw, a - b)
BINARY_SW(subssw, ORC_CLAMP_SW(a - b))
BINARY_UW(subusw, ORC_CLAMP_UW(a - b))
BINARY_SW(xorw, a ^ b)

UNARY_SL(absl, (a<0)?-a:a)
BINARY_SL(addl, a + b)
BINARY_SL(addssl, ORC_CLAMP_SL((orc_int64)a + (orc_int64)b))
BINARY_UL(addusl, ORC_CLAMP_UL((orc_uint64)(orc_uint32)a + (orc_uint64)(orc_uint32)b))
BINARY_SL(andl, a & b)
BINARY_SL(andnl, (~a) & b)
BINARY_SL(avgsl, ((orc_int64)a + (orc_int64)b + 1)>>1)
BINARY_UL(avgul, ((orc_uint64)(orc_uint32)a + (orc_uint64)(orc_uint32)b + 1)>>1)
BINARY_SL(cmpeql, (a == b) ? (~0) : 0)
BINARY_SL(cmpgtsl, (a > b) ? (~0) : 0)
UNARY_SL(copyl, a)
BINARY_SL(maxsl, (a > b) ? a : b)
BINARY_UL(maxul, ((orc_uint32)a > (orc_uint32)b) ? a : b)
BINARY_SL(minsl, (a < b) ? a : b)
BINARY_UL(minul, ((orc_uint32)a < (orc_uint32)b) ? a : b)
BINARY_SL(mulll, (a * b) & 0xffffffff)
BINARY_SL(mulhsl, ((orc_int64)a * (orc_int64)b) >> 32)
BINARY_UL(mulhul, ((orc_uint64)a * (orc_uint64)b) >> 32)
BINARY_SL(orl, a | b)
BINARY_SL(shll, a << b)
BINARY_SL(shrsl, a >> b)
BINARY_UL(shrul, ((orc_uint32)a) >> b)
UNARY_SL(signl, ORC_CLAMP(a,-1,1))
BINARY_SL(subl, a - b)
BINARY_SL(subssl, ORC_CLAMP_SL((orc_int64)a - (orc_int64)b))
BINARY_UL(subusl, (((orc_uint32)a) < ((orc_uint32)b)) ? 0 : a - b)
BINARY_SL(xorl, a ^ b)


#define MUL(name, type1, type2) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  ex->dest_values[0] = ((type2)(type1)ex->src_values[0]) * \
    ((type2)(type1)ex->src_values[1]); \
}

MUL(mulsbw, orc_int8, orc_int16)
MUL(mulubw, orc_uint8, orc_uint16)
MUL(mulswl, orc_int16, orc_int32)
MUL(muluwl, orc_uint16, orc_uint32)
#ifdef ENABLE_INT64
MUL(mulslq, orc_int32, orc_int64)
MUL(mululq, orc_uint32, orc_uint64)
#endif

#define ACC(name, type1) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  ex->dest_values[0] = ((type1)ex->src_values[0]); \
}

ACC(accw, orc_int16);
ACC(accl, orc_int32);

static void
swapw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ((ex->src_values[0]&0xff)<<8)|
    ((ex->src_values[0]&0xff00)>>8);
}

static void
swapl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ((ex->src_values[0]&0xff)<<24)|
    ((ex->src_values[0]&0xff00)<<8)|
    ((ex->src_values[0]&0xff0000)>>8)|
    ((ex->src_values[0]&0xff000000)>>24);
}

static void
select0lw (OrcOpcodeExecutor *ex, void *user)
{
#if WORDS_BIGENDIAN
  ex->dest_values[0] = (ex->src_values[0]>>16)&0xffff;
#else
  ex->dest_values[0] = ex->src_values[0]&0xffff;
#endif
}

static void
select1lw (OrcOpcodeExecutor *ex, void *user)
{
#if WORDS_BIGENDIAN
  ex->dest_values[0] = ex->src_values[0]&0xffff;
#else
  ex->dest_values[0] = (ex->src_values[0]>>16)&0xffff;
#endif
}

static void
select0wb (OrcOpcodeExecutor *ex, void *user)
{
#if WORDS_BIGENDIAN
  ex->dest_values[0] = (ex->src_values[0]>>8)&0xff;
#else
  ex->dest_values[0] = ex->src_values[0]&0xff;
#endif
}

static void
select1wb (OrcOpcodeExecutor *ex, void *user)
{
#if WORDS_BIGENDIAN
  ex->dest_values[0] = ex->src_values[0]&0xff;
#else
  ex->dest_values[0] = (ex->src_values[0]>>8)&0xff;
#endif
}

static void
splitlw (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (ex->src_values[0] >> 16) & 0xffff;
  ex->dest_values[1] = ex->src_values[0] & 0xffff;
}

static void
splitwb (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = (ex->src_values[0] >> 8) & 0xff;
  ex->dest_values[1] = ex->src_values[0] & 0xff;
}

static void
mergewl (OrcOpcodeExecutor *ex, void *user)
{
  union {
    orc_uint16 u16[2];
    orc_uint32 u32;
  } val;
  val.u16[0] = ex->src_values[0];
  val.u16[1] = ex->src_values[1];
  ex->dest_values[0] = val.u32;
}

static void
mergebw (OrcOpcodeExecutor *ex, void *user)
{
  union {
    orc_uint8 u8[2];
    orc_uint16 u16;
  } val;
  val.u8[0] = ex->src_values[0];
  val.u8[1] = ex->src_values[1];
  ex->dest_values[0] = val.u16;
}

static void
accsadubl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = abs((int)((orc_uint8)ex->src_values[0]) -
      (int)((orc_uint8)ex->src_values[1]));
}

/* float ops */

static float
ORC_FLOAT_READ(void *addr)
{
  union {
    float f;
    unsigned int i;
  } x;
  x.i = *(unsigned int *)(addr);
  return x.f;
}

static void
ORC_FLOAT_WRITE(void *addr, float value)
{
  union {
    float f;
    unsigned int i;
  } x;
  x.f = value;
  *(unsigned int *)(addr) = x.i;
}

#if 0
/* Oh noes!  Aliasing rules! */
#define ORC_FLOAT_READ(addr) (*(float *)(addr))
#define ORC_FLOAT_WRITE(addr,value) do{ (*(float *)(addr)) = (value); }while(0
#endif

#define UNARY_F(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  float a = ORC_FLOAT_READ(&ex->src_values[0]); \
  ORC_FLOAT_WRITE(&ex->dest_values[0], code ); \
}

#define BINARY_F(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  void *pa = &ex->src_values[0]; \
  void *pb = &ex->src_values[1]; \
  float a = ORC_FLOAT_READ(pa); \
  float b = ORC_FLOAT_READ(pb); \
  ORC_FLOAT_WRITE(&ex->dest_values[0], code ); \
}

#define BINARY_FL(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  float a = ORC_FLOAT_READ(&ex->src_values[0]); \
  float b = ORC_FLOAT_READ(&ex->src_values[1]); \
  ex->dest_values[0] = code ; \
}

#define MIN_NONDENORMAL (1.1754944909521339405e-38)
#define FLUSH_DENORMALS
#ifdef FLUSH_DENORMALS
#define ORC_DENORMAL(x) \
  (((x) > -MIN_NONDENORMAL && (x) < MIN_NONDENORMAL) ? ((x)<0 ? (-0.0f) : (0.0f)) : (x))
#else
#define ORC_DENORMAL(x) (x)
#endif

BINARY_F(addf, ORC_DENORMAL(ORC_DENORMAL(a) + ORC_DENORMAL(b)))
BINARY_F(subf, ORC_DENORMAL(ORC_DENORMAL(a) - ORC_DENORMAL(b)))
BINARY_F(mulf, ORC_DENORMAL(ORC_DENORMAL(a) * ORC_DENORMAL(b)))
BINARY_F(divf, ORC_DENORMAL(ORC_DENORMAL(a) / ORC_DENORMAL(b)))
UNARY_F(orc_sqrtf, sqrt(ORC_DENORMAL(a)) )
#if 1
/* NEON style maxf/minf */
BINARY_F(maxf, isnan(a) ? a : isnan(b) ? b : (a>b) ? ORC_DENORMAL(a) : ORC_DENORMAL(b))
BINARY_F(minf, isnan(a) ? a : isnan(b) ? b : (a<b) ? ORC_DENORMAL(a) : ORC_DENORMAL(b))
#else
/* SSE style maxf/minf */
BINARY_F(maxf, (a>b) ? ORC_DENORMAL(a) : ORC_DENORMAL(b))
BINARY_F(minf, (a<b) ? ORC_DENORMAL(a) : ORC_DENORMAL(b))
#endif

BINARY_FL(cmpeqf, (ORC_DENORMAL(a) == ORC_DENORMAL(b)) ? (~0) : 0)
BINARY_FL(cmpltf, ((ORC_DENORMAL(a) < ORC_DENORMAL(b)) && (ORC_DENORMAL(b) > ORC_DENORMAL(a))) ? (~0) : 0)
BINARY_FL(cmplef, ((ORC_DENORMAL(a) <= ORC_DENORMAL(b)) && (ORC_DENORMAL(b) >= ORC_DENORMAL(a))) ? (~0) : 0)

static void
convfl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_FLOAT_READ(&ex->src_values[0]);
}

static void
convlf (OrcOpcodeExecutor *ex, void *user)
{
  ORC_FLOAT_WRITE(&ex->dest_values[0], ex->src_values[0]);
}

#include "orc/orcemulateopcodes.h"

static OrcStaticOpcode opcodes[] = {

  /* byte ops */
  { "absb", absb, NULL, 0, { 1 }, { 1 }, emulate_absb },
  { "addb", addb, NULL, 0, { 1 }, { 1, 1 }, emulate_addb },
  { "addssb", addssb, NULL, 0, { 1 }, { 1, 1 }, emulate_addssb },
  { "addusb", addusb, NULL, 0, { 1 }, { 1, 1 }, emulate_addusb },
  { "andb", andb, NULL, 0, { 1 }, { 1, 1 }, emulate_andb },
  { "andnb", andnb, NULL, 0, { 1 }, { 1, 1 }, emulate_andnb },
  { "avgsb", avgsb, NULL, 0, { 1 }, { 1, 1 }, emulate_avgsb },
  { "avgub", avgub, NULL, 0, { 1 }, { 1, 1 }, emulate_avgub },
  { "cmpeqb", cmpeqb, NULL, 0, { 1 }, { 1, 1 }, emulate_cmpeqb },
  { "cmpgtsb", cmpgtsb, NULL, 0, { 1 }, { 1, 1 }, emulate_cmpgtsb },
  { "copyb", copyb, NULL, 0, { 1 }, { 1 }, emulate_copyb },
  { "maxsb", maxsb, NULL, 0, { 1 }, { 1, 1 }, emulate_maxsb },
  { "maxub", maxub, NULL, 0, { 1 }, { 1, 1 }, emulate_maxub },
  { "minsb", minsb, NULL, 0, { 1 }, { 1, 1 }, emulate_minsb },
  { "minub", minub, NULL, 0, { 1 }, { 1, 1 }, emulate_minub },
  { "mullb", mullb, NULL, 0, { 1 }, { 1, 1 }, emulate_mullb },
  { "mulhsb", mulhsb, NULL, 0, { 1 }, { 1, 1 }, emulate_mulhsb },
  { "mulhub", mulhub, NULL, 0, { 1 }, { 1, 1 }, emulate_mulhub },
  { "orb", orb, NULL, 0, { 1 }, { 1, 1 }, emulate_orb },
  { "shlb", shlb, NULL, ORC_STATIC_OPCODE_SCALAR, { 1 }, { 1, 1 }, emulate_shlb },
  { "shrsb", shrsb, NULL, ORC_STATIC_OPCODE_SCALAR, { 1 }, { 1, 1 }, emulate_shrsb },
  { "shrub", shrub, NULL, ORC_STATIC_OPCODE_SCALAR, { 1 }, { 1, 1 }, emulate_shrub },
  { "signb", signb, NULL, 0, { 1 }, { 1 }, emulate_signb },
  { "subb", subb, NULL, 0, { 1 }, { 1, 1 }, emulate_subb },
  { "subssb", subssb, NULL, 0, { 1 }, { 1, 1 }, emulate_subssb },
  { "subusb", subusb, NULL, 0, { 1 }, { 1, 1 }, emulate_subusb },
  { "xorb", xorb, NULL, 0, { 1 }, { 1, 1 }, emulate_xorb },

  /* word ops */
  { "absw", absw, NULL, 0, { 2 }, { 2 }, emulate_absw },
  { "addw", addw, NULL, 0, { 2 }, { 2, 2 }, emulate_addw },
  { "addssw", addssw, NULL, 0, { 2 }, { 2, 2 }, emulate_addssw },
  { "addusw", addusw, NULL, 0, { 2 }, { 2, 2 }, emulate_addusw },
  { "andw", andw, NULL, 0, { 2 }, { 2, 2 }, emulate_andw },
  { "andnw", andnw, NULL, 0, { 2 }, { 2, 2 }, emulate_andnw },
  { "avgsw", avgsw, NULL, 0, { 2 }, { 2, 2 }, emulate_avgsw },
  { "avguw", avguw, NULL, 0, { 2 }, { 2, 2 }, emulate_avguw },
  { "cmpeqw", cmpeqw, NULL, 0, { 2 }, { 2, 2 }, emulate_cmpeqw },
  { "cmpgtsw", cmpgtsw, NULL, 0, { 2 }, { 2, 2 }, emulate_cmpgtsw },
  { "copyw", copyw, NULL, 0, { 2 }, { 2 }, emulate_copyw },
  { "maxsw", maxsw, NULL, 0, { 2 }, { 2, 2 }, emulate_maxsw },
  { "maxuw", maxuw, NULL, 0, { 2 }, { 2, 2 }, emulate_maxuw },
  { "minsw", minsw, NULL, 0, { 2 }, { 2, 2 }, emulate_minsw },
  { "minuw", minuw, NULL, 0, { 2 }, { 2, 2 }, emulate_minuw },
  { "mullw", mullw, NULL, 0, { 2 }, { 2, 2 }, emulate_mullw },
  { "mulhsw", mulhsw, NULL, 0, { 2 }, { 2, 2 }, emulate_mulhsw },
  { "mulhuw", mulhuw, NULL, 0, { 2 }, { 2, 2 }, emulate_mulhuw },
  { "orw", orw, NULL, 0, { 2 }, { 2, 2 }, emulate_orw },
  { "shlw", shlw, NULL, ORC_STATIC_OPCODE_SCALAR, { 2 }, { 2, 2 }, emulate_shlw },
  { "shrsw", shrsw, NULL, ORC_STATIC_OPCODE_SCALAR, { 2 }, { 2, 2 }, emulate_shrsw },
  { "shruw", shruw, NULL, ORC_STATIC_OPCODE_SCALAR, { 2 }, { 2, 2 }, emulate_shruw },
  { "signw", signw, NULL, 0, { 2 }, { 2 }, emulate_signw },
  { "subw", subw, NULL, 0, { 2 }, { 2, 2 }, emulate_subw },
  { "subssw", subssw, NULL, 0, { 2 }, { 2, 2 }, emulate_subssw },
  { "subusw", subusw, NULL, 0, { 2 }, { 2, 2 }, emulate_subusw },
  { "xorw", xorw, NULL, 0, { 2 }, { 2, 2 }, emulate_xorw },

  /* long ops */
  { "absl", absl, NULL, 0, { 4 }, { 4 }, emulate_absl },
  { "addl", addl, NULL, 0, { 4 }, { 4, 4 }, emulate_addl },
  { "addssl", addssl, NULL, 0, { 4 }, { 4, 4 }, emulate_addssl },
  { "addusl", addusl, NULL, 0, { 4 }, { 4, 4 }, emulate_addusl },
  { "andl", andl, NULL, 0, { 4 }, { 4, 4 }, emulate_andl },
  { "andnl", andnl, NULL, 0, { 4 }, { 4, 4 }, emulate_andnl },
  { "avgsl", avgsl, NULL, 0, { 4 }, { 4, 4 }, emulate_avgsl },
  { "avgul", avgul, NULL, 0, { 4 }, { 4, 4 }, emulate_avgul },
  { "cmpeql", cmpeql, NULL, 0, { 4 }, { 4, 4 }, emulate_cmpeql },
  { "cmpgtsl", cmpgtsl, NULL, 0, { 4 }, { 4, 4 }, emulate_cmpgtsl },
  { "copyl", copyl, NULL, 0, { 4 }, { 4 }, emulate_copyl },
  { "maxsl", maxsl, NULL, 0, { 4 }, { 4, 4 }, emulate_maxsl },
  { "maxul", maxul, NULL, 0, { 4 }, { 4, 4 }, emulate_maxul },
  { "minsl", minsl, NULL, 0, { 4 }, { 4, 4 }, emulate_minsl },
  { "minul", minul, NULL, 0, { 4 }, { 4, 4 }, emulate_minul },
  { "mulll", mulll, NULL, 0, { 4 }, { 4, 4 }, emulate_mulll },
  { "mulhsl", mulhsl, NULL, 0, { 4 }, { 4, 4 }, emulate_mulhsl },
  { "mulhul", mulhul, NULL, 0, { 4 }, { 4, 4 }, emulate_mulhul },
  { "orl", orl, NULL, 0, { 4 }, { 4, 4 }, emulate_orl },
  { "shll", shll, NULL, ORC_STATIC_OPCODE_SCALAR, { 4 }, { 4, 4 }, emulate_shll },
  { "shrsl", shrsl, NULL, ORC_STATIC_OPCODE_SCALAR, { 4 }, { 4, 4 }, emulate_shrsl },
  { "shrul", shrul, NULL, ORC_STATIC_OPCODE_SCALAR, { 4 }, { 4, 4 }, emulate_shrul },
  { "signl", signl, NULL, 0, { 4 }, { 4 }, emulate_signl },
  { "subl", subl, NULL, 0, { 4 }, { 4, 4 }, emulate_subl },
  { "subssl", subssl, NULL, 0, { 4 }, { 4, 4 }, emulate_subssl },
  { "subusl", subusl, NULL, 0, { 4 }, { 4, 4 }, emulate_subusl },
  { "xorl", xorl, NULL, 0, { 4 }, { 4, 4 }, emulate_xorl },

  { "convsbw", convsbw, NULL, 0, { 2 }, { 1 }, emulate_convsbw },
  { "convubw", convubw, NULL, 0, { 2 }, { 1 }, emulate_convubw },
  { "convswl", convswl, NULL, 0, { 4 }, { 2 }, emulate_convswl },
  { "convuwl", convuwl, NULL, 0, { 4 }, { 2 }, emulate_convuwl },
#ifdef ENABLE_64BIT
  { "convslq", convslq, NULL, 0, { 8 }, { 4 }, emulate_convslq },
  { "convulq", convulq, NULL, 0, { 8 }, { 4 }, emulate_convulq },
#endif

  { "convwb", convwb, NULL, 0, { 1 }, { 2 }, emulate_convwb },
  { "convssswb", convssswb, NULL, 0, { 1 }, { 2 }, emulate_convssswb },
  { "convsuswb", convsuswb, NULL, 0, { 1 }, { 2 }, emulate_convsuswb },
  { "convusswb", convusswb, NULL, 0, { 1 }, { 2 }, emulate_convusswb },
  { "convuuswb", convuuswb, NULL, 0, { 1 }, { 2 }, emulate_convuuswb },

  { "convlw", convlw, NULL, 0, { 2 }, { 4 }, emulate_convlw },
  { "convssslw", convssslw, NULL, 0, { 2 }, { 4 }, emulate_convssslw },
  { "convsuslw", convsuslw, NULL, 0, { 2 }, { 4 }, emulate_convsuslw },
  { "convusslw", convusslw, NULL, 0, { 2 }, { 4 }, emulate_convusslw },
  { "convuuslw", convuuslw, NULL, 0, { 2 }, { 4 }, emulate_convuuslw },

#ifdef ENABLE_64BIT
  { "convql", convql, NULL, 0, { 4 }, { 8 }, emulate_convql },
  { "convssql", convssql, NULL, 0, { 4 }, { 8 }, emulate_convssql },
  { "convusql", convusql, NULL, 0, { 4 }, { 8 }, emulate_convusql },
#endif

  { "mulsbw", mulsbw, NULL, 0, { 2 }, { 1, 1 }, emulate_mulsbw },
  { "mulubw", mulubw, NULL, 0, { 2 }, { 1, 1 }, emulate_mulubw },
  { "mulswl", mulswl, NULL, 0, { 4 }, { 2, 2 }, emulate_mulswl },
  { "muluwl", muluwl, NULL, 0, { 4 }, { 2, 2 }, emulate_muluwl },
#ifdef ENABLE_64BIT
  { "mulslq", mulslq, NULL, 0, { 8 }, { 4, 4 }, emulate_mulslq },
  { "mululq", mululq, NULL, 0, { 8 }, { 4, 4 }, emulate_mululq },
#endif

  /* accumulators */
  { "accw", accw, NULL, ORC_STATIC_OPCODE_ACCUMULATOR, { 2 }, { 2 }, emulate_accw },
  { "accl", accl, NULL, ORC_STATIC_OPCODE_ACCUMULATOR, { 4 }, { 4 }, emulate_accl },
  { "accsadubl", accsadubl, NULL, ORC_STATIC_OPCODE_ACCUMULATOR, { 4 }, { 1, 1 }, emulate_accsadubl },

  { "swapw", swapw, NULL, 0, { 2 }, { 2 }, emulate_swapw },
  { "swapl", swapl, NULL, 0, { 4 }, { 4 }, emulate_swapl },
  { "select0wb", select0wb, NULL, 0, { 1 }, { 2 }, emulate_select0wb },
  { "select1wb", select1wb, NULL, 0, { 1 }, { 2 }, emulate_select1wb },
  { "select0lw", select0lw, NULL, 0, { 2 }, { 4 }, emulate_select0lw },
  { "select1lw", select1lw, NULL, 0, { 2 }, { 4 }, emulate_select1lw },
  { "mergewl", mergewl, NULL, 0, { 4 }, { 2, 2 }, emulate_mergewl },
  { "mergebw", mergebw, NULL, 0, { 2 }, { 1, 1 }, emulate_mergebw },
  { "splitlw", splitlw, NULL, 0, { 2, 2 }, { 4 }, emulate_splitlw },
  { "splitwb", splitwb, NULL, 0, { 1, 1 }, { 2 }, emulate_splitwb },

  /* float ops */
  { "addf", addf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_addf },
  { "subf", subf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_subf },
  { "mulf", mulf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_mulf },
  { "divf", divf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_divf },
  { "sqrtf", orc_sqrtf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4 }, emulate_sqrtf },
  { "maxf", maxf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_maxf },
  { "minf", minf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4, 4 }, emulate_minf },
  { "cmpeqf", cmpeqf, NULL, ORC_STATIC_OPCODE_FLOAT_SRC, { 4 }, { 4, 4 }, emulate_cmpeqf },
  { "cmpltf", cmpltf, NULL, ORC_STATIC_OPCODE_FLOAT_SRC, { 4 }, { 4, 4 }, emulate_cmpltf },
  { "cmplef", cmplef, NULL, ORC_STATIC_OPCODE_FLOAT_SRC, { 4 }, { 4, 4 }, emulate_cmplef },
  { "convfl", convfl, NULL, ORC_STATIC_OPCODE_FLOAT_SRC, { 4 }, { 4 }, emulate_convfl },
  { "convlf", convlf, NULL, ORC_STATIC_OPCODE_FLOAT_DEST, { 4 }, { 4 }, emulate_convlf },

  { "" }
};

void
orc_opcode_init (void)
{
  orc_opcode_register_static (opcodes, "sys");
}


