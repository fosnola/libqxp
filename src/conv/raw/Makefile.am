## -*- Mode: make; tab-width: 4; indent-tabs-mode: tabs -*-

bin_PROGRAMS = qxp2raw

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(DEBUG_CXXFLAGS)

qxp2raw_DEPENDENCIES = @QXP2RAW_WIN32_RESOURCE@ 
qxp2raw_LDADD = \
	$(top_builddir)/src/lib/libqxp-@QXP_MAJOR_VERSION@.@QXP_MINOR_VERSION@.la \
	$(REVENGE_LIBS) \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@QXP2RAW_WIN32_RESOURCE@

qxp2raw_SOURCES = qxp2raw.cpp

if OS_WIN32

@QXP2RAW_WIN32_RESOURCE@ : qxp2raw.rc $(qxp2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource && \
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource qxp2raw.rc @QXP2RAW_WIN32_RESOURCE@

endif

# Include the qxp2raw_SOURCES in case we build a tarball without stream
EXTRA_DIST = \
	$(qxp2raw_SOURCES) \
	qxp2raw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	qxp2raw.rc
 
## vim:set shiftwidth=4 tabstop=4 noexpandtab:
