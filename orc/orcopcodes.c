
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


static OrcOpcode *opcode_list;
static int n_opcodes;
static int n_opcodes_alloc;

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

#define ORC_CLAMP_SB(x) CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) CLAMP(x,ORC_UB_MIN,ORC_UB_MAX)
#define ORC_CLAMP_SW(x) CLAMP(x,ORC_SW_MIN,ORC_SW_MAX)
#define ORC_CLAMP_UW(x) CLAMP(x,ORC_UW_MIN,ORC_UW_MAX)
#define ORC_CLAMP_SL(x) CLAMP(x,ORC_SL_MIN,ORC_SL_MAX)
#define ORC_CLAMP_UL(x) CLAMP(x,ORC_UL_MIN,ORC_UL_MAX)


int
orc_opcode_get_list (OrcOpcode **list)
{
  (*list) = opcode_list;
  return n_opcodes;
}

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

void
orc_opcode_register_static (OrcStaticOpcode *sopcode)
{
  while (sopcode->name[0]) {
    OrcOpcode *opcode;
    int i;

    if (n_opcodes == n_opcodes_alloc) {
      n_opcodes_alloc += 100;
      opcode_list = realloc(opcode_list, sizeof(OrcOpcode) * n_opcodes_alloc);
    }

    opcode = opcode_list + n_opcodes;

    memset (opcode, 0, sizeof(OrcOpcode));

    opcode->name = sopcode->name;
    for(i=0;i<ORC_STATIC_OPCODE_N_DEST;i++){
      opcode->dest_size[i] = sopcode->dest_size[i];
      if (sopcode->dest_size[i]) opcode->n_dest = i + 1;
    }
    for(i=0;i<ORC_STATIC_OPCODE_N_SRC;i++){
      opcode->src_size[i] = sopcode->src_size[i];
      if (sopcode->src_size[i]) opcode->n_src = i + 1;
    }
    opcode->emulate = sopcode->emulate;
    opcode->emulate_user = sopcode->emulate_user;

    n_opcodes++;
    sopcode++;
  }
}


OrcOpcode *
orc_opcode_find_by_name (const char *name)
{
  int i;

  for(i=0;i<n_opcodes;i++){
    if (!strcmp (name, opcode_list[i].name)) {
      return opcode_list + i;
    }
  }

  return NULL;
}

static void
convsbw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (int8_t)(ex->values[1]);
}

static void
convubw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (uint8_t)(ex->values[1]);
}

static void
convswl (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (int8_t)(ex->values[1]);
}

static void
convuwl (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (uint8_t)(ex->values[1]);
}

static void
convwb (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (int16_t)(ex->values[1]);
}

static void
convssswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_SB((int16_t)(ex->values[1]));
}

static void
convsuswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_UB((int16_t)(ex->values[1]));
}

static void
convusswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_SB((uint16_t)(ex->values[1]));
}

static void
convuuswb (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_UB((uint16_t)(ex->values[1]));
}

static void
convlw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = (int32_t)(ex->values[1]);
}

static void
convssslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_SW((int32_t)(ex->values[1]));
}

static void
convsuslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_UW((int32_t)(ex->values[1]));
}

static void
convusslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_SW((uint32_t)(ex->values[1]));
}

static void
convuuslw (OrcOpcodeExecutor *ex, void *user)
{
  ex->values[0] = ORC_CLAMP_UW((uint32_t)(ex->values[1]));
}

#define UNARY(name,type,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  int a = ex->values[1]; \
  ex->values[0] = ( type )( code ); \
}

#define BINARY(name,type,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  int a = ex->values[1]; \
  int b = ex->values[2]; \
  ex->values[0] = ( type )( code ); \
}

#define BINARY_U(name,type,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  unsigned int a = ex->values[1]; \
  unsigned int b = ex->values[2]; \
  ex->values[0] = ( type )( code ); \
}

#define UNARY_SB(name,code) UNARY(name, int8_t, code)
#define BINARY_SB(name,code) BINARY(name, int8_t, code)
#define BINARY_UB(name,code) BINARY(name, uint8_t, code)

#define UNARY_SW(name,code) UNARY(name, int16_t, code)
#define BINARY_SW(name,code) BINARY(name, int16_t, code)
#define BINARY_UW(name,code) BINARY(name, uint16_t, code)

