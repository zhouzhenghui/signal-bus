/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_UNKNOWN_H
#define __CONTINUATION_COMPILER_UNKNOWN_H

/**
 * @file
 * @ingroup continuation
 * @brief General continuation configuration for unknown compilers.
 * @details Implement the entry point and invocation of continuation based on standard setjmp()/longjmp() calls.
 * 
 * The jmpbuf of setjmp()/longjmp() is taken as an array of pointer values,
 * and the indexed items of the array will be replaced with a program address and others
 * to simulate the invocation of a continuation through longjmp() call.
 */

#include <setjmp.h>
#if HAVE_MEMORY_H
# include <memory.h>
#endif
#include <string.h>
#include "misc/no_omit_frame_pointer.h"

/**
 * @name External variables
 * @{
 */
/**
 * @internal
 * @brief Structure to hold the jump addresses of jmpbuf.
 * @see __continuation_init_jmpcode()
 */
static struct {
  int initialized; /**< indicate the structure is initialized or not. */
  int indexes[sizeof(jmp_buf) / sizeof(void *)]; /**< indexes of jump address in jmpbuf. */
} __continuation_jmpcode = { 0, { 0 } };
/** @} */

/**
 * @internal
 * @fn __continuation_diff_jmpbuf_help
 */

/**
 * @internal
 * @fn __continuation_diff_jmpbuf
 */

/* these functions should be compiled without any optimization */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void __continuation_diff_jmpbuf_help(jmp_buf *) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_diff_jmpbuf() __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void __continuation_diff_jmpbuf_help(jmp_buf *);
  static void __continuation_diff_jmpbuf();
#endif

/**
 * @name Function pointers
 * Pointers to the functions that should not be inlined.
 * @{
 */
/**
 * @internal
 * @brief Initialize the __continuation_jmpcode by comparing the jmp_bufs.
 * @details Find the indexes of jmpbuf contain the jump address for longjmp() call.
 * @see __continuation_patch_jmpbuf()
 */
static void(*__continuation_init_jmpcode)() = __continuation_diff_jmpbuf;
/** @} */

static void __continuation_diff_jmpbuf_help(jmp_buf *environment)
{
  setjmp(*environment);
  FORCE_NO_OMIT_FRAME_POINTER();
}

static void __continuation_diff_jmpbuf()
{
  static void(* volatile diff_jmpbuf_help)(jmp_buf *) = &__continuation_diff_jmpbuf_help;
  volatile int jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  jmp_buf jmpbuf1, jmpbuf2;
  if (setjmp(jmpbuf1) == 0) {
    memcpy((void *)&jmpbuf2, (void *)&jmpbuf1, sizeof(jmp_buf));
    if (setjmp(jmpbuf1) == 0) {
      int i, j = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          jmpcode[j++] = i;
        }
      }
      jmpcode[j] = -1;
      memcpy((void *)&jmpbuf1, (void *)&jmpbuf2, sizeof(jmp_buf));
    }
    longjmp(jmpbuf1, 1);
  } else {
    diff_jmpbuf_help(&jmpbuf1);
    {
      volatile int i, j = 0, k = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          while(jmpcode[j] >= 0 && jmpcode[j] < i) j++;
          if (jmpcode[j] != i) {
            __continuation_jmpcode.indexes[k++] = i;
          }
        }
      }
      __continuation_jmpcode.indexes[k] = -1;
    }
  }
  FORCE_NO_OMIT_FRAME_POINTER();
}

#if HAVE_PTHREAD
# include <pthread.h>
  /**
   * @name External variables of pthread environment
   * @{
   */
  /**
   * @internal
   * @brief Structure to hold the jump addresses of jmpbuf.
   */
  static struct {
    int initialized; /**< indicate the structure is initialized or not. */
    int indexes[sizeof(jmp_buf) / sizeof(void *)]; /**< indexes of jump address in jmpbuf. */
    pthread_once_t once; /**< variable for pthread_once() call */
    pthread_mutex_t jmpbuf_mutex;
    pthread_key_t address_key;
  } __continuation_pthread_jmpcode = { 0, { 0 }, PTHREAD_ONCE_INIT };
  /** @} */

