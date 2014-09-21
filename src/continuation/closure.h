/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CLOSURE_H
#define __CLOSURE_H

#include "closure_base.h"
#include "continuation.h"
#include <preprocessor/is_void_cast.h>

/* these variables will pollute the user's namespace */
#define __CLOSURE__ __closure__
#define __CLOSURE_STUB __CLOSURE__.ptr
#define __CLOSURE_PTR  __closure_ptr

#define CLOSURE_INIT(closure_ptr) \
  closure_init(&(closure_ptr)->closure)

#define CLOSURE_CONNECT(closure_ptr, initialization, continuation, finalization) \
do { \
  struct __Closure *__CLOSURE_PTR = &(closure_ptr)->closure; \
  struct { \
    struct __ClosureStub *ptr; \
    struct __ClosureStub var; \
  } __CLOSURE__; \
  __CLOSURE_STUB = &__CLOSURE__.var; \
  (void)STATIC_ASSERT_OR_ZERO(sizeof(*(closure_ptr)) >= CLOSURE_EMPTY_SIZE, wrong_closure_handle_in_CLOSURE_CONNECT); \
  assert(!__CLOSURE_PTR->connected && "closure " #closure_ptr " had been connected"); \
  CONTINUATION_CONNECT(&__CLOSURE_PTR->cont, __CLOSURE_STUB \
    , initialization \
    , ( \
        CONTINUATION_RESTORE_STACK_FRAME(__CLOSURE_STUB, __CLOSURE_STUB->closure->frame); \
        __CLOSURE_PTR = __CLOSURE_STUB->closure; \
        if (!CLOSURE_IS_EMPTY(closure_ptr) && __CLOSURE_PTR != (void *)(closure_ptr)) { \
          memcpy(&(closure_ptr)->arg, (char *)__CLOSURE_PTR + ((size_t)(&(closure_ptr)->arg) - (size_t)(closure_ptr)), sizeof((closure_ptr)->arg)); \
        } \
        if (__CLOSURE_PTR->connected) { \
          __PP_REMOVE_PARENS(continuation); \
          CLOSURE_COMMIT_RETAIN_VARS(); \
        } else { \
          __PP_REMOVE_PARENS(finalization); \
          CONTINUATION_DESTRUCT(&__CLOSURE_PTR->cont); \
        } \
    ) \
  ); \
  __CLOSURE_PTR->frame = (char *)malloc(CLOSURE_GET_STACK_FRAME_SIZE(__CLOSURE_PTR)); \
  CONTINUATION_BACKUP_STACK_FRAME(&__CLOSURE_PTR->cont, __CLOSURE_PTR->frame); \
  __CLOSURE_INIT_VARS(__CLOSURE_PTR); \
  __CLOSURE_PTR->connected = 1; \
} while (0)

/* with the dynamic stack allocation
 * we can set the stack frame size at runtime.
 */
#if defined(CONTINUATION_SET_FRAME_SIZE)
# define CLOSURE_SET_FRAME_SIZE(size) \
  CONTINUATION_SET_FRAME_SIZE(__CLOSURE_STUB->cont_stub.cont, size)
# define CLOSURE_SET_STACK_FRAME_SIZE(size) CLOSURE_SET_FRAME_SIZE(size)
#endif

/* a macro help to get the stack frame size */
#define CLOSURE_GET_FRAME_SIZE(closure_ptr) \
  CONTINUATION_GET_FRAME_SIZE(&(closure_ptr)->cont)
#define CLOSURE_GET_STACK_FRAME_SIZE(closure_ptr) CLOSURE_GET_FRAME_SIZE(closure_ptr)

#define CLOSURE_GET_FRAME_SIZE_OF_THIS() \
  CONTINUATION_GET_FRAME_SIZE(__CLOSURE_STUB->cont_stub.cont)
#define CLOSURE_GET_STACK_FRAME_SIZE_OF_THIS() CLOSURE_GET_FRAME_SIZE_OF_THIS()

#define CLOSURE_GET_HOST_FRAME(closure_ptr) \
  CONTINUATION_GET_HOST_FRAME(&(closure_ptr)->cont)
#define CLOSURE_GET_HOST_STACK_FRAME(closure_ptr) CLOSURE_GET_HOST_FRAME(closure_ptr)

#define CLOSURE_GET_FRAME_OFFSET() \
  CONTINUATION_GET_FRAME_OFFSET(&__CLOSURE_STUB->cont_stub)
#define CLOSURE_GET_STACK_FRAME_OFFSET() CLOSURE_GET_FRAME_OFFSET()

#define CLOSURE_GET_FRAME() \
  CONTINUATION_GET_FRAME(&__CLOSURE_STUB->cont_stub)
#define CLOSURE_GET_STACK_FRAME() CLOSURE_GET_FRAME()

#define CLOSURE_IS_CONNECTED(closure_ptr) \
  (((struct __Closure *)(closure_ptr))->connected)

#define CLOSURE_HOST_VAR_ADDR(a) \
  CONTINUATION_HOST_VAR_ADDR(&__CLOSURE_STUB->cont_stub, a)

#define CLOSURE_HOST_VAR(a) \
  CONTINUATION_HOST_VAR(&__CLOSURE_STUB->cont_stub, a)

#define CLOSURE_ADDR_OFFSET(a) \
  CONTINUATION_ADDR_OFFSET(&__CLOSURE_STUB->cont_stub, a)

#define CLOSURE_ADDR(a) \
  ((void *)((size_t)__CLOSURE_PTR->frame + CLOSURE_ADDR_OFFSET(a)))

#define CLOSURE_VAR_OFFSET(a) \
  CONTINUATION_VAR_OFFSET(&__CLOSURE_STUB->cont_stub, a)

#if defined(__GNUC__)
# define CLOSURE_VAR_ADDR(a) \
  ((__typeof__(a) *)((size_t)__CLOSURE_PTR->frame + CLOSURE_VAR_OFFSET(a)))
#else
# define CLOSURE_VAR_ADDR(a) \
  (0 : &a ? (size_t)__CLOSURE_PTR->frame + CLOSURE_VAR_OFFSET(a))
#endif

#define CLOSURE_VAR(a) \
  (* CLOSURE_VAR_ADDR(a))

#define CLOSURE_RESERVE_FRAME_ADDR(addr) \
  CONTINUATION_RESERVE_FRAME_ADDR(&__CLOSURE_STUB->cont_stub, addr)

#define CLOSURE_RESERVE_VAR(v) \
  CONTINUATION_RESERVE_VAR(&__CLOSURE_STUB->cont_stub, v)

#define CLOSURE_RESERVE_VARS_N(n, tuple) \
  CONTINUATION_RESERVE_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

#if BOOST_PP_VARIADICS
# define CLOSURE_RESERVE_VARS(...) CLOSURE_RESERVE_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RESERVE_VARS CLOSURE_RESERVE_VARS_N
#endif

#define CLOSURE_RESERVE_VARS0() CLOSURE_RESERVE_VARS_N(0, ())
#define CLOSURE_RESERVE_VARS1(v1) CLOSURE_RESERVE_VARS_N(1, (v1))
#define CLOSURE_RESERVE_VARS2(v1, v2) CLOSURE_RESERVE_VARS_N(2, (v1, v2))
#define CLOSURE_RESERVE_VARS3(v1, v2, v3) CLOSURE_RESERVE_VARS_N(3, (v1, v2, v3))
#define CLOSURE_RESERVE_VARS4(v1, v2, v3, v4) CLOSURE_RESERVE_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_RESERVE_VARS5(v1, v2, v3, v4, v5) CLOSURE_RESERVE_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_RESERVE_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_RESERVE_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_RESERVE_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_RESERVE_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_RESERVE_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_RESERVE_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_RESERVE_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_RESERVE_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_ASSERT_VAR(v) \
  CONTINUATION_ASSERT_VAR(&__CLOSURE_STUB->cont_stub, v)

#define CLOSURE_ASSERT_VARS_N(n, tuple) \
  CONTINUATION_ASSERT_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

#if BOOST_PP_VARIADICS
# define CLOSURE_ASSERT_VARS(...) CLOSURE_ASSERT_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_ASSERT_VARS CLOSURE_ASSERT_VARS_N
#endif

#define CLOSURE_ASSERT_VARS0() CLOSURE_ASSERT_VARS_N(0, ())
#define CLOSURE_ASSERT_VARS1(v1) CLOSURE_ASSERT_VARS_N(1, (v1))
#define CLOSURE_ASSERT_VARS2(v1, v2) CLOSURE_ASSERT_VARS_N(2, (v1, v2))
#define CLOSURE_ASSERT_VARS3(v1, v2, v3) CLOSURE_ASSERT_VARS_N(3, (v1, v2, v3))
#define CLOSURE_ASSERT_VARS4(v1, v2, v3, v4) CLOSURE_ASSERT_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_ASSERT_VARS5(v1, v2, v3, v4, v5) CLOSURE_ASSERT_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_ASSERT_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_ASSERT_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_ASSERT_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_ASSERT_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_ASSERT_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_ASSERT_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_ASSERT_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_ASSERT_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_ENFORCE_VAR(v) \
  CONTINUATION_ENFORCE_VAR(&__CLOSURE_STUB->cont_stub, v)

#define CLOSURE_ENFORCE_VARS_N(n, tuple) \
  CONTINUATION_ENFORCE_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

#if BOOST_PP_VARIADICS
# define CLOSURE_ENFORCE_VARS(...) CLOSURE_ENFORCE_VAR_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_ENFORCE_VARS CLOSURE_ENFORCE_VARS_N
#endif

#define CLOSURE_ENFORCE_VARS0() CLOSURE_ENFORCE_VARS_N(0, ())
#define CLOSURE_ENFORCE_VARS1(v1) CLOSURE_ENFORCE_VARS_N(1, (v1))
#define CLOSURE_ENFORCE_VARS2(v1, v2) CLOSURE_ENFORCE_VARS_N(2, (v1, v2))
#define CLOSURE_ENFORCE_VARS3(v1, v2, v3) CLOSURE_ENFORCE_VARS_N(3, (v1, v2, v3))
#define CLOSURE_ENFORCE_VARS4(v1, v2, v3, v4) CLOSURE_ENFORCE_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_ENFORCE_VARS5(v1, v2, v3, v4, v5) CLOSURE_ENFORCE_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_ENFORCE_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_ENFORCE_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_ENFORCE_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_ENFORCE_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_ENFORCE_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_ENFORCE_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_ENFORCE_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_ENFORCE_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_COMMIT_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((char *)__CLOSURE_PTR->frame + CLOSURE_VAR_OFFSET(v), (void *)&v, sizeof(v)); \
  } while (0)

