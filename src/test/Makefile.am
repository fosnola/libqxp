## -*- Mode: make; tab-width: 4; indent-tabs-mode: tabs -*-

target_test = test detection

check_PROGRAMS = $(target_test)

AM_CXXFLAGS = \
	-DDETECTION_TEST_DIR=\"$(top_srcdir)/src/test/data\" \
	-DTEST_DATA_DIR=\"$(top_srcdir)/src/test/data\" \
	-I$(top_srcdir)/inc \
	-I$(top_srcdir)/src/lib \
	$(CPPUNIT_CFLAGS) \
	$(REVENGE_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

test_LDFLAGS = -L$(top_srcdir)/src/lib
test_LDADD = \
	$(top_builddir)/src/lib/libqxp_internal.la \
	$(CPPUNIT_LIBS) \
	$(ICU_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS)

test_SOURCES = \
	test.cpp \
	QXPBlockParserTest.cpp \
	QXPDeobfuscatorTest.cpp \
	QXPTextParserTest.cpp \
	QXPTypesTest.cpp \
	UtilsTest.cpp

detection_LDFLAGS = -L$(top_srcdir)/src/lib
detection_LDADD = \
	$(top_builddir)/src/lib/libqxp-@QXP_MAJOR_VERSION@.@QXP_MINOR_VERSION@.la \
	$(CPPUNIT_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS)

detection_SOURCES = \
	test.cpp \
	QXPDocumentTest.cpp


TESTS = $(target_test)

EXTRA_DIST = \
	data/qxp1.zip \
	data/qxp31mac \
	data/qxp31win.qxd \
	data/qxp33mac \
	data/qxp33mac_text \
	data/qxp33win.qxd \
	data/qxp33win_text.qxd \
	data/qxp4mac \
	data/qxp4mac_text \
	data/qxp4win.qxd \
	data/qxp4win_text.qxd \
	data/qxp5.qxd \
	data/qxp6.qxd \
	data/unsupported.zip

## vim:set shiftwidth=4 tabstop=4 noexpandtab:
