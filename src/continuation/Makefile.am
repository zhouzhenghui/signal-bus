continuation_include_HEADERS = \
        select_compiler_config.h \
        continuation_config.h \
        continuation_base.h \
        continuation.h \
        closure_base.h \
        closure.h

if HAVE_PTHREAD
  continuation_include_HEADERS += \
        continuation_pthread.h
endif

nobase_continuation_include_HEADERS = \
        compiler/armcc.h \
        compiler/gcc.h \
        compiler/msvc.h \
        compiler/unknown.h \
        misc/vector.h \
        misc/continuation_inline.h \
        misc/continuation_alloca.h \
        misc/no_omit_frame_pointer.h

continuation_includedir = $(includedir)/continuation
