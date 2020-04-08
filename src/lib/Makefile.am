## -*- Mode: make; tab-width: 4; indent-tabs-mode: tabs -*-

if PLATFORM_WIN32
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

noinst_LTLIBRARIES = libqxp_internal.la
lib_LTLIBRARIES = libqxp-@QXP_MAJOR_VERSION@.@QXP_MINOR_VERSION@.la

AM_CXXFLAGS = \
	-DLIBQXP_BUILD=1 \
	-I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(ICU_CFLAGS) \
	$(DEBUG_CXXFLAGS)

if HAVE_VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -DLIBQXP_VISIBILITY
endif

libqxp_@QXP_MAJOR_VERSION@_@QXP_MINOR_VERSION@_la_LIBADD = \
	libqxp_internal.la \
	$(REVENGE_LIBS) \
	$(ICU_LIBS) \
	@LIBQXP_WIN32_RESOURCE@

libqxp_@QXP_MAJOR_VERSION@_@QXP_MINOR_VERSION@_la_DEPENDENCIES = libqxp_internal.la @LIBQXP_WIN32_RESOURCE@
libqxp_@QXP_MAJOR_VERSION@_@QXP_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic -no-undefined
libqxp_@QXP_MAJOR_VERSION@_@QXP_MINOR_VERSION@_la_SOURCES = \
	QXPDocument.cpp

libqxp_internal_la_SOURCES = \
	QXP1Header.cpp \
	QXP1Header.h \
	QXP1Parser.cpp \
	QXP1Parser.h \
	QXP33Deobfuscator.cpp \
	QXP33Deobfuscator.h \
	QXP33Header.cpp \
	QXP33Header.h \
	QXP33Parser.cpp \
	QXP33Parser.h \
	QXP3HeaderBase.cpp \
	QXP3HeaderBase.h \
	QXP4Deobfuscator.cpp \
	QXP4Deobfuscator.h \
	QXP4Header.cpp \
	QXP4Header.h \
	QXP4Parser.cpp \
	QXP4Parser.h \
	QXPBlockParser.cpp \
	QXPBlockParser.h \
	QXPCollector.h \
	QXPContentCollector.cpp \
	QXPContentCollector.h \
	QXPDeobfuscator.cpp \
	QXPDeobfuscator.h \
	QXPDetector.cpp \
	QXPDetector.h \
	QXPHeader.cpp \
	QXPHeader.h \
	QXPMacFileParser.cpp \
	QXPMacFileParser.h \
	QXPMemoryStream.cpp \
	QXPMemoryStream.h \
	QXPParser.cpp \
	QXPParser.h \
	QXPTextParser.cpp \
	QXPTextParser.h \
	QXPTypes.cpp \
	QXPTypes.h \
	libqxp_utils.cpp \
	libqxp_utils.h

if OS_WIN32

@LIBQXP_WIN32_RESOURCE@ : libqxp.rc $(libqxp_@QXP_MAJOR_VERSION@_@QXP_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libqxp.rc @LIBQXP_WIN32_RESOURCE@

endif

EXTRA_DIST = \
	libqxp.rc \
	libqxp.rc.in

## vim:set shiftwidth=4 tabstop=4 noexpandtab:
