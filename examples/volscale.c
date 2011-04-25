
#include <stdio.h>
#ifndef _MSC_VER
#include <sys/time.h>
#endif

#define ORC_ENABLE_UNSTABLE_API

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
  orc_mmx_emit_psrld_imm (p, 16, dest);       /* .. |    0  |   vh  | */
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
  orc_sse_emit_psrld_imm (p, 16, dest);       /* .. |    0  |   vh  | */
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
emulate_mulhslw (OrcOpcodeExecutor *ex, int offset, int n)
{
  int i;
  orc_union32 * ptr0;
  const orc_union32 * ptr4;
  const orc_int16 * ptr5;
  orc_union32 var32;
  orc_int16 var33;
  orc_union32 var34;

  ptr0 = (orc_union32 *)ex->dest_ptrs[0];
  ptr4 = (orc_union32 *)ex->src_ptrs[0];
  ptr5 = (orc_int16 *)ex->src_ptrs[1];

  for (i = 0; i < n; i++) {
    /* 0: loadb */
    var32 = ptr4[i];
    /* 1: loadb */
    var33 = ptr5[i];
    /* 2: mulsbw */
    var34.i = (var32.i * var33)>>16;
    /* 3: storew */
    ptr0[i] = var34;
  }
}

static OrcStaticOpcode opcodes[] = {
  { "mulhslw", 0, { 4 }, { 4, 2 }, emulate_mulhslw },

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
do_volume_c (orc_int16 *dest, const orc_int32 *vols, const orc_int16 *samp, int len)
{
  int i;

  for (i = 0; i < len; i++) {
    orc_int32 t, hi, lo;

    hi = vols[i] >> 16;
    lo = vols[i] & 0xffff;

    t = (orc_int32)(samp[i]);
    t = ((t * lo) >> 16) + (t * hi);
    dest[i] = (orc_int16) ORC_CLAMP (t, -0x8000, 0x7FFF);
  }
}


static void
do_volume_backup (OrcExecutor *ex)
{
  orc_int16 *dest;
  orc_int32 *vols;
  const orc_int16 *samp;
  int len;

  dest = ex->arrays[ORC_VAR_D1];
  vols = ex->arrays[ORC_VAR_S1];
  samp = ex->arrays[ORC_VAR_S2];
  len = ex->n;

  do_volume_c (dest, vols, samp, len);
}

static void
make_volume_orc()
{
  OrcCompileResult res;

  /* int16 destination samples that get scaled by int32 volumes */
  p = orc_program_new ();
  orc_program_set_backup_function (p, do_volume_backup);
  orc_program_add_destination (p, 2, "d1");
  orc_program_add_source (p, 4, "s1");
  orc_program_add_source (p, 2, "s2");

  /* a temporary for the upscaled input samples */
  orc_program_add_temporary (p, 4, "t1");

  /* multiply with the volume, keeping only the high 32bits */
  orc_program_append (p, "mulhslw", ORC_VAR_T1, ORC_VAR_S1, ORC_VAR_S2);
  /* pack an saturate do 16 bits again */
  orc_program_append_ds (p, "convssslw", ORC_VAR_D1, ORC_VAR_T1);

  /* Compile the program */
  res = orc_program_compile (p);
  fprintf (stderr, "result: %d\n", res);

  if (res == ORC_COMPILE_RESULT_OK)
    fprintf (stderr, "%s\n", orc_program_get_asm_code (p));
}

static void
do_volume_orc (orc_int16 *dest, orc_int32 *volumes, orc_int16 *samp, int length)
{
  OrcExecutor _ex;
  OrcExecutor *ex = &_ex;

  /* Set the values on the executor structure */
  orc_executor_set_program (ex, p);
  orc_executor_set_n (ex, length);
  orc_executor_set_array (ex, ORC_VAR_D1, dest);
  orc_executor_set_array (ex, ORC_VAR_S1, volumes);
  orc_executor_set_array (ex, ORC_VAR_S2, samp);

  /* Run the program.  This calls the code that was generated above,
   * or, if the compilation failed, will emulate the program. */
  orc_executor_run (ex);
}

static orc_uint64
get_timestamp ()
{
#ifndef _MSC_VER
  struct timeval now;

  gettimeofday (&now, NULL);

  return now.tv_sec * 1000000LL + now.tv_usec;
#else
  return 0;
#endif
}

#define TIMES 100000
#define N 1024

orc_int16 dest[N];
orc_int16 samp[N];
orc_int32 vols[N];

int
main (int argc, char *argv[])
{
  int i;
  orc_uint64 start, stop;

  /* orc_init() must be called before any other Orc function */
  orc_init ();

  orc_debug_set_level (ORC_DEBUG_LOG);
  register_instr ();

  make_volume_orc();
  orc_debug_set_level (ORC_DEBUG_NONE);

  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    dest[i] = 0;
    samp[i] = i + 1;
    vols[i] = 0x10000 + i;
  }

  start = get_timestamp ();
  for (i = 0; i < TIMES; i++)
    do_volume_c (dest, vols, samp, N);
  stop = get_timestamp ();
  printf ("elapsed C: %d ms\n", (int) (stop - start));


  start = get_timestamp ();
  for (i = 0; i < TIMES; i++)
    do_volume_orc (dest, vols, samp, N);
  stop = get_timestamp ();
  printf ("elapsed ORC: %d ms\n", (int) (stop - start));

  /* Print the results */
  for(i=0;i<20;i++){
    printf("%d: %d -> %d\n", i, samp[i], dest[i]);
  }

  return 0;
}
