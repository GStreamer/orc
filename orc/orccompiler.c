
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(HAVE_PTHREAD_JIT)
  #include <pthread.h>
#endif

#if defined(HAVE_CODEMEM_VIRTUALALLOC)
#include <windows.h>
  #ifdef ORC_WINAPI_ONLY_APP
    #define _virtualprotect VirtualProtectFromApp
  #else
    #define _virtualprotect VirtualProtect
  #endif
#endif

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>

#ifdef HAVE_VALGRIND_VALGRIND_H
#include <valgrind/valgrind.h>
#endif

/**
 * SECTION:orccompiler
 * @title: OrcCompiler
 * @short_description: Compile Orc programs
 *
 * OrcCompiler is the object used to convert Orc programs contained
 * in an OrcProgram object into assembly code and object code.
 *
 * The OrcCompileResult enum is used to indicate whether or not
 * a compilation attempt was successful or not.  The macros
 * ORC_COMPILE_RESULT_IS_SUCCESSFUL() and ORC_COMPILE_RESULT_IS_FATAL()
 * should be used instead of checking values directly.
 *
 * When a program is compiled, the compiler calls the functions
 * contained in various OrcRule structures.  These functions generate
 * assembly and object instructions by calling ORC_ASM_CODE()
 * or functions that use ORC_ASM_CODE() internally.
 */

static void orc_compiler_assign_rules (OrcCompiler *compiler);
static void orc_compiler_global_reg_alloc (OrcCompiler *compiler);
static void orc_compiler_rewrite_insns (OrcCompiler *compiler);
static void orc_compiler_rewrite_vars (OrcCompiler *compiler);
static void orc_compiler_rewrite_vars2 (OrcCompiler *compiler);
static int orc_compiler_dup_temporary (OrcCompiler *compiler, int var, int j);
static int orc_compiler_new_temporary (OrcCompiler *compiler, int size);
static void orc_compiler_check_sizes (OrcCompiler *compiler);

static char **_orc_compiler_flag_list;
int _orc_compiler_flag_backup;
int _orc_compiler_flag_emulate;
int _orc_compiler_flag_debug;
int _orc_compiler_flag_randomize;

/* For Windows */
int _orc_codemem_alignment;

void
_orc_compiler_init (void)
{
  char *envvar;
#ifdef HAVE_CODEMEM_VIRTUALALLOC
  size_t page_size;
  SYSTEM_INFO info;
#endif

  envvar = _orc_getenv ("ORC_CODE");
  if (envvar != NULL) {
    _orc_compiler_flag_list = strsplit (envvar, ',');
    free (envvar);
  }

  _orc_compiler_flag_backup = orc_compiler_flag_check ("backup");
  _orc_compiler_flag_emulate = orc_compiler_flag_check ("emulate");
  _orc_compiler_flag_debug = orc_compiler_flag_check ("debug");
  _orc_compiler_flag_randomize = orc_compiler_flag_check ("randomize");

  /* 16 bytes alignment by default */
  _orc_codemem_alignment = 15;

#ifdef HAVE_CODEMEM_VIRTUALALLOC
  GetNativeSystemInfo(&info);
  page_size = info.dwPageSize;

  /* Protection attribute change via VirtualProtect will be applied per
   * page memory unit. So we should split code memory with page aligned range.
   * Otherwise the protection attribute of previously generated executable code
   * memory can be affected by later generated one.
   */
  _orc_codemem_alignment = info.dwPageSize - 1;
#endif

#ifdef ORC_WINAPI_ONLY_APP
  if (!_orc_compiler_flag_backup && !_orc_compiler_flag_emulate) {
    int can_jit = FALSE;

    /* If backup code is not enabled and emulation is not enabled, that means
     * we will do JIT compilation and call orc_code_region_allocate_codemem().
     * When targeting Windows Store apps, the codeGeneration capability must
     * be enabled in the app manifest, or passing PAGE_EXECUTE to
     * VirtualProtectFromApp will return NULL. In this case, we must force
     * backup C code, and if that's not available, we must emulate. */
    void *mem = VirtualAllocFromApp (NULL, page_size, MEM_COMMIT,
        PAGE_READWRITE);
    if (mem) {
      int old_protect;
      if (VirtualProtectFromApp (mem, page_size, PAGE_EXECUTE, &old_protect) > 0)
        can_jit = TRUE;
      VirtualFree (mem, 0, MEM_RELEASE);
    }

    if (!can_jit) {
      ORC_WARNING ("Unable to allocate executable pages: using backup code or "
        "emulation: codeGeneration capability isn't set in the app manifest?");
      _orc_compiler_flag_backup = TRUE;
      _orc_compiler_flag_emulate = TRUE;
    }
  }
#endif

#if defined(HAVE_PTHREAD_JIT)
  ORC_INFO("pthread_jit_write_protect_supported_np() = %i",
      pthread_jit_write_protect_supported_np());
#endif
}

int
orc_compiler_flag_check (const char *flag)
{
  int i;

  if (_orc_compiler_flag_list == NULL) return FALSE;

  for (i=0;_orc_compiler_flag_list[i];i++){
    if (strcmp (_orc_compiler_flag_list[i], flag) == 0) return TRUE;
  }
  return FALSE;
}

