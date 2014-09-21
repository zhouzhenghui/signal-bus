/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_H
#define __CONTINUATION_H

#include "continuation_base.h"

#ifdef CONTINUATION_DEBUG
# include <stdio.h>
#endif

#if HAVE_MEMORY_H
# include <memory.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/logical.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <preprocessor/remove_parens.h>
#include <preprocessor/variadic_size_or_zero.h>
#include <static_assert/static_assert.h>

#define CONTINUATION_CONNECT(cont_ptr, cont_stub, initialization, continuation) \
do { \
  continuation_init(cont_ptr, &cont_stub); \
  CONTINUATION_CONSTRUCT(cont_ptr); \
  continuation_stub_init((struct __ContinuationStub *)cont_stub, cont_ptr); \
  /* the stack frame should contain variable cont_stub at least */ \
  ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr = (char *)&cont_stub + sizeof(cont_stub); \
  ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = 0; \
  { \
     __PP_REMOVE_PARENS(initialization); \
    CONTINUATION_STUB_ENTRY(cont_stub); \
    if (!((struct __ContinuationStub *)cont_stub)->cont->initialized) { \
      CONTINUATION_INIT_INVOKE((struct __ContinuationStub *)cont_stub, &cont_stub); \
    } else { \
      { \
        size_t volatile anti_optimize = (size_t)&cont_stub; \
        ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = anti_optimize - (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_spot; \
        assert((((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + sizeof(cont_stub)) \
                || (((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset != 0)) \
            && "[CONTINUATION FAULT] incompatible compilation/optimization in address operator or stack frame pointer"); \
        assert(((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + sizeof(cont_stub)) \
            && "[CONTINUATION FAULT] the size of continuation stack frame isn't enough, probably caused by an incompatible compilation/optimization"); \
        assert(((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + ((size_t)&anti_optimize - anti_optimize) + sizeof(anti_optimize)) \
            && "[CONTINUATION FAULT] the size of continuation stack frame isn't enough, try to increase it"); \
        assert(((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr - ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset \
                >= ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail + ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_size); \
        ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_tail = ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail \
                                                                    + ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset; \
      } \
      { \
        __PP_REMOVE_PARENS(continuation); \
      } \
      CONTINUATION_RETURN((struct __ContinuationStub *)cont_stub); \
    } \
    /* assert(((struct __ContinuationStub *)cont_stub)->cont == cont_ptr); */ \
    ((struct __ContinuationStub *)cont_stub)->cont->initialized = 1; \
    ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = 0; \
    ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_tail = ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail; \
  } \
} while (0)

#define CONTINUATION_RETURN(cont_stub) \
  longjmp((cont_stub)->return_buf, 1)

#define CONTINUATION_IS_INITIALIZED(cont) ((cont)->initialized)

/* with the dynamic stack allocation
 * we can set the stack frame size of a continuation at runtime.
 */
#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE) \
  || CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
# define CONTINUATION_SET_FRAME_SIZE(cont, size) \
do { \
  assert(!CONTINUATION_IS_INITIALIZED(cont) && "XXX_SET_FRAME_SIZE is only available in initialization"); \
  (cont)->stack_frame_size = size; \
} while (0)
# define CONTINUATION_SET_STACK_FRAME_SIZE(cont) CONTINUATION_SET_FRAME_SIZE(cont)
#endif

/* a macro help to get the stack frame size of a continuation */
#define CONTINUATION_GET_FRAME_SIZE(cont) \
  ((void)assert(CONTINUATION_IS_INITIALIZED(cont) && "XXX_GET_FRAME_SIZE is only available after initialized") \
    , (cont)->stack_frame_size)
#define CONTINUATION_GET_STACK_FRAME_SIZE(cont) CONTINUATION_GET_FRAME_SIZE(cont)

#define CONTINUATION_GET_HOST_FRAME(cont) \
  ((void)assert(CONTINUATION_IS_INITIALIZED(cont) && "XXX_GET_FRAME_SIZE is only available after initialized") \
    , (cont)->stack_frame_tail)
#define CONTINUATION_GET_HOST_STACK_FRAME(cont) CONTINUATION_GET_HOST_FRAME(cont)

#define CONTINUATION_GET_FRAME_OFFSET(cont_stub) \
  ((cont_stub)->size.stack_frame_offset)
#define CONTINUATION_GET_STACK_FRAME_OFFSET(cont_stub) CONTINUATION_GET_FRAME_OFFSET(cont_stub)

#define CONTINUATION_GET_FRAME(cont_stub) \
  ((cont_stub)->addr.stack_frame_tail)
#define CONTINUATION_GET_STACK_FRAME(cont_stub) CONTINUATION_GET_FRAME(cont_stub)

#define CONTINUATION_RESERVE_FRAME_ADDR(cont_stub, a) \
do { \
  assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_FRAME_ADDR is only available in initialization"); \
  __continuation_reserve_frame_addr(cont_stub, a); \
} while (0)
inline static void __continuation_reserve_frame_addr(struct __ContinuationStub *cont_stub, void *addr)
{
  char * volatile anti_optimize = (char *)addr;
  if (cont_stub->addr.stack_frame_addr < anti_optimize) {
    cont_stub->addr.stack_frame_addr = anti_optimize;
  }
}

#define __CONTINUATION_RESERVE_VAR(cont_stub, v) \
  __continuation_reserve_frame_addr(cont_stub, (char *)&v + sizeof(v))

#define CONTINUATION_RESERVE_VAR(cont_stub, v) \
  do { \
    assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_VAR is only available in initialization"); \
    __CONTINUATION_RESERVE_VAR(cont_stub, v); \
  } while (0)

#define __CONTINUATION_RESERVE_SEQ_ELEM(r, data, elem) \
  __CONTINUATION_RESERVE_VAR(data, elem);

#define CONTINUATION_RESERVE_VARS_N(cont_stub, n, tuple) \
  do { \
    assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_VARS is only available in initialization"); \
    BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_RESERVE_SEQ_ELEM, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CONTINUATION_RESERVE_VARS(cont_stub, ...) CONTINUATION_RESERVE_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_RESERVE_VARS CONTINUATION_RESERVE_VARS_N
#endif

#define CONTINUATION_RESERVE_VARS0(cont_stub) CONTINUATION_RESERVE_VARS_N(cont_stub, 0, ())
#define CONTINUATION_RESERVE_VARS1(cont_stub, v1) CONTINUATION_RESERVE_VARS_N(cont_stub, 1, (v1))
#define CONTINUATION_RESERVE_VARS2(cont_stub, v1, v2) CONTINUATION_RESERVE_VARS_N(cont_stub, 2, (v1, v2))
#define CONTINUATION_RESERVE_VARS3(cont_stub, v1, v2, v3) CONTINUATION_RESERVE_VARS_N(cont_stub, 3, (v1, v2, v3))
#define CONTINUATION_RESERVE_VARS4(cont_stub, v1, v2, v3, v4) CONTINUATION_RESERVE_VARS_N(cont_stub, 4, (v1, v2, v3, v4))
#define CONTINUATION_RESERVE_VARS5(cont_stub, v1, v2, v3, v4, v5) CONTINUATION_RESERVE_VARS_N(cont_stub, 5, (v1, v2, v3, v4, v5))
#define CONTINUATION_RESERVE_VARS6(cont_stub, v1, v2, v3, v4, v5, v6) CONTINUATION_RESERVE_VARS_N(cont_stub, 6, (v1, v2, v3, v4, v5, v6))
#define CONTINUATION_RESERVE_VARS7(cont_stub, v1, v2, v3, v4, v5, v6, v7) CONTINUATION_RESERVE_VARS_N(cont_stub, 7, (v1, v2, v3, v4, v5, v6, v7))
#define CONTINUATION_RESERVE_VARS8(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8) CONTINUATION_RESERVE_VARS_N(cont_stub, 8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CONTINUATION_RESERVE_VARS9(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8, v9) CONTINUATION_RESERVE_VARS_N(cont_stub, 9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

inline static int __continuation_variable_in_stack_frame(const struct __ContinuationStub *cont_stub, const volatile void * volatile addr, size_t size)
{
  return ((size_t)addr + size <= (size_t)cont_stub->addr.stack_frame_tail + cont_stub->cont->stack_frame_size
           && (size_t)addr >= (size_t)cont_stub->addr.stack_frame_tail);
}

#ifdef CONTINUATION_DEBUG
# define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    (void)printf("[CONTINUATION_DEBUG] The variable \"%s\" has an offset of %d in %d bytes stack frame. at: file \"%s\", line %d\n" \
            , BOOST_PP_STRINGIZE(v) \
            , (char *)(&v) - (cont_stub)->addr.stack_frame_tail, (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__) \
    , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, (void *)&v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) without compiler specified implementation" BOOST_PP_RPAREN()) \
  )
#else
# define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, (void *)&v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) without compiler specified implementation" BOOST_PP_RPAREN()) \
  )
#endif

#define CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    assert((cont_stub)->cont->initialized && "XXX_ASSERT_VAR is only available after initialized")\
    , __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  )

#define __CONTINUATION_ASSERT_SEQ_ELEM(r, data, elem) \
  __CONTINUATION_ASSERT_VAR(data, elem);

#define CONTINUATION_ASSERT_VARS_N(cont_stub, n, tuple) \
  ( \
    assert((cont_stub)->cont->initialized && "XXX_ASSERT_VARS is only available after initialized")\
    , BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_ASSERT_VAR, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  ) \

#if BOOST_PP_VARIADICS
# define CONTINUATION_ASSERT_VARS(cont_stub, ...) CONTINUATION_ASSERT_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_ASSERT_VARS CONTINUATION_ASSERT_VARS_N
#endif

#define CONTINUATION_ASSERT_VARS0(cont_stub) CONTINUATION_ASSERT_VARS_N(cont_stub, 0, ())
#define CONTINUATION_ASSERT_VARS1(cont_stub, v1) CONTINUATION_ASSERT_VARS_N(cont_stub, 1, (v1))
#define CONTINUATION_ASSERT_VARS2(cont_stub, v1, v2) CONTINUATION_ASSERT_VARS_N(cont_stub, 2, (v1, v2))
#define CONTINUATION_ASSERT_VARS3(cont_stub, v1, v2, v3) CONTINUATION_ASSERT_VARS_N(cont_stub, 3, (v1, v2, v3))
#define CONTINUATION_ASSERT_VARS4(cont_stub, v1, v2, v3, v4) CONTINUATION_ASSERT_VARS_N(cont_stub, 4, (v1, v2, v3, v4))
#define CONTINUATION_ASSERT_VARS5(cont_stub, v1, v2, v3, v4, v5) CONTINUATION_ASSERT_VARS_N(cont_stub, 5, (v1, v2, v3, v4, v5))
#define CONTINUATION_ASSERT_VARS6(cont_stub, v1, v2, v3, v4, v5, v6) CONTINUATION_ASSERT_VARS_N(cont_stub, 6, (v1, v2, v3, v4, v5, v6))
#define CONTINUATION_ASSERT_VARS7(cont_stub, v1, v2, v3, v4, v5, v6, v7) CONTINUATION_ASSERT_VARS_N(cont_stub, 7, (v1, v2, v3, v4, v5, v6, v7))
#define CONTINUATION_ASSERT_VARS8(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8) CONTINUATION_ASSERT_VARS_N(cont_stub, 8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CONTINUATION_ASSERT_VARS9(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8, v9) CONTINUATION_ASSERT_VARS_Ncont_stub, (9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CONTINUATION_ENFORCE_VAR(cont_stub, v) \
do { \
  if (!(cont_stub)->cont->initialized) { \
    __CONTINUATION_RESERVE_VAR(cont_stub, v); \
  } else { \
    __CONTINUATION_ASSERT_VAR(cont_stub, v); \
  } \
  if (__continuation_enforce_var) __continuation_enforce_var((void *)&v); \
} while (0)

#define __CONTINUATION_ENFORCE_SEQ_ELEM(r, data, elem) \
  CONTINUATION_ENFORCE_VAR(data, elem);

#define CONTINUATION_ENFORCE_VARS_N(cont_stub, n, tuple) \
  do { \
    BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_ENFORCE_SEQ_ELEM, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CONTINUATION_ENFORCE_VARS(cont_stub, ...) CONTINUATION_ENFORCE_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_ENFORCE_VARS CONTINUATION_ENFORCE_VARS_N
#endif

#define CONTINUATION_ENFORCE_VARS0(cont_stub) CONTINUATION_ENFORCE_VARS_N(cont_stub, 0, ())
#define CONTINUATION_ENFORCE_VARS1(cont_stub, v1) CONTINUATION_ENFORCE_VARS_N(cont_stub, 1, (v1))
#define CONTINUATION_ENFORCE_VARS2(cont_stub, v1, v2) CONTINUATION_ENFORCE_VARS_N(cont_stub, 2, (v1, v2))
#define CONTINUATION_ENFORCE_VARS3(cont_stub, v1, v2, v3) CONTINUATION_ENFORCE_VARS_N(cont_stub, 3, (v1, v2, v3))
#define CONTINUATION_ENFORCE_VARS4(cont_stub, v1, v2, v3, v4) CONTINUATION_ENFORCE_VARS_N(cont_stub, 4, (v1, v2, v3, v4))
#define CONTINUATION_ENFORCE_VARS5(cont_stub, v1, v2, v3, v4, v5) CONTINUATION_ENFORCE_VARS_N(cont_stub, 5, (v1, v2, v3, v4, v5))
#define CONTINUATION_ENFORCE_VARS6(cont_stub, v1, v2, v3, v4, v5, v6) CONTINUATION_ENFORCE_VARS_N(cont_stub, 6, (v1, v2, v3, v4, v5, v6))
#define CONTINUATION_ENFORCE_VARS7(cont_stub, v1, v2, v3, v4, v5, v6, v7) CONTINUATION_ENFORCE_VARS_N(cont_stub, 7, (v1, v2, v3, v4, v5, v6, v7))
#define CONTINUATION_ENFORCE_VARS8(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8) CONTINUATION_ENFORCE_VARS_N(cont_stub, 8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CONTINUATION_ENFORCE_VARS9(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8, v9) CONTINUATION_ENFORCE_VARS_N(cont_stub, 9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#if defined(__GNUC__)
# define CONTINUATION_HOST_VAR_ADDR(cont_stub, v) \
  ((__typeof__(v) *) \
    ((void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (!(cont_stub)->cont->initialized || __continuation_variable_in_stack_frame(cont_stub, (void *)&v, sizeof(v))) \
         && "The variable " BOOST_PP_STRINGIZE(v) " is outside of stack frame" BOOST_PP_RPAREN()) \
   , (size_t)&v - (cont_stub)->size.stack_frame_offset) \
  )
#else
# define CONTINUATION_HOST_VAR_ADDR(cont_stub, v) \
  ((void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (!(cont_stub)->cont->initialized || __continuation_variable_in_stack_frame(cont_stub, (void *)&v, sizeof(v))) \
        && "The variable " BOOST_PP_STRINGIZE(v) " is outside of stack frame" BOOST_PP_RPAREN()) \
   , 0 ? &v : (size_t)&v - (cont_stub)->size.stack_frame_offset)
#endif

# define CONTINUATION_HOST_VAR(cont_stub, v) \
  (* CONTINUATION_HOST_VAR_ADDR(cont_stub, v))

#define CONTINUATION_ADDR_OFFSET(cont_stub, a) \
  ((void)assert((cont_stub)->cont->initialized && "XXX_ADDR_XXX is only available after initialized") \
    , (void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (size_t)(a) >= (size_t)(cont_stub)->addr.stack_frame_tail \
                    && (size_t)(a) < (size_t)(cont_stub)->cont->stack_frame_addr + (cont_stub)->size.stack_frame_offset \
                    && "The address " BOOST_PP_STRINGIZE(a) " is outside of stack frame" BOOST_PP_RPAREN()) \
    , ((size_t)(a) - (size_t)(cont_stub)->addr.stack_frame_tail))

#define CONTINUATION_VAR_OFFSET(cont_stub, v) \
  ((void)assert((cont_stub)->cont->initialized && "XXX_VAR_XXX is only available after initialized") \
    , (void)__CONTINUATION_ASSERT_VAR(cont_stub, v) \
    , ((size_t)&v - (size_t)(cont_stub)->addr.stack_frame_tail))

#define CONTINUATION_BACKUP_STACK_FRAME(cont, stack_frame) \
do { \
  assert((cont)->initialized && "XXX_BACKUP_STACK_FRAME is only available after initialized"); \
  assert((size_t)(stack_frame) < (size_t)(cont)->stack_frame_tail \
          || (size_t)(stack_frame) > (size_t)(cont)->stack_frame_tail + (cont)->stack_frame_size); \
  continuation_backup_stack_frame(cont, stack_frame); \
} while (0)

#define CONTINUATION_RESTORE_STACK_FRAME(cont_stub, stack_frame) \
do { \
  assert(((struct __ContinuationStub *)cont_stub)->cont->initialized && "XXX_RESTORE_STACK_FRAME is only available after initialized"); \
  assert((size_t)(stack_frame) < (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail \
    || (size_t)(stack_frame) > (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail + ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_size); \
  *((void **)&cont_stub) = continuation_restore_stack_frame((const struct __ContinuationStub *)cont_stub, stack_frame); \
} while (0)

#if !defined(CONTINUATION_INIT_INVOKE)
# define __CONTINUATION_ASSERT_CONTINUATION_STACK_FRAME_SIZE
# define __CONTINUATION_DEFINED_STACK_FRAME_SIZE BOOST_PP_NOT(__PP_IS_EMPTY(BOOST_PP_CAT(__CONTINUATION_ASSERT_, CONTINUATION_STACK_FRAME_SIZE)))
# define __CONTINUATION_ASSERT_CONTINUATION_STACK_FRAME_REVERSE
# define __CONTINUATION_DEFINED_STACK_FRAME_REVERSE BOOST_PP_NOT(__PP_IS_EMPTY(BOOST_PP_CAT(__CONTINUATION_ASSERT_, CONTINUATION_STACK_FRAME_REVERSE)))
# define __CONTINUATION_IS_STACK_FRAME_REVERSE BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_REVERSE, CONTINUATION_STACK_FRAME_REVERSE, 0)
# define __CONTINUATION_ASSERT_CONTINUATION_EXTEND_STACK_FRAME
# define __CONTINUATION_DEFINED_EXTEND_STACK_FRAME BOOST_PP_NOT(__PP_IS_EMPTY(BOOST_PP_CAT(__CONTINUATION_ASSERT_, CONTINUATION_EXTEND_STACK_FRAME)))
# ifdef CONTINUATION_DEBUG
#   define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub) \
  printf("[CONTINUATION DEBUG] the continuation has a stack frame of %d bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__)
#   define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub) \
  printf("[CONTINUATION DEBUG] the continuation has a reverse stack frame of indeterminable size, which must be greater than %d bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->offset_to_frame_tail, __FILE__, __LINE__)
# else
#   define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub)
#   define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub)
# endif

#if !defined(CONTINUATION_STACK_FRAME_PADDING)
# define CONTINUATION_STACK_FRAME_PADDING 0
#endif

#ifndef CONTINUATION_USE_C99_VLA
# define CONTINUATION_USE_C99_VLA 0
#endif

#ifndef CONTINUATION_USE_ALLOCA
# define CONTINUATION_USE_ALLOCA 0
#endif

/* should not use any function call in CONTINUATION_INIT_INVOKE or be forced inline */
# define CONTINUATION_INIT_INVOKE(cont_stub, stack_frame_spot_addr) \
do { \
  if ((cont_stub)->cont->stack_frame_addr != NULL && (cont_stub)->cont->invoke != __continuation_init_invoke_stub) { \
    /* frame address specified by CONTINUATION_CONSTRUCT of the compiler config, e.g. gcc's __builtin_frame_address() */ \
    if ((cont_stub)->cont->stack_frame_tail == NULL) { \
      (cont_stub)->cont->stack_frame_tail = (char *)__continuation_init_frame_tail(NULL) - CONTINUATION_STACK_FRAME_PADDING; \
    } \
    (cont_stub)->cont->stack_frame_size = (cont_stub)->cont->stack_frame_addr - (cont_stub)->cont->stack_frame_tail; \
    __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub); \
    assert((cont_stub)->cont->stack_frame_addr >= (cont_stub)->addr.stack_frame_addr \
            && "[CONTINUATION FAULT] the stack frame size/address set by CONTINUATION_CONSTRUCT of the compiler config is not enough for reserved variables/address"); \
    BOOST_PP_EXPR_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
      , if (CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size) { \
          (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
        } else \
    ) \
    { \
      BOOST_PP_IF(BOOST_PP_OR(BOOST_PP_OR(BOOST_PP_OR(CONTINUATION_USE_C99_VLA, CONTINUATION_USE_ALLOCA), __CONTINUATION_DEFINED_EXTEND_STACK_FRAME), __CONTINUATION_IS_STACK_FRAME_REVERSE) \
        , (cont_stub)->cont->invoke = __continuation_dynamic_invoke_stub; \
        , BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
            , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size \
                && "[CONTINUATION FAULT] CONTINUATION_STACK_FRAME_SIZE " BOOST_PP_STRINGIZE(CONTINUATION_STACK_FRAME_SIZE) \
                   " is less than the frame size specified by the compiler config" \
                BOOST_PP_RPAREN()); \
              (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
            , assert(0 && "[CONTINUATION FAULT] must define CONTINUATION_STACK_FRAME_SIZE without dynamic stack frame stretching facility"); \
        ) \
      ) \
    } \
  } else { \
    if ((cont_stub)->cont->stack_frame_addr == NULL) { \
      assert((cont_stub)->cont->stack_frame_tail == NULL && (cont_stub)->cont->invoke == NULL && "[CONTINUATION FAULT] compiler config compatible assertion failed"); \
      (cont_stub)->cont->stack_frame_tail = (char *)__continuation_init_frame_tail(NULL) - CONTINUATION_STACK_FRAME_PADDING; \
      (cont_stub)->cont->offset_to_frame_tail = (cont_stub)->addr.stack_frame_addr - (cont_stub)->cont->stack_frame_tail; \
      (cont_stub)->cont->invoke = __continuation_init_invoke_stub; \
      if (setjmp((cont_stub)->return_buf) == 0) { \
        __continuation_invoke_helper(cont_stub); \
      } \
    } else { \
      __continuation_init_invoke_helper(cont_stub, stack_frame_spot_addr); \
    } \
    if ((size_t)(cont_stub)->cont->stack_frame_tail == (size_t)(cont_stub)->addr.stack_frame_tail - (cont_stub)->size.stack_frame_offset) { \
      __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub); \
      /* reverse stack frame detected */ \
      if ((cont_stub)->cont->stack_frame_size == 0) { \
        BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
          , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->offset_to_frame_tail \
              && "[CONTINUATION FAULT] <reverse stack frame> CONTINUATION_STACK_FRAME_SIZE " BOOST_PP_STRINGIZE(CONTINUATION_STACK_FRAME_SIZE) \
                 " isn't enough please increase it or use  XXX_SET_STACK_FRAME_SIZE(size) in continuation initialization" \
              BOOST_PP_RPAREN()); \
            (cont_stub)->cont->stack_frame_size = CONTINUATION_STACK_FRAME_SIZE; \
            __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub); \
            (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
          , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() 0 \
              && "[CONTINUATION FAULT] <reverse stack frame> please define CONTINUATION_STACK_FRAME_SIZE to an enough value or use XXX_SET_STACK_FRAME_SIZE(size) in continuation initialization" \
              BOOST_PP_RPAREN()); \
        ) \
      } else { \
        __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub); \
        BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (cont_stub)->cont->stack_frame_size >= (cont_stub)->cont->offset_to_frame_tail  \
          && "[CONTINUATION FAULT] <reverse stack frame> the dynamic specified frame size isn't enough, please increase the size parameter of XXX_SET_STACK_FRAME_SIZE" \
          BOOST_PP_RPAREN()); \
        BOOST_PP_EXPR_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
          , if (CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size) { \
              (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
            } else \
        ) \
        { \
          BOOST_PP_IF(BOOST_PP_OR(BOOST_PP_OR(BOOST_PP_OR(CONTINUATION_USE_C99_VLA, CONTINUATION_USE_ALLOCA), __CONTINUATION_DEFINED_EXTEND_STACK_FRAME), __CONTINUATION_IS_STACK_FRAME_REVERSE) \
            , (cont_stub)->cont->invoke = __continuation_dynamic_invoke_stub; \
            , BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_REVERSE /* CONTINUATION_STACK_FRAME_REVERSE == 0 */ \
              , BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
                  , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size \
                      && "[CONTINUATION FAULT] <reverse stack frame> CONTINUATION_STACK_FRAME_SIZE " BOOST_PP_STRINGIZE(CONTINUATION_STACK_FRAME_SIZE) " is less than the frame size specified by the the compiler config" \
                      BOOST_PP_RPAREN()); \
                    (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
                  , assert(0 && "[CONTINUATION FAULT] <reverse stack frame> must define CONTINUATION_STACK_FRAME_SIZE without dynamic stack frame stretching facility"); \
                ) \
              , (cont_stub)->cont->invoke = __continuation_dynamic_invoke_stub; \
            ) \
          ) \
        } \
      } \
    } else { \
      /* assert(BOOST_PP_NOT(__CONTINUATION_IS_STACK_FRAME_REVERSE) \
         && "reverse stack frame compliance test failed, please revert the definition of CONTINUATION_STACK_FRAME_REVERSE/CONTINUATION_NO_FRAME_POINTER"); */ \
      (cont_stub)->cont->stack_frame_size = (cont_stub)->cont->stack_frame_addr - (cont_stub)->cont->stack_frame_tail; \
      __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub); \
      assert((cont_stub)->cont->stack_frame_size >= (cont_stub)->cont->offset_to_frame_tail \
              && "[CONTINUATION FAULT] the deduced stack frame size is not enough for reserved variables/address, may be caused by incompatible compilation which has no frame pointer/use a reserve stack frame"); \
      BOOST_PP_EXPR_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
        , if (CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size) { \
              (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
          } else \
      ) \
      { \
        BOOST_PP_IF(BOOST_PP_OR(BOOST_PP_OR(CONTINUATION_USE_C99_VLA, CONTINUATION_USE_ALLOCA), __CONTINUATION_DEFINED_EXTEND_STACK_FRAME) \
            , (cont_stub)->cont->invoke = __continuation_dynamic_invoke_stub; \
            , BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
                , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() CONTINUATION_STACK_FRAME_SIZE >= (cont_stub)->cont->stack_frame_size \
                    && "[CONTINUATION FAULT] CONTINUATION_STACK_FRAME_SIZE " BOOST_PP_STRINGIZE(CONTINUATION_STACK_FRAME_SIZE) \
                       " is less than the frame size auto deduced" \
                    BOOST_PP_RPAREN()); \
                  (cont_stub)->cont->invoke = __continuation_static_invoke_stub; \
                , assert(0 && "[CONTINUATION FAULT] must define CONTINUATION_STACK_FRAME_SIZE without dynamic stack frame stretching facility"); \
            ) \
        ) \
      } \
    } \
  } \
} while (0)
#endif /* CONTINUATION_INIT_INVOKE */


#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void __continuation_init_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void __continuation_init_invoke_stub(struct __ContinuationStub *cont_stub);
  static void __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub);
  static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub);
  static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub);
#endif

#if !CONTINUATION_USE_LONGJMP
struct __ContinuationStubFrameTail {
  struct __ContinuationStub cont_stub;
  void *frame_tail;
};

static int __continuation_invoke_frame_tail_offset(struct __ContinuationStubFrameTail *arg)
{
  static int(* volatile continuation_invoke_frame_tail_offset)(struct __ContinuationStubFrameTail *) = __continuation_invoke_frame_tail_offset;
  static int frame_tail_offset = -1;

  if (frame_tail_offset != -1) return frame_tail_offset;
  if (arg == NULL) {
    {
      volatile struct __ContinuationStubFrameTail temp;
      volatile struct __Continuation cont;
      arg = (struct __ContinuationStubFrameTail *)&temp;
      arg->cont_stub.cont = (struct __Continuation *)&cont;
    }
    CONTINUATION_CONSTRUCT(arg->cont_stub.cont);
    CONTINUATION_STUB_ENTRY(arg);
    if (arg->frame_tail == NULL) {
      continuation_invoke_frame_tail_offset(arg);
    } else {
      frame_tail_offset = (size_t)arg->frame_tail - (size_t)__continuation_init_frame_tail(NULL);
      CONTINUATION_DESTRUCT(arg->cont_stub.cont);
      longjmp(arg->cont_stub.return_buf, 1);
    }
  } else {
    arg->frame_tail = __continuation_init_frame_tail(NULL);
    if (setjmp(arg->cont_stub.return_buf) == 0) {
      (void)STATIC_ASSERT_OR_ZERO(offsetof(struct __ContinuationStubFrameTail, cont_stub) == 0
          , cont_stub_should_be_first_member_of_struct_ContinuationStubFrameTail);
      CONTINUATION_STUB_INVOKE(&arg->cont_stub);
    }
    return frame_tail_offset;
  }
  assert(frame_tail_offset >= 0);
  return frame_tail_offset;
}
#endif

#if defined(CONTINUATION_STACK_FRAME_SIZE)
static void __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub)
{
  volatile char stack_frame[CONTINUATION_STACK_FRAME_SIZE];
  CONTINUATION_STUB_INVOKE(cont_stub);
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
  assert(0 && "error: don't use keyword 'return' inside continuation");
} /* __continuation_static_invoke_stub */
#endif

#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE) \
  || CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub)
{
#if CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
# if defined(CONTINUATION_DEBUG)
  char *stack_frame_tail = (char *)__continuation_init_frame_tail(NULL);
# endif
#endif
#if CONTINUATION_USE_C99_VLA
  volatile char stack_frame[cont_stub->cont->stack_frame_size];
#elif CONTINUATION_USE_ALLOCA
  volatile char *stack_frame = (char *)alloca(cont_stub->cont->stack_frame_size);
#elif defined(CONTINUATION_EXTEND_STACK_FRAME)
  volatile char *stack_frame;
  CONTINUATION_EXTEND_STACK_FRAME(stack_frame, cont_stub->cont->stack_frame_size);
#elif !defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE
  volatile char stack_frame[CONTINUATION_STACK_BLOCK_SIZE];
  if ((size_t)(cont_stub->addr.stack_frame_addr - &stack_frame[0]) < cont_stub->cont->stack_frame_size) {
    static void(* volatile continuation_recursive_invoke)(struct __ContinuationStub *) = &__continuation_recursive_invoke_stub;
    cont_stub->size.stack_frame_size = cont_stub->cont->stack_frame_size;
    continuation_recursive_invoke(cont_stub);
  }
#else
# error "should not reached"
#endif
#if CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
# if defined(CONTINUATION_DEBUG)
  assert((size_t)stack_frame_tail >= (size_t)__continuation_init_frame_tail(NULL) + cont_stub->cont->stack_frame_size
      && BOOST_PP_IF(CONTINUATION_USE_C99_VLA, "c99 VLA stack frame extend facility compliance test failed"
          , BOOST_PP_IF(CONTINUATION_USE_ALLOCA, "alloca() stack frame extend facility compliance test failed"
            , "compiler specified CONTINUATION_EXTEND_STACK_FRAME facility compliance test failed")));
# endif
#endif
  CONTINUATION_STUB_INVOKE(cont_stub);
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
  assert(0 && "error: don't use keyword 'return' inside continuation");
} /* __continuation_dynamic_invoke_stub */
#endif

