orcc = executable ('orcc', 'orcc.c',
                   install: true,
                   dependencies : [libm, orc_dep])

# Make orcc usable for other subprojects
if meson.has_exe_wrapper()
  meson.override_find_program('orcc', orcc)
endif

orc_bugreport = executable ('orc-bugreport', 'orc-bugreport.c',
                            install: true,
                            dependencies : [orc_dep, orc_test_dep])

if host_os == 'windows'
  orcc_filename = 'orcc.exe'
else
  orcc_filename = 'orcc'
endif

tools_variables = [
  'toolsdir=' + ('${prefix}' / get_option('bindir')),
  'orcc=' + ('${toolsdir}' / orcc_filename),
]