#define UNARY_SL(name,code) UNARY(name, int32_t, code)
#define BINARY_SL(name,code) BINARY(name, int32_t, code)
#define BINARY_UL(name,code) BINARY(name, uint32_t, code)

UNARY_SB(absb, (a<0)?-a:a)
BINARY_SB(addb, a + b)
BINARY_SB(addssb, CLAMP(ORC_SB_MIN,ORC_SB_MAX,a + b))
BINARY_UB(addusb, CLAMP(0,ORC_UB_MAX,a + b))
BINARY_SB(andb, a & b)
BINARY_SB(andnb, a & (~b))
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
BINARY_UB(shrub, ((uint8_t)a) >> b)
UNARY_SB(signb, CLAMP(-1,1,a))
BINARY_SB(subb, a - b)
BINARY_SB(subssb, CLAMP(ORC_SB_MIN,ORC_SB_MAX,a - b))
BINARY_UB(subusb, CLAMP(0,ORC_UB_MAX,(uint8_t)a - (uint8_t)b))
BINARY_SB(xorb, a ^ b)

UNARY_SW(absw, (a<0)?-a:a)
BINARY_SW(addw, a + b)
BINARY_SW(addssw, CLAMP(ORC_SW_MIN,ORC_SW_MAX,a + b))
BINARY_UW(addusw, CLAMP(0,ORC_UW_MAX,a + b))
BINARY_SW(andw, a & b)
BINARY_SW(andnw, a & (~b))
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
UNARY_SW(signw, CLAMP(-1,1,a))
BINARY_SW(subw, a - b)
BINARY_SW(subssw, CLAMP(ORC_SW_MIN,ORC_SW_MAX,a - b))
BINARY_UW(subusw, CLAMP(0,ORC_UW_MAX,a - b))
BINARY_SW(xorw, a ^ b)

UNARY_SL(absl, (a<0)?-a:a)
BINARY_SL(addl, a + b)
BINARY_SL(addssl, CLAMP(ORC_SL_MIN,ORC_SL_MAX,(int64_t)a + (int64_t)b))
BINARY_UL(addusl, CLAMP(0,ORC_UL_MAX,(uint64_t)a + (uint64_t)b))
BINARY_SL(andl, a & b)
BINARY_SL(andnl, a & (~b))
BINARY_SL(avgsl, (a + b + 1)>>1)
BINARY_UL(avgul, (a + b + 1)>>1)
BINARY_SL(cmpeql, (a == b) ? (~0) : 0)
BINARY_SL(cmpgtsl, (a > b) ? (~0) : 0)
UNARY_SL(copyl, a)
BINARY_SL(maxsl, (a > b) ? a : b)
BINARY_UL(maxul, (a > b) ? a : b)
BINARY_SL(minsl, (a < b) ? a : b)
BINARY_UL(minul, (a < b) ? a : b)
BINARY_SL(mulll, (a * b) & 0xffffffff)
BINARY_SL(mulhsl, ((int64_t)a * (int64_t)b) >> 32)
BINARY_UL(mulhul, ((uint64_t)a * (uint64_t)b) >> 32)
BINARY_SL(orl, a | b)
BINARY_SL(shll, a << b)
BINARY_SL(shrsl, a >> b)
BINARY_UL(shrul, a >> b)
UNARY_SL(signl, CLAMP(-1,1,a))
BINARY_SL(subl, a - b)
BINARY_SL(subssl, CLAMP(ORC_SL_MIN,ORC_SL_MAX,(int64_t)a - (int64_t)b))
BINARY_UL(subusl, CLAMP(0,ORC_UL_MAX,(uint64_t)a - (uint64_t)b))
BINARY_SL(xorl, a ^ b)


#define MUL(name, type1, type2) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  ex->values[0] = ((type2)(type1)ex->values[1]) * \
    ((type2)(type1)ex->values[2]); \
}

MUL(mulsbw, int8_t, int16_t)
MUL(mulubw, uint8_t, uint16_t)
MUL(mulswl, int16_t, int32_t)
MUL(muluwl, uint16_t, uint32_t)
#ifdef ENABLE_INT64
MUL(mulslq, int32_t, int64_t)
MUL(mululq, uint32_t, uint64_t)
#endif