static int
orc_compiler_allocate_register (OrcCompiler *compiler, int data_reg)
{
  int i;
  int roff;
  int reg;
  int offset;

  if (data_reg) {
    offset = compiler->target->data_register_offset;
  } else {
    offset = ORC_GP_REG_BASE;
  }

  roff = 0;
  if (_orc_compiler_flag_randomize) {
    /* for testing */
    roff = rand()&0x1f;
  }

  for(i=0;i<32;i++){
    reg = offset + ((roff + i)&0x1f);
    if (compiler->valid_regs[reg] &&
        !compiler->save_regs[reg] &&
        compiler->alloc_regs[reg] == 0) {
      compiler->alloc_regs[reg]++;
      compiler->used_regs[reg] = 1;
      return reg;
    }
  }
  for(i=0;i<32;i++){
    reg = offset + ((roff + i)&0x1f);
    if (compiler->valid_regs[reg] &&
        compiler->alloc_regs[reg] == 0) {
      compiler->alloc_regs[reg]++;
      compiler->used_regs[reg] = 1;
      return reg;
    }
  }

  if (data_reg || !compiler->allow_gp_on_stack) {
    orc_compiler_error (compiler, "register overflow for %s register",
        data_reg ? "vector" : "gp");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }

  return 0;
}

/**
 * orc_program_compile:
 * @program: the OrcProgram to compile
 *
 * Compiles an Orc program for the current CPU.  If successful,
 * executable code for the program was generated and can be
 * executed.
 *
 * The return value indicates various levels of success or failure.
 * Success can be determined by checking for a true value of the
 * macro ORC_COMPILE_RESULT_IS_SUCCESSFUL() on the return value.  This
 * indicates that executable code was generated.  If the macro
 * ORC_COMPILE_RESULT_IS_FATAL() on the return value evaluates to
 * true, then there was a syntactical error in the program.  If the
 * result is neither successful nor fatal, the program can still be
 * emulated.
 *
 * Returns: an OrcCompileResult
 */
OrcCompileResult
orc_program_compile (OrcProgram *program)
{
  return orc_program_compile_for_target (program, orc_target_get_default ());
}

/**
 * orc_program_compile_for_target:
 * @program: the OrcProgram to compile
 *
 * Compiles an Orc program for the given target, using the
 * default target flags for that target.
 *
 * Returns: an OrcCompileResult
 */
OrcCompileResult
orc_program_compile_for_target (OrcProgram *program, OrcTarget *target)
{
  unsigned int flags;

  if (target) {
    flags = target->get_default_flags ();
  } else {
    flags = 0;
  }

  return orc_program_compile_full (program, target, flags);
}

#if defined(HAVE_CODEMEM_VIRTUALALLOC)
static orc_bool
_set_virtual_protect (void * mem, size_t size, int code_protect)
{
  char *msg;
  DWORD old_protect;

  /* No code, so we 'succeed' */
  if (size == 0)
    return TRUE;

  if (!mem)
    return FALSE;

  if (_virtualprotect (mem, size, code_protect, &old_protect) > 0)
    return TRUE;

  FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER
      | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
      GetLastError (), 0, (LPTSTR) &msg, 0, NULL);
  ORC_ERROR ("Couldn't set memory protect on %p from %x to %x: %s", mem,
      old_protect, code_protect, msg);
  LocalFree (msg);

  return FALSE;
}
#endif

/**
 * orc_program_compile_full:
 * @program: the OrcProgram to compile
 *
 * Compiles an Orc program for the given target, using the
 * given target flags.
 *
 * Returns: an OrcCompileResult
 */
OrcCompileResult
orc_program_compile_full (OrcProgram *program, OrcTarget *target,
    unsigned int flags)
{
  OrcCompiler *compiler;
  int i;
  OrcCompileResult result;
  const char *error_msg;

  ORC_INFO("initializing compiler for program \"%s\"", program->name);
  error_msg = orc_program_get_error (program);
  if (error_msg && strcmp (error_msg, "")) {
    ORC_WARNING ("program %s failed to compile, reason: %s",
        program->name, error_msg);
    return ORC_COMPILE_RESULT_UNKNOWN_PARSE;
  }

  if (program->orccode) {
    orc_code_free (program->orccode);
    program->orccode = NULL;
  }

  if (program->asm_code) {
    free (program->asm_code);
    program->asm_code = NULL;
  }

  compiler = malloc (sizeof(OrcCompiler));
  memset (compiler, 0, sizeof(OrcCompiler));

  if (program->backup_func) {
    program->code_exec = program->backup_func;
  } else {
    program->code_exec = (void *)orc_executor_emulate;
  }

  compiler->program = program;
  compiler->target = target;
  compiler->target_flags = flags;

  {
    ORC_LOG("variables");
    for(i=0;i<ORC_N_VARIABLES;i++){
      if (program->vars[i].size > 0) {
        ORC_LOG("%d: %s size %d type %d alloc %d", i,
            program->vars[i].name,
            program->vars[i].size,
            program->vars[i].vartype,
            program->vars[i].alloc);
      }
    }
    ORC_LOG("instructions");
    for(i=0;i<program->n_insns;i++){
      ORC_LOG("%d: %s %d %d %d %d", i,
          program->insns[i].opcode->name,
          program->insns[i].dest_args[0],
          program->insns[i].dest_args[1],
          program->insns[i].src_args[0],
          program->insns[i].src_args[1]);
    }
  }

  memcpy (compiler->insns, program->insns,
      program->n_insns * sizeof(OrcInstruction));
  compiler->n_insns = program->n_insns;

  memcpy (compiler->vars, program->vars,
      ORC_N_VARIABLES * sizeof(OrcVariable));
  memset (compiler->vars + ORC_N_VARIABLES, 0,
      (ORC_N_COMPILER_VARIABLES - ORC_N_VARIABLES) * sizeof(OrcVariable));
  compiler->n_temp_vars = program->n_temp_vars;
  compiler->n_dup_vars = 0;

  for(i=0;i<32;i++) {
    compiler->valid_regs[i] = 1;
  }

  orc_compiler_check_sizes (compiler);
  if (compiler->error) goto error;

  if (compiler->target) {
    compiler->target->compiler_init (compiler);
  }

  orc_compiler_rewrite_insns (compiler);
  if (compiler->error) goto error;

  orc_compiler_rewrite_vars (compiler);
  if (compiler->error) goto error;

#if 0
  {
    ORC_ERROR("variables");
    for(i=0;i<ORC_N_VARIABLES;i++){
      if (compiler->vars[i].size > 0) {
        ORC_ERROR("%d: %s size %d type %d alloc %d [%d,%d]", i,
            compiler->vars[i].name,
            compiler->vars[i].size,
            compiler->vars[i].vartype,
            compiler->vars[i].alloc,
            compiler->vars[i].first_use,
            compiler->vars[i].last_use);
      }
    }
    ORC_ERROR("instructions");
    for(i=0;i<compiler->n_insns;i++){
      ORC_ERROR("%d: %s %d %d %d %d", i,
          compiler->insns[i].opcode->name,
          compiler->insns[i].dest_args[0],
          compiler->insns[i].dest_args[1],
          compiler->insns[i].src_args[0],
          compiler->insns[i].src_args[1]);
    }
  }
#endif
  program->orccode = orc_code_new ();

  program->orccode->is_2d = program->is_2d;
  program->orccode->constant_n = program->constant_n;
  program->orccode->constant_m = program->constant_m;
  program->orccode->exec = program->code_exec;

  program->orccode->n_insns = compiler->n_insns;
  program->orccode->insns = malloc(sizeof(OrcInstruction) * compiler->n_insns);
  memcpy (program->orccode->insns, compiler->insns,
      sizeof(OrcInstruction) * compiler->n_insns);

  program->orccode->vars = malloc (sizeof(OrcCodeVariable) * ORC_N_COMPILER_VARIABLES);
  memset (program->orccode->vars, 0,
      sizeof(OrcCodeVariable) * ORC_N_COMPILER_VARIABLES);
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    program->orccode->vars[i].vartype = compiler->vars[i].vartype;
    program->orccode->vars[i].size = compiler->vars[i].size;
    program->orccode->vars[i].value = compiler->vars[i].value;
  }

  if (program->backup_func && (_orc_compiler_flag_backup || target == NULL)) {
    orc_compiler_error (compiler, "Compilation disabled, using backup");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

  if (_orc_compiler_flag_emulate || target == NULL) {
    program->code_exec = (void *)orc_executor_emulate;
    program->orccode->exec = (void *)orc_executor_emulate;
    orc_compiler_error (compiler, "Compilation disabled, using emulation");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

  if (compiler->target) {
    orc_compiler_global_reg_alloc (compiler);

    orc_compiler_rewrite_vars2 (compiler);
  }
  if (compiler->error) goto error;

  orc_compiler_assign_rules (compiler);
  if (compiler->error) goto error;

  ORC_INFO("allocating code memory");
  compiler->code = malloc(65536);
  compiler->codeptr = compiler->code;

  if (compiler->error) goto error;

  ORC_INFO("compiling for target \"%s\"", compiler->target->name);
  compiler->target->compile (compiler);
  if (compiler->error) {
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

  program->orccode->code_size = compiler->codeptr - compiler->code;
  orc_code_allocate_codemem (program->orccode, program->orccode->code_size);

#if defined(HAVE_PTHREAD_JIT)
  pthread_jit_write_protect_np(0);
#endif
#if defined(HAVE_CODEMEM_VIRTUALALLOC)
  /* Ensure that code region is writable before memcpy */
  _set_virtual_protect (program->orccode->code, program->orccode->code_size,
      PAGE_READWRITE);
#endif
  memcpy (program->orccode->code, compiler->code, program->orccode->code_size);

#ifdef VALGRIND_DISCARD_TRANSLATIONS
  VALGRIND_DISCARD_TRANSLATIONS (program->orccode->exec,
      program->orccode->code_size);
#endif

  if (compiler->target->flush_cache) {
    compiler->target->flush_cache (program->orccode);
  }

#if defined(HAVE_PTHREAD_JIT)
  pthread_jit_write_protect_np(1);
#endif
#if defined(HAVE_CODEMEM_VIRTUALALLOC)
  /* Code region is now ready for execution */
 if (!_set_virtual_protect (program->orccode->exec, program->orccode->code_size,
       PAGE_EXECUTE))
   /* Can't set code as executable, force emulation */
   program->orccode->exec = (void *)orc_executor_emulate;
#endif
  program->code_exec = program->orccode->exec;

  program->asm_code = compiler->asm_code;

  result = compiler->result;
  for (i=0;i<compiler->n_dup_vars;i++){
    free(compiler->vars[ORC_VAR_T1 + compiler->n_temp_vars + i].name);
    compiler->vars[ORC_VAR_T1 + compiler->n_temp_vars + i].name = NULL;
  }
  free (compiler->code);
  compiler->code = NULL;
  if (compiler->output_insns) free (compiler->output_insns);
  free (compiler);
  ORC_INFO("finished compiling (success)");

  return result;
error:

  if (compiler->error_msg) {
    ORC_WARNING ("program %s failed to compile, reason: %s",
        program->name, compiler->error_msg);
  } else {
    ORC_WARNING("program %s failed to compile, reason %d",
        program->name, compiler->result);
  }
  result = compiler->result;
  orc_program_set_error (program, compiler->error_msg);
  free (compiler->error_msg);
  if (result == 0) {
    result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
  if (compiler->asm_code) {
    free (compiler->asm_code);
    compiler->asm_code = NULL;
  }
  for (i=0;i<compiler->n_dup_vars;i++){
    free(compiler->vars[ORC_VAR_T1 + compiler->n_temp_vars + i].name);
    compiler->vars[ORC_VAR_T1 + compiler->n_temp_vars + i].name = NULL;
  }
  free (compiler->code);
  compiler->code = NULL;
  if (compiler->output_insns) free (compiler->output_insns);
  free (compiler);
  ORC_INFO("finished compiling (fail)");
  return result;
}

static void
orc_compiler_check_sizes (OrcCompiler *compiler)
{
  int i;
  int j;
  int max_size = 1;

  for(i=0;i<compiler->n_insns;i++) {
    OrcInstruction *insn = compiler->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;
    int multiplier = 1;

    if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
      multiplier = 2;
    } else if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
      multiplier = 4;
    }

    for(j=0;j<ORC_STATIC_OPCODE_N_DEST;j++){
      if (opcode->dest_size[j] == 0) continue;
      if (multiplier * opcode->dest_size[j] !=
          compiler->vars[insn->dest_args[j]].size) {
        ORC_COMPILER_ERROR (compiler, "size mismatch, opcode %s dest[%d] is %d should be %d",
            opcode->name, j, compiler->vars[insn->dest_args[j]].size,
            multiplier * opcode->dest_size[j]);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        return;
      }
      max_size = MAX(max_size, multiplier * opcode->dest_size[j]);
    }
    for(j=0;j<ORC_STATIC_OPCODE_N_SRC;j++){
      if (opcode->src_size[j] == 0) continue;
      if (multiplier * opcode->src_size[j] !=
          compiler->vars[insn->src_args[j]].size &&
          compiler->vars[insn->src_args[j]].vartype != ORC_VAR_TYPE_PARAM &&
          compiler->vars[insn->src_args[j]].vartype != ORC_VAR_TYPE_CONST) {
        ORC_COMPILER_ERROR(compiler, "size mismatch, opcode %s src[%d] is %d should be %d",
            opcode->name, j, compiler->vars[insn->src_args[j]].size,
            multiplier * opcode->src_size[j]);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        return;
      }
      if (opcode->flags & ORC_STATIC_OPCODE_SCALAR && j >= 1 &&
          compiler->vars[insn->src_args[j]].vartype != ORC_VAR_TYPE_PARAM &&
          compiler->vars[insn->src_args[j]].vartype != ORC_VAR_TYPE_CONST) {
        ORC_COMPILER_ERROR(compiler, "opcode %s requires const or param source",
            opcode->name);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        return;
      }
      max_size = MAX(max_size, multiplier * opcode->src_size[j]);
    }
    if (opcode->flags & ORC_STATIC_OPCODE_SCALAR &&
        opcode->src_size[1] == 0 &&
        compiler->vars[insn->src_args[0]].vartype != ORC_VAR_TYPE_PARAM &&
        compiler->vars[insn->src_args[0]].vartype != ORC_VAR_TYPE_CONST) {
      ORC_COMPILER_ERROR(compiler, "opcode %s requires const or param source",
          opcode->name);
      compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
      return;
    }
  }
  compiler->max_var_size = max_size;
}

static OrcStaticOpcode *
get_load_opcode_for_size (int size)
{
  switch (size) {
    case 1:
      return orc_opcode_find_by_name ("loadb");
    case 2:
      return orc_opcode_find_by_name ("loadw");
    case 4:
      return orc_opcode_find_by_name ("loadl");
    case 8:
      return orc_opcode_find_by_name ("loadq");
    default:
      ORC_ASSERT(0);
  }
  return NULL;
}

static OrcStaticOpcode *
get_loadp_opcode_for_size (int size)
{
  switch (size) {
    case 1:
      return orc_opcode_find_by_name ("loadpb");
    case 2:
      return orc_opcode_find_by_name ("loadpw");
    case 4:
      return orc_opcode_find_by_name ("loadpl");
    case 8:
      return orc_opcode_find_by_name ("loadpq");
    default:
      ORC_ASSERT(0);
  }
  return NULL;
}

static OrcStaticOpcode *
get_store_opcode_for_size (int size)
{
  switch (size) {
    case 1:
      return orc_opcode_find_by_name ("storeb");
    case 2:
      return orc_opcode_find_by_name ("storew");
    case 4:
      return orc_opcode_find_by_name ("storel");
    case 8:
      return orc_opcode_find_by_name ("storeq");
    default:
      ORC_ASSERT(0);
  }
  return NULL;
}

static void
orc_compiler_rewrite_insns (OrcCompiler *compiler)
{
  int i;
  int j;
  OrcStaticOpcode *opcode;
  OrcProgram *program = compiler->program;

  compiler->n_insns = 0;
  for(j=0;j<program->n_insns;j++){
    OrcInstruction insn;
    OrcInstruction *xinsn;

    memcpy (&insn, program->insns + j, sizeof(OrcInstruction));
    opcode = insn.opcode;

    if (!(opcode->flags & ORC_STATIC_OPCODE_LOAD)) {
      for(i=0;i<ORC_STATIC_OPCODE_N_SRC;i++){
        OrcVariable *var;

        if (opcode->src_size[i] == 0) continue;
        var = compiler->vars + insn.src_args[i];

        if (i > 0 && (opcode->flags & ORC_STATIC_OPCODE_SCALAR) &&
            (!compiler->load_params || var->vartype != ORC_VAR_TYPE_PARAM))
          continue;

        if (var->vartype == ORC_VAR_TYPE_SRC ||
            var->vartype == ORC_VAR_TYPE_DEST) {
          OrcInstruction *cinsn;

          cinsn = compiler->insns + compiler->n_insns;
          compiler->n_insns++;

          cinsn->flags = insn.flags;
          cinsn->flags |= ORC_INSN_FLAG_ADDED;
          cinsn->flags &= ~(ORC_INSTRUCTION_FLAG_X2|ORC_INSTRUCTION_FLAG_X4);
          cinsn->opcode = get_load_opcode_for_size (var->size);
          cinsn->dest_args[0] = orc_compiler_new_temporary (compiler,
              var->size);
          cinsn->src_args[0] = insn.src_args[i];
          insn.src_args[i] = cinsn->dest_args[0];
        } else if (var->vartype == ORC_VAR_TYPE_CONST ||
            var->vartype == ORC_VAR_TYPE_PARAM) {
          OrcInstruction *cinsn;
          int l, multiplier, loaded;

          multiplier = 1;
          if (insn.flags & ORC_INSTRUCTION_FLAG_X2) {
            multiplier = 2;
          }
          if (insn.flags & ORC_INSTRUCTION_FLAG_X4) {
            multiplier = 4;
          }

          loaded = -1;
          for(l=0;l<ORC_N_COMPILER_VARIABLES;l++){
            if (compiler->vars[l].name == NULL) continue;
            if (!compiler->vars[l].has_parameter) continue;
            if (compiler->vars[l].parameter != insn.src_args[i]) continue;
            if (compiler->vars[l].size != opcode->src_size[i] * multiplier) continue;
            loaded = l;
            break;
          }
          if (loaded != -1) {
            insn.src_args[i] = loaded;
            continue;
          }
          cinsn = compiler->insns + compiler->n_insns;
          compiler->n_insns++;

          cinsn->flags = insn.flags;
          cinsn->flags |= ORC_INSN_FLAG_ADDED;
          cinsn->opcode = get_loadp_opcode_for_size (opcode->src_size[i]);
          cinsn->dest_args[0] = orc_compiler_new_temporary (compiler,
              opcode->src_size[i] * multiplier);
          if (var->vartype == ORC_VAR_TYPE_CONST) {
            compiler->vars[cinsn->dest_args[0]].flags |=
                ORC_VAR_FLAG_VOLATILE_WORKAROUND;
          }
          compiler->vars[cinsn->dest_args[0]].has_parameter = TRUE;
          compiler->vars[cinsn->dest_args[0]].parameter = insn.src_args[i];
          cinsn->src_args[0] = insn.src_args[i];
          insn.src_args[i] = cinsn->dest_args[0];
        }
      }
    }

    xinsn = compiler->insns + compiler->n_insns;
    memcpy (xinsn, &insn, sizeof(OrcInstruction));
    compiler->n_insns++;

    if (!(opcode->flags & ORC_STATIC_OPCODE_STORE)) {
      for(i=0;i<ORC_STATIC_OPCODE_N_DEST;i++){
        OrcVariable *var;

        if (opcode->dest_size[i] == 0) continue;

        var = compiler->vars + insn.dest_args[i];
        if (var->vartype == ORC_VAR_TYPE_DEST) {
          OrcInstruction *cinsn;

          cinsn = compiler->insns + compiler->n_insns;
          compiler->n_insns++;

          cinsn->flags = xinsn->flags;
          cinsn->flags |= ORC_INSN_FLAG_ADDED;
          cinsn->flags &= ~(ORC_INSTRUCTION_FLAG_X2|ORC_INSTRUCTION_FLAG_X4);
          cinsn->opcode = get_store_opcode_for_size (var->size);
          cinsn->src_args[0] = orc_compiler_new_temporary (compiler, var->size);
          cinsn->dest_args[0] = xinsn->dest_args[i];
          xinsn->dest_args[i] = cinsn->src_args[0];
        }
      }
    }

  }
}

static void
orc_compiler_assign_rules (OrcCompiler *compiler)
{
  int i;

  for(i=0;i<compiler->n_insns;i++) {
    OrcInstruction *insn = compiler->insns + i;

    insn->rule = orc_target_get_rule (compiler->target, insn->opcode,
        compiler->target_flags);

    if (insn->rule == NULL || insn->rule->emit == NULL) {
      orc_compiler_error (compiler, "no code generation rule for %s on "
          "target %s", insn->opcode->name, compiler->target->name);
      compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
      return;
    }
  }
}

int
orc_compiler_get_temp_reg (OrcCompiler *compiler)
{
  int j;

  for(j=0;j<ORC_N_REGS;j++){
    compiler->alloc_regs[j] = 0;
  }
  for(j=0;j<ORC_N_COMPILER_VARIABLES;j++){
    if (!compiler->vars[j].alloc) continue;

    ORC_DEBUG("var %d: %d  %d %d", j, compiler->vars[j].alloc,
        compiler->vars[j].first_use,
        compiler->vars[j].last_use);

    if (compiler->vars[j].first_use == -1) {
      compiler->alloc_regs[compiler->vars[j].alloc] = 1;
    } else if (compiler->vars[j].first_use <= compiler->insn_index &&
        compiler->vars[j].last_use >= compiler->insn_index) {
      compiler->alloc_regs[compiler->vars[j].alloc] = 1;
    }
  }
  for(j=0;j<compiler->n_constants;j++){
    if (compiler->constants[j].alloc_reg) {
      compiler->alloc_regs[compiler->constants[j].alloc_reg] = 1;
    }
  }

  ORC_DEBUG("at insn %d %s", compiler->insn_index,
      compiler->insns[compiler->insn_index].opcode->name);

  for(j=compiler->min_temp_reg;j<ORC_VEC_REG_BASE+32;j++){
    if (compiler->valid_regs[j] && !compiler->alloc_regs[j]) {
      compiler->min_temp_reg = j+1;
      if (compiler->max_used_temp_reg < j) compiler->max_used_temp_reg = j;
      return j;
    }
  }

  orc_compiler_error (compiler, "no temporary register available");
  compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;

  return 0;
}

static void
orc_compiler_rewrite_vars (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  int var;
  int actual_var;

  for(j=0;j<ORC_N_COMPILER_VARIABLES;j++){
    if (compiler->vars[j].alloc) continue;
    compiler->vars[j].last_use = -1;
  }
  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    /* set up args */
    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      if (opcode->src_size[k] == 0) continue;

      var = insn->src_args[k];
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_DEST) {
        compiler->vars[var].load_dest = TRUE;
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_SRC ||
          compiler->vars[var].vartype == ORC_VAR_TYPE_DEST ||
          compiler->vars[var].vartype == ORC_VAR_TYPE_CONST ||
          compiler->vars[var].vartype == ORC_VAR_TYPE_PARAM) {
        continue;
      }

      actual_var = var;
      if (compiler->vars[var].replaced) {
        actual_var = compiler->vars[var].replacement;
        insn->src_args[k] = actual_var;
      }

      if (!compiler->vars[var].used) {
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          ORC_COMPILER_ERROR(compiler, "using uninitialized temp var at line %d", insn->line);
          compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        }
        compiler->vars[var].used = TRUE;
        compiler->vars[var].first_use = j;
      }
      compiler->vars[actual_var].last_use = j;
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      if (opcode->dest_size[k] == 0) continue;

      var = insn->dest_args[k];

      if (compiler->vars[var].vartype == ORC_VAR_TYPE_DEST) {
        continue;
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_SRC) {
        ORC_COMPILER_ERROR(compiler,"using src var as dest at line %d", insn->line);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_CONST) {
        ORC_COMPILER_ERROR(compiler,"using const var as dest at line %d", insn->line);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
      }
      if (compiler->vars[var].vartype == ORC_VAR_TYPE_PARAM) {
        ORC_COMPILER_ERROR(compiler,"using param var as dest at line %d", insn->line);
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
      }
      if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
        if (compiler->vars[var].vartype != ORC_VAR_TYPE_ACCUMULATOR) {
          ORC_COMPILER_ERROR(compiler,"accumulating opcode to non-accumulator dest at line %d", insn->line);
          compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        }
      } else {
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
          ORC_COMPILER_ERROR(compiler,"non-accumulating opcode to accumulator dest at line %d", insn->line);
          compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        }
      }

      actual_var = var;
      if (compiler->vars[var].replaced) {
        actual_var = compiler->vars[var].replacement;
        insn->dest_args[k] = actual_var;
      }

      if (!compiler->vars[var].used) {
        compiler->vars[actual_var].used = TRUE;
        compiler->vars[actual_var].first_use = j;
      } else {
#if 0
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_DEST) {
          ORC_COMPILER_ERROR(compiler,"writing dest more than once at line %d", insn->line);
          compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        }
#endif
        if (compiler->vars[var].vartype == ORC_VAR_TYPE_TEMP) {
          actual_var = orc_compiler_dup_temporary (compiler, var, j);
          compiler->vars[var].replaced = TRUE;
          compiler->vars[var].replacement = actual_var;
          insn->dest_args[k] = actual_var;
          compiler->vars[actual_var].used = TRUE;
          compiler->vars[actual_var].first_use = j;
        }
      }
      compiler->vars[actual_var].last_use = j;
    }
  }
}

static void
orc_compiler_global_reg_alloc (OrcCompiler *compiler)
{
  int i;
  OrcVariable *var;

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    var = compiler->vars + i;
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        break;
      case ORC_VAR_TYPE_PARAM:
        break;
      case ORC_VAR_TYPE_SRC:
        var->ptr_register = orc_compiler_allocate_register (compiler, FALSE);
        if (compiler->need_mask_regs) {
          var->mask_alloc = orc_compiler_allocate_register (compiler, TRUE);
          var->ptr_offset = orc_compiler_allocate_register (compiler, FALSE);
          var->aligned_data = orc_compiler_allocate_register (compiler, TRUE);
        }
        if (var->need_offset_reg) {
          var->ptr_offset = orc_compiler_allocate_register (compiler, FALSE);
        }
        break;
      case ORC_VAR_TYPE_DEST:
        var->ptr_register = orc_compiler_allocate_register (compiler, FALSE);
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        var->first_use = -1;
        var->last_use = -1;
        var->alloc = orc_compiler_allocate_register (compiler, TRUE);
        break;
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (compiler, "bad vartype");
        compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE;
        break;
    }

    if (compiler->error) break;
  }

  for(i=0;i<compiler->n_insns;i++){
    OrcInstruction *insn = compiler->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;

    if (opcode->flags & ORC_STATIC_OPCODE_INVARIANT) {
      var = compiler->vars + insn->dest_args[0];

      var->first_use = -1;
      var->last_use = -1;
      var->alloc = orc_compiler_allocate_register (compiler, TRUE);
      insn->flags |= ORC_INSN_FLAG_INVARIANT;
    }

    if (opcode->flags & ORC_STATIC_OPCODE_ITERATOR) {
      compiler->has_iterator_opcode = TRUE;
    }
  }

  if (compiler->alloc_loop_counter && !compiler->error) {
    compiler->loop_counter = orc_compiler_allocate_register (compiler, FALSE);
    /* FIXME massive hack */
    if (compiler->loop_counter == 0) {
      compiler->error = FALSE;
      compiler->result = ORC_COMPILE_RESULT_OK;
    }
  }
}

static void
orc_compiler_rewrite_vars2 (OrcCompiler *compiler)
{
  int i;
  int j;
  int k;

  for(j=0;j<compiler->n_insns;j++){
#if 1
    /* must be true to chain src1 to dest:
     *  - rule must handle it
     *  - src1 must be last_use
     *  - only one dest
     */
    if (compiler->insns[j].flags & ORC_INSN_FLAG_INVARIANT) continue;

    if (!(compiler->insns[j].opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR)) {
      int src1 = compiler->insns[j].src_args[0];
      int dest;

      if (compiler->insns[j].opcode->dest_size[1] == 0)
        dest = compiler->insns[j].dest_args[0];
      else
        dest = compiler->insns[j].dest_args[1];

      if (compiler->vars[src1].last_use == j) {
        if (compiler->vars[src1].first_use == j) {
          k = orc_compiler_allocate_register (compiler, TRUE);
          compiler->vars[src1].alloc = k;
        }
        compiler->alloc_regs[compiler->vars[src1].alloc]++;
        compiler->vars[dest].alloc = compiler->vars[src1].alloc;
      }
    }
#endif

    if (0) {
      /* immediate operand, don't load */
      int src2 = compiler->insns[j].src_args[1];
      compiler->vars[src2].alloc = 1;
    } else {
      int src2 = compiler->insns[j].src_args[1];
      if (src2 != -1 && compiler->vars[src2].alloc == 1) {
        compiler->vars[src2].alloc = 0;
      }
    }

    for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
      if (compiler->vars[i].name == NULL) continue;
      if (compiler->vars[i].last_use == -1) continue;
      if (compiler->vars[i].first_use == j) {
        if (compiler->vars[i].alloc) continue;
        k = orc_compiler_allocate_register (compiler, TRUE);
        compiler->vars[i].alloc = k;
      }
    }
    for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
      if (compiler->vars[i].name == NULL) continue;
      if (compiler->vars[i].last_use == j) {
        compiler->alloc_regs[compiler->vars[i].alloc]--;
      }
    }
  }

}