#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE)
static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub)
{
  static void(* volatile continuation_recursive_invoke)(struct __ContinuationStub *) = &__continuation_recursive_invoke_stub;
  volatile char stack_frame[CONTINUATION_STACK_BLOCK_SIZE];
  if ((size_t)(cont_stub->addr.stack_frame_addr - &stack_frame[0]) < cont_stub->size.stack_frame_size) {
    if (cont_stub->cont->invoke == __continuation_init_invoke_stub) {
      cont_stub->cont->stack_frame_addr = (char *)&stack_frame[0];
    }
    continuation_recursive_invoke(cont_stub);
  }
  if (cont_stub->cont->invoke == __continuation_init_invoke_stub) {
    cont_stub->addr.stack_frame_tail = (char *)__continuation_init_frame_tail(NULL);
#if !CONTINUATION_USE_LONGJMP
    {
      static int frame_tail_offset = -1;
      if (frame_tail_offset == -1) {
        static int(* volatile continuation_invoke_frame_tail_offset)(struct __ContinuationStubFrameTail *) = __continuation_invoke_frame_tail_offset;
        frame_tail_offset = continuation_invoke_frame_tail_offset(NULL);
        assert(frame_tail_offset > 0);
      }
      cont_stub->addr.stack_frame_tail -= frame_tail_offset;
    }
#endif
  }
  CONTINUATION_STUB_INVOKE(cont_stub);
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
  assert(0 && "error: don't use keyword 'return' inside the continuation");
} /* __continuation_recursive_invoke_stub */
#endif

