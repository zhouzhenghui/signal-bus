/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CLOSURE_H
#define __CLOSURE_H

/**
 * @defgroup closure closure
 * @brief Closure implementation based on source level continuation.
 * @details A closure is a continuation together with an environment. It performs like a anonymous function
 *  and can be invoked as needed. The environment contains free variables to be referenced when it is called.
 *  Unlike a function, a closure allows a piece of code access those captured variables even it is invoked
 *  outside their scope.
 *
 * @see continuation
 *
 * @{
 */

/**
 * @file
 * @brief The declarations for closure.
 */

#ifdef CLOSURE_DEBUG
# define CONTINUATION_DEBUG
#endif

#include "closure_base.h"
#include "continuation.h"
#include <preprocessor/is_void_cast.h>

/**
 * @name Closure variable names
 * These names will intrude into the user's namespace.
 * @{
 */
/** @brief Name for closure structure within closure. */
#define __CLOSURE__ __closure__
/** @brief Name for pointer to closure structure within closure. */
#define __CLOSURE_PTR  __CLOSURE__.ptr
/** @brief Name for invocation stub within closure. */
#define __CLOSURE_STUB __closure_stub
/** @} */

/** @cond */
/**
 * @name Dummy external variables
 * Dummy declaration of external variable that will be hide if a local variable of same name is defined.
 * @{
 */
extern char __CLOSURE__;
static char __CLOSURE_STUB[1];
STATIC_ASSERT(sizeof(*__CLOSURE_STUB) != sizeof(struct __ClosureStub), internal_constraint_of_variable_CLOSURE_STUB_failed);
/** @} */
/** @endcond */

/** @cond */
#if defined(CLOSURE_DEBUG) && !defined(__cplusplus)
/** @brief Inhibit return in closure */
# define return switch ("return should not be used within a closure"[*__CLOSURE_STUB]) default: return
#endif
/** @endcond */

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE()
 */
#define __CLOSURE_FIELDS(z, n, seq) \
  BOOST_PP_SEQ_ELEM(n, seq) BOOST_PP_CAT(_, BOOST_PP_INC(n));
/** @endcond */

/**
 * @brief Anonymous structure type to declare a closure.
 * @details The anonymous structure has \p n fields representing
 * the parameters that will be passed in when closure is invoked.
 * 
 * @param n: the number of parameters.
 * @param tuple: boost preprocessor tuple contains type of parameters.
 * 
 * @see CLOSURE()
 * @see CLOSURE0(), CLOSURE1(), ..., CLOSUREn()
 * @par Example:
 * @code
 *  CLOSURE_N(1, (int)) closure_int;
 * or
 *  typedef CLOSURE_N(1, (int)) ClosureInt;
 *  ClosureInt closure_int;
 * @endcode
 */
