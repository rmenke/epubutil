TESTS = 01-container.test 02-package.test 03-media.test 04-image.test

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/build-aux/tap-driver.sh

AM_DEFAULT_SOURCE_EXT = .cpp
AM_CPPFLAGS = -I$(top_srcdir) -DTESTDIR=\"$(abs_srcdir)\"
AM_CXXFLAGS = -Wall -Wpedantic

LDADD = $(top_builddir)/libepubutil.la

AM_CPPFLAGS += $(LIBXML2_CPPFLAGS)
LIBS = $(LIBXML2_LIBS)

AM_CPPFLAGS += $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS += $(CODE_COVERAGE_CXXFLAGS)
LIBS += $(CODE_COVERAGE_LIBS)

AM_TEST_LOG_DRIVER_FLAGS = $(am__test_driver_flags_@AM_V@)
am__test_driver_flags_ = $(am__test_driver_flags_@AM_DEFAULT_V@)
am__test_driver_flags_0 =
am__test_driver_flags_1 = --comments

EXTRA_DIST = tap.hpp pach1.xhtml pach2.xhtml pach3.xhtml	\
             img_wrong_ext.gif

04_image_test_LDADD = $(top_builddir)/src/image_ref.o

check_PROGRAMS = $(TESTS)