/**
 * @internal
 * @fn __continuation_pthread_diff_jmpbuf_help
 */

/**
 * @internal
 * @fn __continuation_pthread_diff_jmpbuf
 */

/* these functions should be compiled without any optimization */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void *__continuation_pthread_diff_jmpbuf_help(jmp_buf *) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_pthread_diff_jmpbuf() __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void *__continuation_pthread_diff_jmpbuf_help(jmp_buf *);
  static void __continuation_pthread_diff_jmpbuf();
#endif

  /**
   * @name Function pointers of pthread environment
   * Pointers to the functions that should not be inlined.
   * @{
   */
  /**
   * @internal
   * @brief Initialize the __continuation_pthread_jmpcode by comparing the jmp_bufs.
   * @see __continuation_pthread_patch_jmpbuf()
   */
  static void(*__continuation_pthread_init_jmpcode)() = __continuation_pthread_diff_jmpbuf;
  /** @} */

static void *__continuation_pthread_diff_jmpbuf_help(jmp_buf *environment)
{
  pthread_mutex_lock(&__continuation_pthread_jmpcode.jmpbuf_mutex);
  setjmp(*environment);
  pthread_mutex_unlock(&__continuation_pthread_jmpcode.jmpbuf_mutex);
  FORCE_NO_OMIT_FRAME_POINTER();
  return NULL;
}

static void __continuation_pthread_diff_jmpbuf()
{
  static void *(* volatile diff_jmpbuf_help)(jmp_buf *environment) = &__continuation_pthread_diff_jmpbuf_help;
  volatile int jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  jmp_buf jmpbuf1, jmpbuf2;
  pthread_t pthread_id;
  pthread_key_create(&__continuation_pthread_jmpcode.address_key, NULL);
  pthread_mutex_init(&__continuation_pthread_jmpcode.jmpbuf_mutex, NULL);
  pthread_mutex_lock(&__continuation_pthread_jmpcode.jmpbuf_mutex);
  setjmp(jmpbuf1);
  pthread_create(&pthread_id, NULL, (void *(*)(void *))diff_jmpbuf_help, (void *)&jmpbuf1);;
  memcpy((void *)&jmpbuf2, (void *)&jmpbuf1, sizeof(jmp_buf));
  setjmp(jmpbuf1);
  {
    int i, j = 0;
    for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
      if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
        jmpcode[j++] = i;
      }
    }
    jmpcode[j] = -1;
  }

  pthread_mutex_unlock(&__continuation_pthread_jmpcode.jmpbuf_mutex);
  pthread_join(pthread_id, NULL);
  {
    volatile int i, j = 0, k = 0;
    for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
      if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
        while(jmpcode[j] >= 0 && jmpcode[j] < i) j++;
        if (jmpcode[j] != i) {
          __continuation_pthread_jmpcode.indexes[k++] = i;
        }
      }
    }
    __continuation_pthread_jmpcode.indexes[k] = -1;
  }
  pthread_mutex_destroy(&__continuation_pthread_jmpcode.jmpbuf_mutex);
  FORCE_NO_OMIT_FRAME_POINTER();
}

#endif /* HAVE_PTHREAD */

#ifdef __cplusplus
extern "C" {
#endif
  /**
   * @internal
   * @brief Patch he destination jmpbuf with the source jmpbuf with __continuation_jmpcode.
   * @details Replace the corresponding contents of the \p dst with \p src to allow jumping to the address of later..
   * @param jmpcode: pointer to the indexes of addresses in a jmp_buf array.
   * @param dst: destination jmpbuf of current context that should be patched.
   * @param src: the original jmpbuf contains the address info to be jumped to.
   */
  extern void __continuation_patch_jmpbuf(int *jmpcode, jmp_buf *dst,  jmp_buf *src);
#ifdef __cplusplus
} /* extern "C" */
#endif