#define CLOSURE_N(n, tuple) \
struct { \
  struct __Closure closure; \
  struct { \
      BOOST_PP_REPEAT(n, __CLOSURE_FIELDS, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
      char end; /* for MSVC compatible */ \
  } arg; \
}

/** @cond */
/**
 * @internal
 * @brief Internal special structure of closure with none of parameters.
 */
struct __ClosureEmpty { struct __Closure closure; struct { char end; } arg; };
/** @endcond */

/**
 * @brief Determine whether the closure structure has parameters.
 * @param closure_ptr: pointer to the closure structure.
 * @see CLOSURE_N()
 * @par Example:
 * @code
 *  CLOSURE() closure_empty;
 *  assert(CLOSURE_IS_EMPTY(&closure_empty));
 * @endcode
 */
#ifdef __GNUC__
# define CLOSURE_IS_EMPTY(closure_ptr) \
  (&((__typeof__((closure_ptr)->arg)*)0)->end == 0)
#else
# define CLOSURE_IS_EMPTY(closure_ptr) \
  ((void *)&(closure_ptr)->arg == (void *)&(closure_ptr)->arg.end)
#endif

/**
 * @copybrief CLOSURE_N()
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_N() otherwise.
 * 
 * @param ...: type of parameters seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_N()
 * @see CLOSURE1(), CLOSURE2(), ..., CLOSUREn()
 * @par Example:
 * @code
 *  CLOSURE(int, int) closure_with_2_param;
 * or
 *  typedef ClOSURE(int, int) ClosureWith2Param;
 *  ClosureWith2Param closure_with_2_param;
 * @endcode
 */
#define CLOSURE() /* Empty definition for Doxygen */
#undef CLOSURE

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE(...) CLOSURE_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE CLOSURE_N
#endif
/** @endcond */

/**
 * @brief Anonymous structure type to declare a closure of a certain number of parameters.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE0(), CLOSURE1(), ..., CLOSURE9().
 * 
 * The number is specified as the suffix.
 * 
 * @see CLOSURE()
 * @see CLOSURE_N()
 * 
 * @par Example:
 * @code
 *  CLOSURE1(int) closure_int;
 * @endcode
 */
#define CLOSUREn() /* Empty definition for Doxygen */
#undef CLOSUREn

#define CLOSURE0() CLOSURE_N(0, ())
#define CLOSURE1(t1) CLOSURE_N(1, (t1))
#define CLOSURE2(t1, t2) CLOSURE_N(2, (t1, t2))
#define CLOSURE3(t1, t2, t3) CLOSURE_N(3, (t1, t2, t3))
#define CLOSURE4(t1, t2, t3, t4) CLOSURE_N(4, (t1, t2, t3, t4))
#define CLOSURE5(t1, t2, t3, t4, t5) CLOSURE_N(5, (t1, t2, t3, t4, t5))
#define CLOSURE6(t1, t2, t3, t4, t5, t6) CLOSURE_N(6, (t1, t2, t3, t4, t5, t6))
#define CLOSURE7(t1, t2, t3, t4, t5, t6, t7) CLOSURE_N(7, (t1, t2, t3, t4, t5, t6, t7))
#define CLOSURE8(t1, t2, t3, t4, t5, t6, t7, t8) CLOSURE_N(8, (t1, t2, t3, t4, t5, t6, t7, t8))
#define CLOSURE9(t1, t2, t3, t4, t5, t6, t7, t8, t9) CLOSURE_N(9, (t1, t2, t3, t4, t5, t6, t7, t8, t9))

/**
 * @brief Initialize a closure to bring it to a consistent state in the life cycle.
 * @details The closure is marked as unconnected.
 * @param closure_ptr: pointer to the closure defined with CLOSURE() or other variants.
 * @note After initialized, a closure can be invoked or freed though nothing will happen until it is connected.
 * 
 * @see CLOSURE()
 * @see CLOSURE_CONNECT()
 */
#define CLOSURE_INIT(closure_ptr) \
  __closure_init(&(closure_ptr)->closure)

/**
 * @brief Alias to CLOSURE_INIT().
 */
#define closure_init(closure_ptr) CLOSURE_INIT(closure_ptr)

/** @cond */
/**
 * @internal
 * @brief Set the value storage of variables retained in initialization block.
 * @details It is an internal help macro to CLOSURE_CONNECT().
 * @see CLOSURE_CONNECT()
 * @see CLOSURE_RETAIN_VARS()
 */
#ifdef CLOSURE_DEBUG
# define __CLOSURE_INIT_VARS(closure_ptr) \
  __closure_init_vars_debug(closure_ptr, &(closure_ptr)->argv, __FILE__, __LINE__)
#else
# define __CLOSURE_INIT_VARS(closure_ptr) \
  __closure_init_vars(closure_ptr, &(closure_ptr)->argv)
#endif
/** @endcond */

/**
 * @internal
 * @brief Commit retained variables so that the last modification of them can been seen in the follwing invcations.
 * @details It is an internal help macro to CLOSURE_CONNECT().
 * @see CLOSURE_CONNECT()
 * @see CLOSURE_RETAIN_VARS()
 */
#ifdef CLOSURE_DEBUG
# define CLOSURE_COMMIT_RETAIN_VARS() \
  __closure_commit_vars_debug(&__CLOSURE_STUB->closure->argv, CLOSURE_GET_STACK_FRAME_OFFSET(), __FILE__, __LINE__)
#else
# define CLOSURE_COMMIT_RETAIN_VARS() \
  __closure_commit_vars(&__CLOSURE_STUB->closure->argv, CLOSURE_GET_STACK_FRAME_OFFSET())
#endif

/**
 * @brief Connect a closure with the execution statements based on continuation.
 * @details The continuation statements are specified in place and will be executed when the closure is connected,
 *  invoked or freed respectively.
 * @param closure_ptr: pointer to the closure.
 * @param initialization: statements to be executed immediately together with the closure.
 * @param continuation: statements to be executed when the closure is invoked.
 * @param finalization: statements to be executed when the closure is disconnected/freed.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE()
 * @see CLOSURE_RUN()
 * @see CLOSURE_FREE()
 * @see CONTINUATION_CONNECT()
 * 
 * @par Example:
 * @code
 *  CLOSURE1(const char *) closure_sayhello;
 *  CLOSURE_CONNECT(&closure_sayhello,
 *    , (
 *      // Preparation within the closure.
 *    )
 *    , (
 *      // Something happen when closure is invoked.
 *      printf("Hello %s!\n", CLOSURE_ARG_OF_(&closure_sayhello)->_1));
 *    )
 *    , (
 *      // Final things such as recycling the occupied resources among the executions of the closure and so on.
 *      printf("Goodbye %s!\n", CLOSURE_ARG_OF_(&closure_sayhello)->_1));
 *    )
 *  );
 *  CLOSURE_RUN(&closure_sayhello, "Closure");
 *  CLOSURE_FREE(&closure_sayhello);
 * @endcode
 */
#define CLOSURE_CONNECT(closure_ptr, initialization, continuation, finalization) \
  switch (0) \
  while (!(closure_ptr)->closure.connected && ((closure_ptr)->closure.connected = 1)) \
  if (0) case 0: { \
    struct { \
      const int has_external_closure_stub; \
      struct __ClosureStub ** CONTINUATION_ATTRIBUTE_MAY_ALIAS external_closure_stub; \
      struct __Closure *ptr; \
      struct __ClosureStub stub; \
    } __CLOSURE__ = { sizeof(*__CLOSURE_STUB) == sizeof(struct __ClosureStub) }; \
    (void)STATIC_ASSERT_OR_ZERO(sizeof(__CLOSURE__) > sizeof(char), internal_contrain_of_variable__CLOSURE__failed); \
    (void)STATIC_ASSERT_OR_ZERO(sizeof(*(closure_ptr)) >= sizeof(struct __ClosureEmpty), wrong_closure_handle_in_CLOSURE_CONNECT); \
    assert(!(closure_ptr)->closure.connected && "closure " #closure_ptr " had been connected"); \
    if (__CLOSURE__.has_external_closure_stub) { \
      __CLOSURE__.external_closure_stub = (struct __ClosureStub **)&__CLOSURE_STUB; \
    } \
    { \
      struct __ClosureStub * CONTINUATION_ATTRIBUTE_MAY_ALIAS __CLOSURE_STUB; \
      __CLOSURE_STUB = &__CLOSURE__.stub; \
      __CLOSURE_STUB->closure = &(closure_ptr)->closure; \
      __CLOSURE_PTR = NULL; \
      VECTOR_INIT(&(closure_ptr)->closure.argv); \
      CONTINUATION_CONNECT(&(closure_ptr)->closure.cont, __CLOSURE_STUB \
        , ( \
            __PP_REMOVE_PARENS(initialization); \
            CLOSURE_RESERVE_VAR(__CLOSURE__); \
            CLOSURE_RESERVE_VAR(__CLOSURE_STUB); \
            if (__CLOSURE__.has_external_closure_stub) { \
              CLOSURE_RESERVE_FRAME_ADDR(__CLOSURE__.external_closure_stub, sizeof(__CLOSURE_STUB)); \
            } \
        ) \
        , ( \
            CONTINUATION_RESTORE_STACK_FRAME(__CLOSURE_STUB, __CLOSURE_STUB->closure->frame); \
            __CLOSURE_PTR = __CLOSURE_STUB->closure; \
            assert(__CLOSURE_STUB->cont_stub.cont == &__CLOSURE_PTR->cont); \
            if (!CLOSURE_IS_EMPTY(closure_ptr) && __CLOSURE_PTR != (void *)(closure_ptr)) { \
              void * volatile anti_optimize = &(closure_ptr)->arg; \
              memcpy(anti_optimize, (char *)__CLOSURE_PTR + ((size_t)(&(closure_ptr)->arg) - (size_t)(closure_ptr)), sizeof((closure_ptr)->arg)); \
            } \
            if (__CLOSURE_PTR->connected) { \
              __CLOSURE_BLOCK( \
                __PP_REMOVE_PARENS(continuation); \
                CLOSURE_COMMIT_RETAIN_VARS(); \
              ); \
            } else { \
              __CLOSURE_BLOCK(finalization); \
            } \
        ) \
      ) { \
        (closure_ptr)->closure.frame = (char *)malloc(CLOSURE_GET_STACK_FRAME_SIZE(&(closure_ptr)->closure)); \
        CONTINUATION_BACKUP_STACK_FRAME(&(closure_ptr)->closure.cont, (closure_ptr)->closure.frame); \
        __CLOSURE_INIT_VARS(&(closure_ptr)->closure); \
        /* (closure_ptr)->closure.connected = 1; */ \
      } \
      if (__CLOSURE_PTR) { \
        if (__CLOSURE__.has_external_closure_stub) { \
          *((struct __ClosureStub **)((char *)__CLOSURE__.external_closure_stub + CLOSURE_GET_FRAME_OFFSET())) \
              = __CLOSURE_STUB; \
        } else { \
          assert(0 && "no __CLOSURE_STUB defined with undelimited closure."); \
        } \
        break; \
      } \
    } \
  } else

/** @cond */
/**
 * @internal
 * @def __CLOSURE_BLOCK()
 * @brief Internal help macro encloses continuation statements for CLOSURE_CONNECT().
 * @details Use C++ exception handle for return from middle of continuation.
 * @param continuation: the continuation statements of the closure.
 */
#ifdef __cplusplus
  struct __ClosureException {};
  inline void __closure_return_by_throw() {
    throw __ClosureException();
  }
# define __CLOSURE_BLOCK(continuation) \
    try { \
      __PP_REMOVE_PARENS(continuation); \
    } catch (__ClosureException &) {}
#else
# define __CLOSURE_BLOCK(continuation) \
    { \
      __PP_REMOVE_PARENS(continuation); \
    }
#endif
/** @endcond */

/**
 * @brief Return from a closure to the caller.
 * @details The execution of the closure continuation is terminated.
 * @warning Don't or be careful to use it in C++ with undelimited continuation for the unsafety longjmp().
 * @see CLOSURE_CONNECT()
 */
#ifdef __cplusplus
# define CLOSURE_RETURN() \
    (sizeof(__CLOSURE__) > sizeof(char) ? (CLOSURE_COMMIT_RETAIN_VARS(), __closure_return_by_throw()) \
                                        : (CLOSURE_COMMIT_RETAIN_VARS(), CONTINUATION_STUB_RETURN(&__CLOSURE_STUB->cont_stub)))
#else
# define CLOSURE_RETURN() \
   (CLOSURE_COMMIT_RETAIN_VARS(), CONTINUATION_RETURN(__CLOSURE_STUB))
#endif

/** @cond */
/**
 * @internal
 * @brief Return from a closure to the caller without retaining the variables.
 * @warning Changes of local variables, include those to be retained, are discarded.
 * @see CLOSURE_CONNECT()
 * @see CLOSURE_RETURN()
 * @see CLOSURE_RETAIN_VAR()
 */
#ifdef __cplusplus
# define CLOSURE_RETURN_NO_RETAIN() \
     (sizeof(__CLOSURE__) > sizeof(char) ? CONTINUATION_RETURN(&__CLOSURE_STUB->cont_stub) \
                                         : CONTINUATION_STUB_RETURN(&__CLOSURE_STUB->cont_stub))
#else
# define CLOSURE_RETURN_NO_RETAIN() \
    CONTINUATION_RETURN(&__CLOSURE_STUB->cont_stub)
#endif
/** @endcond */

/**
 * @brief Language keyword-like macro interface of closure connection when C99/C++ loop initial declartion is available.
 * @details It takes a followed clause as its continuation and an else clause as the finalization.
 * @param closure_ptr: pointer to the closure.
 * @see CLOSURE_CONNECT()
 * @par Example:
 * @code
 *  CLOSURE(const char *) closure_sayhello;
 *  closure_init(&closure_sayhello);
 *  closure_if (&closure_sayhello) {
 *    // Something happen when closure is invoked.
 *    printf("Hello %s!\n", CLOSURE_ARG_OF_(&closure_sayhello)->_1);
 *  } else {
 *    // Final things such as recycling the occupied resources among the executions of the closure and so on.
 *    printf("Goodbye %s!\n", CLOSURE_ARG_OF_(&closure_sayhello)->_1));
 *  }
 *  closure_run(&closure_sayhello, "Closure");
 *  closure_free(&closure_sayhello);
 * @endcode
 */
#define closure_if(closure_ptr) \
  for (struct __ClosureStub *__CLOSURE_STUB = NULL;;) \
    if (!__CLOSURE_STUB) { \
      CLOSURE_CONNECT(closure_ptr, (), break;, break;); \
      if (!__CLOSURE_STUB) break; \
    } else for (;; CLOSURE_RETURN_NO_RETAIN()) switch(0) default: \
      if (__CLOSURE_STUB->closure->connected) \
        for (;; CLOSURE_RETURN()) switch(0) default:

/**
 * @brief Alias to CLOSURE_RETURN().
 */
#define closure_return() CLOSURE_RETURN()

/**
 * @name Closure state
 * @{
 */
/**
 * @brief Determine whether a closure is connected.
 * @param closure_ptr: pointer to the closure.
 * @see CLOSURE_CONNECT()
 * @see CLOSURE_INIT()
 */
#define CLOSURE_IS_CONNECTED(closure_ptr) \
  (((struct __Closure *)(closure_ptr))->connected)
/** @} */

/**
 * @name Environment evaluation and enforcement 
 * @{
 */
/**
 * @brief Set the size of the execution stack frame of a closure.
 * @details It is a special macro to CONTINUATION_SET_FRAME_SIZE().
 * @param size: size of the stack frame.
 * @see CONTINUATION_SET_FRAME_SIZE()
 */
#define CLOSURE_SET_FRAME_SIZE(size) /* Empty defintion for Doxygen */
#undef CLOSURE_SET_FRAME_SIZE

/** @brief Alias to CLOSURE_SET_FRAME_SIZE(). */
#define CLOSURE_SET_STACK_FRAME_SIZE(size) CLOSURE_SET_FRAME_SIZE(size)
#undef CLOSURE_SET_STACK_FRAME_SIZE

/** @cond */
#if defined(CONTINUATION_SET_FRAME_SIZE)
# define CLOSURE_SET_FRAME_SIZE(size) \
  CONTINUATION_SET_FRAME_SIZE(__CLOSURE_STUB->cont_stub.cont, size)
# define CLOSURE_SET_STACK_FRAME_SIZE(size) CLOSURE_SET_FRAME_SIZE(size)
#endif
/** @endcond */

/**
 * @brief Set the size of parameters space before the stack frame of a closure.
 * @details It is special macro to CONTINUATION_SET_PARAMS_SIZE().
 * @param size: size of the parameters space.
 * @see CONTINUATION_SET_PARAMS_SIZE()
 */ 
#define CLOSURE_SET_PARAMS_SIZE(size) \
  CONTINUATION_SET_PARAMS_SIZE(__CLOSURE_STUB->cont_stub.cont, size)
/** @brief Alias to CLOSURE_SET_PARAMS_SIZE(). */
#define CLOSURE_SET_STACK_PARAMS_SIZE(size) CLOSURE_SET_PARAMS_SIZE(size)

/**
 * @brief Get the size of the execution stack frame of a closure.
 * @details It is a special macro to CONTINUATION_GET_FRAME_SIZE().
 * @param closure_ptr: pointer to the closure.
 * @see CONTINUATION_GET_FRAME_SIZE()
 */
#define CLOSURE_GET_FRAME_SIZE(closure_ptr) \
  CONTINUATION_GET_FRAME_SIZE(&(closure_ptr)->cont)
/** @brief Alias to CLOSURE_GET_FRAME_SIZE(). */
#define CLOSURE_GET_STACK_FRAME_SIZE(closure_ptr) CLOSURE_GET_FRAME_SIZE(closure_ptr)

/**
 * @brief Get the size of the execution stack frame within a closure.
 * @details It is a special macro to CONTINUATION_GET_FRAME_SIZE().
 * @see CONTINUATION_GET_FRAME_SIZE()
 */
#define CLOSURE_GET_FRAME_SIZE_OF_THIS() \
  CONTINUATION_GET_FRAME_SIZE(__CLOSURE_STUB->cont_stub.cont)
/** @brief Alias to CLOSURE_GET_FRAME_SIZE_OF_THIS(). */
#define CLOSURE_GET_STACK_FRAME_SIZE_OF_THIS() CLOSURE_GET_FRAME_SIZE_OF_THIS()

/**
 * @brief Get pointer to the stack frame of the host function within a closure.
 * @details It is a special macro to CONTINUATION_GET_HOST_FRAME().
 * @param closure_ptr: pointer to the closure.
 * @see CONTINUATION_GET_HOST_FRAME().
 */
#define CLOSURE_GET_HOST_FRAME(closure_ptr) \
  CONTINUATION_GET_HOST_FRAME(&(closure_ptr)->cont)
/** @brief Alias to CLOSURE_GET_HOST_FRAME(). */
#define CLOSURE_GET_HOST_STACK_FRAME(closure_ptr) CLOSURE_GET_HOST_FRAME(closure_ptr)

/**
 * @brief Get address offset of the current stack frame to the host function's within a closure.
 * @details It is a special macro to CONTINUATION_GET_FRAME_OFFSET().
 * @see CONTINUATION_GET_FRAME_OFFSET()
 */
#define CLOSURE_GET_FRAME_OFFSET() \
  CONTINUATION_GET_FRAME_OFFSET(&__CLOSURE_STUB->cont_stub)
/** @brief Alias to CLOSURE_GET_FRAME_OFFSET(). */
#define CLOSURE_GET_STACK_FRAME_OFFSET() CLOSURE_GET_FRAME_OFFSET()

/**
 * @brief Get pointer to the current stack frame within a closure.
 * @details It is a special macro to CONTINUATION_GET_FRAME().
 * @see CONTINUATION_GET_FRAME()
 */
#define CLOSURE_GET_FRAME() \
  CONTINUATION_GET_FRAME(&__CLOSURE_STUB->cont_stub)
/** @brief Alias to CLOSURE_GET_FRAME(). */
#define CLOSURE_GET_STACK_FRAME() CLOSURE_GET_FRAME()

/**
 * @brief Reserve a specified memory space in the stack frame.
 * @details It is a special macro to CONTINUATION_RESERVE_FRAME_ADDR().
 * @param addr: address of the memory space be in the stack frame of the host function.
 * @param size: size of the memory space.
 * @see CONTINUATION_RESERVE_FRAME_ADDR()
 */
#define CLOSURE_RESERVE_FRAME_ADDR(addr, size) \
  CONTINUATION_RESERVE_FRAME_ADDR(&__CLOSURE_STUB->cont_stub, addr, size)

/**
 * @brief Reserve the memory space of a local variable in the stack frame.
 * @details It is a special macro to CONTINUATION_RESERVE_VAR().
 * 
 * @param v: name of the variable.
 * 
 * @see CLOSURE_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VAR()
 */
#define CLOSURE_RESERVE_VAR(v) \
  CONTINUATION_RESERVE_VAR(&__CLOSURE_STUB->cont_stub, v)

/**
 * @brief Reserve the memory space of a number of local variables in the stack frame all at once.
 * @details It is a special macro to CONTINUATION_RESERVE_VARS_N().
 * 
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains the variables.
 * 
 * @see CLOSURE_RESERVE_VAR()
 * @see CLOSURE_RESERVE_VARS()
 * @see CLOSURE_RESERVE_VARS1(), CLOSURE_RESERVE_VARS2(), ... CLOSURE_RESERVE_VARSn()
 * @see CONTINUATION_RESERVE_VARS_N()
 */
#define CLOSURE_RESERVE_VARS_N(n, tuple) \
  CONTINUATION_RESERVE_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

/**
 * @copybrief CLOSURE_RESERVE_VARS_N().
 * @details If variadic macros are available, the parameters in BOOST tuple preprocessor
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_RESERVE_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_RESERVE_VAR()
 * @see CLOSURE_RESERVE_VARS_N()
 * @see CLOSURE_RESERVE_VARS1(), CLOSURE_RESERVE_VARS2(), ..., CLOSURE_RESERVE_VARSn()
 */
#define CLOSURE_RESERVE_VARS() /* Empty definition of Doxygen */
#undef CLOSURE_RESERVE_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_RESERVE_VARS(...) \
  CLOSURE_RESERVE_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RESERVE_VARS CLOSURE_RESERVE_VARS_N
#endif
/** @endcond */

/**
 * @brief Reserve the memory space of a certain number of local variables in the stack frame all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_RESERVE_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_RESERVE_VARS0(), CLOSURE_RESERVE_VARS1(), ..., CLOSURE_RESERVE_VARS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by commas.
 * 
 * @see CLOSURE_RESERVE_VAR()
 * @see CLOSURE_RESERVE_VARS()
 * @see CLOSURE_RESERVE_VARS_N()
 */
#define CLOSURE_RESERVE_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_RESERVE_VARSn

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

/**
 * @brief Assert a local variable resides in the stack frame.
 * @details it is a special macro to CONTINUATION_ASSERT_VAR().
 * 
 * @param v: name of the variable.
 * 
 * @see CONTINUATION_ASSERT_VAR()
 * @see CLOSURE_ASSERT_VARS()
 */
#define CLOSURE_ASSERT_VAR(v) \
  CONTINUATION_ASSERT_VAR(&__CLOSURE_STUB->cont_stub, v)

/**
 * @brief Assert a number of local variables reside in the stack frame all at once.
 * @details It is a special macro to CONTINUATION_ASSERT_VAR_N().
 * 
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains those variables.
 * 
 * @see CONTINUATION_ASSERT_VAR()
 * @see CLOSURE_ASSERT_VAR()
 * @see CLOSURE_ASSERT_VARS()
 * @see CLOSURE_ASSERT_VARS1(), CLOSURE_ASSERT_VARS2(), ..., CLOSURE_ASSERT_VARSn()
 */
#define CLOSURE_ASSERT_VARS_N(n, tuple) \
  CONTINUATION_ASSERT_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

/**
 * @copybrief CLOSURE_ASSERT_VARS_N()
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_ASSERT_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_ASSERT_VAR()
 * @see CLOSURE_ASSERT_VARS_N()
 * @see CLOSURE_ASSERT_VARS1(), CLOSURE_ASSERT_VARS2(), ..., CLOSURE_ASSERT_VARSn()
 */
#define CLOSURE_ASSERT_VARS() /* Empty definition for Doxygen */
#undef CLOSURE_ASSERT_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_ASSERT_VARS(...) CLOSURE_ASSERT_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_ASSERT_VARS CLOSURE_ASSERT_VARS_N
#endif
/** @endcond */

/**
 * @brief Assert a certain number of local variables reside in the stack frame all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_ASSERT_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_ASSERT_VARS0(), CLOSURE_ASSERT_VARS1(), ..., CLOSURE_ASSERT_VARS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by comma.
 * 
 * @see CLOSURE_ASSERT_VAR()
 * @see CLOSURE_ASSERT_VARS()
 * @see CLOSURE_ASSERT_VARS_N()
 */
#define CLOSURE_ASSERT_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_ASSERT_VARSn

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

/**
 * @brief Enforce a local variable resides in the stack frame.
 * @details It is a special macro to CONTINUATION_ENFORCE_VAR().
 * 
 * @param v: name of the variable.
 * 
 * @see CLOSURE_ENFORCE_VARS()
 * @see CONTINUATION_ENFORCE_VAR()
 */
#define CLOSURE_ENFORCE_VAR(v) \
  CONTINUATION_ENFORCE_VAR(&__CLOSURE_STUB->cont_stub, v)

/**
 * @brief Enforce a number of local variables reside in the stack frame all at once.
 * @details It is a special macro to CONTINUATION_ENFORCE_VARS_N().
 * 
 * @param n: the variable names.
 * @param tuple: BOOST preprocessor tuple that contains variable names.
 * 
 * @see CLOSURE_ENFORCE_VAR()
 * @see CLOSURE_ENFORCE_VARS()
 * @see CLOSURE_ENFORCE_VARS1(), CLOSURE_ENFORCE_VARS2(), ..., CLOSURE_ENFORCE_VARSn()
 */
#define CLOSURE_ENFORCE_VARS_N(n, tuple) \
  CONTINUATION_ENFORCE_VARS_N(&__CLOSURE_STUB->cont_stub, n, tuple)

/**
 * @copybrief CLOSURE_ENFORCE_VARS_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_ENFORCE_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_ENFORCE_VAR()
 * @see CLOSURE_ENFORCE_VARS_N()
 * @see CLOSURE_ENFORCE_VARS1(), CLOSURE_ENFORCE_VARS2(), ..., CLOSURE_ENFORCE_VARSn()
 */
#define CLOSURE_ENFORCE_VARS() /* Empty definition for Doxygen */
#undef CLOSURE_ENFORCE_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_ENFORCE_VARS(...) CLOSURE_ENFORCE_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_ENFORCE_VARS CLOSURE_ENFORCE_VARS_N
#endif
/** @endcond */

/**
 * @brief Enforce a certain number of local variables reside in the stack frame all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_ENFORCE_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_ENFORCE_VARS0(), CLOSURE_ENFORCE_VARS1(), ..., CLOSURE_ENFORCE_VARS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by commas.
 * 
 * @see CLOSURE_ENFORCE_VAR()
 * @see CLOSURE_ENFORCE_VARS()
 * @see CLOSURE_ENFORCE_VARS_N()
 */
#define CLOSURE_ENFORCE_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_ENFORCE_VARSn

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
/** @} Environment evaluation and enforcement */

/**
 * @name Variable evaluation
 * @{
 */
/**
 * @brief Get pointer to a local variable in the host function within a closure.
 * @details It is special macro to CONTINUATION_HOST_VAR_ADDR().
 * @param a: name of the variable.
 * @see CLOSURE_HOST_VAR()
 */
#define CLOSURE_HOST_VAR_ADDR(a) \
  CONTINUATION_HOST_VAR_ADDR(&__CLOSURE_STUB->cont_stub, a)

/**
 * @brief Get reference of a local variable of the host function within a closure.
 * @details It is special macro to CONTINUATION_HOST_VAR().
 * @param v: name of the variable.
 * @see CLOSURE_HOST_VAR_ADDR()
 */
#define CLOSURE_HOST_VAR(v) \
  CONTINUATION_HOST_VAR(&__CLOSURE_STUB->cont_stub, v)

/**
 * @brief Get the offset of an address in the stack frame within a closure.
 * @details It is a special macro to CONTINUATION_ADDR_OFFSET().
 * @param a: maybe the address of a local variable.
 * @see CLOSURE_ADDR()
 */ 
#define CLOSURE_ADDR_OFFSET(a) \
  CONTINUATION_ADDR_OFFSET(&__CLOSURE_STUB->cont_stub, a)

/**
 * @brief Get the address in the backup frame storage that corresponds to an address
 * in the stack frame within a closure.
 * @param a: maybe the address of a local variable.
 * @note The address is unique in the life cycle of a closure.
 * @see CLOSURE_VAR_ADDR()
 * @see CLOSURE_ADDR_OFFSET()
 */
#define CLOSURE_ADDR(a) \
  ((void *)((size_t)__CLOSURE_STUB->closure->frame + CLOSURE_ADDR_OFFSET(a)))

/**
 * @brief Get the offset in the stack frame of a local variable.
 * @details It is a special macro to CONTINUATION_VAR_OFFSET().
 * @param v: name of the variable.
 * @see CLOSURE_VAR_ADDR()
 */
#define CLOSURE_VAR_OFFSET(v) \
  CONTINUATION_VAR_OFFSET(&__CLOSURE_STUB->cont_stub, v)

/**
 * @def CLOSURE_VAR_ADDR(v)
 * @brief Get the address in backup frame storage of a local variables within a closure.
 * @param v: name of the variable.
 * @see CLOSURE_VAR()
 */
#if defined(__GNUC__)
# define CLOSURE_VAR_ADDR(v) \
  ((__typeof__(v) *)((size_t)__CLOSURE_STUB->closure->frame + CLOSURE_VAR_OFFSET(v)))
#else
# define CLOSURE_VAR_ADDR(v) \
  (0 ? &v : (size_t)__CLOSURE_STUB->closure->frame + CLOSURE_VAR_OFFSET(v))
#endif

/**
 * @brief Get the reference in backup frame storage of a local variables within a closure.
 * @param v: name of the variable.
 * @see CLOSURE_VAR_ADDR()
 */
#define CLOSURE_VAR(v) \
  (* CLOSURE_VAR_ADDR(v))

/**
 * @brief Commit the value of a local variable so that it can be seen in the following invocation.
 * 
 * @details The modification of a local variable in continuation block will be discarded unless be
 * commited by the macro explicitly or be automatically commited at the end of each invocation
 * as the variable is retained by CLOSURE_RETAIN_VAR() or CLOSURE_RETAIN_VARS().
 * 
 * @param v: name of the variable.
 * 
 * @see CLOSURE_RETAIN_VAR(), CLOSURE_RETAIN_VARS()
 * @see CLOSURE_COMMIT_VARS()
 */
#define CLOSURE_COMMIT_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((char *)__CLOSURE_STUB->closure->frame + CLOSURE_VAR_OFFSET(v), (char *)&v, sizeof(v)); \
  } while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE_COMMIT_VARS_N()
 * @see CLOSURE_COMMIT_VARS_N()
 */
#define __CLOSURE_COMMIT_VAR_SEQ(z, n, seq) \
  CLOSURE_COMMIT_VAR(BOOST_PP_SEQ_ELEM(n, seq))
/** @endcond */

/**
 * @brief Commit the modification of a number of variables all at once.
 * 
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains variable names.
 * 
 * @see CLOSURE_COMMIT_VAR()
 * @see CLOSURE_COMMIT_VARS()
 * @see CLOSURE_COMMIT_VARS1(), CLOSURE_COMMIT_VARS2(), ..., CLOSURE_COMMIT_VARSn()
 */
#define CLOSURE_COMMIT_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_COMMIT_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CLOSURE_COMMIT_VARS_N()
 *
 * @details If variadic macros is available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_COMMIT_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_COMMIT_VAR()
 * @see CLOSURE_COMMIT_VARS_N()
 * @see CLOSURE_COMMIT_VARS1(), CLOSURE_COMMIT_VARS2(), ..., CLOSURE_COMMIT_VARSn()
 */
#define CLOSURE_COMMIT_VARS() /* Empty definition for Doxygen */
#undef CLOSURE_COMMIT_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_COMMIT_VARS(...) CLOSURE_COMMIT_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_COMMIT_VARS CLOSURE_COMMIT_VARS_N
#endif
/** @endcond */

/**
 * @brief Commit the modification of a certain number of variables all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_COMMIT_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_COMMIT_VARS0(), CLOSURE_COMMIT_VARS1(), ..., CLOSURE_COMMIT_VARS9().
 * 
 * The number of variables is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by commas.
 * 
 * @see CLOSURE_COMMIT_VAR()
 * @see CLOSURE_COMMIT_VARS()
 * @see CLOSURE_COMMIT_VARS_N()
 */
#define CLOSURE_COMMIT_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_COMMIT_VARSn

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

/**
 * @brief Update the value of a local variable so that external modification to the variable can be seen.
 * @param v: name of the variable.
 * 
 * @note It is almost not necessary since any commited ore retained modification prior to the invocation
 *  are restored automatically.
 * 
 * @see CLOSURE_COMMIT_VAR()
 * @see CLOSURE_UPDATE_VARS()
 */
#define CLOSURE_UPDATE_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((char *)&v, (char *)__CLOSURE_STUB->closure->frame + CLOSURE_VAR_OFFSET(v), sizeof(v)); \
  } while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE_UPDATE_VARS_N().
 * @see CLOSURE_UPDATE_VARS_N()
 */