#define __CLOSURE_COMMIT_VAR_SEQ(z, n, seq) \
  CLOSURE_COMMIT_VAR(BOOST_PP_SEQ_ELEM(n, seq))

#define CLOSURE_COMMIT_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_COMMIT_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CLOSURE_COMMIT_VARS(...) CLOSURE_COMMIT_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_COMMIT_VARS CLOSURE_COMMIT_VARS_N
#endif

#define CLOSURE_COMMIT_VARS0() CLOSURE_COMMIT_VARS_N(0, ())
#define CLOSURE_COMMIT_VARS1(v1) CLOSURE_COMMIT_VARS_N(1, (v1))
#define CLOSURE_COMMIT_VARS2(v1, v2) CLOSURE_COMMIT_VARS_N(2, (v1, v2))
#define CLOSURE_COMMIT_VARS3(v1, v2, v3) CLOSURE_COMMIT_VARS_N(3, (v1, v2, v3))
#define CLOSURE_COMMIT_VARS4(v1, v2, v3, v4) CLOSURE_COMMIT_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_COMMIT_VARS5(v1, v2, v3, v4, v5) CLOSURE_COMMIT_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_COMMIT_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_COMMIT_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_COMMIT_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_COMMIT_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_COMMIT_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_COMMIT_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_COMMIT_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_COMMIT_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_UPDATE_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((void *)&v, (char *)__CLOSURE_PTR->frame + CLOSURE_VAR_OFFSET(v), sizeof(v)); \
  } while (0)

