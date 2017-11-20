project ('orc', 'c', version : '0.4.28.1',
                     meson_version : '>= 0.32.0',
                     default_options : ['buildtype=debugoptimized',
                                        'warning_level=1',
                                        'c_std=gnu99'] )

orc_api = '0.4'
orc_version_major = meson.project_version().split('.')[0]
orc_version_minor = meson.project_version().split('.')[1]
orc_version_micro = meson.project_version().split('.')[2]

# maintaining compatibility with the previous libtool versioning
soversion = 0
libversion = '@0@.@1@.0'.format(soversion, orc_version_micro.to_int())

add_global_arguments('-DHAVE_CONFIG_H', language : 'c')

orc_inc = include_directories('.')

cc = meson.get_compiler('c')

cdata = configuration_data()      # config.h
pc_conf = configuration_data()    # orc.pc

cdata.set('ORC_EXPORTS', true)

all_backends = ['sse', 'mmx', 'altivec', 'neon', 'mips', 'c64x'] # 'arm'

backend = get_option('enable-backend')
foreach b : all_backends
  if backend == 'all' or backend == b
    cdata.set('ENABLE_BACKEND_' + b.to_upper(), 1)
  endif
endforeach

cpu_family = host_machine.cpu_family()
if cpu_family == 'x86'
  cdata.set('HAVE_I386', true)
elif cpu_family == 'x86_64'
  cdata.set('HAVE_AMD64', true)
elif cpu_family == 'powerpc'
  cdata.set('HAVE_POWERPC', true)
elif cpu_family == 'arm'
  cdata.set('HAVE_ARM', true)
elif cpu_family == 'mipsel'
  cdata.set('HAVE_MIPSEL', true)
endif

libm = cc.find_library('m', required : false)

librt = []
pc_conf.set('LIBRT', '')
if cc.has_function('clock_gettime')
  cdata.set('HAVE_CLOCK_GETTIME', true)
else
  # On glibc older than 2.17, clock_gettime is provided by time.h and -lrt
  librt = cc.find_library('rt', required : false)
  if librt.found() and cc.has_function('clock_gettime', dependencies : librt)
    pc_conf.set('LIBRT', '-lrt')
  endif
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
  pc_conf.set('EXEEXT', '.exe')
  pc_conf.set('PTHREAD_LIBS', '')
else
  # If it is not windows, we just assume it is a unix of sorts for now.
  cdata.set('HAVE_CODEMEM_MMAP', true)
  cdata.set('HAVE_THREAD_PTHREAD', true)
  pc_conf.set('EXEEXT', '')
  if host_os == 'android'
    pc_conf.set('PTHREAD_LIBS', '')
  else
    pc_conf.set('PTHREAD_LIBS', '-lpthread')
  endif
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
cdata.set('HAVE_POSIX_MEMALIGN', cc.has_function('posix_memalign'))
cdata.set('HAVE_MMAP', cc.has_function('mmap'))

cdata.set('HAVE_SYS_TIME_H', cc.has_header('sys/time.h'))
cdata.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))
cdata.set('HAVE_VALGRIND_VALGRIND_H', cc.has_header('valgrind/valgrind.h'))

cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('VERSION', meson.project_version())

subdir('orc')
subdir('orc-test')
subdir('tools')
subdir('examples')

if not get_option('disable_tests')
  subdir('testsuite')
endif

if not meson.is_cross_build() and not get_option('disable_gtkdoc') and host_os != 'windows'
  subdir('doc')
endif

pc_conf.set('prefix', get_option('prefix'))
pc_conf.set('exec_prefix', get_option('prefix'))
pc_conf.set('libdir', join_paths(get_option('prefix'), get_option('libdir')))
pc_conf.set('includedir', join_paths(get_option('prefix'), 'include'))
pc_conf.set('VERSION', meson.project_version())
pc_conf.set('ORC_MAJORMINOR', orc_api)
pc_conf.set('LIBM', libm.found() ? '-lm' : '')

pkgconfigdir = join_paths (get_option('libdir'), 'pkgconfig')
configure_file(input : 'orc.pc.in',
  output : 'orc-' + orc_api + '.pc',
  configuration : pc_conf,
  install_dir : pkgconfigdir)

# Install m4 macro that other projects use
install_data('orc.m4', install_dir : 'share/aclocal')

configure_file(output : 'config.h', configuration : cdata)
