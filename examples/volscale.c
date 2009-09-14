
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <orc/orc.h>

static OrcProgram *p = NULL;

static void
mmx_rule_mulhslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmp1 = X86_MM4;
  int tmp2 = X86_MM5;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  if (dest != src1) {
    orc_mmx_emit_movq (p, src1, dest);
  }

  orc_mmx_emit_pxor (p, tmp1, tmp1);      /* .. |    0  |    0  | */
  orc_mmx_emit_punpcklwd (p, tmp1, src2); /* .. |    0  |   p0  | */
  orc_mmx_emit_pcmpgtw (p, dest, tmp1);   /* .. |    0  | s(vl) | */ 
  orc_mmx_emit_pand (p, src2, tmp1);      /* .. |    0  |  (p0) |  (vl >> 15) & p */
  orc_mmx_emit_movq (p, src2, tmp2);     
  orc_mmx_emit_pmulhw (p, src1, src2);    /* .. |    0  | vl*p0 | */
  orc_mmx_emit_paddw (p, tmp1, src2);     /* .. |    0  | vl*p0 | + sign correct */
  orc_mmx_emit_psrld (p, 16, dest);       /* .. |    0  |   vh  | */
  orc_mmx_emit_pmaddwd (p, tmp2, dest);   /* .. |    p0 * vh    | */ 
  orc_mmx_emit_paddd (p, src2, dest);     /* .. |    p0 * v0    | */
}

static void
sse_rule_mulhslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmp1 = X86_XMM4;
  int tmp2 = X86_XMM5;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  if (dest != src1) {
    orc_sse_emit_movdqa (p, src1, dest);
  }

  orc_sse_emit_pxor (p, tmp1, tmp1);      /* .. |    0  |    0  | */
  orc_sse_emit_punpcklwd (p, tmp1, src2); /* .. |    0  |   p0  | */
  orc_sse_emit_pcmpgtw (p, dest, tmp1);   /* .. |    0  | s(vl) | */ 
  orc_sse_emit_pand (p, src2, tmp1);      /* .. |    0  |  (p0) |  (vl >> 15) & p */
  orc_sse_emit_movdqa (p, src2, tmp2);     
  orc_sse_emit_pmulhw (p, src1, src2);    /* .. |    0  | vl*p0 | */
  orc_sse_emit_paddw (p, tmp1, src2);     /* .. |    0  | vl*p0 | + sign correct */
  orc_sse_emit_psrld (p, 16, dest);       /* .. |    0  |   vh  | */
  orc_sse_emit_pmaddwd (p, tmp2, dest);   /* .. |    p0 * vh    | */ 
  orc_sse_emit_paddd (p, src2, dest);     /* .. |    p0 * v0    | */
}

static void
mmx_register_rules (void)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("pulse"),
      orc_target_get_by_name ("mmx"), ORC_TARGET_MMX_MMX);

  orc_rule_register (rule_set, "mulhslw", mmx_rule_mulhslw, NULL);
}

static void
sse_register_rules (void)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("pulse"),
      orc_target_get_by_name ("sse"), ORC_TARGET_SSE_SSE2);

  orc_rule_register (rule_set, "mulhslw", sse_rule_mulhslw, NULL);
}

/* calculate the high 32 bits of a 32x16 signed multiply */
static void
mulhslw (OrcOpcodeExecutor *ex, void *user)
{
  int32_t src, sl, sh, scale;

  scale = ex->src_values[0];
  src = ex->src_values[1];

  sh = scale >> 16;
  sl = scale & 0xffff;

  ex->dest_values[0] = (int32_t) ((src * sl) >> 16) + (src * sh);
}

static OrcStaticOpcode opcodes[] = {
  { "mulhslw", mulhslw, NULL, 0, { 4 }, { 4, 2 } },

  { "" }
};

static void
register_instr (void)
{
  orc_opcode_register_static (opcodes, "pulse");
  mmx_register_rules ();
  sse_register_rules ();
}

static void
make_volume_orc()
{
  OrcCompileResult res;

  /* int16 destination samples that get scaled by int32 volumes */
  p = orc_program_new ();
  orc_program_add_destination (p, 2, "d1");
  orc_program_add_source (p, 4, "s1");
  orc_program_add_source (p, 2, "s2");

  /* a temporary for the upscaled input samples */
  orc_program_add_temporary (p, 4, "t1");

  /* multiply with the volume, keeping only the high 32bits */
  orc_program_append_str (p, "mulhslw", "t1", "s1", "s2");
  /* pack an saturate do 16 bits again */
  orc_program_append_ds_str (p, "convssslw", "d1", "t1");

  /* Compile the program */
  res = orc_program_compile (p);
  fprintf (stderr, "result: %d\n", res);

  if (res == ORC_COMPILE_RESULT_OK)
    fprintf (stderr, "%s\n", orc_program_get_asm_code (p));
}

static void
do_volume_orc (int16_t *ptr, int32_t *volumes, int n_channels, int length)
{
  OrcExecutor _ex;
  OrcExecutor *ex = &_ex;

  /* Set the values on the executor structure */
  orc_executor_set_program (ex, p);
  orc_executor_set_n (ex, length);
  orc_executor_set_array_str (ex, "s1", volumes);
  orc_executor_set_array_str (ex, "s2", ptr);
  orc_executor_set_array_str (ex, "d1", ptr);

  /* Run the program.  This calls the code that was generated above,
   * or, if the compilation failed, will emulate the program. */
  orc_executor_run (ex);
}

#define TIMES 100000
//#define TIMES 10000
#define N 1024
#define MAX_CHANNELS 32

static uint64_t 
get_timestamp ()
{
  struct timeval now;

  gettimeofday (&now, NULL);

  return now.tv_sec * 1000000LL + now.tv_usec;
}

int16_t a[N];
int16_t b[N];
int32_t volume[MAX_CHANNELS + N];

int
main (int argc, char *argv[])
{
  int i;
  uint64_t start, stop;

  /* orc_init() must be called before any other Orc function */
  orc_init ();

  orc_debug_set_level (ORC_DEBUG_LOG);
  register_instr ();

  make_volume_orc();

  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    a[i] = b[i] = i + 1;
    volume[i] = 0x10000 + i;
  }

  start = get_timestamp ();
  for (i = 0; i < TIMES; i++)
  //for (i = 0; i < 0; i++)
    do_volume_orc (b, volume, 2, N);
  stop = get_timestamp ();
  printf ("elapsed 32: %llu ms\n", (long long unsigned int) (stop - start));

  /* Print the results */
  for(i=0;i<20;i++){
    printf("%d: %d -> %d\n", i, a[i], b[i]);
  }

  return 0;
}
