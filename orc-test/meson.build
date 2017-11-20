orc_test_headers = ['orctest.h', 'orcarray.h', 'orcrandom.h', 'orcprofile.h']
orc_test_sources = ['orctest.c', 'orcarray.c', 'orcrandom.c', 'orcprofile.c']

install_headers(orc_test_headers, subdir : 'orc-' + orc_api + '/orc-test')

orc_test_lib = library ('orc-test-' + orc_api, orc_test_sources,
  c_args : [ orc_c_args , '-DORC_ENABLE_UNSTABLE_API' ],
  dependencies : [libm, orc_dep],
  install : true)

orc_test_dep = declare_dependency(include_directories : orc_inc,
  link_with : orc_test_lib)