static void __continuation_init_invoke_stub(struct __ContinuationStub *cont_stub)
{
  static void(* volatile continuation_recursive_invoke)(struct __ContinuationStub *) = &__continuation_recursive_invoke_stub;
/*
#if defined(CONTINUATION_STACK_FRAME_SIZE) && CONTINUATION_STACK_FRAME_SIZE > CONTINUATION_STACK_BLOCK_SIZE
  static const size_t static_frame_size = CONTINUATION_STACK_FRAME_SIZE;
# else
  static const size_t static_frame_size = CONTINUATION_STACK_BLOCK_SIZE;
# endif
  volatile char stack_frame[static_frame_size];
*/
#if CONTINUATION_STACK_FRAME_SIZE
  volatile char stack_frame[CONTINUATION_STACK_FRAME_SIZE];
#else
  volatile char stack_frame[1];
#endif
#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE) \
  || CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
# if CONTINUATION_USE_C99_VLA \
    || CONTINUATION_USE_ALLOCA \
    || defined(CONTINUATION_EXTEND_STACK_FRAME)
  volatile size_t dynamic_frame_size; /* use volatile to anti-optimize the rest calculation */
#   if defined(CONTINUATION_DEBUG)
  char *stack_frame_tail = (char *)__continuation_init_frame_tail(NULL);
#   endif
#   if CONTINUATION_STACK_FRAME_SIZE
  if (cont_stub->cont->stack_frame_size > CONTINUATION_STACK_FRAME_SIZE) {
    dynamic_frame_size = cont_stub->cont->stack_frame_size - CONTINUATION_STACK_FRAME_SIZE;
  } else {
#   endif
    dynamic_frame_size = 1; /* CONTINUATION_STACK_BLOCK_SIZE; */
#   if CONTINUATION_STACK_FRAME_SIZE
  }
#   endif
  {
#   if defined(CONTINUATION_USE_C99_VLA) && CONTINUATION_USE_C99_VLA
    volatile char temp[dynamic_frame_size];
#   elif defined(CONTINUATION_USE_ALLOCA) && CONTINUATION_USE_ALLOCA
    volatile char *temp = (char *)alloca(dynamic_frame_size);
#   elif defined(CONTINUATION_EXTEND_STACK_FRAME)
    volatile char *temp;
    CONTINUATION_EXTEND_STACK_FRAME(temp, dynamic_frame_size);
#   else
#     error "should not reached"
#   endif /* defined(CONTINUATION_USE_C99_VLA) && CONTINUATION_USE_C99_VLA */
#   if defined(CONTINUATION_DEBUG)
    assert((size_t)stack_frame_tail >= (size_t)__continuation_init_frame_tail(NULL) + dynamic_frame_size
      && BOOST_PP_IF(CONTINUATION_USE_C99_VLA, "c99 VLA stack frame extend facility compliance test failed"
          , BOOST_PP_IF(CONTINUATION_USE_ALLOCA, "alloca() stack frame extend facility compliance test failed"
            , "compiler specified CONTINUATION_EXTEND_STACK_FRAME facility compliance test failed")));
#   endif
#   if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE)
    if ((size_t)(cont_stub->addr.stack_frame_addr - &stack_frame[0]) < cont_stub->cont->offset_to_frame_tail) {
      cont_stub->cont->stack_frame_addr = (char *)&temp[0];
      cont_stub->size.stack_frame_size = cont_stub->cont->offset_to_frame_tail;
      continuation_recursive_invoke(cont_stub);
    }
#   endif
# else /* !(CONTINUATION_USE_C99_VLA || CONTINUATION_USE_ALLOCA || defined(CONTINUATION_EXTEND_STACK_FRAME)) */
#   if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE)
  if (cont_stub->cont->offset_to_frame_tail < cont_stub->cont->stack_frame_size) {
    cont_stub->size.stack_frame_size = cont_stub->cont->stack_frame_size;
  } else {
    cont_stub->size.stack_frame_size = cont_stub->cont->offset_to_frame_tail;
  }
  if ((size_t)(cont_stub->addr.stack_frame_addr - &stack_frame[0]) < cont_stub->size.stack_frame_size) {
    cont_stub->cont->stack_frame_addr = (char *)&stack_frame[0];
    continuation_recursive_invoke(cont_stub);
  }
  {
#   else
#     error "should not reached"
#   endif
# endif
#else
  {
#endif
    /* initialize continuation stack frame addr */
    cont_stub->cont->stack_frame_addr = cont_stub->addr.stack_frame_addr;
    cont_stub->addr.stack_frame_tail = (char *)__continuation_init_frame_tail(NULL);
#if !CONTINUATION_USE_LONGJMP
    {
      static int frame_tail_offset = -1;
      if (frame_tail_offset == -1) {
        static int(* volatile continuation_invoke_frame_tail_offset)(struct __ContinuationStubFrameTail *) = __continuation_invoke_frame_tail_offset;
        frame_tail_offset = continuation_invoke_frame_tail_offset(NULL);
        assert(frame_tail_offset > 0);
      }
      cont_stub->addr.stack_frame_tail -= frame_tail_offset;
    }
#endif
    CONTINUATION_STUB_INVOKE(cont_stub);
# if CONTINUATION_USE_C99_VLA \
    || CONTINUATION_USE_ALLOCA \
    || defined(CONTINUATION_EXTEND_STACK_FRAME)
    temp[0] = 0;
# endif
  }
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
  assert(0 && "error: don't use keyword 'return' inside the continuation");
} /* __continuation_init_invoke_stub */

#endif /* __CONTINUATION_H */
