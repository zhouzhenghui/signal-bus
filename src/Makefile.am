SUBDIRS=continuation

AM_CFLAGS=@PICFLAG@
AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src

lib_LIBRARIES = libsignalbus.a
libsignalbus_a_SOURCES = continuation.c closure.c

if HAVE_PTHREAD
  libsignalbus_a_SOURCES += continuation_pthread.c
endif
