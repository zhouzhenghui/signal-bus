/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_NO_OMIT_FRAME_POINTER_H
#define __CONTINUATION_NO_OMIT_FRAME_POINTER_H

#include "continuation_alloca.h"

/* force optimize with no-omit-frame-pointer, may refer to https://bugs.kde.org/show_bug.cgi?id=243270
 * also discussed/confirmed by some gcc developers: http://gcc.gnu.org/ml/gcc/2011-10/msg00253.html
 */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4))
# define FORCE_NO_OMIT_FRAME_POINTER()
#elif defined(__GNUC__) || HAVE_BUILTIN_RETURN_ADDRESS
# define FORCE_NO_OMIT_FRAME_POINTER() \
  do { \
    void * volatile no_omit_frame_pointer = 0; \
    if (no_omit_frame_pointer) no_omit_frame_pointer = __builtin_frame_address(0); \
  } while (0)
#elif defined(HAVE_ALLOCA)
# define FORCE_NO_OMIT_FRAME_POINTER() \
  do { \
     void * volatile no_omit_frame_pointer = 0; \
     if (no_omit_frame_pointer) no_omit_frame_pointer = alloca(0); \
  } while (0)
#elif __STDC_VERSION__ >= 199901L
  do { \
     int volatile n = 0; \
     if (n) { \
       char volatile no_omit_frame_pointer[n] = { 0 }; \
     } \
  } while (0)
#else
/* TODO: add more compiler compitible */
# define FORCE_NO_OMIT_FRAME_POINTER()
#endif

#endif /* __CONTINUATION_NO_OMIT_FRAME_POINTER_H */