#define __CLOSURE_UPDATE_VAR_SEQ(z, n, seq) \
  CLOSURE_UPDATE_VAR(BOOST_PP_SEQ_ELEM(n, seq))

#define CLOSURE_UPDATE_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_UPDATE_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CLOSURE_UPDATE_VARS(...) CLOSURE_UPDATE_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_UPDATE_VARS CLOSURE_UPDATE_VARS_N
#endif

#define CLOSURE_UPDATE_VARS0() CLOSURE_UPDATE_VARS_N(0, ())
#define CLOSURE_UPDATE_VARS1(v1) CLOSURE_UPDATE_VARS_N(1, (v1))
#define CLOSURE_UPDATE_VARS2(v1, v2) CLOSURE_UPDATE_VARS_N(2, (v1, v2))
#define CLOSURE_UPDATE_VARS3(v1, v2, v3) CLOSURE_UPDATE_VARS_N(3, (v1, v2, v3))
#define CLOSURE_UPDATE_VARS4(v1, v2, v3, v4) CLOSURE_UPDATE_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_UPDATE_VARS5(v1, v2, v3, v4, v5) CLOSURE_UPDATE_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_UPDATE_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_UPDATE_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_UPDATE_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_UPDATE_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_UPDATE_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_UPDATE_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_UPDATE_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_UPDATE_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_UPDATE_HOST_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((char *)&v - CLOSURE_GET_FRAME_OFFSET(), (void *)&v, sizeof(v)); \
  } while (0)

