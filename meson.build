project ('orc', 'c', version : '0.4.41.1',
                     meson_version : '>= 1.3.0',
                     default_options : ['buildtype=debugoptimized',
                                        'warning_level=1'] )

orc_api = '0.4'
orc_version_major = meson.project_version().split('.')[0]
orc_version_minor = meson.project_version().split('.')[1]
orc_version_micro = meson.project_version().split('.')[2]

# maintaining compatibility with the previous libtool versioning
soversion = 0
curversion = orc_version_micro.to_int()
libversion = '@0@.@1@.0'.format(soversion, curversion)
osxversion = curversion + 1

add_project_arguments('-DHAVE_CONFIG_H', language : 'c')

cc = meson.get_compiler('c')

# Error out if a function's missing
add_project_arguments(cc.get_supported_arguments([
  '-we4013',
  '-Werror=implicit-int',
  '-Werror=implicit-function-declaration',
]), language : 'c')

orc_inc = include_directories('.')

cdata = configuration_data()      # config.h

pkg = import('pkgconfig')

# -Bsymbolic-functions
if cc.has_link_argument('-Wl,-Bsymbolic-functions')
  add_project_link_arguments('-Wl,-Bsymbolic-functions', language : 'c')
endif

all_targets = ['avx', 'sse', 'mmx']
extra_targets = ['altivec', 'neon', 'mips', 'c64x', 'riscv', 'lsx', 'lasx'] # 'arm'
enabled_targets = []

host_system = host_machine.system()
if host_system != 'windows'
  all_targets += extra_targets
endif

# To be removed once the orc-target option becomes mainstream
backend = get_option('orc-backend')
if not backend.contains('all')
  warning('Setting old orc-backend option, please update your tools to use orc-target instead')
endif

target = get_option('orc-target')
if target.contains('all') and not backend.contains('all')
  target = backend
endif

foreach b : all_targets
  if target.contains('all') or target.contains(b)
    cdata.set('ENABLE_TARGET_' + b.to_upper(), 1)
    enabled_targets += [b]
  endif
endforeach

cpu_family = host_machine.cpu_family()
if cpu_family == 'x86'
  cdata.set('HAVE_I386', true)
elif cpu_family == 'x86_64'
  cdata.set('HAVE_AMD64', true)
elif cpu_family == 'ppc' or cpu_family == 'ppc64'
  cdata.set('HAVE_POWERPC', true)
# TODO: Windows ARM device is not tested
elif cpu_family == 'arm' and host_system != 'windows'
  cdata.set('HAVE_ARM', true)
# TODO: Add support for Windows
# https://gitlab.freedesktop.org/gstreamer/orc/-/issues/38
elif cpu_family == 'aarch64' and host_system != 'windows'
  cdata.set('HAVE_AARCH64', true)
elif cpu_family == 'mips' and host_machine.endian() == 'little'
  cdata.set('HAVE_MIPSEL', true)
elif cpu_family.startswith('riscv')
  cdata.set('HAVE_RISCV', true)
elif cpu_family.startswith('loongarch')
  cdata.set('HAVE_LOONGARCH', true)
else
  warning(cpu_family + ' with ' + host_system + ' isn\'t a supported configuration for optimization')
endif

threads = dependency('threads')

libm = cc.find_library('m', required : false)

# glibc <= 2.06 has mkstemp(2) with permissions 0666
glibc_needs_posix = '''#ifdef __GLIBC__
#include <features.h>
#if !__GLIBC_PREREQ(2, 7)
  #error Too old glibc
#endif
#endif
int main () { return 0; }
'''
if not cc.compiles(glibc_needs_posix, name: 'Has glibc >= 2.07 (or no glibc at all)')
  error('This requires glibc >= 2.07 to avoid a mkstemp(2) permissions bug, aborting')
endif

# Check for libatomic for use of C11 atomics: some architectures need
# to link explicitly to this library.
# TODO: Once meson 1.7 is in use, this can be updated to
# dependency('atomic')
atomic_dep = cc.find_library('atomic', required: false)

librt = []
if cc.has_function('clock_gettime')
  cdata.set('HAVE_CLOCK_GETTIME', true)
else
  # On glibc older than 2.17, clock_gettime is provided by time.h and -lrt
  librt = cc.find_library('rt', required : false)
endif

liblog = []
if cc.has_header_symbol('android/log.h', '__android_log_print')
  cdata.set('HAVE_ANDROID_LIBLOG', true)
  liblog = [cc.find_library('log', required : true)]
endif

host_os = host_machine.system()
if host_os == 'windows'
  cdata.set('HAVE_CODEMEM_VIRTUALALLOC', true)
  cdata.set('HAVE_OS_WIN32', true)
  cdata.set('HAVE_THREAD_WIN32', true)

  code = '''
  #include <windows.h>
  #if !(WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
  #error "Not building for UWP"
  #endif'''
  if cc.compiles(code, name : 'building for UWP')
    cdata.set('ORC_WINAPI_ONLY_APP', true)
  endif

  # RtlAddFunctionTable is only available for UWP starting with
  # Windows SDK 10.0.22621.0. A simple check is to validate if it's usable.
  if cc.has_function('RtlAddFunctionTable', prefix: '#include <windows.h>') \
    and cc.has_function('RtlLookupFunctionEntry', prefix: '#include <windows.h>') \
    and cc.has_function('RtlDeleteFunctionTable', prefix: '#include <windows.h>') 
    cdata.set('ORC_SUPPORTS_BACKTRACE_FROM_JIT', true)
  endif
