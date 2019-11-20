/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_GCC_H
#define __CONTINUATION_COMPILER_GCC_H

/**
 * @file
 * @ingroup continuation
 * @brief Continuation configuration of GNU C Compiler.
 */

#ifdef __SANITIZE_ADDRESS__
# error "The library cannot compiled with address sanitizer enabled by option -fsanitize=address"
#endif

#if !defined(BOOST_PP_VARIADICS) && !defined(__STRICT_ANSI__)
/**
 * @brief Enable variadic macros by default.
 */
# define BOOST_PP_VARIADICS 1
#endif

#include <boost/preprocessor/cat.hpp>

/**
 * @brief Macro represents a continuation entry point for GCC Compiler on various platforms.
 * @see CONTINUATION_CONNECT()
 */
#define CONTINUATION_STUB_ENTRY(cont_stub) /* Empty definition for Doxygen */
#undef CONTINUATION_STUB_ENTRY

/**
 * @brief Macro invokes a continuation for GCC Compiler on various platforms.
 * @see CONTINUATION_CONNECT()
 */
#define CONTINUATION_STUB_INVOKE(cont_stub) /* Empty definition for Doxygen */
#undef CONTINUATION_STUB_INVOKE

/** @cond */
#if (!defined(CONTINUATION_USE_LONGJMP) || !CONTINUATION_USE_LONGJMP)
# if defined(__i386__) && !defined(__STRICT_ANSI__)
#   if __GNUC__ >= 3
#     define CONTINUATION_STUB_ENTRY(cont_stub) \
      do { \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        { \
          void * volatile anti_optimize = && BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
          ((struct __ContinuationStub *)cont_stub)->cont->func_addr = anti_optimize; \
        } \
        if (((struct __ContinuationStub *)cont_stub)->cont->func_addr) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        else { \
          /* Clobber registers using pretend setjmp */ \
          if (setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf) == 0) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__): \
        { \
          __asm__("movl 0x4(%%esp), %%eax; mov %%eax, %0":"=m"(cont_stub)::"ax","bx","cx","dx","si","di","memory"); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__): \
        /* suppress the compile warning */; \
      } while (0)

/*
#define CONTINUATION_STUB_ENTRY(cont_stub) \
  ((struct __ContinuationStub *)cont_stub)->cont->func_addr = && BOOST_PP_CAT(LABEL_BEGIN_, __LINE__); \
  if ((size_t)(((struct __ContinuationStub *)cont_stub)->cont->func_addr) + 1) goto BOOST_PP_CAT(LABEL_END_, __LINE__); \
  BOOST_PP_CAT(LABEL_BEGIN_, __LINE__): \
  { \
    __asm__ __volatile__("movl %%eax, %0":"=m"(cont_stub)::"ax","bx","cx","dx","si","di","memory"); \
  } \
  BOOST_PP_CAT(LABEL_END_, __LINE__):

#define  CONTINUATION_STUB_INVOKE(cont_stub) \
  __asm__ __volatile__("movl %0, %%eax\n\t jmp %1"::"m"(cont_stub), "m"(((struct __ContinuationStub *)cont_stub)->cont->func_addr):"memory");
*/
#   endif /* __GNUC__ >= 3 */
# elif defined(__x86_64__)
#   if defined(__CYGWIN__)
#     define CONTINUATION_STUB_ENTRY(cont_stub) \
      do { \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        { \
          void * volatile anti_optimize = && BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
          ((struct __ContinuationStub *)cont_stub)->cont->func_addr = anti_optimize; \
        } \
        if (((struct __ContinuationStub *)cont_stub)->cont->func_addr) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        else { \
          /* Clobber registers using pretend setjmp */ \
          if (setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf) == 0) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__): \
        { \
          __asm__("movq %%rcx, %0":"=m"(cont_stub)::"ax","bx","cx","dx","si","di","r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory"); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__): \
        /* suppress the compile warning */; \
      } while (0)
#     define CONTINUATION_STUB_INVOKE(cont_stub) \
      do { \
        /* restore the alignment (16 bytes) of stack variables */ \
        __asm__ __volatile__("movq %0, %%rcx\n\t " \
                              "movq %1, %%rax \n\t" \
                              "movb %2, %%dl \n\t " \
                              "andb $0x0f, %%dl \n\t " \
                              "movw %%bp, %%bx \n\t " \
                              "andb $0xf0, %%bl \n\t" \
                              "orb %%dl, %%bl \n\t" \
                              "movw %%bx, %%bp \n\t " \
                              "jmp *%%rax" \
                              :: "p"(cont_stub) \
                                , "m"(((struct __ContinuationStub *)(cont_stub))->cont->func_addr) \
                                , "m"(((struct __ContinuationStub *)(cont_stub))->cont->stack_frame_addr) \
                              :"memory"); \
      } while (0)
