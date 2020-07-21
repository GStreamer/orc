gnome = import('gnome')

version_entities = configuration_data()
version_entities.set('ORC_MAJORMINOR', orc_api)
version_entities.set('PACKAGE_VERSION', meson.project_version())

# FIXME: xml files should be changed to include ../version.entities instead
configure_file (input : 'version.entities.in',
                output : 'version.entities',
                configuration : version_entities)

# FIXME: hack to copy these files from the source dir into the build dir,
# should really be an option to gnome.gtkdoc() if tools can be made to
# look in the srcdir as well.
if false
  content_files = ['building.xml', 'concepts.xml', 'opcode_table.xml',
                   'opcodes.xml', 'program.xml', 'running.xml',
                   'tutorial.xml', 'table.xml']

  # FIXME: these just need to be copied from srcdir to builddir
  dummy_config = configuration_data()
  foreach f : content_files
    configure_file (input : f,
                    output : f,
                    configuration : dummy_config)
  endforeach
endif

# FIXME: there should be a way to specify the gtk-doc requirement
# html_args : '--path=' + meson.current_build_dir() + ':' + meson.current_source_dir(),
gnome.gtkdoc('orc',
  src_dir : meson.current_source_dir() / '..' / 'orc',
  main_sgml : 'orc-docs.sgml',
  install : true)
