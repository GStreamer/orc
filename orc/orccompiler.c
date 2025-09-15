
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __APPLE__
#include <pthread.h>

#include <AvailabilityMacros.h>
#include <TargetConditionals.h>

/* TARGET_OS_OSX is not defined on older macOS, but is always defined
   to 0 for Apple mobile targets. */
#if !defined(TARGET_OS_OSX) || TARGET_OS_OSX
#include <libkern/OSCacheControl.h>
#endif
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
#include <orc/orcutils-private.h>

#ifdef HAVE_VALGRIND_VALGRIND_H
#include <valgrind/valgrind.h>
#endif

#if defined(_WIN64)
#if defined(_M_ARM64)
#define THUNK_SIZE 20
typedef struct {
  orc_uint32 FunctionLength : 18;
  orc_uint32 Version : 2;
  orc_uint32 HasExceptionHandler : 1;
  orc_uint32 PackedEpilog : 1;
  orc_uint32 EpilogCount : 5;
  orc_uint32 CodeWords : 5;
  orc_uint8  UnwindCode[4];
  orc_uint32 ExceptionHandler;
} UNWIND_INFO;
#else // _M_X64
#define THUNK_SIZE 12
#define UWOP_SET_FPREG 3
#define UWOP_PUSH_NONVOL 0
#define UWOP_REG_RBP 5
typedef union {
  struct {
    orc_uint8 CodeOffset;
    orc_uint8 UnwindOp : 4;
    orc_uint8 OpInfo : 4;
  };
  orc_uint16 FrameOffset;
} UNWIND_CODE;
typedef struct {
  orc_uint8   Version : 3;
  orc_uint8   Flags : 5;
  orc_uint8   SizeOfProlog;
  orc_uint8   CountOfCodes;
  orc_uint8   FrameRegister : 4;
  orc_uint8   FrameOffset : 4;
  UNWIND_CODE UnwindCode[2];
  orc_uint32  ExceptionHandler;
} UNWIND_INFO;
#endif // defined(_M_ARM64)
typedef struct {
  RUNTIME_FUNCTION function_table;
  UNWIND_INFO unwind_info;
  orc_uint8 thunk[THUNK_SIZE];
} OrcUnwindInfo;
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
static orc_bool _orc_compiler_flag_backup;
static orc_bool _orc_compiler_flag_emulate;
static orc_bool _orc_compiler_flag_debug;
static orc_bool _orc_compiler_flag_randomize;

/* For Windows */
int _orc_codemem_alignment;

#if defined(_WIN64)
static DWORD orc_exception_handler(PEXCEPTION_RECORD exceptionRecord,
                                   PEXCEPTION_REGISTRATION_RECORD _unused,
                                   PCONTEXT context,
                                   PEXCEPTION_REGISTRATION_RECORD *_unused2)
{
  return ExceptionContinueSearch;
}
#endif

orc_bool
orc_compiler_is_debug ()
{
  return _orc_compiler_flag_debug != FALSE;
}