#define __CLOSURE_UPDATE_VAR_SEQ(z, n, seq) \
  CLOSURE_UPDATE_VAR(BOOST_PP_SEQ_ELEM(n, seq))
/** @endcond */

/**
 * @brief Update the value of a number of variables all at once.
 * 
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains variable names.
 * 
 * @see CLOSURE_UPDATE_VAR()
 * @see CLOSURE_UPDATE_VARS()
 * @see CLOSURE_UPDATE_VARS1(), CLOSURE_UPDATE_VARS2(), ..., CLOSURE_UPDATE_VARSn()
 */
#define CLOSURE_UPDATE_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_UPDATE_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CLOSURE_UPDATE_VARS_N()
 *
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_UPDATE_VARS_N() otherwise.
 * 
 * @param ...: one or more variable names seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_UPDATE_VAR()
 * @see CLOSURE_UPDATE_VARS_N()
 * @see CLOSURE_UPDATE_VARS1(), CLOSURE_UPDATE_VARS2(), ..., CLOSURE_UPDATE_VARSn()
 */
#define CLOSURE_UPDATE_VARS() /* Empty defintion for Doxygen */
#undef CLOSURE_UPDATE_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_UPDATE_VARS(...) CLOSURE_UPDATE_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_UPDATE_VARS CLOSURE_UPDATE_VARS_N
#endif
/** @endcond */

