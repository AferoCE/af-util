AUTOMAKE_OPTIONS = subdir-objects
lib_LTLIBRARIES = libaf_util.la
include_HEADERS = af_log.h af_util.h af_mempool.h

if BUILD_TARGET_DEBUG
CFLAGS_BUILD_TYPE = -DBUILD_TARGET_DEBUG
else
CFLAGS_BUILD_TYPE = -DBUILD_TARGET_RELEASE
endif

# util library
LIBPATH=$(CURDIR)/.libs
libaf_util_la_LDFLAGS = -module -Wall -ggdb3 -std=gnu99 -shared -fPIC -soname, libaf_util.so.0
libaf_util_la_CFLAGS = -Wall -std=gnu99 $(CFLAGS_BUILD_TYPE)
libaf_util_la_SOURCES = log_buffer.c af_util.c af_mempool.c

.PHONY : build_info.h
$(libaf_util_la_SOURCES) : build_info.h
build_info.h :
	$(srcdir)/make-build-info.sh $(srcdir)