#ifndef CONTINUATION_STUB_ENTRY

/**
 * @internal
 * @brief Macro represents a continuation entry point with setjmp() call.
 * @param cont_stub: pointer to the stub of a continuation.
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_STUB_INVOKE()
 */
# define CONTINUATION_STUB_ENTRY(cont_stub) /* Empty defintion for doxygen */
# undef CONTINUATION_STUB_ENTRY

/**
 * @internal
 * @brief Macro represents a continuation invocation with longjmp() call.
 * @param cont_stub: pointer to the stub of a continuation.
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_STUB_ENTRY()
 */
# define CONTINUATION_STUB_INVOKE(cont_stub) /* Empty defintion for doxygen */
# undef CONTINUATION_STUB_INVOKE

/** @cond */
# if HAVE_PTHREAD
#   define CONTINUATION_STUB_ENTRY(cont_stub) \
  do { \
    if (!__continuation_pthread_jmpcode.initialized) { \
      pthread_once(&__continuation_pthread_jmpcode.once, __continuation_pthread_init_jmpcode); \
      __continuation_pthread_jmpcode.initialized = 1; \
    } \
    { \
      int __continuation_addr__; \
      __continuation_addr__ = setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
      if (__continuation_addr__) { \
        *((void **)&cont_stub) = pthread_getspecific(__continuation_pthread_jmpcode.address_key); \
      } \
    } \
  } while (0)
#   define CONTINUATION_STUB_INVOKE(cont_stub) \
  do { \
    jmp_buf __continuation_env__; \
    jmp_buf __continuation_jmp__; \
    memcpy(&__continuation_jmp__, &(cont_stub)->cont->invoke_buf, sizeof(jmp_buf)); \
    if (setjmp(__continuation_env__) == 0) { \
      __continuation_patch_jmpbuf(__continuation_pthread_jmpcode.indexes, &__continuation_jmp__, &__continuation_env__); \
      pthread_setspecific(__continuation_pthread_jmpcode.address_key, cont_stub); \
      longjmp(__continuation_jmp__, 1); \
    } \
  } while (0)
# else /* !HAVE_PTHREAD */
#   define CONTINUATION_STUB_ENTRY(cont_stub) \
  do { \
    if (!__continuation_jmpcode.initialized) { \
      __continuation_init_jmpcode(); \
      __continuation_jmpcode.initialized = 1; \
    } \
    { \
      union { \
        int i; \
        int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
      } __continuation_addr__; \
      __continuation_addr__.i = setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
      if (__continuation_addr__.i) { \
        *((struct __ContinuationStub **)&cont_stub) = (struct __ContinuationStub *)__continuation_addr__.i; \
      } \
    } \
  } while (0)
#   define CONTINUATION_STUB_INVOKE(cont_stub) \
  do { \
    jmp_buf __continuation_env__; \
    jmp_buf __continuation_jmp__; \
    memcpy(&__continuation_jmp__, &(cont_stub)->cont->invoke_buf, sizeof(jmp_buf)); \
    if (setjmp(__continuation_env__) == 0) { \
      union { \
        int i; \
        int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
      } __continuation_addr__; \
      __continuation_addr__.i = (int)(cont_stub); \
      __continuation_patch_jmpbuf(__continuation_jmpcode.indexes, &__continuation_jmp__, &__continuation_env__); \
      longjmp(__continuation_jmp__, __continuation_addr__.i); \
    } \
  } while (0)
# endif /* HAVE_PTHREAD or not */
/** @endcond */
#endif /* !defined(CONTINUATION_STUB_ENTRY) */

#endif /* __CONTINUATION_COMPILER_UNKNOWN_H */
