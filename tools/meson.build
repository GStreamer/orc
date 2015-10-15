orcc = executable ('orcc', 'orcc.c',
                   install: true,
                   dependencies : [libm, orc_dep])

orc_bugreport = executable ('orc-bugreport', 'orc-bugreport.c',
                            install: true,
                            dependencies : [orc_dep, orc_test_dep])
