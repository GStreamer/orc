
#include <orc-pixel/orcpixel.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static OrcStaticOpcode opcodes[];

void orc_pixel_sse_register_rules (void);

void
orc_pixel_init (void)
{
  orc_init ();

  orc_opcode_register_static (opcodes, "pixel");
  orc_pixel_sse_register_rules ();
}


#define COMPOSITE_OVER(d,s,m) ((d) + (s) - ORC_MULDIV_255((d),(m)))
#define COMPOSITE_ADD(d,s) ORC_CLAMP((d) + (s), 0, 255)
#define COMPOSITE_IN(s,m) ORC_MULDIV_255((s),(m))

#define ORC_DIVIDE_255(x) ((((x)+128) + (((x)+128)>>8))>>8)
#define ORC_MULDIV_255(a,b) ORC_DIVIDE_255((a)*(b))


#define ORC_ARGB_A(x) (((x)>>24)&0xff)
#define ORC_ARGB_R(x) (((x)>>16)&0xff)
#define ORC_ARGB_G(x) (((x)>>8)&0xff)
#define ORC_ARGB_B(x) (((x)>>0)&0xff)
#define ORC_ARGB(a,r,g,b) ((((b)&0xff)<<0)|(((g)&0xff)<<8)|(((r)&0xff)<<16)|(((a)&0xff)<<24))


static void
compin (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src = ex->src_values[0];
  unsigned int mask = ex->src_values[1]&0xff;

  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_IN(ORC_ARGB_A(src), mask),
      COMPOSITE_IN(ORC_ARGB_R(src), mask),
      COMPOSITE_IN(ORC_ARGB_G(src), mask),
      COMPOSITE_IN(ORC_ARGB_B(src), mask));
}

static void
compover (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];
  unsigned int a;

  a = ORC_ARGB_A(src2);
  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_OVER(ORC_ARGB_A(src1), ORC_ARGB_A(src2), a),
      COMPOSITE_OVER(ORC_ARGB_R(src1), ORC_ARGB_R(src2), a),
      COMPOSITE_OVER(ORC_ARGB_G(src1), ORC_ARGB_G(src2), a),
      COMPOSITE_OVER(ORC_ARGB_B(src1), ORC_ARGB_B(src2), a));
}

static void
compovera (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];

  ex->dest_values[0] = COMPOSITE_OVER(src1, src2, src2);
}

static void
compadd (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];

  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_ADD(ORC_ARGB_A(src1), ORC_ARGB_A(src2)),
      COMPOSITE_ADD(ORC_ARGB_R(src1), ORC_ARGB_R(src2)),
      COMPOSITE_ADD(ORC_ARGB_G(src1), ORC_ARGB_G(src2)),
      COMPOSITE_ADD(ORC_ARGB_B(src1), ORC_ARGB_B(src2)));
}

static OrcStaticOpcode opcodes[] = {
  { "compin", compin, NULL, 0, { 4 }, { 4, 1 } },
  { "compover", compover, NULL, 0, { 4 }, { 4, 4 } },
  { "compovera", compovera, NULL, 0, { 1 }, { 1, 1 } },
  { "compadd", compadd, NULL, 0, { 4 }, { 4, 4 } },

  { "" }
};

