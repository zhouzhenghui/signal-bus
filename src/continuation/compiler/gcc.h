/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_GCC_H
#define __CONTINUATION_COMPILER_GCC_H

#ifndef BOOST_PP_VARIADICS
# define BOOST_PP_VARIADICS 1
#endif

#include <boost/preprocessor/cat.hpp>

#ifdef __i386__

# if !defined(CONTINUATION_USE_LONGJMP) || !CONTINUATION_USE_LONGJMP
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
# endif /* CONTINUATION_USE_LONGJMP */
#endif /* __i386__ */

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

#if __GNUC__ > 2 || __GNUC_MINOR__ >= 9
# ifndef CONTINUATION_USE_C99_VLA
#   define CONTINUATION_USE_C99_VLA 1
# endif
#endif

#ifndef __MINGW32__
# if !CONTINUATION_USE_C99_VLA
#   if !defined(CONTINUATION_USE_ALLOCA) && !defined(alloca)
#     define alloca __builtin_alloca
#     define CONTINUATION_USE_ALLOCA 1
#   endif
# endif 
#endif /* __MINGW32__ */

#endif /* __CONTINUATION_COMPILER_GCC_H */
