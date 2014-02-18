# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgsturishmsrc.la

# sources used to compile this plug-in
libgsturishmsrc_la_SOURCES = gsturishmsrc.c gsturishmsrc.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsturishmsrc_la_CFLAGS = $(GST_CFLAGS)
libgsturishmsrc_la_LIBADD = $(GST_LIBS)
libgsturishmsrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgsturishmsrc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gsturishmsrc.h