void
_orc_compiler_init (void)
{
  char *envvar;
#ifdef HAVE_CODEMEM_VIRTUALALLOC
  #ifdef ORC_WINAPI_ONLY_APP
    size_t page_size;
  #else
    size_t page_size ORC_GNUC_UNUSED;
  #endif

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

#ifdef HAVE_CODEMEM_VIRTUALALLOC
  GetNativeSystemInfo(&info);
  page_size = info.dwPageSize;

  /* Protection attribute change via VirtualProtect will be applied per
   * page memory unit. So we should split code memory with page aligned range.
   * Otherwise the protection attribute of previously generated executable code
   * memory can be affected by later generated one.
   */
  _orc_codemem_alignment = info.dwPageSize - 1;
#else
  /* 16 bytes alignment by default */
  _orc_codemem_alignment = 15;
#endif

  int can_jit = TRUE;

  if (!_orc_compiler_flag_backup && !_orc_compiler_flag_emulate) {
    /* If backup code is not enabled and emulation is not enabled, that means
     * we will do JIT compilation and call orc_code_region_allocate_codemem().
     */
#if defined(HAVE_CODEMEM_VIRTUALALLOC) && defined(ORC_WINAPI_ONLY_APP)
    /* When targeting Windows Store apps, the codeGeneration capability must
     * be enabled in the app manifest, or passing PAGE_EXECUTE to
     * VirtualProtectFromApp will return NULL. In this case, we must force
     * backup C code, and if that's not available, we must emulate. */
    can_jit = FALSE;
    void *mem = VirtualAllocFromApp (NULL, page_size, MEM_COMMIT,
        PAGE_READWRITE);
    if (mem) {
      int old_protect;
      if (VirtualProtectFromApp (mem, page_size, PAGE_EXECUTE, &old_protect) > 0)
        can_jit = TRUE;
      VirtualFree (mem, 0, MEM_RELEASE);
    } else {
      ORC_WARNING ("Unable to allocate executable pages: using backup code or "
        "emulation: codeGeneration capability isn't set in the app manifest?");
    }
#elif defined(HAVE_CODEMEM_MMAP)
    /* In this case, we need to check that we can mmap pages as executable.
     * This is not the case under the combination of SELinux and sandboxing
     * profiles.
     */
    can_jit = FALSE;
    OrcCodeRegion *region = orc_code_region_alloc();
    if (region) {
      can_jit = TRUE;
      // FIXME: the file descriptor should be kept somewhere
      // Currently the underlying mmap'd pages are leaked
      free(region);
    } else {
      ORC_WARNING ("Unable to allocate executable pages: using backup code or emulation");
    }
#endif

    if (!can_jit) {
      _orc_compiler_flag_backup = TRUE;
      _orc_compiler_flag_emulate = TRUE;
    }
  }
}

orc_bool
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

  for (i = 0; i < ORC_N_REGS; i++) {
    // For volatile registers, try up to 32 registers from offset
    // Amyspark: this ensures that we don't run over vector registers
    reg = offset + ((roff + i) & 0x1f);
    if (compiler->valid_regs[reg] &&
        !compiler->save_regs[reg] &&
        compiler->alloc_regs[reg] == 0) {
      compiler->alloc_regs[reg]++;
      compiler->used_regs[reg] = 1;
      return reg;
    }
  }
  // Use ORC_N_REGS to ensure forward proofed iteration
  for (i = 0; i < ORC_N_REGS; i++) {
    // Try with up to 64 registers starting from offset
    // If this lands us within vector range, bail out
    reg = offset + ((roff + i) & 0x3f);
    if (reg >= compiler->target->data_register_offset && !data_reg)
      break;
    if (compiler->valid_regs[reg] && compiler->alloc_regs[reg] == 0) {
      compiler->alloc_regs[reg]++;
      compiler->used_regs[reg] = 1;
      return reg;
    }
  }

  if (data_reg || !compiler->allow_gp_on_stack) {
    ORC_COMPILER_ERROR (compiler, "register overflow for %s register",
        data_reg ? "vector" : "gp");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }

  return 0;
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