static int
orc_compiler_dup_temporary (OrcCompiler *compiler, int var, int j)
{
  int i = ORC_VAR_T1 + compiler->n_temp_vars + compiler->n_dup_vars;

  compiler->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  compiler->vars[i].size = compiler->vars[var].size;
  compiler->vars[i].name = malloc (strlen(compiler->vars[var].name) + 10);
  sprintf(compiler->vars[i].name, "%s.dup%d", compiler->vars[var].name, j);
  compiler->n_dup_vars++;

  return i;
}

static int
orc_compiler_new_temporary (OrcCompiler *compiler, int size)
{
  int i = ORC_VAR_T1 + compiler->n_temp_vars + compiler->n_dup_vars;

  compiler->vars[i].vartype = ORC_VAR_TYPE_TEMP;
  compiler->vars[i].size = size;
  compiler->vars[i].name = malloc (10);
  sprintf(compiler->vars[i].name, "tmp%d", i);
  compiler->n_dup_vars++;

  return i;
}

#if 0
static void
orc_compiler_dump_asm (OrcCompiler *compiler)
{
  printf("%s", compiler->asm_code);
}
#endif

/**
 * orc_compiler_append_code:
 * @p: an OrcCompiler object
 * @fmt: a printf-style format string
 * @...: optional printf-style arguments
 *
 * Generates a string using sprintf() on the given format and
 * arguments, and appends that string to the generated assembly
 * code for the compiler.
 *
 * This function is used by the ORC_ASM_CODE() macro.
 *
 * This function is useful in a function implementing an OrcRule
 * or implementing a target.
 */
void
orc_compiler_append_code (OrcCompiler *p, const char *fmt, ...)
{
  char tmp[200];
  va_list varargs;
  int n;

  va_start (varargs, fmt);
  vsnprintf(tmp, 200 - 1, fmt, varargs);
  va_end (varargs);

  n = strlen (tmp);
  p->asm_code = realloc (p->asm_code, p->asm_code_len + n + 1);
  memcpy (p->asm_code + p->asm_code_len, tmp, n + 1);
  p->asm_code_len += n;
}

int
orc_compiler_label_new (OrcCompiler *compiler)
{
  return compiler->n_labels++;
}

static void
orc_compiler_load_constant (OrcCompiler *compiler, int reg, int size,
    int value)
{
  compiler->target->load_constant (compiler, reg, size, value);
}

static void
orc_compiler_load_constant_long (OrcCompiler *compiler, int reg,
    OrcConstant *constant)
{
  compiler->target->load_constant_long (compiler, reg, constant);
}

int
orc_compiler_get_temp_constant (OrcCompiler *compiler, int size, int value)
{
  int tmp;

  tmp = orc_compiler_get_temp_reg (compiler);
  orc_compiler_load_constant (compiler, tmp, size, value);
  return tmp;
}

int
orc_compiler_get_constant (OrcCompiler *compiler, int size, int value)
{
  int i;
  int tmp;
  unsigned int v = value;

  if (size < 4) {
    if (size < 2) {
      v &= 0xff;
      v |= (v<<8);
    }
    v &= 0xffff;
    v |= (v<<16);
  }

  for(i=0;i<compiler->n_constants;i++){
    if (compiler->constants[i].is_long == FALSE &&
        compiler->constants[i].value == v) {
      break;
    }
  }
  if (i == compiler->n_constants) {
    compiler->n_constants++;
    compiler->constants[i].value = v;
    compiler->constants[i].alloc_reg = 0;
    compiler->constants[i].use_count = 0;
    compiler->constants[i].is_long = FALSE;
  }

  compiler->constants[i].use_count++;

  if (compiler->constants[i].alloc_reg != 0) {;
    return compiler->constants[i].alloc_reg;
  }
  tmp = orc_compiler_get_temp_reg (compiler);
  orc_compiler_load_constant (compiler, tmp, size, value);
  return tmp;
}

int
orc_compiler_get_constant_long (OrcCompiler *compiler,
    orc_uint32 a, orc_uint32 b, orc_uint32 c, orc_uint32 d)
{
  int tmp;

  tmp = orc_compiler_try_get_constant_long (compiler, a, b, c, d);
  if (tmp == ORC_REG_INVALID) {
    tmp = orc_compiler_get_temp_reg (compiler);
    orc_compiler_load_constant_long (compiler, tmp,
        &compiler->constants[compiler->n_constants - 1]);
  }
  return tmp;
}

int
orc_compiler_try_get_constant_long (OrcCompiler *compiler,
    orc_uint32 a, orc_uint32 b, orc_uint32 c, orc_uint32 d)
{
  int i;

  for(i=0;i<compiler->n_constants;i++){
    if (compiler->constants[i].is_long == TRUE &&
        compiler->constants[i].full_value[0] == a &&
        compiler->constants[i].full_value[1] == b &&
        compiler->constants[i].full_value[2] == c &&
        compiler->constants[i].full_value[3] == d) {
      break;
    }
  }
  if (i == compiler->n_constants) {
    compiler->n_constants++;
    compiler->constants[i].full_value[0] = a;
    compiler->constants[i].full_value[1] = b;
    compiler->constants[i].full_value[2] = c;
    compiler->constants[i].full_value[3] = d;
    compiler->constants[i].is_long = TRUE;
    compiler->constants[i].alloc_reg = 0;
    compiler->constants[i].use_count = 0;
  }

  compiler->constants[i].use_count++;

  if (compiler->constants[i].alloc_reg != 0) {;
    return compiler->constants[i].alloc_reg;
  }
  return ORC_REG_INVALID;
}


int
orc_compiler_get_constant_reg (OrcCompiler *compiler)
{
  int j;

  for(j=0;j<ORC_N_REGS;j++){
    compiler->alloc_regs[j] = 0;
  }
  for(j=0;j<ORC_N_COMPILER_VARIABLES;j++){
    if (!compiler->vars[j].alloc) continue;

    ORC_DEBUG("var %d: %d  %d %d", j, compiler->vars[j].alloc,
        compiler->vars[j].first_use,
        compiler->vars[j].last_use);

    if (compiler->vars[j].first_use == -1) {
      compiler->alloc_regs[compiler->vars[j].alloc] = 1;
    } else if (compiler->vars[j].last_use != -1) {
      compiler->alloc_regs[compiler->vars[j].alloc] = 1;
    }
  }
  for(j=0;j<compiler->n_constants;j++){
    if (compiler->constants[j].alloc_reg) {
      compiler->alloc_regs[compiler->constants[j].alloc_reg] = 1;
    }
  }
  if (compiler->max_used_temp_reg < compiler->min_temp_reg)
    compiler->max_used_temp_reg = compiler->min_temp_reg;

  for(j=ORC_VEC_REG_BASE;j<=compiler->max_used_temp_reg;j++) {
    compiler->alloc_regs[j] = 1;
  }

  for(j=compiler->max_used_temp_reg;j<ORC_VEC_REG_BASE+32;j++){
    if (compiler->valid_regs[j] && !compiler->alloc_regs[j]) {
      return j;
    }
  }

  return 0;
}

#define ORC_COMPILER_ERROR_BUFFER_SIZE 200

static void
orc_compiler_error_valist (OrcCompiler *compiler, const char *fmt,
    va_list args)
{
  char *s;

  if (compiler->error_msg) return;

  s = malloc (ORC_COMPILER_ERROR_BUFFER_SIZE);
  vsprintf (s, fmt, args);
  compiler->error_msg = s;
  compiler->error = TRUE;
  compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
}

void
orc_compiler_error (OrcCompiler *compiler, const char *fmt, ...)
{
  va_list var_args;

  va_start (var_args, fmt);
  orc_compiler_error_valist (compiler, fmt, var_args);
  va_end (var_args);
}

void
orc_compiler_emit_invariants (OrcCompiler *compiler)
{
  int j;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    if (!(insn->flags & ORC_INSN_FLAG_INVARIANT)) continue;

    ORC_ASM_CODE(compiler,"# %d: %s\n", j, opcode->name);

    compiler->insn_shift = compiler->loop_shift;
    if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
      compiler->insn_shift += 1;
    }
    if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
      compiler->insn_shift += 2;
    }

    rule = insn->rule;
    if (rule && rule->emit) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_error (compiler, "no code generation rule for %s",
          opcode->name);
    }
  }
}
