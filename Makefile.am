
AUTOMAKE_OPTIONS = foreign

if BUILD_TESTS
ORCTEST_DIR = orc-test
TESTSUITE_DIR= testsuite
else
ORCTEST_DIR =
TESTSUITE_DIR =
endif

SUBDIRS = orc $(ORCTEST_DIR) tools $(TESTSUITE_DIR) examples doc

DIST_SUBDIRS = orc orc-test tools examples testsuite doc


EXTRA_DIST = COPYING RELEASE autogen.sh gtk-doc.make orc.m4 \
	$(shell find "$(top_srcdir)" -type f -name meson.build ! -path "$(top_srcdir)/orc-0.4.*" ) \
	meson_options.txt orc/orc.map

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

pkgconfig_DATA = orc-$(ORC_MAJORMINOR).pc

cp_v_gen = $(cp_v_gen_$(V))
cp_v_gen_ = $(cp_v_gen_$(AM_DEFAULT_VERBOSITY))
cp_v_gen_0 = @echo "  CP     $@";

orc-$(ORC_MAJORMINOR).pc: orc.pc
	$(cp_v_gen)cp orc.pc orc-$(ORC_MAJORMINOR).pc

orc-$(ORC_MAJORMINOR)-uninstalled.pc: orc-uninstalled.pc
	$(cp_v_gen)cp orc-uninstalled.pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES=orc-$(ORC_MAJORMINOR)-uninstalled.pc

CLEANFILES = orc-$(ORC_MAJORMINOR).pc orc-$(ORC_MAJORMINOR)-uninstalled.pc

ACLOCAL_AMFLAGS = -I m4

aclocaldir = $(datadir)/aclocal
aclocal_DATA = orc.m4


