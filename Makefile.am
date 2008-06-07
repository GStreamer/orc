
AUTOMAKE_OPTIONS = foreign

SUBDIRS = orc testsuite examples doc

EXTRA_DIST = COPYING autogen.sh gtk-doc.make HACKING BUG-REPORTING

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

pkgconfig_DATA = orc-$(ORC_MAJORMINOR).pc

orc-$(ORC_MAJORMINOR).pc: orc.pc
	cp orc.pc orc-$(ORC_MAJORMINOR).pc

orc-$(ORC_MAJORMINOR)-uninstalled.pc: orc-uninstalled.pc
	cp orc-uninstalled.pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES=orc-$(ORC_MAJORMINOR)-uninstalled.pc

CLEANFILES = orc-$(ORC_MAJORMINOR).pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

ACLOCAL_FLAGS = -I m4