else
  # If it is not windows, we just assume it is a unix of sorts for now.
  cdata.set('HAVE_CODEMEM_MMAP', true)
  cdata.set('HAVE_THREAD_PTHREAD', true)
endif

if cpu_family.startswith('x86') and cc.get_define('_MSC_VER') == ''
  xgetbv = cc.has_header_symbol('immintrin.h', '_xgetbv') or cc.has_header_symbol('xsaveintrin.h', '_xgetbv')
  cdata.set('ORC_NEEDS_ASM_XSAVE', not xgetbv)
endif

monotonic_test = '''
#include <time.h>
#include <unistd.h>
int main() {
  #if !(defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0 && defined(CLOCK_MONOTONIC))
  #error No monotonic clock
  #endif
  return 0;
}
'''
cdata.set('HAVE_MONOTONIC_CLOCK', cc.compiles(monotonic_test))
cdata.set('HAVE_GETTIMEOFDAY', cc.has_function('gettimeofday'))
cdata.set('HAVE_VASPRINTF', cc.has_function('vasprintf'))
cdata.set('HAVE_POSIX_MEMALIGN', cc.has_function('posix_memalign', prefix : '#include <stdlib.h>'))
cdata.set('HAVE_MMAP', cc.has_function('mmap'))
cdata.set('HAVE_SYS_TIME_H', cc.has_header('sys/time.h'))
cdata.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))
cdata.set('HAVE_VALGRIND_VALGRIND_H', cc.has_header('valgrind/valgrind.h'))

cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('VERSION', meson.project_version())

subdir('orc')

opt_benchmarks = get_option('benchmarks')
opt_examples = get_option('examples')
opt_orctest = get_option('orc-test')
opt_tests = get_option('tests')
opt_tools = get_option('tools')

if cc.compiles ('''#include <Availability.h>
#include <TargetConditionals.h>
#if defined(APPLE)
# if !defined(TARGET_OS_OSX) || TARGET_OS_OSX
# error "Targeting macOS device"
# endif
#else
#error "Not building for Apple devices"
#endif''', name : 'building for non-macOS Darwin platform')
  if not opt_orctest.disabled()
    warning('Tests are only supported in macOS.')
    opt_tests = disabler()
  endif
endif

if not opt_orctest.disabled()
  subdir('orc-test')
else
  if opt_tests.enabled()
    error('Tests were enabled explicitly, but the orc-test library was disabled.')
  endif
  orc_test_dep = disabler() # for testsuites + orc-bugreport
endif

if not opt_tools.disabled()
  subdir('tools')
else
  orcc = disabler() # for testsuite/orcc/
  tools_variables = []
endif

if not opt_examples.disabled()
  subdir('examples')
endif

if not opt_tests.disabled()
  subdir('testsuite')
endif

have_docs = false
if build_machine.system() == 'windows'
  message('Disabling gtk-doc while building on Windows')
  gtk_doc_disabled_reason = 'disabled on windows'
else
  if find_program('gtkdoc-scan', required : get_option('gtk_doc')).found()
    subdir('doc')
    have_docs = true
    gtk_doc_disabled_reason = ''
  elif get_option('gtk_doc').disabled()
    message('Not building documentation (disabled)')
    gtk_doc_disabled_reason = 'disabled'
  else
    message('Not building documentation as gtk-doc was not found')
    gtk_doc_disabled_reason = 'gtk-doc not found'
  endif
endif

pkg.generate (orc_lib,
  subdirs : 'orc-' + orc_api,
  description : 'Library of Optimized Inner Loops Runtime Compiler',
  variables : tools_variables,
  extra_cflags: orc_dep_cargs,
  libraries_private : orc_dependencies)

configure_file(output : 'config.h', configuration : cdata)

# summary
summary({
  'AVX': 'avx' in enabled_targets,
  'SSE': 'sse' in enabled_targets,
  'MMX': 'mmx' in enabled_targets,
  'NEON': 'neon' in enabled_targets,
  'MIPS': 'mips' in enabled_targets,
  'c64x': 'c64x' in enabled_targets,
  'Altivec': 'altivec' in enabled_targets,
  'RISCV': 'riscv' in enabled_targets,
  'LSX': 'lsx' in enabled_targets,
  'LASX': 'lasx' in enabled_targets,
  }, section: 'Targets', bool_yn: true)

if not have_docs
  doc_summary = [have_docs, gtk_doc_disabled_reason]
else
  doc_summary = have_docs
endif

summary({
  'Tools': not opt_tools.disabled(),
  'Tests': not opt_tests.disabled(),
  'Examples': not opt_examples.disabled(),
  'Benchmarks': not opt_benchmarks.disabled(),
  'Documentation': doc_summary,
  'Orc-test library': not opt_orctest.disabled(),
  }, section: 'Build options', bool_yn: true, list_sep: '  ')