OrcCompileResult
orc_compiler_compile_program (OrcCompiler *compiler, OrcProgram *program, OrcTarget *target, unsigned int flags)
{
  int i;
  OrcCompileResult result;
  const char *error_msg;

  ORC_INFO("initializing compiler for program \"%s\"", program->name);
  error_msg = orc_program_get_error (program);
  if (error_msg && strcmp (error_msg, "")) {
    ORC_WARNING ("program %s failed to compile, reason: %s",
        program->name, error_msg);
    free (compiler);
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

  if (program->backup_func) {
    program->code_exec = program->backup_func;
  } else {
    program->code_exec = (void *)orc_executor_emulate;
  }

  compiler->program = program;
  compiler->target = target;
  compiler->target_flags = flags;

  {
    ORC_LOG("Program variables");
    for(i=0;i<ORC_N_VARIABLES;i++){
      if (program->vars[i].size > 0) {
        ORC_LOG("%d: %s size %d type %d alignment %d is_aligned %d alloc %d", i,
            program->vars[i].name,
            program->vars[i].size,
            program->vars[i].vartype,
            program->vars[i].alignment,
            program->vars[i].is_aligned,
            program->vars[i].alloc);
      }
    }
    ORC_LOG("Program instructions");
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

  /* Reset temp registers */
  for (i = 0; i < ORC_N_REGS; i++)
    compiler->temp_regs[i] = 0;

  orc_compiler_check_sizes (compiler);
  if (compiler->error) goto error;

  if (compiler->target) {
    compiler->target->compiler_init (compiler);
  }

  orc_compiler_rewrite_insns (compiler);
  if (compiler->error) goto error;

  orc_compiler_rewrite_vars (compiler);
  if (compiler->error) goto error;

  {
    ORC_LOG("Compiler variables");
    for(i=0;i<ORC_N_VARIABLES;i++){
      if (compiler->vars[i].size > 0) {
        ORC_LOG("%d: %s size %d type %d alignment %d is_aligned %d alloc %d [%d,%d]", i,
            compiler->vars[i].name,
            compiler->vars[i].size,
            compiler->vars[i].vartype,
            compiler->vars[i].alignment,
            compiler->vars[i].is_aligned,
            compiler->vars[i].alloc,
            compiler->vars[i].first_use,
            compiler->vars[i].last_use);
      }
    }
    ORC_LOG("Compiler instructions");
    for(i=0;i<compiler->n_insns;i++){
      ORC_LOG("%d: %s %d %d %d %d", i,
          compiler->insns[i].opcode->name,
          compiler->insns[i].dest_args[0],
          compiler->insns[i].dest_args[1],
          compiler->insns[i].src_args[0],
          compiler->insns[i].src_args[1]);
    }
  }
  program->orccode = orc_code_new ();

  program->orccode->is_2d = program->is_2d;
  program->orccode->constant_n = program->constant_n;
  program->orccode->constant_m = program->constant_m;
  program->orccode->exec = program->code_exec;

  program->orccode->n_insns = compiler->n_insns;
  program->orccode->insns = orc_malloc(sizeof(OrcInstruction) * compiler->n_insns);
  memcpy (program->orccode->insns, compiler->insns,
      sizeof(OrcInstruction) * compiler->n_insns);

  program->orccode->vars = orc_malloc (sizeof(OrcCodeVariable) * ORC_N_COMPILER_VARIABLES);
  memset (program->orccode->vars, 0,
      sizeof(OrcCodeVariable) * ORC_N_COMPILER_VARIABLES);
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    program->orccode->vars[i].vartype = compiler->vars[i].vartype;
    program->orccode->vars[i].size = compiler->vars[i].size;
    program->orccode->vars[i].value = compiler->vars[i].value;
  }

  if (program->backup_func && (_orc_compiler_flag_backup || target == NULL)) {
    ORC_COMPILER_ERROR (compiler, "Compilation disabled, using backup");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

  if (_orc_compiler_flag_emulate || target == NULL) {
    program->code_exec = (void *)orc_executor_emulate;
    program->orccode->exec = (void *)orc_executor_emulate;
    ORC_COMPILER_ERROR (compiler, "Compilation disabled, using emulation");
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
  compiler->code = orc_malloc(65536);
  compiler->codeptr = compiler->code;

  if (compiler->error) goto error;

  ORC_INFO("compiling for target \"%s\"", compiler->target->name);
  compiler->target->compile (compiler);
  if (compiler->error) {
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

#if defined(_WIN64) && defined(ORC_SUPPORTS_BACKTRACE_FROM_JIT)
  OrcUnwindInfo table;
  // The structures must be DWORD aligned in memory.
  const unsigned char *alignas_offset =
      (unsigned char *)((DWORD64)(compiler->codeptr + 3) & (~3));

  if (compiler->use_frame_pointer) {
    memset(&table, 0, sizeof(OrcUnwindInfo));
    const DWORD64 start_of_orcunwindinfo = alignas_offset - compiler->code;

    table.function_table.BeginAddress = 0;

    // The following Arm64 block is based on the Firefox changeset:
    // https://hg.mozilla.org/mozilla-central/rev/4d932b82695c
    // which I fixed to work with pure C.

    // The program counter (and the stack pointer, on Arm64) must be modified
    // for the handler to be recognised as valid.
#ifdef _M_ARM64
    table.function_table.UnwindData =
        start_of_orcunwindinfo + ORC_STRUCT_OFFSET(OrcUnwindInfo, unwind_info);

    // one 32-bit word gives us up to 4 codes
    table.unwind_info.CodeWords = 1;
    // alloc_s small stack of size 1*16
    table.unwind_info.UnwindCode[0] = 0x1;
    // end
    table.unwind_info.UnwindCode[1] = 0xe4;

    // xip0/r16 should be safe to clobber: Windows just used it to call the thunk.
    const orc_uint8 reg = 16;

    const void *handler = (void *)&orc_exception_handler;
    const uint16_t *addr = (uint16_t *)&handler;

    // Say `handler` is 0x4444333322221111, then:
    table.thunk[0] = 0xd2800000 | addr[0] << 5 | reg;  // mov  xip0, 1111
    table.thunk[1] = 0xf2a00000 | addr[1] << 5 | reg;  // movk xip0, 2222 lsl #0x10
    table.thunk[2] = 0xf2c00000 | addr[2] << 5 | reg;  // movk xip0, 3333 lsl #0x20
    table.thunk[3] = 0xf2e00000 | addr[3] << 5 | reg;  // movk xip0, 4444 lsl #0x30
    table.thunk[4] = 0xd61f0000 | reg << 5;                // br xip0
#else
    table.function_table.EndAddress = compiler->codeptr - compiler->code;
#ifdef __MINGW32__
    // They're the same member, but MSYS2 only defines the former under x86.
    table.function_table.UnwindData =
#else
    table.function_table.UnwindInfoAddress =
#endif
        start_of_orcunwindinfo + ORC_STRUCT_OFFSET(OrcUnwindInfo, unwind_info);

    table.unwind_info.Version = 1;
    // We handle the exception (let the UCRT terminate the app)
    table.unwind_info.Flags = UNW_FLAG_EHANDLER;
    table.unwind_info.SizeOfProlog = 8;
    // See below
    table.unwind_info.CountOfCodes = 2;
    // RAX = 50, RBP = 55
    // https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170#operation-info
    // https://www.felixcloutier.com/x86/mov
    table.unwind_info.FrameRegister = UWOP_REG_RBP;
    // No offset from RSP
    table.unwind_info.FrameOffset = 0;
    // Bytes 0-4 are the ENDBR64 CET instruction
    // The docs kind of lie when they say that offsets are not allowed
    // until SET_FPREG. I guess they apply to OpInfo instead?
    table.unwind_info.UnwindCode[0].CodeOffset = 5;
    table.unwind_info.UnwindCode[0].UnwindOp = UWOP_SET_FPREG;
    table.unwind_info.UnwindCode[0].OpInfo = 0;  // mov rbp, rsp
    table.unwind_info.UnwindCode[1].CodeOffset = 4;
    table.unwind_info.UnwindCode[1].UnwindOp = UWOP_PUSH_NONVOL;
    table.unwind_info.UnwindCode[1].OpInfo = UWOP_REG_RBP; // push rbp
    // Same as above -- the exception handler is required, and must do
    // something to actually be dispatched.
    table.unwind_info.ExceptionHandler =
        start_of_orcunwindinfo + ORC_STRUCT_OFFSET(OrcUnwindInfo, thunk);
    // mov %rax, (address of handler as imm64)
    // jmp %rax
    static const orc_uint8 thunk[THUNK_SIZE] = {
      0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xe0,
    };
    memcpy (&table.thunk, &thunk, THUNK_SIZE);
    memcpy (&table.thunk[2], &orc_exception_handler, 8);
#endif

    program->orccode->code_size = (unsigned char *)alignas_offset -
                                  compiler->code + sizeof(OrcUnwindInfo);
  } else {
    program->orccode->code_size = compiler->codeptr - compiler->code;
  }
#else
  program->orccode->code_size = compiler->codeptr - compiler->code;
#endif

  orc_code_allocate_codemem (program->orccode, program->orccode->code_size);
  if (program->orccode->chunk == NULL) {
    program->code_exec = (void *)orc_executor_emulate;
    program->orccode->exec = (void *)orc_executor_emulate;
    ORC_COMPILER_ERROR (compiler, "Cannot reserve executable memory, using emulation");
    compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
    goto error;
  }

#if defined(__APPLE__) && (!defined(TARGET_OS_OSX) || TARGET_OS_OSX)
#if defined(MAC_OS_VERSION_11_0) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_VERSION_11_0
  if (__builtin_available (macOS 11.0, *)) {
    if (pthread_jit_write_protect_supported_np ())
      pthread_jit_write_protect_np (0);
  }
#endif
#endif
#if defined(HAVE_CODEMEM_VIRTUALALLOC)
  /* Ensure that code region is writable before memcpy */
  _set_virtual_protect (program->orccode->code, program->orccode->code_size,
      PAGE_READWRITE);
#endif
#if defined(_WIN64) && defined(ORC_SUPPORTS_BACKTRACE_FROM_JIT)
  if (compiler->use_frame_pointer) {
    void *const program_unwind_info =
        program->orccode->code + (alignas_offset - compiler->code);
    PRUNTIME_FUNCTION const runtime_function_address =
          (PRUNTIME_FUNCTION)((orc_uint8*)program_unwind_info +
                              ORC_STRUCT_OFFSET(OrcUnwindInfo, function_table));
    const size_t real_code_size = compiler->codeptr - compiler->code;
    memcpy (program->orccode->code, compiler->code, real_code_size);
    memcpy (program_unwind_info, &table, sizeof (OrcUnwindInfo));
    if (RtlAddFunctionTable (runtime_function_address, 1, (DWORD64)program->orccode->code)) {
      DWORD64 dyn_base = 0;
      PRUNTIME_FUNCTION const p = RtlLookupFunctionEntry (
          (DWORD64)program->orccode->code + 20, &dyn_base, NULL);
      if (p != runtime_function_address) {
        ORC_ERROR ("Runtime function for program %s %p is bogus "
                    "(dynbase=%llx, info=%p, expected=%p)",
                    program->name, program->orccode->code, dyn_base, p,
                    runtime_function_address);
      }
    } else {
      ORC_WARNING ("Unable to install unwind info for program %s %p",
                  program->name, program->orccode->code);
    }
  } else {
    memcpy(program->orccode->code, compiler->code, program->orccode->code_size);
  }
#else
  memcpy(program->orccode->code, compiler->code, program->orccode->code_size);
#endif

#ifdef VALGRIND_DISCARD_TRANSLATIONS
  VALGRIND_DISCARD_TRANSLATIONS (program->orccode->exec,
      program->orccode->code_size);
#endif

  if (compiler->target->flush_cache) {
    compiler->target->flush_cache (program->orccode);
  }

#if defined(__APPLE__) && (!defined(TARGET_OS_OSX) || TARGET_OS_OSX)
#if defined(MAC_OS_VERSION_11_0) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_VERSION_11_0
  if (__builtin_available (macOS 11.0, *)) {
    if (pthread_jit_write_protect_supported_np ()) {
      pthread_jit_write_protect_np (1);
      sys_icache_invalidate (program->orccode->exec, program->orccode->code_size);
    }
  }
#endif
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
      ORC_COMPILER_ERROR (compiler, "no code generation rule for %s on "
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

  ORC_DEBUG("at insn %d %s", compiler->insn_index,
      compiler->insns[compiler->insn_index].opcode->name);

  /* Mark old temp registers as unused */
  for(j=0;j<ORC_N_REGS;j++){
    compiler->alloc_regs[j] = compiler->temp_regs[j];
  }

  /* Mark variables using a temp register as used */
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

  /* Mark constants using a temp register as used */
  for(j=0;j<compiler->n_constants;j++){
    if (compiler->constants[j].alloc_reg) {
      compiler->alloc_regs[compiler->constants[j].alloc_reg] = 1;
    }
  }

  for (j = compiler->min_temp_reg; j < ORC_N_REGS; j++) {
    if (compiler->valid_regs[j] && !compiler->alloc_regs[j]) {
      ORC_DEBUG("Available register %d found", j);
      compiler->temp_regs[j] = 1;
      return j;
    }
  }

  ORC_ERROR("No temporary register available at insn %d %s",
      compiler->insn_index,
      compiler->insns[compiler->insn_index].opcode->name);
  ORC_COMPILER_ERROR (compiler, "no temporary register available");
  compiler->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;

  return 0;
}

void
orc_compiler_release_temp_reg (OrcCompiler *compiler, int reg)
{
  ORC_DEBUG("Releasing register %d", reg);
  compiler->temp_regs[reg] = 0;
}

void
orc_compiler_reset_temp_regs (OrcCompiler *compiler, int start)
{
  int i;

  for (i = compiler->min_temp_reg; i < ORC_N_REGS; i++)
    compiler->temp_regs[i] = 0;
  compiler->min_temp_reg = start;
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
        ORC_COMPILER_ERROR (compiler, "bad vartype");
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
    // FIXME: If loop_counter is invalid, the counter must be set manually
    // in the executor 
    if (compiler->loop_counter == ORC_REG_INVALID) {
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
  compiler->vars[i].name = orc_malloc (strlen(compiler->vars[var].name) + 10);
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
  compiler->vars[i].name = orc_malloc (10);
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

int
orc_compiler_has_float (OrcCompiler *compiler)
{
  int j;
  for(j=0;j<compiler->n_insns;j++){
    OrcInstruction *insn = compiler->insns + j;
    OrcStaticOpcode *opcode = insn->opcode;
    if (opcode->flags & ORC_STATIC_OPCODE_FLOAT) return TRUE;
  }
  return FALSE;
}

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
  p->asm_code = orc_realloc (p->asm_code, p->asm_code_len + n + 1);
  memcpy (p->asm_code + p->asm_code_len, tmp, n + 1);
  p->asm_code_len += n;
}

int
orc_compiler_label_new (OrcCompiler *compiler)
{
  return compiler->n_labels++;
}

/* Compatibility function to be removed once the deprecated
 * APIs are removed
 */
static void
orc_compiler_constant_from_u128 (OrcConstant *cnst, 
    orc_uint32 a, orc_uint32 b, orc_uint32 c, orc_uint32 d)
{
  cnst->type = ORC_CONST_FULL;
  cnst->v[0].x2[0] = a;
  cnst->v[0].x2[1] = b;
  cnst->v[1].x2[0] = c;
  cnst->v[1].x2[1] = d;
}

static void
orc_compiler_constant_from_splat (OrcConstant *cnst, int size, orc_uint64 value)
{
  switch (size) {
    case 1:
      cnst->type = ORC_CONST_SPLAT_B;
      cnst->v[0].x8[0] = value & 0xff;
      break;

    case 2:
      cnst->type = ORC_CONST_SPLAT_W;
      cnst->v[0].x4[0] = value & 0xffff;
      break;

    case 4:
      cnst->type = ORC_CONST_SPLAT_L;
      cnst->v[0].x2[0] = value & 0xffffffff;
      break;

    case 8:
      cnst->type = ORC_CONST_SPLAT_Q;
      cnst->v[0].i = value;
      break;

    default:
      ORC_ASSERT(0);
  }
}

/**
 * orc_compiler_load_constant:
 *
 * Loads a constant into a register
 *
 * @c: The compiler that loads the constant
 * @reg: The register to load the constant into
 * @cnst: The #OrcConstant to load
 *
 * Loads a constant into the register provided
 */
void
orc_compiler_load_constant (OrcCompiler *c, int reg, OrcConstant *cnst)
{
  /* TODO we should check if the desired constant already exists, if so,
   * just move the constant register into reg, instead of loading again.
   * This will require a generic (per-target) mov operation
   */
  c->target->load_constant_long (c, reg, cnst);
}

/**
 * orc_compiler_load_constant:
 *
 * Loads a constant into a register
 *
 * @c: The compiler that loads the constant
 * @size: The size of the value to load. One, two, four or eight bytes
 * @value: The constant value to load
 *
 * Loads a constant into the preovided register @reg, splatting the value into
 * the whole register. The @size defines what will be splatted
 * into registry from @value.
 */
void
orc_compiler_load_constant_from_size_and_value (OrcCompiler *c, int reg,
    int size, orc_uint64 value)
{
  OrcConstant cnst = { 0 };

  orc_compiler_constant_from_splat (&cnst, size, value);
  orc_compiler_load_constant (c, reg, &cnst);
}

/**
 * orc_compiler_get_temp_constant:
 *
 * Loads a temporary constant into a register
 *
 * @compiler: The compiler that loads the temporary constant
 * @size: The size of the value. One, two or four bytes
 * @value: The constant value
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Gets a constant as a registry splatting the value into
 * the whole register. The @size defines what will be splatted
 * into registry from @value.
 *
 * Temporary constants are generated inplace on every call
 * This is different from general constants that will be generate
 * code at the beginning of the generated code.
 *
 * This function is deprecated and orc_compiler_get_temp_constant_full()
 * should be used.
 */
int
orc_compiler_get_temp_constant (OrcCompiler *compiler, int size, int value)
{
  OrcConstant cnst = { 0 };

  orc_compiler_constant_from_splat (&cnst, size, value);
  return orc_compiler_get_temp_constant_full (compiler, &cnst);
}

/**
 * orc_compiler_get_constant:
 *
 * Loads a constant into a register
 *
 * @compiler: The compiler that loads the constant
 * @size: The size of the value. One, two or four bytes
 * @value: The constant value
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Gets a constant as a registry splatting the value into
 * the whole register. The @size defines what will be splatted
 * into registry from @value.
 *
 * In case a constant can not be generated, due to missing temporary
 * registers, or too many compiler constants #ORC_REG_INVALID will
 * be returned. Failing will trigger an ORC_COMPILER_ERROR()
 *
 * This function is deprecated and orc_compiler_get_constant_full()
 * should be used.
 */
int
orc_compiler_get_constant (OrcCompiler *compiler, int size, int value)
{
  OrcConstant cnst = { 0 };

  orc_compiler_constant_from_splat (&cnst, size, value);
  return orc_compiler_get_constant_full (compiler, &cnst);
}

/**
 * orc_compiler_get_constant_long:
 *
 * Loads a 128-bit constant into a register
 *
 * @compiler: The compiler that loads the constant
 * @a: The first 32-bits of the value to load as a constant
 * @b: The second 32-bits of the value to load as a constant
 * @c: The third 32-bits of the value to load as a constant
 * @d: The fourth 32-bits of the value to load as a constant
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Gets a constant as a registry filling 128-bits only of the whole
 * register.
 *
 * In case a constant can not be generated, due to missing temporary
 * registers, or too many compiler constants #ORC_REG_INVALID will
 * be returned. Failing will trigger an ORC_COMPILER_ERROR()
 *
 * This function is deprecated and orc_compiler_get_constant_full()
 * should be used. Note that @a, @b, @c, and @d are stored in little-endian
 * form in a 128-bits little-endian memory area, therefore, they are stored
 * like [@a, @b, @c, @d] being @a at memory address 0, @b at memory address
 * 32, @c at memory address 64, and @d at memory address 96
 */
int
orc_compiler_get_constant_long (OrcCompiler *compiler,
    orc_uint32 a, orc_uint32 b, orc_uint32 c, orc_uint32 d)
{
  OrcConstant cnst = { 0 };

  orc_compiler_constant_from_u128 (&cnst, a, b, c, d);
  return orc_compiler_get_constant_full (compiler, &cnst);
}

/**
 * orc_compiler_try_get_constant_long:
 *
 * Tries to load a 128-bit constant into a register
 *
 * @compiler: The compiler that loads the constant
 * @a: The first 32-bits of the value to load as a constant
 * @b: The second 32-bits of the value to load as a constant
 * @c: The third 32-bits of the value to load as a constant
 * @d: The fourth 32-bits of the value to load as a constant
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Similar to orc_compiler_get_constant_long() but will not trigger
 * an ORC_COMPILER_ERROR() if failing. This is useful for rules that
 * have different altrnatives of implementation depending if the constant
 * can be loaded or not.
 *
 * This function is deprecated and orc_compiler_try_get_constant_full()
 * should be used. Note that @a, @b, @c, and @d are stored in little-endian
 * form in a 128-bits little-endian memory area, therefore, they are stored
 * like [@a, @b, @c, @d] being @a at memory address 0, @b at memory address
 * 32, @c at memory address 64, and @d at memory address 96
 */
int
orc_compiler_try_get_constant_long (OrcCompiler *compiler,
    orc_uint32 a, orc_uint32 b, orc_uint32 c, orc_uint32 d)
{
  OrcConstant cnst = { 0 };

  orc_compiler_constant_from_u128 (&cnst, a, b, c, d);
  return orc_compiler_try_get_constant_full (compiler, &cnst);
}

/**
 * orc_compiler_get_temp_constant_full:
 *
 * Loads an #OrcConstant into a register
 *
 * @c: The compiler that loads the constant
 * @cnst: The constant to load
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Temporary constants are generated inplace on every call
 * This is different from general constants that will be generate
 * code at the beginning of the generated code.
 */
int
orc_compiler_get_temp_constant_full (OrcCompiler *c, OrcConstant *cnst)
{
  int tmp;

  tmp = orc_compiler_get_temp_reg (c);
  orc_compiler_load_constant (c, tmp, cnst);
  return tmp;
}

/**
 * orc_compiler_try_get_constant_full:
 *
 * Tries to load an #OrcConstant into a register
 *
 * @c: The compiler that loads the constant
 * @cnst: The constant to load
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 * Similar to orc_compiler_get_constant_full() but will not trigger
 * an ORC_COMPILER_ERROR() if failing. This is useful for rules that
 * have different altrnatives of implementation depending if the constant
 * can be loaded or not.
 *
 */
int
orc_compiler_try_get_constant_full (OrcCompiler *c, OrcConstant *cnst)
{
  int i;

  /* Search for a constant equal to the requested */
  for (i = 0; i < c->n_constants; i++) {
    if (orc_constant_is_equal (cnst, &c->constants[i], c->target->register_size)) {
      ORC_DEBUG ("Same constant found at %d, reusing register %d", i,
          c->constants[i].alloc_reg);
      c->constants[i].use_count++;
      return c->constants[i].alloc_reg;
    }
  }

  /* Check that we can allocate more constants */
  if (c->n_constants == ORC_N_CONSTANTS) {
    ORC_WARNING ("Max number of constants reached");
    return ORC_REG_INVALID;
  }
  /* As someone requested a constant, and the compiler is a two-step
   * compiler, reserve a new constant so in the next compilation
   * the constant will be available
   */

  memcpy (c->constants + c->n_constants, cnst, sizeof(OrcConstant));
  c->constants[c->n_constants].use_count = 1;
  c->n_constants++;

  return ORC_REG_INVALID;
}

/**
 * orc_compiler_get_constant_full:
 *
 * Loads an #OrcConstant into a register
 *
 * @c: The compiler that loads the constant
 * @cnst: The constant to load
 * @returns: The register with the constant stored or #ORC_REG_INVALID
 *
 */
int
orc_compiler_get_constant_full (OrcCompiler *c, OrcConstant *cnst)
{
  int tmp;

  tmp = orc_compiler_try_get_constant_full (c, cnst);
  if (tmp)
    return tmp;

  /* We can not assume that cnst is the last requested compiler constant
   * as it might fail due to no more compiler constants available.
   * Load directly the constant
   */
  return orc_compiler_get_temp_constant_full (c, cnst);
}

int
orc_compiler_get_constant_reg (OrcCompiler *compiler)
{
  int j;

  /* Mark temp registers as used */
  for(j=0;j<ORC_N_REGS;j++){
    compiler->alloc_regs[j] = compiler->temp_regs[j];
  }

  /* Mark variables using a temp register as used */
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

  /* Mark constants using a temp register as used */
  for(j=0;j<compiler->n_constants;j++){
    if (compiler->constants[j].alloc_reg) {
      compiler->alloc_regs[compiler->constants[j].alloc_reg] = 1;
    }
  }

  for (j = compiler->min_temp_reg; j < ORC_N_REGS; j++) {
    if (compiler->valid_regs[j] && !compiler->alloc_regs[j]) {
      compiler->temp_regs[j] = 1;
      ORC_DEBUG("Available register %d found", j);
      return j;
    }
  }

  ORC_WARNING("No temporary register available for a constant");
  return 0;
}

#define ORC_COMPILER_ERROR_BUFFER_SIZE 200

static void
orc_compiler_error_valist (OrcCompiler *compiler, const char *fmt,
    va_list args)
{
  char *s = NULL;

  if (compiler->error_msg) return;

#ifdef HAVE_VASPRINTF
  if (vasprintf (&s, fmt, args) < 0)
    ORC_ASSERT (0);
#elif defined(_UCRT)
  s = orc_malloc (ORC_COMPILER_ERROR_BUFFER_SIZE);
  vsnprintf_s (s, ORC_COMPILER_ERROR_BUFFER_SIZE, _TRUNCATE, fmt, args);
#else
  s = orc_malloc (ORC_COMPILER_ERROR_BUFFER_SIZE);
  vsnprintf (s, ORC_COMPILER_ERROR_BUFFER_SIZE, fmt, args);
#endif
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
      ORC_COMPILER_ERROR (compiler, "no code generation rule for %s",
          opcode->name);
    }
  }
}

const OrcVariable *
orc_compiler_get_variable (OrcCompiler *c, OrcVariableId idx)
{
  if (idx < 0 || idx > ORC_N_VARIABLES)
    return NULL;
  return &c->vars[idx];
}
