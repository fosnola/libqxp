## -*- Mode: make; tab-width: 4; indent-tabs-mode: tabs -*-

bin_PROGRAMS = qxp2svg

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(REVENGE_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(DEBUG_CXXFLAGS)

qxp2svg_DEPENDENCIES = @QXP2SVG_WIN32_RESOURCE@ 
qxp2svg_LDADD = \
	$(top_builddir)/src/lib/libqxp-@QXP_MAJOR_VERSION@.@QXP_MINOR_VERSION@.la \
	$(REVENGE_LIBS) \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@QXP2SVG_WIN32_RESOURCE@

qxp2svg_SOURCES = qxp2svg.cpp

if OS_WIN32

@QXP2SVG_WIN32_RESOURCE@ : qxp2svg.rc $(qxp2svg_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource && \
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource qxp2svg.rc @QXP2SVG_WIN32_RESOURCE@

endif

# Include the qxp2svg_SOURCES in case we build a tarball without stream
EXTRA_DIST = \
	$(qxp2svg_SOURCES) \
	qxp2svg.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	qxp2svg.rc
 
## vim:set shiftwidth=4 tabstop=4 noexpandtab:
