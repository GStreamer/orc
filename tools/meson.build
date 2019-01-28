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