#   else /* WINDOWS or SYSV ABI */
#     define CONTINUATION_STUB_ENTRY(cont_stub) \
      do { \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
        __label__ BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        { \
          void * volatile anti_optimize = && BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__); \
          ((struct __ContinuationStub *)cont_stub)->cont->func_addr = anti_optimize; \
        } \
        if (((struct __ContinuationStub *)cont_stub)->cont->func_addr) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        else { \
          /* Clobber registers using pretend setjmp */ \
          if (setjmp(((struct __ContinuationStub *)cont_stub)->cont->invoke_buf) == 0) goto BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_BEGIN_, __LINE__): \
        { \
          __asm__("movq %%rdi, %0":"=m"(cont_stub)::"ax","bx","cx","dx","si","di","r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory"); \
        } \
        BOOST_PP_CAT(LABEL_CONTINUATION_ENTRY_END_, __LINE__): \
        /* suppress the compile warning */; \
      } while (0)
#     define CONTINUATION_STUB_INVOKE(cont_stub) \
      do { \
        __asm__ __volatile__("movq %0, %%rdi\n\t " \
                              "jmp *%1" \
                              :: "p"(cont_stub) \
                                , "m"(((struct __ContinuationStub *)(cont_stub))->cont->func_addr) \
                              :"memory"); \
      } while (0)
#     if defined(_WIN64) /* MINGW64 */
#       define continuation_setjmp __builtin_setjmp
#       define continuation_longjmp __builtin_longjmp
#     endif /* MINGW64 */
#   endif /* WINDOWS or SYSV ABI */
# endif /* __i386__ or __x86_64__*/
#endif /* !CONTINUATION_USE_LONGJMP */
/** @endcond */

/**
 * @brief Construct continuation structure under GCC compiler.
 * @details Initialize stack frame address of continuation with GCC built-in functions.
 * @param cont: the continuation variable.
 * @see CONTINUATION_CONNECT()
 */
#define CONTINUATION_CONSTRUCT(cont) /* Empty definition for Doxygen */
#undef CONTINUATION_CONSTRUCT

/** @cond */
#if defined(__GCC_HAVE_DWARF2_CFI_ASM) /* Code taken from valgrind.h, under BSD License */ \
    && (((defined(__amd64__) || defined(__x86_64__)) && (defined(__linux__) || defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))) \
       || (defined(__linux__) && defined(__s390__) && defined(__s390x__)))
/* for amd64_linux or amd64_darwin or s390x_linux */
# define CONTINUATION_CONSTRUCT(cont) \
  do { \
    (cont)->stack_frame_addr = (char *)__builtin_dwarf_cfa(); \
  } while (0)
#elif defined(__ppc__) || defined(__ppc64__) /* Code taken from thread_stack_pcs.c in apple's libc */
# define CONTINUATION_CONSTRUCT(cont) \
  do { \
    volatile void *no_omit_frame_pointer; \
    /* __builtin_frame_address IS BROKEN IN BEAKER: RADAR #2340421 */ \
    __asm__ volatile("mr %0, r1" : "=r" (no_omit_frame_pointer)); \
    /* back up the stack pointer up over the current stack frame */ \
    (cont)->stack_frame_addr = *(void **)no_omit_frame_pointer; \
  } while (0)
#else
# define CONTINUATION_CONSTRUCT(cont) \
  do { \
    (cont)->stack_frame_addr = (char *)__builtin_frame_address(0); \
  } while (0)
#endif
/** @endcond */

#if __GNUC__ > 2 || __GNUC_MINOR__ >= 9
# if !defined(CONTINUATION_USE_C99_VLA) && !defined(__STRICT_ANSI__)
/**
 * @brief Enable C99 VLA support by default.
 */
#   define CONTINUATION_USE_C99_VLA 1
# endif
#endif

#ifndef __MINGW32__
# if !CONTINUATION_USE_C99_VLA
#   if !defined(CONTINUATION_USE_ALLOCA)
/**
 * @brief Enable alloca() on non-windows platforms.
 */
#     define CONTINUATION_USE_ALLOCA 1
#   endif
# endif
#endif /* __MINGW32__ */

#endif /* __CONTINUATION_COMPILER_GCC_H */
