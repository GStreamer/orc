
AUTOMAKE_OPTIONS = foreign

SUBDIRS = orc orc-test tools examples doc testsuite

EXTRA_DIST = COPYING autogen.sh gtk-doc.make orc.m4

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

pkgconfig_DATA = orc-$(ORC_MAJORMINOR).pc

orc-$(ORC_MAJORMINOR).pc: orc.pc
	cp orc.pc orc-$(ORC_MAJORMINOR).pc

orc-$(ORC_MAJORMINOR)-uninstalled.pc: orc-uninstalled.pc
	cp orc-uninstalled.pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES=orc-$(ORC_MAJORMINOR)-uninstalled.pc

CLEANFILES = orc-$(ORC_MAJORMINOR).pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

ACLOCAL_AMFLAGS = -I m4

aclocaldir = $(datadir)/aclocal
aclocal_DATA = orc.m4


