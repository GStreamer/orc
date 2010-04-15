
#include <orc-float/orcfloat.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* This should be static, but compilers can't agree on what to use
 * for forward declarations of static arrays. */
OrcStaticOpcode opcodes[];

void orc_float_sse_register_rules (void);
void orc_float_neon_register_rules (void);

void
orc_float_init (void)
{
  orc_init ();

  orc_opcode_register_static (opcodes, "float");
  orc_float_sse_register_rules ();
  orc_float_neon_register_rules ();
}

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

//#define ORC_FLOAT_READ(addr) (*(float *)(addr))
//#define ORC_FLOAT_WRITE(addr,value) do{ (*(float *)(addr)) = (value); }while(0)

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

UNARY_F(invf, (1.0f/a) )
UNARY_F(invsqrtf, 1.0f/sqrtf(a))


static double
ORC_DOUBLE_READ(void *addr)
{
  union {
    double f;
    unsigned long long i;
  } x;
  x.i = *(unsigned long long *)(addr);
  return x.f;
}

static void
ORC_DOUBLE_WRITE(void *addr, double value)
{
  union {
    double f;
    unsigned long long i;
  } x;
  x.f = value;
  *(unsigned long long *)(addr) = x.i;
}

//#define ORC_DOUBLE_READ(addr) (*(double *)(void *)(addr))
//#define ORC_DOUBLE_WRITE(addr,value) do{ (*(double *)(void *)(addr)) = (value); }while(0)

#define UNARY_G(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  double a = ORC_DOUBLE_READ(&ex->src_values[0]); \
  ORC_DOUBLE_WRITE(&ex->dest_values[0], code ); \
}

#define BINARY_G(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  double a = ORC_DOUBLE_READ(&ex->src_values[0]); \
  double b = ORC_DOUBLE_READ(&ex->src_values[1]); \
  ORC_DOUBLE_WRITE(&ex->dest_values[0], code ); \
}

#define BINARY_GQ(name,code) \
static void \
name (OrcOpcodeExecutor *ex, void *user) \
{ \
  double a = ORC_DOUBLE_READ(&ex->src_values[0]); \
  double b = ORC_DOUBLE_READ(&ex->src_values[1]); \
  ex->dest_values[0] = code ; \
}

BINARY_G(addg, a + b)
BINARY_G(subg, a - b)
BINARY_G(mulg, a * b)
BINARY_G(divg, a / b)
UNARY_G(invg, (1.0f/a) )
UNARY_G(sqrtg, sqrt(a) )
BINARY_G(maxg, (a>b) ? a : b)
BINARY_G(ming, (a<b) ? a : b)
UNARY_G(invsqrtg, 1.0f/sqrt(a))

BINARY_GQ(cmpeqg, (a == b) ? (~0) : 0)
BINARY_GQ(cmpltg, (a < b) ? (~0) : 0)
BINARY_GQ(cmpleg, (a <= b) ? (~0) : 0)

static void
convgl (OrcOpcodeExecutor *ex, void *user)
{
  ex->dest_values[0] = ORC_DOUBLE_READ(&ex->src_values[0]);
}

static void
convlg (OrcOpcodeExecutor *ex, void *user)
{
  ORC_DOUBLE_WRITE(&ex->dest_values[0], ex->src_values[0]);
}

static void
convgf (OrcOpcodeExecutor *ex, void *user)
{
  ORC_FLOAT_WRITE(&ex->dest_values[0], ORC_DOUBLE_READ(&ex->src_values[0]));
}

static void
convfg (OrcOpcodeExecutor *ex, void *user)
{
  ORC_DOUBLE_WRITE(&ex->dest_values[0], ORC_FLOAT_READ(&ex->src_values[0]));
}



OrcStaticOpcode opcodes[] = {
  { "invf", invf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4 } },
  { "invsqrtf", invsqrtf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 4 } },

  { "addg", addg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "subg", subg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "mulg", mulg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "divg", divg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "invg", invg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8 } },
  { "sqrtg", sqrtg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8 } },
  { "maxg", maxg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "ming", ming, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "invsqrtg", invsqrtg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8 } },

  { "cmpeqg", cmpeqg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "cmpltg", cmpltg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },
  { "cmpleg", cmpleg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 8, 8 } },

  { "convgl", convgl, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 8 } },
  { "convlg", convlg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 4 } },

  { "convgf", convgf, NULL, ORC_STATIC_OPCODE_FLOAT, { 4 }, { 8 } },
  { "convfg", convfg, NULL, ORC_STATIC_OPCODE_FLOAT, { 8 }, { 4 } },

  { "" }
};

