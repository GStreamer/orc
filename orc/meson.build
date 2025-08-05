orc_sources = [
  'orc.c',
  'orcbytecode.c',
  'orccode.c',
  'orccodemem.c',
  'orccompiler.c',
  'orcconstant.c',
  'orccpu.c',
  'orcdebug.c',
  'orcemulateopcodes.c',
  'orcexecutor.c',
  'orcfunctions.c',
  'orconce.c',
  'orcopcode.c',
  'orcopcodes-sys.c',
  'orcparse.c',
  'orcprogram.c',
  'orcprogram-c.c',
  'orcrule.c',
  'orctarget.c',
  'orcutils.c',
  'orcvariable.c',
]

orc_headers = [
  'orc.h',
  'orcarm.h',
  'orcavx.h',
  'orcavxinsn.h',
  'orcbytecode.h',
  'orcbytecodes.h',
  'orccode.h',
  'orccompiler.h',
  'orcconstant.h',
  'orccpu.h',
  'orcdebug.h',
  'orcexecutor.h',
  'orcfunctions.h',
  'orcinstruction.h',
  'orclimits.h',
  'orcmmx.h',
  'orcmmxinsn.h',
  'orcneon.h',
  'orconce.h',
  'orcopcode.h',
  'orcparse.h',
  'orcpowerpc.h',
  'orcprogram.h',
  'riscv/orcriscv.h',
  'riscv/orcriscvinsn.h',
  'orcrule.h',
  'orcsseinsn.h',
  'orcsse.h',
  'orctarget.h',
  'orcutils.h',
  'orcvariable.h',
  'orcx86.h',
  'orcx86insn.h',
  'orcmips.h',
  'orc-stdint.h',
  'loongarch/orcloongarch.h',
  'loongarch/orcloongarchinsn.h',
  'loongarch/orclsxinsn.h',
]
install_headers(orc_headers, subdir : 'orc-' + orc_api + '/orc')

if 'avx' in enabled_targets or 'sse' in enabled_targets or 'mmx' in enabled_targets
  orc_sources += ['orcx86.c', 'orcx86insn.c', 'orcprogram-x86.c']
endif

if 'avx' in enabled_targets
  orc_sources += ['orcavx.c', 'orcavxinsn.c', 'orcrules-avx.c', 'orcprogram-avx.c']
endif

if 'sse' in enabled_targets
  orc_sources += ['orcsse.c', 'orcsseinsn.c', 'orcrules-sse.c', 'orcprogram-sse.c']
endif

if 'mmx' in enabled_targets
  orc_sources += ['orcmmx.c', 'orcmmxinsn.c', 'orcrules-mmx.c', 'orcprogram-mmx.c']
endif

if 'altivec' in enabled_targets
  orc_sources += ['orcrules-altivec.c', 'orcprogram-altivec.c', 'orcpowerpc.c']
endif

if 'neon' in enabled_targets
  orc_sources += ['orcprogram-neon.c', 'orcrules-neon.c', 'orcarm.c']
endif

# ARM target is disabled until it has decent coverage
if 'arm' in enabled_targets
  # we assume it is ok to include the same file (orcarm) twice
  # in case all targets are selected (ie neon and arm)
  # orc_sources += ['orcprogram-arm.c', 'orcrules-arm.c', 'orcarm.c']
endif

if 'c64x' in enabled_targets
  orc_sources += ['orcprogram-c64x-c.c']
endif

if 'mips' in enabled_targets
  orc_sources += ['orcmips.c', 'orcprogram-mips.c', 'orcrules-mips.c']
endif

if cpu_family.startswith('x86')
  orc_sources += ['orccpu-x86.c']
elif cpu_family == 'ppc' or cpu_family == 'ppc64'
  orc_sources += ['orccpu-powerpc.c']
# TODO: Add support for Windows
# https://gitlab.freedesktop.org/gstreamer/orc/-/issues/38
elif (cpu_family == 'arm' or cpu_family == 'aarch64') and host_system != 'windows'
  orc_sources += ['orccpu-arm.c']
elif cpu_family == 'mips' and host_machine.endian() == 'little'
  orc_sources += ['orccpu-mips.c']
endif

orc_c_args = ['-DORC_ENABLE_UNSTABLE_API', '-D_GNU_SOURCE']

orc_dependencies = [libm, librt, liblog, atomic_dep]
if host_os != 'windows'
  # winpthread may be detected but we don't use it
  orc_dependencies += threads
elif cc.get_id() == 'msvc'
  # Enabling the conforming preprocessor gives support to the '##' token within macros
  orc_c_args += '-Zc:preprocessor'
endif

orc_lib_args = orc_c_args + ['-DBUILDING_ORC']
orc_static_cargs = ['-DORC_STATIC_COMPILATION']

subdir('riscv')
subdir('loongarch')

orc_lib = library ('orc-' + orc_api,
  orc_sources,
  version : libversion,
  soversion : soversion,
  darwin_versions : osxversion,
  include_directories : orc_inc,
  c_args : orc_lib_args,
  c_static_args : orc_lib_args + orc_static_cargs,
  dependencies : orc_dependencies,
  gnu_symbol_visibility: 'inlineshidden',
  install : true)

# https://github.com/mesonbuild/meson/pull/12632
orc_dep_cargs = []
if get_option('default_library') == 'both'
  orc_dep_lib = orc_lib.get_shared_lib()
else
  orc_dep_lib = orc_lib
  if get_option('default_library') == 'static'
    orc_dep_cargs = orc_static_cargs
  endif
endif

orc_dep = declare_dependency(include_directories : orc_inc,
                             dependencies : orc_dependencies,
                             compile_args : orc_dep_cargs,
                             link_with : orc_dep_lib)