/**
 * @brief Update the value of a certain number of variables all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_UPDATE_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_UPDATE_VARS0(), CLOSURE_UPDATE_VARS1(), ..., CLOSURE_UPDATE_VARS9().
 * 
 * The number of variables is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by commas.
 * 
 * @see CLOSURE_UPDATE_VAR()
 * @see CLOSURE_UPDATE_VARS()
 * @see CLOSURE_UPDATE_VARS_N()
 */
#define CLOSURE_UPDATE_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_UPDATE_VARSn

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

/**
 * @brief Update the value of a variable of the host function.
 * @details The value of a variable in the stack frame of host function
 * will be replaced with the current value of the closure.
 * @param v: name of the variable.
 * @warning It can be only used when the host function in which the closure was
 * created is still running.
 * Also it is not thread safety by default to modify a variable
 * of the host function, and the modification is not guaranteed to be seen
 * unless the variable is marked as "volatile" or read with explicit memory barrier.
 */
#define CLOSURE_UPDATE_HOST_VAR(v) \
  do { \
    CLOSURE_ASSERT_VAR(v); \
    memcpy((char *)&v - CLOSURE_GET_FRAME_OFFSET(), (char *)&v, sizeof(v)); \
  } while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE_UPDATE_HOST_VARS().
 * @see CLOSURE_UPDATE_HOST_VARS()
 */
#define __CLOSURE_UPDATE_HOST_SEQ(z, n, seq) \
  CLOSURE_UPDATE_HOST_VAR(BOOST_PP_SEQ_ELEM(n, seq))
/** @endcond */

/**
 * @brief Replace a number of variables of host function all at once.
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains variables.
 * @see CLOSURE_UPDATE_HOST_VAR()
 * @see CLOSURE_UPDATE_HOST_VARS()
 * @see CLOSURE_UPDATE_HOST_VARS1(), CLOSURE_UPDATE_HOST_VARS2(), ..., CLOSURE_UPDATE_HOST_VARSn()
 */
#define CLOSURE_UPDATE_HOST_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_UPDATE_HOST_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CLOSURE_UPDATE_HOST_VARS_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_UPDATE_HOST_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_UPDATE_HOST_VAR()
 * @see CLOSURE_UPDATE_HOST_VARS_N()
 * @see CLOSURE_UPDATE_HOST_VARS1(), CLOSURE_UPDATE_HOST_VARS2(), ..., CLOSURE_UPDATE_HOST_VARSn()
 */
#define CLOSURE_UPDATE_HOST_VARS() /* Empty defintion for Doxygen */
#undef CLOSURE_UPDATE_HOST_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_UPDATE_HOST_VARS(...) CLOSURE_UPDATE_HOST_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_UPDATE_HOST_VARS CLOSURE_UPDATE_HOST_VARS_N
#endif
/** @endcond */

/**
 * @brief Replace a certain number of variables of host function all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_UPDATE_HOST_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_UPDATE_HOST_VARS0(), CLOSURE_UPDATE_HOST_VARS1(), ..., CLOSURE_UPDATE_HOST_VARS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by comma.
 * 
 * @see CLOSURE_UPDATE_HOST_VAR()
 * @see CLOSURE_UPDATE_HOST_VARS()
 * @see CLOSURE_UPDATE_HOST_VARS_N()
 */
#define CLOSURE_UPDATE_HOST_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_UPDATE_HOST_VARSn

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

/** @cond */
/**
 * @internal
 * @fn __closure_var_vector_append(__ClosureVarVector *argv, void *addr, size_t size, void *value)
 * @brief Append a variable to a variables vector.
 * @details It is the internal help function behind __CLOSURE_VAR_VECTOR_APPEND().
 * @param argv: pointer to the variables vector of type __ClosureVarVector.
 * @param addr, size, value: fields of vector items.
 * @see __ClosureVarVector
 * @see __CLOSURE_VAR_VECTOR_APPEND()
 */

/**
 * @internal
 * @fn __closure_var_vector_append_debug(__ClosureVarDebugVector *argv, const char *name, void *addr, size_t size, void *value)
 * @brief Append a variable to a variables vector when CLOSURE_DEBUG if defined.
 * @details It is the internal help function behind __CLOSURE_VAR_VECTOR_APPEND().
 * @param argv: pointer to the variables vector of type __ClosureVarDebugVector.
 * @param name, addr, size, value: fields of vector items.
 * @see __ClosureVarDebugVector
 * @see __CLOSURE_VAR_VECTOR_APPEND()
 */

/**
 * @internal
 * @def __CLOSURE_VAR_VECTOR_APPEND(arg, value)
 * @brief Append a variable to a variables vector of the current closure.
 * @details It is an internal help macro to CLOSURE_RETAIN_VAR().
 * @param arg: name of the variable.
 * @param value: pointer to the retained value of the variable..
 * @see CLOSURE_RETAIN_VAR()
 */

#ifdef CLOSURE_DEBUG
inline static void __closure_var_vector_append_debug(__ClosureVarDebugVector *argv, const char *name, void *addr, size_t size, void *value)
{
  struct __ClosureVarDebug temp;
  temp.name = name;
  temp.addr = addr;
  temp.size = size;
  temp.value = value;
  VECTOR_APPEND(argv, temp);
}

# if defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8
#   if defined(_WIN64) /* MSC or MINGW */
#     define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  do { \
    __closure_var_vector_append_debug(&__CLOSURE_STUB->closure->argv, BOOST_PP_STRINGIZE(arg), (char *)CLOSURE_HOST_VAR_ADDR(arg) \
        , sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (char *)(value))); \
    BOOST_PP_EXPR_IF(BOOST_PP_NOT(__PP_IS_EMPTY(value)) \
        , printf("[CLOSURE_DEBUG] The variable \"%s\" has an offset of %lld in %lld bytes stack frame. at: file \"%s\", line %d\n" \
                  , BOOST_PP_STRINGIZE(arg), CLOSURE_VAR_OFFSET(arg), CLOSURE_GET_FRAME_SIZE_OF_THIS(), __FILE__, __LINE__); \
    ) \
  } while (0)
