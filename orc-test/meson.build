orc_test_headers = ['orctest.h', 'orcarray.h', 'orcrandom.h', 'orcprofile.h']
orc_test_sources = ['orctest.c', 'orcarray.c', 'orcrandom.c', 'orcprofile.c']

install_headers(orc_test_headers, subdir : 'orc-' + orc_api + '/orc-test')

if cc.get_id() != 'msvc'
  link_args = ['-Wl,--version-script,' + meson.current_source_dir() + '/orc-test.map']
else
  link_args = []
endif

orc_test_shr = shared_library ('orc-test-' + orc_api,
  orc_test_sources,
  version : libversion,
  soversion : soversion,
  include_directories : orc_inc,
  c_args : [ orc_c_args , '-DORC_ENABLE_UNSTABLE_API' ],
  dependencies : [libm, orc_dep],
  link_args : link_args,
  link_depends : 'orc-test.map',
  install : true)

orc_test_sta = static_library ('orc-test-' + orc_api,
  objects : orc_test_shr.extract_all_objects(),
  include_directories : orc_inc,
  c_args : [ orc_c_args , '-DORC_ENABLE_UNSTABLE_API' ],
  dependencies : [libm, orc_dep],
  install : true)

if get_option('default_library') == 'static'
  orc_test_lib = orc_test_sta
else
  orc_test_lib = orc_test_shr
endif

orc_test_dep = declare_dependency(include_directories : orc_inc,
  link_with : orc_test_lib)
