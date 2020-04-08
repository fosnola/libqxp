## -*- Mode: make; tab-width: 4; indent-tabs-mode: tabs -*-

noinst_PROGRAMS = qxpfuzzer

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

qxpfuzzer_LDADD = \
	$(top_builddir)/src/lib/libqxp-@QXP_MAJOR_VERSION@.@QXP_MINOR_VERSION@.la \
	$(REVENGE_LIBS) \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	-lFuzzingEngine

qxpfuzzer_SOURCES = \
	qxpfuzzer.cpp

## vim:set shiftwidth=4 tabstop=4 noexpandtab:
