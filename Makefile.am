## Process this file with automake to produce Makefile.in

include $(top_srcdir)/aminclude_static.am

ACLOCAL_AMFLAGS = -I m4 --install

## Force top-level build before subdirectory builds
SUBDIRS = . test

EXTRA_DIST = git-semver

noinst_LTLIBRARIES = libepubutil.la
libepubutil_la_SOURCES = container.hpp container.cpp manifest.hpp	\
                         media-type.hpp metadata.hpp metadata.cpp	\
                         navigation.hpp navigation.cpp package.hpp	\
                         spine.hpp xml.hpp xml.cpp

AM_CPPFLAGS = $(LIBXML2_CPPFLAGS)
AM_CXXFLAGS =
LIBS = $(LIBXML2_LIBS)

AM_CPPFLAGS += $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS += $(CODE_COVERAGE_CXXFLAGS)
LIBS += $(CODE_COVERAGE_LIBS)

CODE_COVERAGE_IGNORE_PATTERN = "$(abspath $(abs_top_builddir))/test/*"

git-add:
	$(MAKE) distdir
	@(cd "$(distdir)"; find . -depth -type f -print0) | (cd "$(top_srcdir)"; xargs -0t git add --verbose)
	@(cd "$(top_srcdir)"; find . -name .gitignore -print0 | xargs -0t git add --verbose)
	$(RM) -r "$(distdir)"

doc: Doxyfile $(libepubutil_la_SOURCES)
	$(RM) -r "$(top_builddir)/doc"
	cd "$(top_srcdir)"; doxygen "$(abs_top_builddir)/Doxyfile"
	@set -x; TMPDIR="`mktemp -d`"; trap "$(RM) -r $${TMPDIR}" 0 1 2 15; mv "$(top_srcdir)/doc" "$${TMPDIR}/doc" && $(RM) -r "$(top_builddir)/doc" && mv "$${TMPDIR}/doc" "$(top_builddir)/doc"

git-doc: doc
	@set -x; TMPDIR="`mktemp -d`"; trap "$(RM) -r $${TMPDIR}" 0 1 2 15; cp -r "$(top_builddir)/doc" "$${TMPDIR}/doc"; cd "$(top_srcdir)"; git checkout gh-pages && $(RM) -r doc && cp -r "$${TMPDIR}/doc" doc && git add doc

git-tag:
	cd "$(top_srcdir)"; git tag -f -a "v$(PACKAGE_VERSION)" -m "v$(PACKAGE_VERSION)"

clean-local: code-coverage-clean
	@find "$(abs_top_srcdir)" -name '*~' -print0 | xargs -0t $(RM)
