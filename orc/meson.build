orc_sources = [
  'orc.c',
  'orcbytecode.c',
  'orccode.c',
  'orccodemem.c',
  'orccompiler.c',
  'orcdebug.c',
  'orcemulateopcodes.c',
  'orcexecutor.c',
  'orcfunctions.c',
  'orconce.c',
  'orcopcodes.c',
  'orcparse.c',
  'orcprogram.c',
  'orcprogram-c.c',
  'orcrule.c',
  'orcutils.c',
]

orc_headers = [
  'orc.h',
  'orcarm.h',
  'orcbytecode.h',
  'orcbytecodes.h',
  'orccode.h',
  'orccompiler.h',
  'orcconstant.h',
  'orccpu.h',
  'orccpuinsn.h',
  'orcdebug.h',
  'orcemulateopcodes.h', # FIXME: this probably shouldn't be installed, symbols are not exported or useful
  'orcexecutor.h',
  'orcfunctions.h',
  'orcinstruction.h',
  'orcinternal.h', # FIXME: this probably shouldn't be installed, symbols are not exported or useful
  'orclimits.h',
  'orcmmx.h',
  'orcneon.h',
  'orconce.h',
  'orcopcode.h',
  'orcparse.h',
  'orcpowerpc.h',
  'orcprogram.h',
  'orcrule.h',
  'orcsse.h',
  'orctarget.h',
  'orcutils.h',
  'orcvariable.h',
  'orcx86.h',
  'orcx86insn.h',
  'orcmips.h',
  'orc-stdint.h',
]
install_headers(orc_headers, subdir : 'orc-' + orc_api + '/orc')

if backend == 'sse' or backend == 'all'
  orc_sources += ['orcsse.c', 'orcrules-sse.c', 'orcprogram-sse.c',
    'orcx86.c', 'orcx86insn.c']
endif

if backend == 'mmx' or backend == 'all'
  # we assume it is ok to include the same file (orcx86) twice
  # in case all backends are selected (ie mmx and sse)
  orc_sources += ['orcmmx.c', 'orcrules-mmx.c', 'orcprogram-mmx.c', 'orcx86.c']
endif

if backend == 'altivec' or backend == 'all'
  orc_sources += ['orcrules-altivec.c', 'orcprogram-altivec.c', 'orcpowerpc.c']
endif

if backend == 'neon' or backend == 'all'
  orc_sources += ['orcprogram-neon.c', 'orcrules-neon.c', 'orcarm.c']
endif

# ARM backend is disabled until it has decent coverage
if backend == 'arm' or backend == 'all'
  # we assume it is ok to include the same file (orcarm) twice
  # in case all backends are selected (ie neon and arm)
  # orc_sources += ['orcprogram-arm.c', 'orcrules-arm.c', 'orcarm.c']
endif

if backend == 'c64x' or backend == 'all'
  orc_sources += ['orcprogram-c64x-c.c']
endif

if backend == 'mips' or backend == 'all'
  orc_sources += ['orcmips.c', 'orcprogram-mips.c', 'orcrules-mips.c']
endif

if cpu_family.startswith('x86')
  orc_sources += ['orccpu-x86.c']
elif cpu_family == 'ppc' or cpu_family == 'ppc64'
  orc_sources += ['orccpu-powerpc.c']
elif cpu_family == 'arm' or cpu_family == 'aarch64'
  orc_sources += ['orccpu-arm.c']
elif cpu_family == 'mips' and host_machine.endian() == 'little'
  orc_sources += ['orccpu-mips.c']
endif

orc_c_args = ['-DORC_ENABLE_UNSTABLE_API', '-D_GNU_SOURCE']

orc_dependencies = [libm, librt, liblog, threads]

orc_lib = library ('orc-' + orc_api,
  orc_sources,
  version : libversion,
  soversion : soversion,
  darwin_versions : osxversion,
  include_directories : orc_inc,
  c_args : orc_c_args + ['-DBUILDING_ORC'],
  dependencies : orc_dependencies,
  install : true)

orc_dep_cargs = []
if get_option('default_library') == 'static'
  orc_dep_cargs += ['-DORC_STATIC_COMPILATION']
endif

orc_dep = declare_dependency(include_directories : orc_inc,
                             dependencies : orc_dependencies,
                             compile_args : orc_dep_cargs,
                             link_with : orc_lib)

executable ('generate-bytecode',
  'generate-bytecode.c',
  dependencies : orc_dep,
  install : false)

executable ('generate-emulation',
  'generate-emulation.c',
  dependencies : orc_dep,
  install : false)
