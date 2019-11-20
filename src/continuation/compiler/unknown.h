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
 */

#include <setjmp.h>

/**
 * @def continuation_setjmp
 * @brief macro to define the function that is actually used.
 */
#ifndef continuation_setjmp
# define continuation_setjmp setjmp
#endif

/**
 * @def continuation_longjmp
 * @brief macro to define the function that is actually used.
 */
#ifndef continuation_longjmp
# define continuation_longjmp longjmp
#endif

/**
 * @name External variables
 * @{
 */
/**
 * @internal
 * @brief Indicate the configuration is initialized or not.
 * @see __continuation_diff_jmpbuf()
 */
extern int __continuation_jmpbuf_initialized;

/** 
 * @internal
 * @brief Container of the indexes of items that indicate the address to be jumped in a jmpbuf.
 *
 * @details The jmpbuf of setjmp()/longjmp() is taken as an array of pointer values,
 * and the indexed items of the array will be replaced with a program address and others
 * to simulate the invocation of a continuation through longjmp() call.
 * 
 * @see __continuation_diff_jmpbuf()
 */
extern int __continuation_jmpcode[sizeof(jmp_buf) / sizeof(void *)];
/** @} */

/**
 * @name Function pointers
 * Pointers to the functions that should not be inlined.
 * @{
 */
/**
 * @internal
 * @brief Initialize the __continuation_jmpcode by comparing the jmpbufs.
 * @details Find the indexes of jmpbuf contain the jump address for longjmp() call.
 * @see __continuation_patch_jmpbuf()
 */
extern void(*__continuation_diff_jmpbuf)(void);
/** @} */

#ifdef __cplusplus
extern "C" {
#endif
  /**
   * @internal
   * @brief Patch he destination jmpbuf with the source jmpbuf with __continuation_jmpcode.
   * @details Replace the corresponding contents of the \p dst with \p src to allow jumping to the address of later..
   * @param dst: destination jmpbuf of current context that should be patched.
   * @param src: the original jmpbuf contains the address info to be jumped to.
   */
  extern void __continuation_patch_jmpbuf(jmp_buf *dst,  jmp_buf *src);
#ifdef __cplusplus
} /* extern "C" */
#endif

#if HAVE_PTHREAD
# include <pthread.h>
  /**
   * @name External variables of pthread environment
   * @{
   */
  /**
   * @internal
   * @brief Indicate the configuration is initialized or not.
   * @see __continuation_jmpbuf_initialized
   */
  extern int __continuation_pthread_jmpbuf_initialized;

  /**
   * @internal
   * @brief Indexes of jump address in jmpbuf.
   * @see __continuation_jmpcode
   */
  extern int __continuation_pthread_jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  
  /**
   * @internal
   * @brief Variable for pthread_once() call
   * @see __continuation_pthread_diff_jmpbuf()
   */
  extern pthread_once_t __continuation_pthread_once;
  /** @} */

  /**
   * @name Function pointers of pthread environment
   * Pointers to the functions that should not be inlined.
   * @{
   */
  /**
   * @internal
   * @brief Initialize the __continuation_pthread_jmpcode by comparing the jmpbufs..
   * @see __continuation_diff_jmpbuf()
   * @see __continuation_pthread_patch_jmpbuf()
   */
  extern void(*__continuation_pthread_diff_jmpbuf)(void);
  /** @} */

# ifdef __cplusplus
  extern "C" {
# endif
    /**
     * @internal
     * @brief Patch he destination jmpbuf with the source jmpbuf with __continuation_pthread_jmpcode.
     * @param dst: destination jmpbuf of current context that should be patched.
     * @param src: the original jmpbuf contains the address info to be jumped to.
     * @see __continuation_patch_jmpbuf()
     * @see __continuation_pthread_diff_jmpbuf()
     */
    extern void __continuation_pthread_patch_jmpbuf(jmp_buf *dest, jmp_buf *src);
# ifdef __cplusplus
  } /* extern "C" */
# endif
#endif /* HAVE_PTHREAD */

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
    if (!__continuation_pthread_jmpbuf_initialized) { \
      pthread_once(&__continuation_pthread_once, __continuation_pthread_diff_jmpbuf); \
    } \
    { \
      union { \
        int i; \
        int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
      } __continuation_addr__; \
      __continuation_addr__.i = continuation_setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
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
    if (continuation_setjmp(__continuation_env__) == 0) { \
      __continuation_pthread_patch_jmpbuf(&__continuation_jmp__, &__continuation_env__); \
      continuation_longjmp(__continuation_jmp__, (int)(cont_stub)); \
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
      __continuation_addr__.i = continuation_setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf); \
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
    if (continuation_setjmp(__continuation_env__) == 0) { \
      union { \
        int i; \
        int Fixme_for_unknown_platform_that_sizeof_pointer_is_not_equal_to_int : !!(sizeof(cont_stub) == sizeof(int)); \
      } __continuation_addr__; \
      __continuation_addr__.i = (int)(cont_stub); \
      __continuation_patch_jmpbuf(&__continuation_jmp__, &__continuation_env__); \
      continuation_longjmp(__continuation_jmp__, __continuation_addr__.i); \
    } \
  } while (0)
# endif /* HAVE_PTHREAD or not */
/** @endcond */
#endif /* !defined(CONTINUATION_STUB_ENTRY) */

#endif /* __CONTINUATION_COMPILER_UNKNOWN_H */
