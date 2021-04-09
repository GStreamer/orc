project ('orc', 'c', version : '0.4.32',
                     meson_version : '>= 0.54.0',
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

orc_inc = include_directories('.')

cc = meson.get_compiler('c')

cdata = configuration_data()      # config.h

pkg = import('pkgconfig')

# -Bsymbolic-functions
if cc.has_link_argument('-Wl,-Bsymbolic-functions')
  add_project_link_arguments('-Wl,-Bsymbolic-functions', language : 'c')
endif

# Symbol visibility
if cc.get_id() == 'msvc'
  export_define = '__declspec(dllexport) extern'
elif cc.has_argument('-fvisibility=hidden')
  add_project_arguments('-fvisibility=hidden', language: 'c')
  export_define = 'extern __attribute__ ((visibility ("default")))'
else
  export_define = 'extern'
endif
# Passing this through the command line would be too messy
cdata.set('ORC_API_EXPORT', export_define)

all_backends = ['sse', 'mmx', 'altivec', 'neon', 'mips', 'c64x'] # 'arm'
enabled_backends = []

backend = get_option('orc-backend')
foreach b : all_backends
  if backend == 'all' or backend == b
    cdata.set('ENABLE_BACKEND_' + b.to_upper(), 1)
    enabled_backends += [b]
  endif
endforeach

cpu_family = host_machine.cpu_family()
if cpu_family == 'x86'
  cdata.set('HAVE_I386', true)
elif cpu_family == 'x86_64'
  cdata.set('HAVE_AMD64', true)
elif cpu_family == 'ppc' or cpu_family == 'ppc64'
  cdata.set('HAVE_POWERPC', true)
elif cpu_family == 'arm'
  cdata.set('HAVE_ARM', true)
elif cpu_family == 'aarch64'
  cdata.set('HAVE_AARCH64', true)
elif cpu_family == 'mips' and host_machine.endian() == 'little'
  cdata.set('HAVE_MIPSEL', true)
else
  warning(cpu_family + ' isn\'t a supported cpu family for optimization')
endif

threads = dependency('threads')

libm = cc.find_library('m', required : false)

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
else
  # If it is not windows, we just assume it is a unix of sorts for now.
  cdata.set('HAVE_CODEMEM_MMAP', true)
  cdata.set('HAVE_THREAD_PTHREAD', true)
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
cdata.set('HAVE_POSIX_MEMALIGN', cc.has_function('posix_memalign', prefix : '#include <stdlib.h>'))
cdata.set('HAVE_MMAP', cc.has_function('mmap'))
cdata.set('HAVE_PTHREAD_JIT', cc.has_function('pthread_jit_write_protect_np'))

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
  libraries_private : orc_dependencies)

# Install m4 macro that other projects use
install_data('orc.m4', install_dir : 'share/aclocal')

configure_file(output : 'config.h', configuration : cdata)

# summary
summary({
  'SSE': 'sse' in enabled_backends,
  'MMX': 'mmx' in enabled_backends,
  'NEON': 'neon' in enabled_backends,
  'MIPS': 'mips' in enabled_backends,
  'c64x': 'c64x' in enabled_backends,
  'Altivec': 'altivec' in enabled_backends,
  }, section: 'Backends', bool_yn: true)

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