#   else
#     define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  do { \
    __closure_var_vector_append_debug(&__CLOSURE_STUB->closure->argv, BOOST_PP_STRINGIZE(arg), (char *)CLOSURE_HOST_VAR_ADDR(arg) \
        , sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (char *)(value))); \
    BOOST_PP_EXPR_IF(BOOST_PP_NOT(__PP_IS_EMPTY(value)) \
        , printf("[CLOSURE_DEBUG] The variable \"%s\" has an offset of %zd in %zd bytes stack frame. at: file \"%s\", line %d\n" \
                  , BOOST_PP_STRINGIZE(arg), CLOSURE_VAR_OFFSET(arg), CLOSURE_GET_FRAME_SIZE_OF_THIS(), __FILE__, __LINE__); \
    ) \
  } while (0)
#   endif
# else /* !(defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8) */
#   define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  do { \
    __closure_var_vector_append_debug(&__CLOSURE_STUB->closure->argv, BOOST_PP_STRINGIZE(arg), (char *)CLOSURE_HOST_VAR_ADDR(arg) \
        , sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (char *)(value))); \
    BOOST_PP_EXPR_IF(BOOST_PP_NOT(__PP_IS_EMPTY(value)) \
        , printf("[CLOSURE_DEBUG] The variable \"%s\" has an offset of %d in %d bytes stack frame. at: file \"%s\", line %d\n" \
                  , BOOST_PP_STRINGIZE(arg), CLOSURE_VAR_OFFSET(arg), CLOSURE_GET_FRAME_SIZE_OF_THIS(), __FILE__, __LINE__); \
    ) \
  } while (0)
# endif /* defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8 */
#else /* !CLOSURE_DEBUG */
inline static void __closure_var_vector_append(__ClosureVarVector *argv, void *addr, size_t size, void *value)
{
  struct __ClosureVar temp;
  temp.addr = addr;
  temp.size = size;
  temp.value = value;
  VECTOR_APPEND(argv, temp);
}

# define __CLOSURE_VAR_VECTOR_APPEND(arg, value) \
  __closure_var_vector_append(&__CLOSURE_STUB->closure->argv, (char *)CLOSURE_HOST_VAR_ADDR(arg), sizeof(arg), BOOST_PP_IF(__PP_IS_EMPTY(value), NULL, (char *)(value)))
#endif /* CLOSURE_DEBUG */
/** @endcond */

/**
 * @brief Retain the value of a local variable between invocations.
 * @details The last modification of the retained variable will be committed automactically
 * at the end of the invocation so that it can be seen in the following invocations. It will 
 * always happen after a variable is retained.
 *
 * @param v: name of the variable.
 *
 * @note Modification to a variable is not retained during the execution in a same invocation of the continuation
 * except it is commited explicitly with CLOSURE_COMMIT_VAR().
 * Reference to the retained variable can also be accessed with CLOSURE_VAR() directly.
 *
 * @see CLOSURE_VAR()
 * @see CLOSURE_COMMIT_VAR(), CLOSURE_UPDATE_VAR()
 * @see CLOSURE_RETAIN_VARS()
 */
#define CLOSURE_RETAIN_VAR(v) \
do { \
  if (!CONTINUATION_IS_INITIALIZED(__CLOSURE_STUB->cont_stub.cont)) { \
    CLOSURE_RESERVE_VAR(v); \
    __CLOSURE_VAR_VECTOR_APPEND(v, ); \
  } else { \
    CLOSURE_ASSERT_VAR(v); \
    __CLOSURE_VAR_VECTOR_APPEND(v, CLOSURE_VAR_ADDR(v)); \
  } \
} while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE_RETAIN_VARS_N().
 * @see CLOSURE_RETAIN_VARS_N()
 */
#define __CLOSURE_RETAIN_VAR_SEQ(z, n, seq) \
  CLOSURE_RETAIN_VAR(BOOST_PP_SEQ_ELEM(n, seq))
/** @endcond */

/**
 * @brief Retain values of a number of variables between invocations all at once.
 * @param n: the number of variables.
 * @param tuple: BOOST preprocessor tuple contains the variables.
 * @see CLOSURE_RETAIN_VAR()
 * @see CLOSURE_RETAIN_VARS()
 * @see CLOSURE_RETAIN_VARS1(), CLOSURE_RETAIN_VARS2(), ..., CLOSURE_RETAIN_VARSn()
 */
#define CLOSURE_RETAIN_VARS_N(n, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_RETAIN_VAR_SEQ, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CLOSURE_RETAIN_VARS_N()
 * @details If variadic macros are available , the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_RETAIN_VARS_N() otherwise.
 * 
 * @param ...: variable names seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CLOSURE_RETAIN_VAR()
 * @see CLOSURE_RETAIN_VARS_N()
 * @see CLOSURE_RETAIN_VARS1(), CLOSURE_RETAIN_VARS2(), ..., CLOSURE_RETAIN_VARSn()
 */
#define CLOSURE_RETAIN_VARS() /* Empty definition for Doxygen */
#undef CLOSURE_RETAIN_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_RETAIN_VARS(...) CLOSURE_RETAIN_VARS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RETAIN_VARS CLOSURE_RETAIN_VAR_N
#endif
/** @endcond */

/**
 * @brief Retain a certain number of local variables between invocations all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_RETAIN_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE_RETAIN_VARS0(), CLOSURE_RETAIN_VARS1(), ..., CLOSURE_RETAIN_VARS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n variable names seperated by comma.
 * 
 * @see CLOSURE_RETAIN_VAR()
 * @see CLOSURE_RETAIN_VARS()
 * @see CLOSURE_RETAIN_VARS_N()
 */
#define CLOSURE_RETAIN_VARSn() /* Empty definition for Doxygen */
#undef CLOSURE_RETAIN_VARSn

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
/** @} Variable evaluation */

/**
 * @name Closure argument
 * 
 * Closure argument is a structure that has the values of its fields are equals to the parameters
 * last passed in the invocations of a closure.
 * 
 * A macro CLOSURE_ARG_OF_() is used for a pointer to the structure.
 * 
 * Closure argument is not thread safe, which means invocations to the closure should be synchronized
 * if any parameters are passed in and accessed within the closure. By contrast, the invocations themselves
 * are thread safe which allows a closure be called simultaneously if no accessing conflicts within it.
 * 
 * @{
 */
/** @cond */
/**
 * @internal
 * @brief Internal help macro to CLOSURE_INIT_ARGS_N().
 * @see CLOSURE_INIT_ARGS_N()
 */
#define __CLOSURE_INIT_ARGS(z, n, closure_params) \
  BOOST_PP_CAT((BOOST_PP_TUPLE_ELEM(2, 0, closure_params))->arg._, BOOST_PP_INC(n)) \
  = \
  BOOST_PP_SEQ_ELEM(n, BOOST_PP_TUPLE_ELEM(2, 1, closure_params));
/** @endcond */

/**
 * @brief Initialize the parameters of a closure.
 * @param n: the number of parameters.
 * @param closure_ptr: pointer to the closure.
 * @param tuple: BOOST preprocessor tuple contains the parameters.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_INIT_ARGS()
 * @see CLOSURE_INIT_ARGS1(), CLOSURE_INIT_ARGS2(), ..., CLOSURE_INIT_ARGSn()
 */