#define __CLOSURE_UPDATE_HOST_SEQ(z, n, seq) \
  CLOSURE_UPDATE_HOST_VAR(BOOST_PP_SEQ_ELEM(n, seq))

#define CLOSURE_UPDATE_HOST_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_UPDATE_HOST_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CLOSURE_UPDATE_HOST_VARS(...) CLOSURE_UPDATE_HOST_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_UPDATE_HOST_VARS CLOSURE_UPDATE_HOST_VARS_N
#endif

#define CLOSURE_UPDATE_HOST_VARS0() CLOSURE_UPDATE_HOST_VARS_N(0, ())
#define CLOSURE_UPDATE_HOST_VARS1(v1) CLOSURE_UPDATE_HOST_VARS_N(1, (v1))
#define CLOSURE_UPDATE_HOST_VARS2(v1, v2) CLOSURE_UPDATE_HOST_VARS_N(2, (v1, v2))
#define CLOSURE_UPDATE_HOST_VARS3(v1, v2, v3) CLOSURE_UPDATE_HOST_VARS_N(3, (v1, v2, v3))
#define CLOSURE_UPDATE_HOST_VARS4(v1, v2, v3, v4) CLOSURE_UPDATE_HOST_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_UPDATE_HOST_VARS5(v1, v2, v3, v4, v5) CLOSURE_UPDATE_HOST_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_UPDATE_HOST_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_UPDATE_HOST_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_UPDATE_HOST_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_UPDATE_HOST_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_UPDATE_HOST_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_UPDATE_HOST_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_UPDATE_HOST_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_UPDATE_HOST_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#ifdef CLOSURE_DEBUG
inline static void __closure_var_vector_append_debug(__ClosureVarDebugVector *argv, const char *name, void *addr, size_t size, void *value) {
  struct __ClosureVarDebug temp = { name, addr, size, value };
  VECTOR_APPEND(argv, temp);
}
# define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  do { \
    __closure_var_vector_append_debug(&__CLOSURE_PTR->argv, BOOST_PP_STRINGIZE(arg), (void *)CLOSURE_HOST_VAR_ADDR(arg) \
        , sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (void *)(value))); \
    BOOST_PP_EXPR_IF(BOOST_PP_NOT(__PP_IS_EMPTY(value)) \
        , printf("[CLOSURE_DEBUG] The variable \"%s\" has an offset of %d in %d bytes stack frame. at: file \"%s\", line %d\n" \
                  , BOOST_PP_STRINGIZE(arg), CLOSURE_VAR_OFFSET(arg), CLOSURE_GET_FRAME_SIZE_OF_THIS(), __FILE__, __LINE__); \
    ) \
  } while (0)
#else
inline static void __closure_var_vector_append(__ClosureVarVector *argv, void *addr, size_t size, void *value) {
  struct __ClosureVar temp = { addr, size, value };
  VECTOR_APPEND(argv, temp);
}
# define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  __closure_var_vector_append(&__CLOSURE_PTR->argv, (void *)CLOSURE_HOST_VAR_ADDR(arg), sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (void *)(value)))
#endif

