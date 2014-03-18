/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_ARMCC_H
#define __CONTINUATION_COMPILER_ARMCC_H

#ifndef BOOST_PP_VARIADICS
# define BOOST_PP_VARIADICS 1
#endif
#define CONTINUATION_STACK_FRAME_REVERSE 1

#if !defined(CONTINUATION_USE_LONGJMP) || !CONTINUATION_USE_LONGJMP
static void *__continuation_entry(const void *cont_stub, void **func_addr)
{
#if __ARMCC_VERSION < 200000
  __asm {
    MOV (*func_addr), lr
#  ifdef __thumb
    ORR (*func_addr), #1
#  endif
  }
#else
  (*func_addr) = (void *)__return_address();
#endif
  return cont_stub;
}

__inline static void __continuation_dummy() { }

# define CONTINUATION_STUB_ENTRY(cont_stub) \
  do { \
    static void *(* volatile __continuation_entry__)(const void *, void **) = &__continuation_entry; \
    cont_stub = __continuation_entry__(cont_stub, &((struct __ContinuationStub *)cont_stub)->cont->func_addr); \
    __asm { \
      BL __continuation_dummy, {}, {}, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11}; \
    } \
  } while (0)
#endif /* CONTINUATION_USE_LONGJMP */

#endif /* __CONTINUATION_COMPILER_ARMCC_H */