#define CLOSURE_INIT_ARGS_N(n, closure_ptr, tuple) \
  do { \
    BOOST_PP_REPEAT(n, __CLOSURE_INIT_ARGS, (closure_ptr, BOOST_PP_TUPLE_TO_SEQ(n, tuple))) \
  } while (0)

/**
 * @copybrief CLOSURE_INIT_ARGS_N()
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_INIT_ARGS_N() otherwise.
 * 
 * @param closure_ptr: pointer to the closure.
 * @param ...: one or more parameters seperated by commas if BOOST_PP_VARIADICS isn't 0.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_INIT_ARGS_N()
 * @see CLOSURE_INIT_ARGS1(), CLOSURE_INIT_ARGS2(), ..., CLOSURE_INIT_ARGSn()
 */
#define CLOSURE_INIT_ARGS(closure_ptr) /* Empty definition for Doxygen */
#undef CLOSURE_INIT_ARGS

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_INIT_ARGS(closure_ptr, ...) CLOSURE_INIT_ARGS_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), closure_ptr, BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_INIT_ARGS CLOSURE_INIT_ARGS_N
#endif
/** @endcond */

/**
 * @brief Initialize a certain number of parameters of a closure.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE_INIT_ARGS<em>n</em>
 * with the suffix <em>n</em> from 1 to 9, a.k.a. CLOSURE_INIT_ARGS1(), CLOSURE_INIT_ARGS2(), ..., CLOSURE_INIT_ARGS9().
 * 
 * The number is specified as the suffix.
 * 
 * @param ...: \p n parameters seperated by commas.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_INIT_ARGS()
 * @see CLOSURE_INIT_ARGS_N()
 */
#define CLOSURE_INIT_ARGSn() /* Empty definition for Doxygen */
#undef CLOSURE_INIT_ARGSn

#define CLOSURE_INIT_ARGS1(closure_ptr, v1) CLOSURE_INIT_ARGS_N(1, closure_ptr, (v1))
#define CLOSURE_INIT_ARGS2(closure_ptr, v1, v2) CLOSURE_INIT_ARGS_N(2, closure_ptr, (v1, v2))
#define CLOSURE_INIT_ARGS3(closure_ptr, v1, v2, v3) CLOSURE_INIT_ARGS_N(3, closure_ptr, (v1, v2, v3))
#define CLOSURE_INIT_ARGS4(closure_ptr, v1, v2, v3, v4) CLOSURE_INIT_ARGS_N(4, closure_ptr, (v1, v2, v3, v4))
#define CLOSURE_INIT_ARGS5(closure_ptr, v1, v2, v3, v4, v5) CLOSURE_INIT_ARGS_N(5, closure_ptr, (v1, v2, v3, v4, v5))
#define CLOSURE_INIT_ARGS6(closure_ptr, v1, v2, v3, v4, v5, v6) CLOSURE_INIT_ARGS_N(6, closure_ptr, (v1, v2, v3, v4, v5, v6))
#define CLOSURE_INIT_ARGS7(closure_ptr, v1, v2, v3, v4, v5, v6, v7) CLOSURE_INIT_ARGS_N(7, closure_ptr, (v1, v2, v3, v4, v5, v6, v7))
#define CLOSURE_INIT_ARGS8(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8) CLOSURE_INIT_ARGS_N(8, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CLOSURE_INIT_ARGS9(closure_ptr, v1, v2, v3, v4, v5, v6, v7, v8, v9) CLOSURE_INIT_ARGS_N(9, closure_ptr, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

/**
 * @brief Get pointer to the argument structure within a closure.
 * @details The argument structure has the value of its fields are equals to the parameters last passed in
 * the invocations of a closure.
 * 
 * The members of the structure have the same types as the parameters in CLOSURE() delcaration of the closure,
 * and are named as _1, _2,..., and so on.
 * 
 * @param closure_ptr: pointer to the closure.
 * 
 * @warning No thread safety 
 * 
 * @see CLOSURE(), CLOSURE_INIT_ARGS()
 * @see CLOSURE_RUN()
 * @see CLOSURE_CONNECT()
 * 
 * @par Example:
 * @code
 *  CLOSURE1(int) closure_int;
 *  CLOSURE_CONNECT(&closure_int
 *    , ()
 *    , (
 *      printf("The integer value is %d\n", CLOSURE_ARG_OF_(&closure_int)->_1);
 *    )
 *    , ()
 *  );
 * @endcode
 */
#define CLOSURE_ARG_OF_(closure_ptr) \
  (&(closure_ptr)->arg)
/** @} Closure argument */

/**
 * @brief Invoke a closure with a number of parameters.
 * @param n: the number of parameters.
 * @param closure_ptr: pointer to the closure.
 * @param tuple: BOOST preprocessor tuple contains parameters.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_INIT()
 * @see CLOSURE_INIT_ARGS(), CLOSURE_INIT_ARGS_N()
 * @see CLOSURE_FREE()
 * @see CLOSURE_RUN()
 * @see CLOSURE0_RUN(), CLOSURE1_RUN(), ..., CLOSUREn_RUN()
 */
#define CLOSURE_RUN_N(n, closure_ptr, tuple) \
do { \
  CLOSURE_INIT_ARGS_N(n, closure_ptr, tuple); \
  __closure_run(&(closure_ptr)->closure); \
} while (0)

/**
 * @copybrief CLOSURE_RUN_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CLOSURE_RUN_N() otherwise.
 * 
 * @param closure_ptr: pointer to the closure.
 * @param ...: one ore more parameters seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_RUN_N()
 * @see CLOSURE1_RUN(), CLOSURE2_RUN(), ..., CLOSUREn_RUN()
 */
#define CLOSURE_RUN(closure_ptr) /* Empty defintion for Doxygen */
#undef CLOSURE_RUN

/** @cond */
#if BOOST_PP_VARIADICS
# define CLOSURE_RUN(closure_ptr, ...) CLOSURE_RUN_N(__PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), closure_ptr, BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CLOSURE_RUN(n, closure_ptr, tuple) CLOSURE_RUN_N(n, closure_ptr, tuple)
#endif
/** @endcond */

/**
 * @brief Alias to CLOSURE_RUN().
 */
#define closure_run CLOSURE_RUN

/**
 * @brief Invoke a closure with a certain number of parameters.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CLOSURE<em>n</em>_RUN
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CLOSURE0_RUN(), CLOSURE1_RUN(), ..., CLOSURE9_RUN().
 * 
 * The number is specified as the suffix.
 * 
 * @param closure_ptr: pointer to the closure.
 * @param ...: \p n parameters seperated by comma.
 * 
 * @warning \p closure_ptr is evaluated multiple times!
 * 
 * @see CLOSURE_RUN_N()
 * @see CLOSURE_RUN()
 */
#define CLOSUREn_RUN(closure_ptr) /* Empty definition for Doxygen */
#undef CLOSUREn_RUN

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

/**
 * @brief Disconnect and free a closure.
 * @details If a closure is connected, it will be unconnected and the finalization statement of it will be executed,
 * or nothing will happen otherwise.
 * @param closure_ptr: pointer to the closure.
 * @see CLOSURE_CONNECT()
 */
#define CLOSURE_FREE(closure_ptr) \
  __closure_free(&(closure_ptr)->closure)

/**
 * @brief Alias to CLOSURE_FREE().
 */
#define closure_free(closure_ptr) CLOSURE_FREE(closure_ptr)

#endif /* CLOSURE_H */
