
#include <orc-float/orcfloat.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orc/x86.h>

#define X86_MODRM(mod, rm, reg) ((((mod)&3)<<6)|(((rm)&7)<<0)|(((reg)&7)<<3))

#ifdef HAVE_AMD64
int x86_64 = 1;
int x86_ptr_size = 8;
int x86_exec_ptr = X86_EDI;
#else
int x86_64 = 0;
int x86_ptr_size = 4;
int x86_exec_ptr = X86_EBP;
#endif

const char *
x86_get_regname_sse(int i)
{
  static const char *x86_regs[] = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };

  if (i>=X86_XMM0 && i<X86_XMM0 + 16) return x86_regs[i - X86_XMM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

int
x86_get_regnum(int i)
{
  return (i&0xf);
}

void
x86_emit_rex (OrcCompiler *compiler, int size, int reg1, int reg2, int reg3)
{
  int rex = 0x40;

  if (x86_64) {
    if (size >= 8) rex |= 0x08;
    if (reg1 == 1 || (x86_get_regnum(reg1)>=8)) rex |= 0x4;
    if (reg2 == 1 || (x86_get_regnum(reg2)>=8)) rex |= 0x2;
    if (reg3 == 1 || (x86_get_regnum(reg3)>=8)) rex |= 0x1;

    if (rex != 0x40) *compiler->codeptr++ = rex;
  }
}

void
x86_emit_modrm_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  *compiler->codeptr++ = X86_MODRM(3, reg1, reg2);
}

void
sse_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0xf2;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}

void
sse_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0xf3;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}

void
sse_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}

void
sse_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}


#define UNARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_0f (p, insn_name, code, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_0f (p, insn_name, code, \
      p->vars[insn->src_args[1]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}


BINARY(addf, "addps", 0x58)
BINARY(subf, "subps", 0x5c)
BINARY(mulf, "mulps", 0x59)
BINARY(divf, "divps", 0x5e)
BINARY(maxf, "maxps", 0x5f)
BINARY(minf, "minps", 0x5d)
UNARY(invf, "rcpps", 0x53)
UNARY(sqrtf, "sqrtps", 0x51)
UNARY(invsqrtf, "rsqrtps", 0x52)

static void
sse_rule_cmpeqf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_0f (p, "cmpeqps", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x00;
}

static void
sse_rule_cmpltf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_0f (p, "cmpltps", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x01;
}

static void
sse_rule_cmplef (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_0f (p, "cmpleps", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x02;
}

static void
sse_rule_convfl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "cvtps2dq", 0x5b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convlf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_0f (p, "cvtdq2ps", 0x5b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

#define UNARY_66(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_660f (p, insn_name, code, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY_66(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_660f (p, insn_name, code, \
      p->vars[insn->src_args[1]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

BINARY_66(addg, "addpd", 0x58)
BINARY_66(subg, "subpd", 0x5c)
BINARY_66(mulg, "mulpd", 0x59)
BINARY_66(divg, "divpd", 0x5e)
BINARY_66(maxg, "maxpd", 0x5f)
BINARY_66(ming, "minpd", 0x5d)
#if 0
/* These don't actually exist */
UNARY_66(invg, "rcppd", 0x53)
UNARY_66(sqrtg, "sqrtpd", 0x51)
UNARY_66(invsqrtg, "rsqrtpd", 0x52)
#endif

static void
sse_rule_cmpeqg (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "cmpeqpd", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x00;
}

static void
sse_rule_cmpltg (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "cmpltpd", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x01;
}

static void
sse_rule_cmpleg (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "cmplepd", 0xc2,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
  *p->codeptr++ = 0x02;
}

static void
sse_rule_convgl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_f20f (p, "cvtpd2dq", 0xe6,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convlg (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_f30f (p, "cvtdq2pd", 0xe6,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convgf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "cvtpd2ps", 0x5a,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convfg (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_0f (p, "cvtps2pd", 0x5a,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}


void
orc_float_sse_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("float"),
      orc_target_get_by_name ("sse"));

  orc_rule_register (rule_set, "addf", sse_rule_addf, NULL);
  orc_rule_register (rule_set, "subf", sse_rule_subf, NULL);
  orc_rule_register (rule_set, "mulf", sse_rule_mulf, NULL);
  orc_rule_register (rule_set, "divf", sse_rule_divf, NULL);
  orc_rule_register (rule_set, "minf", sse_rule_minf, NULL);
  orc_rule_register (rule_set, "maxf", sse_rule_maxf, NULL);
  orc_rule_register (rule_set, "invf", sse_rule_invf, NULL);
  orc_rule_register (rule_set, "sqrtf", sse_rule_sqrtf, NULL);
  orc_rule_register (rule_set, "invsqrtf", sse_rule_invsqrtf, NULL);
  orc_rule_register (rule_set, "cmpeqf", sse_rule_cmpeqf, NULL);
  orc_rule_register (rule_set, "cmpltf", sse_rule_cmpltf, NULL);
  orc_rule_register (rule_set, "cmplef", sse_rule_cmplef, NULL);
  orc_rule_register (rule_set, "convfl", sse_rule_convfl, NULL);
  orc_rule_register (rule_set, "convlf", sse_rule_convlf, NULL);

  orc_rule_register (rule_set, "addg", sse_rule_addg, NULL);
  orc_rule_register (rule_set, "subg", sse_rule_subg, NULL);
  orc_rule_register (rule_set, "mulg", sse_rule_mulg, NULL);
  orc_rule_register (rule_set, "divg", sse_rule_divg, NULL);
  orc_rule_register (rule_set, "ming", sse_rule_ming, NULL);
  orc_rule_register (rule_set, "maxg", sse_rule_maxg, NULL);
#if 0
/* These don't actually exist */
  orc_rule_register (rule_set, "invg", sse_rule_invg, NULL);
  orc_rule_register (rule_set, "sqrtg", sse_rule_sqrtg, NULL);
  orc_rule_register (rule_set, "invsqrtg", sse_rule_invsqrtg, NULL);
#endif
  orc_rule_register (rule_set, "cmpeqg", sse_rule_cmpeqg, NULL);
  orc_rule_register (rule_set, "cmpltg", sse_rule_cmpltg, NULL);
  orc_rule_register (rule_set, "cmpleg", sse_rule_cmpleg, NULL);
  orc_rule_register (rule_set, "convgl", sse_rule_convgl, NULL);
  orc_rule_register (rule_set, "convlg", sse_rule_convlg, NULL);

  orc_rule_register (rule_set, "convgf", sse_rule_convgf, NULL);
  orc_rule_register (rule_set, "convfg", sse_rule_convfg, NULL);
}