#define CLOSURE_RETAIN_VAR(v) \
do { \
  if (!CONTINUATION_IS_INITIALIZED(&__CLOSURE_PTR->cont)) { \
    CLOSURE_RESERVE_VAR(v); \
    __CLOSURE_VAR_VECTOR_APPEND(v, ); \
  } else { \
    CLOSURE_ASSERT_VAR(v); \
    __CLOSURE_VAR_VECTOR_APPEND(v, CLOSURE_VAR_ADDR(v)); \
  } \
} while (0)

/* macro for binding local variables which or which value may be missing
 *
 * usage: CLOSURE_RETAIN_VARS_N(n, (v1, v2, ..., vn)); // v1 - vn is the local variable name
 *
 * so the statement
 *     CLOSURE_RETAIN_VARS_N(1, (i));
 * means the CLOSURE bind the current value of a local variable i,
 * so you can reference it in CLOSURE code no matter the host function exited or took other affect on it before then.
 *
 * when macro BOOST_PP_VARIADICS defined as 1,
 * a simple and unified interface will be:
 *
 * CLOSURE_RETAIN_VARS(v1, v2, ... vn); // n is any number of parameters less than a max limitation.
 */
#define __CLOSURE_RETAIN_VAR_SEQ(z, n, seq) \
  CLOSURE_RETAIN_VAR(BOOST_PP_SEQ_ELEM(n, seq))

#define CLOSURE_RETAIN_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_RETAIN_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

#if BOOST_PP_VARIADICS
# define CLOSURE_RETAIN_VARS(...) CLOSURE_RETAIN_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RETAIN_VARS CLOSURE_RETAIN_VAR_N
#endif

/* also the simplified macros
 *
 * #define CLOSURE_RETAIN_VARS[n](...) CLOSURE_RETAIN_VARS_N(n, (__VA_ARGS__))
 *
 * example usage: CLOSURE_RETAIN_VARS1(i);
 */
#define CLOSURE_RETAIN_VARS0() CLOSURE_RETAIN_VARS_N(0, ())
#define CLOSURE_RETAIN_VARS1(v1) CLOSURE_RETAIN_VARS_N(1, (v1))
#define CLOSURE_RETAIN_VARS2(v1, v2) CLOSURE_RETAIN_VARS_N(2, (v1, v2))
#define CLOSURE_RETAIN_VARS3(v1, v2, v3) CLOSURE_RETAIN_VARS_N(3, (v1, v2, v3))
#define CLOSURE_RETAIN_VARS4(v1, v2, v3, v4) CLOSURE_RETAIN_VARS_N(4, (v1, v2, v3, v4))
#define CLOSURE_RETAIN_VARS5(v1, v2, v3, v4, v5) CLOSURE_RETAIN_VARS_N(5, (v1, v2, v3, v4, v5))
#define CLOSURE_RETAIN_VARS6(v1, v2, v3, v4, v5, v6) CLOSURE_RETAIN_VARS_N(6, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_RETAIN_VARS7(v1, v2, v3, v4, v5, v6, v7) CLOSURE_RETAIN_VARS_N(7, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_RETAIN_VARS8(v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_RETAIN_VARS_N(8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_RETAIN_VARS9(v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_RETAIN_VARS_N(9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#ifdef CLOSURE_DEBUG
# define __CLOSURE_INIT_VARS(closure_ptr) \
  __closure_init_vars_debug(closure_ptr, &(closure_ptr)->argv, __FILE__, __LINE__)
#else
# define __CLOSURE_INIT_VARS(closure_ptr) \
  __closure_init_vars(closure_ptr, &(closure_ptr)->argv)
#endif

/* 
 * the modification of  retained variables will commited to continuation stack frame automatically
 */
#ifdef CLOSURE_DEBUG
# define CLOSURE_COMMIT_RETAIN_VARS() \
  __closure_commit_vars_debug(&__CLOSURE_PTR->argv, CLOSURE_GET_STACK_FRAME_OFFSET(), __FILE__, __LINE__)
#else
# define CLOSURE_COMMIT_RETAIN_VARS() \
  __closure_commit_vars(&__CLOSURE_PTR->argv, CLOSURE_GET_STACK_FRAME_OFFSET())
#endif


