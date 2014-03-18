/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_UNKNOWN_H
#define __CONTINUATION_COMPILER_UNKNOWN_H

#include <setjmp.h>

extern int __continuation_jmpbuf_initialized;
extern int __continuation_jmpcode[sizeof(jmp_buf) / sizeof(void *)];
extern void(*__continuation_diff_jmpbuf)(void);

#ifdef __cplusplus
extern "C" {
#endif
  extern void __continuation_patch_jmpbuf(jmp_buf *dst,  jmp_buf *src);
#ifdef __cplusplus
}  /* extern "C" */
#endif

#if HAVE_PTHREAD
# include <pthread.h>
  extern int __continuation_pthread_jmpbuf_initialized;
  extern int __continuation_pthread_jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  extern void(*__continuation_pthread_diff_jmpbuf)(void);
  extern pthread_once_t __continuation_pthread_once;

# ifdef __cplusplus
  extern "C" {
# endif
    extern void __continuation_pthread_patch_jmpbuf(jmp_buf *dest, jmp_buf *src);
# ifdef __cplusplus
  } /* extern "C" */
# endif
#endif /* HAVE_PTHREAD */

#ifndef CONTINUATION_STUB_ENTRY
# if HAVE_PTHREAD
#   define CONTINUATION_STUB_ENTRY(cont_stub) \
do { \
  if (!__continuation_pthread_jmpbuf_initialized) { \
    pthread_once(&__continuation_pthread_once, __continuation_pthread_diff_jmpbuf); \
  } \
  { \
    union { \
      int i; \
      int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
    } __continuation_addr__; \
    __continuation_addr__.i = setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
    if (__continuation_addr__.i) { \
      cont_stub = (void *)__continuation_addr__.i; \
    } \
  } \
} while (0)

#   define CONTINUATION_STUB_INVOKE(cont_stub) \
do { \
  jmp_buf __continuation_env__; \
  jmp_buf __continuation_jmp__; \
  memcpy(&__continuation_jmp__, &(cont_stub)->cont->invoke_buf, sizeof(jmp_buf)); \
  if (setjmp(__continuation_env__) == 0) { \
    __continuation_pthread_patch_jmpbuf(&__continuation_jmp__, &__continuation_env__); \
    longjmp(__continuation_jmp__, (int)(cont_stub)); \
  } \
} while (0)
# else
#   define CONTINUATION_STUB_ENTRY(cont_stub) \
do { \
  if (!__continuation_jmpbuf_initialized) { \
    __continuation_diff_jmpbuf(); \
  } \
  { \
    union { \
      int i; \
      int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
    } __continuation_addr__; \
    __continuation_addr__.i = setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
    if (__continuation_addr__.i) { \
      cont_stub = (void *)__continuation_addr__.i; \
    } \
  } \
} while (0)

#   define CONTINUATION_STUB_INVOKE(cont_stub) \
do { \
  jmp_buf __continuation_env__; \
  jmp_buf __continuation_jmp__; \
  memcpy(&__continuation_jmp__, &(cont_stub)->cont->invoke_buf, sizeof(jmp_buf)); \
  if (setjmp(__continuation_env__) == 0) { \
    __continuation_pthread_patch_jmpbuf(&__continuation_jmp__, &__continuation_env__); \
    longjmp(__continuation_jmp__, (int)(cont_stub)); \
  } \
} while (0)
# endif /* HAVE_PTHREAD */
#endif /* defined(CONTINUATION_STUB_ENTRY)  */

#endif /* __CONTINUATION_COMPILER_UNKNOWN_H */