static OrcStaticOpcode opcodes[] = {

  /* byte ops */
  { "absb", absb, NULL, { 1 }, { 1 } },
  { "addb", addb, NULL, { 1 }, { 1, 1 } },
  { "addssb", addssb, NULL, { 1 }, { 1, 1 } },
  { "addusb", addusb, NULL, { 1 }, { 1, 1 } },
  { "andb", andb, NULL, { 1 }, { 1, 1 } },
  { "andnb", andnb, NULL, { 1 }, { 1, 1 } },
  { "avgsb", avgsb, NULL, { 1 }, { 1, 1 } },
  { "avgub", avgub, NULL, { 1 }, { 1, 1 } },
  { "cmpeqb", cmpeqb, NULL, { 1 }, { 1, 1 } },
  { "cmpgtsb", cmpgtsb, NULL, { 1 }, { 1, 1 } },
  { "copyb", copyb, NULL, { 1 }, { 1 } },
  { "maxsb", maxsb, NULL, { 1 }, { 1, 1 } },
  { "maxub", maxub, NULL, { 1 }, { 1, 1 } },
  { "minsb", minsb, NULL, { 1 }, { 1, 1 } },
  { "minub", minub, NULL, { 1 }, { 1, 1 } },
  { "mullb", mullb, NULL, { 1 }, { 1, 1 } },
  { "mulhsb", mulhsb, NULL, { 1 }, { 1, 1 } },
  { "mulhub", mulhub, NULL, { 1 }, { 1, 1 } },
  { "orb", orb, NULL, { 1 }, { 1, 1 } },
  { "shlb", shlb, NULL, { 1 }, { 1, 1 } },
  { "shrsb", shrsb, NULL, { 1 }, { 1, 1 } },
  { "shrub", shrub, NULL, { 1 }, { 1, 1 } },
  { "signb", signb, NULL, { 1 }, { 1, 1 } },
  { "subb", subb, NULL, { 1 }, { 1, 1 } },
  { "subssb", subssb, NULL, { 1 }, { 1, 1 } },
  { "subusb", subusb, NULL, { 1 }, { 1, 1 } },
  { "xorb", xorb, NULL, { 1 }, { 1, 1 } },

  /* word ops */
  { "absw", absw, NULL, { 2 }, { 2 } },
  { "addw", addw, NULL, { 2 }, { 2, 2 } },
  { "addssw", addssw, NULL, { 2 }, { 2, 2 } },
  { "addusw", addusw, NULL, { 2 }, { 2, 2 } },
  { "andw", andw, NULL, { 2 }, { 2, 2 } },
  { "andnw", andnw, NULL, { 2 }, { 2, 2 } },
  { "avgsw", avgsw, NULL, { 2 }, { 2, 2 } },
  { "avguw", avguw, NULL, { 2 }, { 2, 2 } },
  { "cmpeqw", cmpeqw, NULL, { 2 }, { 2, 2 } },
  { "cmpgtsw", cmpgtsw, NULL, { 2 }, { 2, 2 } },
  { "copyw", copyw, NULL, { 2 }, { 2 } },
  { "maxsw", maxsw, NULL, { 2 }, { 2, 2 } },
  { "maxuw", maxuw, NULL, { 2 }, { 2, 2 } },
  { "minsw", minsw, NULL, { 2 }, { 2, 2 } },
  { "minuw", minuw, NULL, { 2 }, { 2, 2 } },
  { "mullw", mullw, NULL, { 2 }, { 2, 2 } },
  { "mulhsw", mulhsw, NULL, { 2 }, { 2, 2 } },
  { "mulhuw", mulhuw, NULL, { 2 }, { 2, 2 } },
  { "orw", orw, NULL, { 2 }, { 2, 2 } },
  { "shlw", shlw, NULL, { 2 }, { 2, 2 } },
  { "shrsw", shrsw, NULL, { 2 }, { 2, 2 } },
  { "shruw", shruw, NULL, { 2 }, { 2, 2 } },
  { "signw", signw, NULL, { 2 }, { 2, 2 } },
  { "subw", subw, NULL, { 2 }, { 2, 2 } },
  { "subssw", subssw, NULL, { 2 }, { 2, 2 } },
  { "subusw", subusw, NULL, { 2 }, { 2, 2 } },
  { "xorw", xorw, NULL, { 2 }, { 2, 2 } },

  /* long ops */
  { "absl", absl, NULL, { 4 }, { 4 } },
  { "addl", addl, NULL, { 4 }, { 4, 4 } },
  { "addssl", addssl, NULL, { 4 }, { 4, 4 } },
  { "addusl", addusl, NULL, { 4 }, { 4, 4 } },
  { "andl", andl, NULL, { 4 }, { 4, 4 } },
  { "andnl", andnl, NULL, { 4 }, { 4, 4 } },
  { "avgsl", avgsl, NULL, { 4 }, { 4, 4 } },
  { "avgul", avgul, NULL, { 4 }, { 4, 4 } },
  { "cmpeql", cmpeql, NULL, { 4 }, { 4, 4 } },
  { "cmpgtsl", cmpgtsl, NULL, { 4 }, { 4, 4 } },
  { "copyl", copyl, NULL, { 4 }, { 4 } },
  { "maxsl", maxsl, NULL, { 4 }, { 4, 4 } },
  { "maxul", maxul, NULL, { 4 }, { 4, 4 } },
  { "minsl", minsl, NULL, { 4 }, { 4, 4 } },
  { "minul", minul, NULL, { 4 }, { 4, 4 } },
  { "mulll", mulll, NULL, { 4 }, { 4, 4 } },
  { "mulhsl", mulhsl, NULL, { 4 }, { 4, 4 } },
  { "mulhul", mulhul, NULL, { 4 }, { 4, 4 } },
  { "orl", orl, NULL, { 4 }, { 4, 4 } },
  { "shll", shll, NULL, { 4 }, { 4, 4 } },
  { "shrsl", shrsl, NULL, { 4 }, { 4, 4 } },
  { "shrul", shrul, NULL, { 4 }, { 4, 4 } },
  { "signl", signl, NULL, { 4 }, { 4, 4 } },
  { "subl", subl, NULL, { 4 }, { 4, 4 } },
  { "subssl", subssl, NULL, { 4 }, { 4, 4 } },
  { "subusl", subusl, NULL, { 4 }, { 4, 4 } },
  { "xorl", xorl, NULL, { 4 }, { 4, 4 } },

  { "convsbw", convsbw, NULL, { 2 }, { 1 } },
  { "convubw", convubw, NULL, { 2 }, { 1 } },
  { "convswl", convswl, NULL, { 4 }, { 2 } },
  { "convuwl", convuwl, NULL, { 4 }, { 2 } },
#ifdef ENABLE_64BIT
  { "convslq", convslq, NULL, { 8 }, { 4 } },
  { "convulq", convulq, NULL, { 8 }, { 4 } },
#endif

  { "convwb", convwb, NULL, { 1 }, { 2 } },
  { "convssswb", convssswb, NULL, { 1 }, { 2 } },
  { "convsuswb", convsuswb, NULL, { 1 }, { 2 } },
  { "convusswb", convusswb, NULL, { 1 }, { 2 } },
  { "convuuswb", convuuswb, NULL, { 1 }, { 2 } },

  { "convlw", convlw, NULL, { 1 }, { 2 } },
  { "convssslw", convssslw, NULL, { 1 }, { 2 } },
  { "convsuslw", convsuslw, NULL, { 1 }, { 2 } },
  { "convusslw", convusslw, NULL, { 1 }, { 2 } },
  { "convuuslw", convuuslw, NULL, { 1 }, { 2 } },

#ifdef ENABLE_64BIT
  { "convql", convql, NULL, { 4 }, { 8 } },
  { "convssql", convssql, NULL, { 4 }, { 8 } },
  { "convusql", convusql, NULL, { 4 }, { 8 } },
#endif

  { "mulsbw", mulsbw, NULL, { 2 }, { 1, 1 } },
  { "mulubw", mulubw, NULL, { 2 }, { 1, 1 } },
  { "mulswl", mulswl, NULL, { 4 }, { 2, 2 } },
  { "muluwl", muluwl, NULL, { 4 }, { 2, 2 } },
#ifdef ENABLE_64BIT
  { "mulslq", mulslq, NULL, { 8 }, { 4, 4 } },
  { "mululq", mululq, NULL, { 8 }, { 4, 4 } },
#endif

  { "" }
};

void
orc_opcode_init (void)
{
  orc_opcode_register_static (opcodes);
}