#define CLOSURE_INIT_ARGS_N(n, closure_ptr, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_INIT_ARGS, (closure_ptr, BOOST_PP_TUPLE_TO_SEQ(n, tuple))) \
  } while (0)

#define __CLOSURE_INIT_ARGS(z, n, closure_params) \
  BOOST_PP_CAT((BOOST_PP_TUPLE_ELEM(2, 0, closure_params))->arg._, BOOST_PP_INC(n)) \
  = \
  BOOST_PP_SEQ_ELEM(n, BOOST_PP_TUPLE_ELEM(2, 1, closure_params));

#if BOOST_PP_VARIADICS
# define CLOSURE_INIT_ARGS(closure_ptr, ...) CLOSURE_INIT_ARGS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), closure_ptr, BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_INIT_ARGS CLOSURE_INIT_ARGS_N
#endif

#define CLOSURE_INIT_ARGS1(closure_ptr, v1) CLOSURE_INIT_ARGS_N(1, closure_ptr, (v1))
#define CLOSURE_INIT_ARGS2(closure_ptr, v1, v2) CLOSURE_INIT_ARGS_N(2, closure_ptr, (v1, v2))
#define CLOSURE_INIT_ARGS3(closure_ptr, v1, v2, v3) CLOSURE_INIT_ARGS_N(3, closure_ptr, (v1, v2, v3))
#define CLOSURE_INIT_ARGS4(closure_ptr, v1, v2, v3, v4) CLOSURE_INIT_ARGS_N(4, closure_ptr, (v1, v2, v3, v4))
#define CLOSURE_INIT_ARGS5(closure_ptr, v1, v2, v3, v4, v5) CLOSURE_INIT_ARGS_N(5, closure_ptr, (v1, v2, v3, v4, v5))
#define CLOSURE_INIT_ARGS6(closure_ptr, v1, v2, v3, v4, v5, v6) CLOSURE_INIT_ARGS_N(6, closure_ptr, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_INIT_ARGS7(closure_ptr, v1, v2, v3, v4, v5, v6, v7) CLOSURE_INIT_ARGS_N(7, closure_ptr, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_INIT_ARGS8(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_INIT_ARGS_N(8, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_INIT_ARGS9(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_INIT_ARGS_N(9, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_ARG_OF_(closure_ptr) \
  (&(closure_ptr)->arg)

#define CLOSURE_RUN_N(n, closure_ptr, tuple) \
do { \
  CLOSURE_INIT_ARGS_N(n, closure_ptr, tuple); \
  closure_run(closure_ptr); \
} while (0)

#if BOOST_PP_VARIADICS
# define CLOSURE_RUN(closure_ptr, ...) CLOSURE_RUN_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), closure_ptr, BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RUN CLOSURE_RUN_N
#endif

#define CLOSURE0_RUN(closure_ptr) CLOSURE_RUN_N(0, closure_ptr, ())
#define CLOSURE1_RUN(closure_ptr, v1) CLOSURE_RUN_N(1, closure_ptr, (v1))
#define CLOSURE2_RUN(closure_ptr, v1, v2) CLOSURE_RUN_N(2, closure_ptr, (v1, v2))
#define CLOSURE3_RUN(closure_ptr, v1, v2, v3) CLOSURE_RUN_N(3, closure_ptr, (v1, v2, v3))
#define CLOSURE4_RUN(closure_ptr, v1, v2, v3, v4) CLOSURE_RUN_N(4, closure_ptr, (v1, v2, v3, v4))
#define CLOSURE5_RUN(closure_ptr, v1, v2, v3, v4, v5) CLOSURE_RUN_N(5, closure_ptr, (v1, v2, v3, v4, v5))
#define CLOSURE6_RUN(closure_ptr, v1, v2, v3, v4, v5, v6) CLOSURE_RUN_N(6, closure_ptr, (v1, v2, v3, v4, v5, v6))
#define CLOSURE7_RUN(closure_ptr, v1, v2, v3, v4, v5, v6, v7) CLOSURE_RUN_N(7, closure_ptr, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE8_RUN(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_RUN_N(8, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE9_RUN(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_RUN_N(9, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

#define CLOSURE_FREE(closure_ptr) \
  closure_free(&(closure_ptr)->closure)

#endif /* CLOSURE_H */
