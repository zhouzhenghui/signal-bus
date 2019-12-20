/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_H
#define __CONTINUATION_H

/**
 * @defgroup continuation continuation
 * @brief Source level continuation facility for C language.
 * @details A continuation is a data structure which represents the control state of a computer program.
 * It can be caputured at run time and let a piece of code be executed later. Especially,
 *  the current context can be restored when the continuation is invoked, like call/cc in scheme.
 * Continuations are useful for encoding other control mechanism such as closure, coroutine, and so on.
 * 
 * @see closure
 * @see coroutine
 * 
 * @{
 */

/**
 * @file
 * @brief the header file of continuation.
 * @see continuation
 */

#ifdef CONTINUATION_DEBUG
# include <stdio.h>
#endif

#if HAVE_MEMORY_H
# include <memory.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "continuation_base.h"
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

/**
 * @brief Capture a continuation at runtime.
 *
 * A statements block is connected to a continuation and
 * will be executed whenever the continuation is invoked later.
 * The captured continuation is a delimited continuation
 * which will return when the execution of statements block
 * reaches the end.
 * 
 * Early exit from the continuation with \c break or \c continute to make it undelimited.
 * 
 * @param cont_ptr: pointer to the continuation structure.
 * @param cont_stub: a local variable point to the struct __ContinuationStub.
 * @param initialization: initialization code block executes when the continuation is connected.
 * @param continuation: continuation code block executes whenever the continuation is invoked.
 * 
 * @note Variadic macros are necessary when any "," operator exists
 * in the code block and makes it not a valid macro parameter.
 * All of the statements must be enclosed by parentheses for the situation.
 * It is also recommended to always enclose the statements with parentheses
 * to make it looks like a statements block.
 * 
 * @see struct __Continuation()
 * @see struct __ContinuationStub()
 * @see CONTINUATION_STUB_ENTRY()
 * @see CONTINUATION_STUB_INVOKE()
 * @see CONTINUATION_CONSTRUCT()
 * @see CONTINUATION_DESTRUCT()
 * 
 * @par Example:
 * @code
 *  struct Foo {
 *    struct __Continuation cont;
 *    char *string;
 *  } foo_cont;
 *  // ...
 *  struct {
 *    __ContinuationStub *cont_stub;
 *    struct Foo *foo;
 *  } *cont_stub, init_stub;
 *  cont_stub = &init_stub;
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_CONNECT(cont_ptr, cont_stub, initialization, continuation) \
  switch (0) \
  while (!(cont_ptr)->initialized && ((cont_ptr)->initialized = 1)) \
  if (0) case 0: { \
    continuation_init(cont_ptr, &cont_stub); \
    CONTINUATION_CONSTRUCT(cont_ptr); \
    assert(((cont_ptr)->stack_frame_addr != NULL || ((cont_ptr)->stack_frame_tail == NULL && (cont_ptr)->invoke == NULL)) \
            && "[CONTINUATION FAULT] compiler configuration compatible assertion failed"); \
    continuation_stub_init((struct __ContinuationStub *)cont_stub, cont_ptr); \
    /* the stack frame should contain variable cont_stub at least */ \
    ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr = (char *)&cont_stub + sizeof(cont_stub); \
    ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = 0; \
    if ((cont_ptr)->stack_frame_tail == NULL || (cont_ptr)->stack_frame_tail > (char *)&cont_stub) \
      (cont_ptr)->stack_frame_tail = (char *)&cont_stub; \
    { \
      __PP_REMOVE_PARENS(initialization); \
      CONTINUATION_STUB_ENTRY(cont_stub); \
      if (!((struct __ContinuationStub *)cont_stub)->cont->initialized) { \
        CONTINUATION_INIT_INVOKE((struct __ContinuationStub *)cont_stub, &cont_stub); \
      } else { \
        { \
          size_t volatile anti_optimize = (size_t)&cont_stub; \
          ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = anti_optimize - (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_spot; \
          CONTINUATION_STACK_FRAME_DEBUG(((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset); \
          assert((((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + sizeof(cont_stub)) \
                  || (((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset != 0)) \
              && "[CONTINUATION FAULT] incompatible compilation/optimization in address operator or stack frame pointer"); \
          assert(((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + sizeof(cont_stub)) \
              && "[CONTINUATION FAULT] the size of continuation stack frame isn't enough, probably caused by an incompatible compilation/optimization"); \
          assert(((size_t)((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr > anti_optimize + ((size_t)&anti_optimize - anti_optimize) + sizeof(anti_optimize)) \
              && "[CONTINUATION FAULT] the size of continuation stack frame isn't enough, try to increase it"); \
          assert(((struct __ContinuationStub *)cont_stub)->addr.stack_frame_addr - ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset \
                  >= ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail + ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_size - ((struct __ContinuationStub *)cont_stub)->cont->stack_parameters_size); \
          ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_tail = ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail \
                                                                      + ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset; \
        } \
        do switch (0) default: { /* catch break and continue keywords inside continuation */ \
          __CONTINUATION_BLOCK(continuation); \
          CONTINUATION_STUB_RETURN(cont_stub); \
        } while (0); \
        break; /* break to undelimited continuation outward */ \
      } \
      assert(((struct __ContinuationStub *)cont_stub)->cont == cont_ptr); \
      /* ((struct __ContinuationStub *)cont_stub)->cont->initialized = 1; */ \
      ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_size += ((struct __ContinuationStub *)cont_stub)->cont->stack_parameters_size; \
      ((struct __ContinuationStub *)cont_stub)->size.stack_frame_offset = 0; \
      ((struct __ContinuationStub *)cont_stub)->addr.stack_frame_tail = ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail; \
    } \
  } else

/** @cond */
/**
 * @internal
 * @def CONTINUATION_STACK_FRAME_DEBUG
 * @brief Internal macro to print the stack frame offset of a continuation that may help debug problems.
 * @param offset: offset of the stack frame to the original host function.
 * @see CONTINUATION_CONNECT()
 */
#ifdef CONTINUATION_DEBUG
# ifdef _WIN64
#   define CONTINUATION_STACK_FRAME_DEBUG(offset) \
      do { \
        printf("[CONTINUATION_DEBUG] The continuation stack frame offset is: %llx\n", offset); \
        if ((offset) & 0xF) { \
          printf("[CONTINUATION_DEBUG] Warning: Illegal addressing of SSE instructions may occur since the frame offset is not 16 bytes aligned!\n" \
                 "                              Try to disable the optimization of compiler if runtime error happened.\n"); \
        } \
      } while (0)
# elif defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8
#   define CONTINUATION_STACK_FRAME_DEBUG(offset) \
      printf("[CONTINUATION_DEBUG] The continuation stack frame offset is: %zx\n", offset)
# else
#   define CONTINUATION_STACK_FRAME_DEBUG(offset) \
      printf("[CONTINUATION_DEBUG] The continuation stack frame offset is: %x\n", offset)
# endif
#else
# define CONTINUATION_STACK_FRAME_DEBUG(offset)
#endif

/**
 * @internal
 * @def __CONTINUATION_BLOCK()
 * @brief Internal help macro encloses continuation statements for CONTINUATION_CONNECT()
 * @details Use C++ exception handle for return from middle of continuation.
 * @param continuation: the continuation statements.
 */
#ifdef __cplusplus
  struct __ContinuationException {};
  inline void continuation_return_by_throw() {
    throw __ContinuationException();
  }
# define __CONTINUATION_BLOCK(continuation) \
  try { \
    __PP_REMOVE_PARENS(continuation); \
  } catch (__ContinuationException &) {}
#else
# define __CONTINUATION_BLOCK(continuation) \
  { \
    __PP_REMOVE_PARENS(continuation); \
  }
#endif
/** @endcond */

/**
 * @brief Return from a continuation block.
 * @details The execution of the continuation is terminated with longjmp().
 *
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 *
 * @note A continuation will return automatically when the execution of
 * continuation statements reaches the end. So that goto statement
 * with a label at the end can be used for a safer alternative.
 *
 * @warning Don't or be careful to use it in C++ with undelimited continuation for the unsafety longjmp().
 * 
 * @see CONTINUATION_CONNECT()
 */
#ifdef __cplusplus
# define CONTINUATION_RETURN(cont_stub) \
    continuation_return_by_throw()
#else
# define CONTINUATION_RETURN(cont_stub) \
    CONTINUATION_STUB_RETURN(cont_stub)
#endif

/**
 * @internal
 * @brief Return from a invocated continuation.
 * @param cont_stub: pointer to the continuation stub used for continuation invocation.
 * @see CONTINUATION_CONNECT()
 */
#if !defined(CONTINUATION_STUB_RETURN)
# define CONTINUATION_STUB_RETURN(cont_stub) \
    continuation_stub_return(((struct __ContinuationStub*)(cont_stub)))
#endif

/**
 * @name Continuation state
 * @{
 */
/**
 * @brief Determine whether a continuation is connected or not.
 * @param cont: pointer to the continuation structure.
 * 
 * @see CONTINUATION_CONNECT()
 */
#define CONTINUATION_IS_INITIALIZED(cont) ((cont)->initialized)
/** @} */

/**
 * @name Environment evaluation and enforcement
 * @{
 */
/**
 * @brief Set the size of the execution stack frame of a continuation.
 * 
 * The stack frame size of a continuation can be set at runtime
 * if any dynamic stack allocation facilities are enabled.
 * It is not necessary to set the frame size mostly except for
 * compilation that has reversed stack frame and has no compiler
 * specific configurations.
 * 
 * @param cont: pointer to the continuation.
 * @param size: desired size of the stack frame.
 * 
 * @warning It should be used only in initialization block, or a runtime assertion will fail otherwise. 
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_GET_FRAME_SIZE()
 * @see CONTINUATION_SET_PARAMS_SIZE()
 * @see CONTINUATION_SET_STACK_FRAME_SIZE()
 * @see CONTINUATION_EXTEND_STACK_FRAME()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *      CONTINUATION_SET_FRAME_SIZE(&foo_cont.cont, 1024);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_SET_FRAME_SIZE(cont, size) /* Empty defintion for Doxygen */
#undef CONTINUATION_SET_FRAME_SIZE

/**
 * @brief Alias of the CONTINUATION_SET_FRAME_SIZE().
 * @see CONTINUATION_SET_FRAME_SIZE()
 */
#define CONTINUATION_SET_STACK_FRAME_SIZE(cont) CONTINUATION_SET_FRAME_SIZE(cont)
#undef CONTINUATION_SET_STACK_FRAME_SIZE

/** @cond */
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
/** @endcond */

/**
 * @brief Get the size of the execution stack frame of a continuation.
 * 
 * The macro expansion is an expression and evaluated as
 * the size of the stack frame.
 *
 * @param cont: pointer to the continuation.
 * 
 * @warning The stack frame size of a continuation can only be determined
 * after connected, or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_SET_FRAME_SIZE()
 * @see CONTINUATION_SET_PARAMS_SIZE()
 * @see CONTINUATION_GET_STACK_FRAME_SIZE()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 *  printf("The size of continuation stack frame is: %d\n", CONTINUATION_GET_FRAME_SIZE(&foo_cont.cont));
 * @endcode
 */
#define CONTINUATION_GET_FRAME_SIZE(cont) \
  ((void)assert(CONTINUATION_IS_INITIALIZED(cont) && "XXX_GET_FRAME_SIZE is only available after initialized") \
    , (cont)->stack_frame_size)
/**
 * @brief Alias to CONTINUATION_GET_FRAME_SIZE().
 * @see CONTINUATION_GET_FRAME_SIZE()
 */
#define CONTINUATION_GET_STACK_FRAME_SIZE(cont) CONTINUATION_GET_FRAME_SIZE(cont)

/**
 * @brief Set the size of parameters space before the stack frame of a continuation.
 * 
 * @details The parameters space is the space to passing the parameters of the host function,
 * it should be reserved particularly in compilation with reversed stack frame.
 * 
 * @param cont: pointer to the continuation.
 * @param size: size to be reserved.
 * 
 * @warning It should be used only in initialization block, or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_SET_FRAME_SIZE()
 * @see CONTINUATION_GET_FRAME_SIZE()
 * @see CONTINUATION_SET_STACK_PARAMS_SIZE()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *      CONTINUATION_SET_FRAME_SIZE(&foo_cont.cont, 1024);
 *      CONTINUATION_SET_PARAMS_SIZE(&foo_cont.cont, 64);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 *  printf("The size of continuation stack frame is: %d\n", CONTINUATION_GET_FRAME_SIZE(&foo_cont.cont));
 * @endcode
 */
#define CONTINUATION_SET_PARAMS_SIZE(cont, size) \
do { \
  assert(!CONTINUATION_IS_INITIALIZED(cont) && "XXX_SET_PARAMS_SIZE is only available in initialization"); \
  (cont)->stack_parameters_size = size; \
} while (0)
/**
 * @brief Alias to CONTINUATION_SET_PARAMS_SIZE().
 * @see CONTINUATION_SET_PARAMS_SIZE()
 */
#define CONTINUATION_SET_STACK_PARAMS_SIZE(cont) CONTINUATION_SET_PARAMS_SIZE(cont)

/**
 * @brief Get pointer to the stack frame of the host function.
 * 
 * @details The macro expansion is a expression and is evaluated as the base
 * address of the stack frame of the host function the continuation resides.
 *
 * @param cont: pointer to the continuation.
 * 
 * @warning it is only avaliable after the continuation is connected,
 * or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_GET_HOST_STACK_FRAME()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 *  printf("The host stack frame address is: %x\n", CONTINUATION_GET_HOST_FRAME(&foo_cont.cont));
 * @endcode
 */
#define CONTINUATION_GET_HOST_FRAME(cont) \
  ((void)assert(CONTINUATION_IS_INITIALIZED(cont) && "XXX_GET_FRAME_SIZE is only available after initialized") \
    , (cont)->stack_frame_tail)

/**
 * @brief Alias to CONTINUATION_GET_HOST_FRAME().
 * @see CONTINUATION_GET_HOST_FRAME()
 */
#define CONTINUATION_GET_HOST_STACK_FRAME(cont) CONTINUATION_GET_HOST_FRAME(cont)

/**
 * @brief Get address offset of the current stack frame to the host function's within a continuation.
 * 
 * @details The macro expansion is a expression and can be evaluated in
 * continuation block whenever the continuation is invoked.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_GET_HOST_FRAME()
 * @see CONTINUATION_GET_STACK_FRAME_OFFSET()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      printf("The current continuation has a stack frame at offset: %d\n"
 *              , CONTINUATION_GET_FRAME_OFFSET(cont_stub));
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_GET_FRAME_OFFSET(cont_stub) \
  ((cont_stub)->size.stack_frame_offset)

/**
 * @brief Alias to CONTINUATION_GET_FRAME_OFFSET().
 * 
 * @see CONTINUATION_GET_FRAME_OFFSET()
 */
#define CONTINUATION_GET_STACK_FRAME_OFFSET(cont_stub) CONTINUATION_GET_FRAME_OFFSET(cont_stub)

/**
 * @brief Get pointer to the current stack frame within a continuation.
 * 
 * @details The macro expansion is a expression and can be evaluated in
 * continuation block whenever the continuation is invoked.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_GET_FRAME_OFFSET()
 * @see CONTINUATION_GET_HOST_FRAME()
 * @see CONTINUATION_GET_STACK_FRAME()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      printf("The current continuation stack frame address is: %x\n"
 *              , CONTINUATION_GET_FRAME(cont_stub));
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_GET_FRAME(cont_stub) \
  ((cont_stub)->addr.stack_frame_tail)

/**
 * @brief Alias to CONTINUATION_GET_FRAME().
 * 
 * @see CONTINUATION_GET_FRAME()
 */
#define CONTINUATION_GET_STACK_FRAME(cont_stub) CONTINUATION_GET_FRAME(cont_stub)

/**
 * @brief Reserve a specified memory space to the stack frame of a continuation.
 *
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param addr: address of the memory space be in the stack frame of the host function.
 * @param size: size of the memory space.
 *
 * @warning It can be only used in initialization block,
 * or a runtime assertion will fail otherwise.
 *
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESERVE_VAR()
 * @see CONTINUATION_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VARS_N()
 * @see CONTINUATION_RESERVE_VARSn()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *      CONTINUATION_RESERVE_ADDR(cont_stub, &foo, sizeof(foo));
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_RESERVE_FRAME_ADDR(cont_stub, addr, size) \
do { \
  assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_FRAME_ADDR is only available in initialization"); \
  __continuation_reserve_frame_addr(cont_stub, addr, size); \
} while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro for CONTINUATION_RESERVE_VAR() and CONTINUATION_RESERVE_VARS_N(), etc.
 * 
 * @see CONTINUATION_RESERVE_VAR()
 * @see CONTINUATION_RESERVE_VARS_N()
 * @see CONTINUATION_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VARSn()
 */
#define __CONTINUATION_RESERVE_VAR(cont_stub, v) \
  __continuation_reserve_frame_addr(cont_stub, (char *)&v, sizeof(v))
/** @endcond */

/**
 * @brief Reserve the memory space of a local variable in the stack frame for a continuation.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param v: name of the variable.
 * 
 * @warning It can be only used in initialization block, or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESERVE_VARS_N()
 * @see CONTINUATION_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VARSn()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *      CONTINUATION_RESERVE_VAR(cont_stub, foo);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_RESERVE_VAR(cont_stub, v) \
  do { \
    assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_VAR is only available in initialization"); \
    __CONTINUATION_RESERVE_VAR(cont_stub, v); \
  } while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro for CONTINUATION_RESERVE_VARS_N() and others.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESERVE_VARS_N()
 * @see CONTINUATION_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VARSn()
 */
#define __CONTINUATION_RESERVE_SEQ_ELEM(r, data, elem) \
  __CONTINUATION_RESERVE_VAR(data, elem);
/** @endcond */

/**
 * @brief Reserve the memory space of a number of local variables in the stack frame for a continuation all at once.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param n: number of the variables.
 * @param tuple: boost preprocessor tuple contains the variables.
 * 
 * @warning It can be only used in initialization block, or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESERVE_VAR()
 * @see CONTINUATION_RESERVE_VARS()
 * @see CONTINUATION_RESERVE_VARSn()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_RESERVE_VARS_N(cont_stub, 2, (i, foo));
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_RESERVE_VARS_N(cont_stub, n, tuple) \
  do { \
    assert(!(cont_stub)->cont->initialized && "XXX_RESERVE_VARS is only available in initialization"); \
    BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_RESERVE_SEQ_ELEM, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CONTINUATION_RESERVE_VARS_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CONTINUATION_RESERVE_VARS_N() otherwise.
 * 
 * @param cont_stub: pointer to the local continuation stub variable.
 * @param ...: variable names seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESERVE_VAR()
 * @see CONTINUATION_RESERVE_VARS_N()
 * @see CONTINUATION_RESERVE_VARSn()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_RESERVE_VARS(cont_stub, i, foo);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_RESERVE_VARS(cont_stub) /* Empty defintion for Doxygen */
#undef CONTINUATION_RESERVE_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CONTINUATION_RESERVE_VARS(cont_stub, ...) CONTINUATION_RESERVE_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_RESERVE_VARS CONTINUATION_RESERVE_VARS_N
#endif
/** @endcond */

/**
 * @brief Reserve the memory space of a certain number of local variables in the stack frame for a continuation all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CONTINUATION_RESERVE_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CONTINUATION_RESERVE_VARS0(), CONTINUATION_RESERVE_VARS1(), ..., CONTINUATION_RESERVE_VARS9().
 * 
 * The number of variables is specified as the suffix.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param ...: <em>n</em> number of variables seperated by comma.
 *
 * @par See Also:
 *  CONTINUATION_CONNECT(),
 *  CONTINUATION_RESERVE_VAR(),
 *  CONTINUATION_RESERVE_VARS(),
 *  CONTINUATION_RESERVE_VARS_N()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_RESERVE_VARS2(cont_stub, i, foo);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_RESERVE_VARSn(cont_stub) /* Empty definition for Doxygen */
#undef CONTINUATION_RESERVE_VARSn

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

/** @cond */
/**
 * @internal
 * @brief Internal help macro for CONTINUATION_ASSERT_VAR() and CONTINUATION_ASSERT_VARS(), etc.
 *
 * @note The memory map of the variable will be printed to stdout
 * if macro CONTINUATION_DEBUG is defined at the moment the
 * header file is last included.
 * 
 * @see CONTINUATION_ASSERT_VAR()
 * @see CONTINUATION_ASSERT_VARS()
 */
#define __CONTINUATION_ASSERT_VAR(cont_stub, v) /* Empty definition for Doxygen */
#undef __CONTINUATION_ASSERT_VAR

#ifdef CONTINUATION_DEBUG
# if defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8
#   if defined(_WIN64) /* MSC or MINGW */
#     define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    (void)printf("[CONTINUATION_DEBUG] The variable \"%s\" has an offset of %lld in %lld bytes stack frame. at: file \"%s\", line %d\n" \
            , BOOST_PP_STRINGIZE(v) \
            , (char *)(&v) - (cont_stub)->addr.stack_frame_tail, (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__) \
    , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) to fix a non-compiler specific implementation" BOOST_PP_RPAREN()) \
  )
#   else
#     define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    (void)printf("[CONTINUATION_DEBUG] The variable \"%s\" has an offset of %zd in %zd bytes stack frame. at: file \"%s\", line %d\n" \
            , BOOST_PP_STRINGIZE(v) \
            , (char *)(&v) - (cont_stub)->addr.stack_frame_tail, (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__) \
    , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) to fix a non-compiler specific implementation" BOOST_PP_RPAREN()) \
  )
#   endif
# else
#   define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    (void)printf("[CONTINUATION_DEBUG] The variable \"%s\" has an offset of %d in %d bytes stack frame. at: file \"%s\", line %d\n" \
            , BOOST_PP_STRINGIZE(v) \
            , (char *)(&v) - (cont_stub)->addr.stack_frame_tail, (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__) \
    , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) to fix a non-compiler specific implementation" BOOST_PP_RPAREN()) \
  )
# endif
#else
# define __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() \
            __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v)) \
              && "variable " BOOST_PP_STRINGIZE(v) " is outside of the continuation stack frame" \
              && "try XXX_RESERVE_VAR(S)/XXX_ENFORCE_VAR(S) to fix a non-compiler specific implementation" BOOST_PP_RPAREN()) \
  )
#endif /* CONTINUATION_DEBUG */
/** @endcond */

/**
 * @brief Assert a local variable resides in the stack frame.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param v: name of the variable.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ASSERT_VARS()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      CONTINUATION_ASSERT_VAR(cont_stub, foo);
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
*/
#define CONTINUATION_ASSERT_VAR(cont_stub, v) \
  ( \
    assert((cont_stub)->cont->initialized && "XXX_ASSERT_VAR is only available after initialized") \
    , __CONTINUATION_ASSERT_VAR(cont_stub, v) \
  )

/** @cond */
/**
 * @internal
 * @brief Help macro for CONTINUATION_ASSERT_VARS_N().
 * @see CONTINUATION_ASSERT_VARS_N()
 */
#define __CONTINUATION_ASSERT_SEQ_ELEM(r, data, elem) \
  __CONTINUATION_ASSERT_VAR(data, elem),
/** @endcond */

/**
 * @brief Assert a number of local variables reside in the stack frame all at once.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param n: the number of variables.
 * @param tuple: boost preprocessor tuple contains the variables.
 *
 * @see CONTINUATION_CONNECT(),
 * @see CONTINUATION_ASSERT_VAR(),
 * @see CONTINUATION_ASSERT_VARS(),
 * @see CONTINUATION_ASSERT_VARSn()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *    )
 *    , (
 *      CONTINUATION_ASSERT_VARS_N(cont_stub, 2, (i, foo));
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ASSERT_VARS_N(cont_stub, n, tuple) \
  ( \
    assert((cont_stub)->cont->initialized && "XXX_ASSERT_VARS is only available after initialized") \
    , BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_ASSERT_SEQ_ELEM, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
    (void)0 \
  )

/**
 * @copybrief CONTINUATION_ASSERT_VARS_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CONTINUATION_ASSERT_VARS_N() otherwise.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param ...: variable names seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ASSERT_VAR()
 * @see CONTINUATION_ASSERT_VARS_N()
 * @see CONTINUATION_ASSERT_VARSn()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *    )
 *    , (
 *      CONTINUATION_ASSERT_VARS(cont_stub, i, foo);
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ASSERT_VARS(cont_stub) /* Empty definition for Doxygen */
#undef CONTINUATION_ASSERT_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CONTINUATION_ASSERT_VARS(cont_stub, ...) CONTINUATION_ASSERT_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_ASSERT_VARS CONTINUATION_ASSERT_VARS_N
#endif
/** @endcond */

/**
 * @brief Assert a certain number of local variables reside in the stack frame all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CONTINUATION_ASSERT_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CONTINUATION_ASSERT_VARS0(), CONTINUATION_ASSERT_VARS1(), ..., CONTINUATION_ASSERT_VARS9().
 * 
 * The number of variables is specified as the suffix.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param ...: <em>n</em> number of variables seperated by comma.
 *
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ASSERT_VAR()
 * @see CONTINUATION_ASSERT_VARS_N()
 * @see CONTINUATION_ASSERT_VARS()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *    )
 *    , (
 *      CONTINUATION_ASSERT_VARS2(cont_stub, i, foo);
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ASSERT_VARSn(cont_stub) /* Empty defintion for Doxgen */
#undef CONTINUATION_ASSERT_VARSn

#define CONTINUATION_ASSERT_VARS0(cont_stub) CONTINUATION_ASSERT_VARS_N(cont_stub, 0, ())
#define CONTINUATION_ASSERT_VARS1(cont_stub, v1) CONTINUATION_ASSERT_VARS_N(cont_stub, 1, (v1))
#define CONTINUATION_ASSERT_VARS2(cont_stub, v1, v2) CONTINUATION_ASSERT_VARS_N(cont_stub, 2, (v1, v2))
#define CONTINUATION_ASSERT_VARS3(cont_stub, v1, v2, v3) CONTINUATION_ASSERT_VARS_N(cont_stub, 3, (v1, v2, v3))
#define CONTINUATION_ASSERT_VARS4(cont_stub, v1, v2, v3, v4) CONTINUATION_ASSERT_VARS_N(cont_stub, 4, (v1, v2, v3, v4))
#define CONTINUATION_ASSERT_VARS5(cont_stub, v1, v2, v3, v4, v5) CONTINUATION_ASSERT_VARS_N(cont_stub, 5, (v1, v2, v3, v4, v5))
#define CONTINUATION_ASSERT_VARS6(cont_stub, v1, v2, v3, v4, v5, v6) CONTINUATION_ASSERT_VARS_N(cont_stub, 6, (v1, v2, v3, v4, v5, v6))
#define CONTINUATION_ASSERT_VARS7(cont_stub, v1, v2, v3, v4, v5, v6, v7) CONTINUATION_ASSERT_VARS_N(cont_stub, 7, (v1, v2, v3, v4, v5, v6, v7))
#define CONTINUATION_ASSERT_VARS8(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8) CONTINUATION_ASSERT_VARS_N(cont_stub, 8, (v1, v2, v3, v4, v5, v6, v7, v8))
#define CONTINUATION_ASSERT_VARS9(cont_stub, v1, v2, v3, v4, v5, v6, v7, v8, v9) CONTINUATION_ASSERT_VARS_N(cont_stub, (9, (v1, v2, v3, v4, v5, v6, v7, v8, v9))

/**
 * @brief Enforce a local variable resides in the stack frame.
 * @details It is implemented by incorporating the CONTINUATION_RESERVER_VAR() and CONTINUATION_ASSERT_VAR().
 *
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param v: the local variable.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ENFORCE_VARS()
 * @see CONTINUATION_ENFORCE_VARS_N()
 * @see CONTINUATION_ENFORCE_VARSn()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *      CONTINUATION_ENFORCE_VAR(cont_stub, foo);
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ENFORCE_VAR(cont_stub, v) \
do { \
  if (!(cont_stub)->cont->initialized) { \
    __CONTINUATION_RESERVE_VAR(cont_stub, v); \
  } else { \
    __CONTINUATION_ASSERT_VAR(cont_stub, v); \
  } \
  if (__continuation_enforce_var) __continuation_enforce_var((char *)&v); \
} while (0)

/** @cond */
/**
 * @internal
 * @brief Internal help macro to CONTINUATION_ENFORCE_VARS_N()
 * @see CONTINUATION_ENFORCE_VARS_N()
 */
#define __CONTINUATION_ENFORCE_SEQ_ELEM(r, data, elem) \
  CONTINUATION_ENFORCE_VAR(data, elem);
/** @endcond */

/**
 * @brief Enforce a number of local variables reside in the stack frame all at once.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param n: the number of variables.
 * @param tuple: boost preprocessor tuple contains the local variables.
 *
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ENFORCE_VAR()
 * @see  CONTINUATION_ENFORCE_VARS()
 * @see CONTINUATION_ENFORCE_VARSn()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_ASSERT_VARS_N(cont_stub, 2, (i, foo));
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ENFORCE_VARS_N(cont_stub, n, tuple) \
  do { \
    BOOST_PP_SEQ_FOR_EACH(__CONTINUATION_ENFORCE_SEQ_ELEM, cont_stub, BOOST_PP_TUPLE_TO_SEQ(n, tuple)) \
  } while (0)

/**
 * @copybrief CONTINUATION_ENFORCE_VARS_N()
 * 
 * @details If variadic macros are available, the parameters in BOOST preprocessor tuple
 * can be transefered directly without the number and tuple specification,
 * or it is the alias to CONTINUATION_ENFORCE_VARS_N() otherwise.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param ...: variable names seperated by comma if BOOST_PP_VARIADICS isn't 0.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ENFORCE_VAR()
 * @see CONTINUATION_ENFORCE_VARS_N()
 * @see CONTINUATION_ENFORCE_VARSn()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_ENFORCE_VARS(cont_stub, i, foo));
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ENFORCE_VARS(cont_stub) /* Empty defintion for Doxygen */
#undef CONTINUATION_ENFORCE_VARS

/** @cond */
#if BOOST_PP_VARIADICS
# define CONTINUATION_ENFORCE_VARS(cont_stub, ...) CONTINUATION_ENFORCE_VARS_N(cont_stub, __PP_VARIADIC_SIZE_OR_ZERO(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))
#else
# define CONTINUATION_ENFORCE_VARS CONTINUATION_ENFORCE_VARS_N
#endif
/** @endcond */

/**
 * @brief Enforce a certain number of local variables reside in the stack frame all at once.
 * 
 * @details It is a dummy macro to document a set of macros which have the name pattern CONTINUATION_ENFORCE_VARS<em>n</em>
 * with the suffix <em>n</em> from 0 to 9, a.k.a. CONTINUATION_ENFORCE_VARS0(), CONTINUATION_ENFORCE_VARS1(), ..., CONTINUATION_ENFORCE_VARS9().
 * 
 * The number of variables is specified as the suffix.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param ...: <em>n</em> number of variables seperated by comma.
 *
 * @note 
 *
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ENFORCE_VAR()
 * @see CONTINUATION_ENFORCE_VARS()
 * @see CONTINUATION_ENFORCE_VARS_N()
 *
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      int i;
 *      struct Foo *foo;
 *      CONTINUATION_ENFORCE_VARS2(cont_stub, i, foo));
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", i, foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ENFORCE_VARSn(cont_stub) /* Empty definition for Doxygen */
#undef CONTINUATION_ENFORCE_VARSn

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
/** @} Environment evaluation and enforcement */

/**
 * @name Variable evaluation
 * @{
 */
/**
 * @def CONTINUATION_HOST_VAR_ADDR(cont_stub, v)
 * @brief Get pointer to a local variable in the host function within a continuation.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param v: the local variable.
 * 
 * @note The variable should reside in the stack frame,
 * or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_HOST_VAR()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *      if (foo == CONTINUATION_HOST_VAR_ADDR(foo_cont)) {
 *        // ...
 *      }
 *    )
 *  );
 * @endcode
 */
#if defined(__GNUC__)
# define CONTINUATION_HOST_VAR_ADDR(cont_stub, v) \
  ((__typeof__(v) *) \
    ((void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (!(cont_stub)->cont->initialized || __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v))) \
         && "The variable " BOOST_PP_STRINGIZE(v) " is outside of stack frame" BOOST_PP_RPAREN()) \
   , (size_t)&v - (cont_stub)->size.stack_frame_offset) \
  )
#else
# define CONTINUATION_HOST_VAR_ADDR(cont_stub, v) \
  ((void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (!(cont_stub)->cont->initialized || __continuation_variable_in_stack_frame(cont_stub, &v, sizeof(v))) \
        && "The variable " BOOST_PP_STRINGIZE(v) " is outside of stack frame" BOOST_PP_RPAREN()) \
   , 0 ? &v : (size_t)&v - (cont_stub)->size.stack_frame_offset)
#endif

/**
 * @brief Get reference of a local variable in the host function within a continuation.
 * 
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param v: the local variable.
 * 
 * @note The variable should reside in the continuation stack frame,
 * or a runtime assertion will fail otherwise.
 *
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_HOST_VAR_ADDR()
 *  
 * @par Example:
 * @code
 *  int i = 0;
 *  // ...
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      printf("%d, %s\n", CONTINUATION_HOST_VAR(i), foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_HOST_VAR(cont_stub, v) \
  (* CONTINUATION_HOST_VAR_ADDR(cont_stub, v))

/**
 * @brief Get the offset of an address in the stack frame within a continuation.
 * @param cont_stub: the \p cont_stub variable in CONTINUATION_CONNECT().
 * @param a: maybe the address of a local variable.
 * 
 * @warning The address should reside in the stack frame of the continuation,
 * or a runtime assertion will fail otherwise.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_VAR_OFFSET()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      printf("The offset of foo in stack frame is: %d\n", CONTINUATION_ADDR_OFFSET(cont_stub, &foo));
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_ADDR_OFFSET(cont_stub, a) \
  ((void)assert((cont_stub)->cont->initialized && "XXX_ADDR_XXX is only available after initialized") \
    , (void)BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (size_t)(a) >= (size_t)(cont_stub)->addr.stack_frame_tail \
                    && (size_t)(a) < (size_t)(cont_stub)->cont->stack_frame_tail + (cont_stub)->size.stack_frame_offset + (cont_stub)->cont->stack_frame_size \
                    && "The address " BOOST_PP_STRINGIZE(a) " is outside of stack frame" BOOST_PP_RPAREN()) \
    , ((size_t)(a) - (size_t)(cont_stub)->addr.stack_frame_tail))

/**
 * @brief Get the offset in the stack frame of a local variable within a continuation.
 * 
 * @param cont_stub: pointer to the local continuation stub variable.
 * @param v: name of the variable.
 * 
 * @note CONTINUATION_ASSERT_VAR() is applied to \p v too when CONTINUATION_DEBUG is defined.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_ADDR_OFFSET()
 * @see CONTINUATION_ASSERT_VAR()
 * 
 * @par Example:
 * @code
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      printf("The offset of foo in stack frame is: %d\n", CONTINUATION_VAR_OFFSET(cont_stub, foo));
 *      foo = (struct Foo *)cont_stub->cont;
 *      puts(foo->string);
 *    )
 *  );
 * @endcode
 */
#define CONTINUATION_VAR_OFFSET(cont_stub, v) \
  ((void)assert((cont_stub)->cont->initialized && "XXX_VAR_XXX is only available after initialized") \
    , (void)__CONTINUATION_ASSERT_VAR(cont_stub, v) \
    , ((size_t)&v - (size_t)(cont_stub)->addr.stack_frame_tail))
/** @} Variable evaluation */

/**
 * @brief Backup the stack frame of host function.
 * 
 * @details the stack frame that a continuation is captured is copied to a specified storage
 * so that it can be restored later.
 *
 * @param cont: the continuation.
 * @param stack_frame: the backup storage for restoring the stack frame of continuation.
 * 
 * @note Runtime assertion will fail if the continuation is not properly implemented. 
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_RESTORE_STACK_FRAME()
 * 
 * @par Example:
 * @code
 *  struct Foo {
 *    struct __Continuation cont;
 *    char *string;
 *    char stack_frame[MAX_STACK_FRAME_SIZE];
 *  } foo_cont;
 *  // ...
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      CONTINUATION_RESTORE_STACK_FRAME(&foo->cont, foo->stack_frame);
 *      puts(foo->string);
 *    )
 *  );
 *  CONTINUATION_BACKUP_STACK_FRAME(&foo_cont.cont, foo_cont.stack_frame);
 * @endcode
 */
#define CONTINUATION_BACKUP_STACK_FRAME(cont, stack_frame) \
do { \
  assert((cont)->initialized && "XXX_BACKUP_STACK_FRAME is only available after initialized"); \
  assert((size_t)(stack_frame) < (size_t)(cont)->stack_frame_tail \
          || (size_t)(stack_frame) > (size_t)(cont)->stack_frame_tail + (cont)->stack_frame_size); \
  continuation_backup_stack_frame(cont, stack_frame); \
} while (0)

/**
 * @brief Restore the stack frame of continuation from a backup storage.
 *
 * @details The stack frame of continuation should be restored at the moment of each execution
 * whenever the continuation is invoked.
 *  
 * @param cont_stub: pointer to the local continuation stub variable.
 * @param stack_frame: the continuation stack frame contains the backup of the host stack frame.
 * 
 * @note Runtime assertions will fail if the continuation is not properly implemented.
 * 
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_BACKUP_STACK_FRAME()
 * 
 * @par Example:
 * @code
 *  struct Foo {
 *    struct __Continuation cont;
 *    char *string;
 *    char stack_frame[MAX_STACK_FRAME_SIZE];
 *  } foo_cont;
 *  // ...
 *  CONTINUATION_CONNECT(&foo_cont.cont, cont_stub
 *    , (
 *      struct Foo *foo;
 *    )
 *    , (
 *      foo = (struct Foo *)cont_stub->cont;
 *      CONTINUATION_RESTORE_STACK_FRAME(&foo->cont, foo->stack_frame);
 *      puts(foo->string);
 *    )
 *  );
 *  CONTINUATION_BACKUP_STACK_FRAME(&foo_cont.cont, foo_cont.stack_frame);
 * @endcode
 */
#define CONTINUATION_RESTORE_STACK_FRAME(cont_stub, stack_frame) \
do { \
  assert(((struct __ContinuationStub *)cont_stub)->cont->initialized && "XXX_RESTORE_STACK_FRAME is only available after initialized"); \
  assert((size_t)(stack_frame) < (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail \
    || (size_t)(stack_frame) > (size_t)((struct __ContinuationStub *)cont_stub)->cont->stack_frame_tail + ((struct __ContinuationStub *)cont_stub)->cont->stack_frame_size); \
  *((struct __ContinuationStub **)&cont_stub) = continuation_restore_stack_frame((const struct __ContinuationStub *)cont_stub, stack_frame); \
} while (0)

/** @cond */
/**
 * @name Miscellaneous
 * @{
 */
/**
 * @internal
 * @brief Internal help macro for CONTINUATION_CONNECT().
 * 
 * @details To calculates the size of the stack frame, determines the growth direction of stack
 * and the variable arrangement in stack frame, etc.
 * 
 * @param cont_stub: pointer to the local continuation stub variable.
 * @param stack_frame_spot_addr: an anchor address changes according to the address of stack frame.
 * 
 * @see CONTINUATION_CONNECT()
 */
#define CONTINUATION_INIT_INVOKE(cont_stub, stack_frame_spot_addr) /* Empty definition for Doxygen */
#undef CONTINUATION_INIT_INVOKE

#if defined(CONTINUATION_STACK_FRAME_SIZE)
# define __CONTINUATION_DEFINED_STACK_FRAME_SIZE 1
#else
# define __CONTINUATION_DEFINED_STACK_FRAME_SIZE 0
#endif
#if defined(CONTINUATION_STACK_FRAME_REVERSE)
# define __CONTINUATION_DEFINED_STACK_FRAME_REVERSE 1
# define __CONTINUATION_IS_STACK_FRAME_REVERSE CONTINUATION_STACK_FRAME_REVERSE
#else
# define __CONTINUATION_DEFINED_STACK_FRAME_REVERSE 0
# define __CONTINUATION_IS_STACK_FRAME_REVERSE 0 
#endif
#if defined(CONTINUATION_EXTEND_STACK_FRAME)
# define __CONTINUATION_DEFINED_EXTEND_STACK_FRAME 1
#else
# define __CONTINUATION_DEFINED_EXTEND_STACK_FRAME 0
#endif
#ifdef CONTINUATION_DEBUG
# if defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ >= 8
#   if defined(_WIN64) /* MSC or MINGW */
#     define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a stack frame of %lld bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__)
#     define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a reverse stack frame of indeterminable size, which must be greater than %lld bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->offset_to_frame_tail, __FILE__, __LINE__)
#   else
#     define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a stack frame of %zd bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__)
#     define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a reverse stack frame of indeterminable size, which must be greater than %zd bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->offset_to_frame_tail, __FILE__, __LINE__)
#   endif
# else
#   define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a stack frame of %d bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->stack_frame_size, __FILE__, __LINE__)
#   define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub) \
  printf("[CONTINUATION_DEBUG] the continuation has a reverse stack frame of indeterminable size, which must be greater than %d bytes, at: file \"%s\", line %d\n" \
            , (cont_stub)->cont->offset_to_frame_tail, __FILE__, __LINE__)
# endif
#else
# define __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub)
# define __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub)
#endif

/* should not use any function call in CONTINUATION_INIT_INVOKE or be forced inline. */
#define CONTINUATION_INIT_INVOKE(cont_stub, stack_frame_spot_addr) \
do { \
  char *stack_frame_tail; \
  assert((cont_stub)->cont->stack_frame_tail != NULL); \
  if ((cont_stub)->cont->stack_frame_addr != NULL && (cont_stub)->cont->invoke != __continuation_init_invoke_stub) { \
    /* frame address specified by CONTINUATION_CONSTRUCT of the compiler config, e.g. gcc's __builtin_frame_address() */ \
    stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL); \
    if ((cont_stub)->cont->stack_frame_tail > stack_frame_tail) (cont_stub)->cont->stack_frame_tail = stack_frame_tail; \
    (cont_stub)->cont->stack_frame_size = (cont_stub)->cont->stack_frame_addr - (cont_stub)->cont->stack_frame_tail; \
    __CONTINUATION_STACK_FRAME_SIZE_DEBUG(cont_stub); \
    assert((cont_stub)->cont->stack_frame_addr + (cont_stub)->cont->stack_parameters_size >= (cont_stub)->addr.stack_frame_addr \
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
    char *reserved_stack_frame_addr; \
    if ((cont_stub)->cont->stack_frame_addr == NULL) { \
      reserved_stack_frame_addr = (cont_stub)->addr.stack_frame_addr; \
      stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL); \
      if ((cont_stub)->cont->stack_frame_tail > stack_frame_tail) (cont_stub)->cont->stack_frame_tail = stack_frame_tail; \
      if ((cont_stub)->addr.stack_frame_addr < (char *)&reserved_stack_frame_addr) { \
        (cont_stub)->addr.stack_frame_addr  = (char *)&reserved_stack_frame_addr + sizeof(reserved_stack_frame_addr); \
      } \
      if ((cont_stub)->cont->stack_frame_tail > (char *)&reserved_stack_frame_addr) { \
        (cont_stub)->cont->stack_frame_tail = (char *)&reserved_stack_frame_addr; \
      } \
      (cont_stub)->cont->offset_to_frame_tail = (cont_stub)->addr.stack_frame_addr - (cont_stub)->cont->stack_frame_tail; \
      (cont_stub)->cont->invoke = __continuation_init_invoke_stub; \
      if (continuation_stub_setjmp((cont_stub)->return_buf) == 0) { \
        __continuation_invoke_helper(cont_stub); \
      } \
    } else { \
      static void (*volatile continuation_init_invoke_return)(struct __ContinuationStub *, const void *stack_frame_spot) = __continuation_init_invoke_return; \
      continuation_init_invoke_return(cont_stub, stack_frame_spot_addr); \
    } \
    if ((size_t)(cont_stub)->cont->stack_frame_tail == (size_t)(cont_stub)->addr.stack_frame_tail - (cont_stub)->size.stack_frame_offset) { \
      __CONTINUATION_STACK_FRAME_REVERSE_DEBUG(cont_stub); \
      /* reverse stack frame detected */ \
      if ((cont_stub)->cont->stack_frame_size == 0) { \
        BOOST_PP_IF(__CONTINUATION_DEFINED_STACK_FRAME_SIZE \
          , BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() CONTINUATION_STACK_FRAME_SIZE + (cont_stub)->cont->stack_parameters_size >= (cont_stub)->cont->offset_to_frame_tail \
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
        BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (cont_stub)->cont->stack_frame_size + (cont_stub)->cont->stack_parameters_size >= (cont_stub)->cont->offset_to_frame_tail  \
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
      assert(reserved_stack_frame_addr <= (cont_stub)->cont->stack_frame_addr + (cont_stub)->cont->stack_parameters_size \
              && "[CONTINUATION FAULT] the deduced stack frame with parameters is not enough for reserved variables/address, try to increase the CONTINUATION_STACK_PARAMETERS_SIZE"); \
      assert((cont_stub)->cont->stack_frame_size + (cont_stub)->cont->stack_parameters_size >= (cont_stub)->cont->offset_to_frame_tail \
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

/**
 * @internal
 * @fn __continuation_init_invoke_stub()(struct __ContinuationStub *cont_stub)
 * @brief Internal help function to invoke the continuation when it is connected.
 * @param cont_stub: pointer to the local continuation stub.
 */

#if defined(CONTINUATION_STACK_FRAME_SIZE)
/**
 * @internal
 * @fn __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub)
 * @brief Internal help function to invoke the continuation when the stack frame size is specified statically.
 * @param cont_stub: pointer to the local continuation stub.
 */
#endif

/**
 * @internal
 * @fn __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub)
 * @brief Internal help function to invoke the continuation when the stack frame can be allocated dynamically.
 * @param cont_stub: pointer to the local continuation stub.
 */

/**
 * @internal
 * @fn __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub)
 * @brief Internal help function to __continuation_dynamic_invoke_stub() when stack frame is reversed.
 * @details The reversed stack frame can be extended by calling itself recursively.
 * @param cont_stub: pointer to the local continuation stub.
 */

#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void __continuation_init_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
# if defined(CONTINUATION_STACK_FRAME_SIZE)
  static void __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
# endif
  static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
  static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub) __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void __continuation_init_invoke_stub(struct __ContinuationStub *cont_stub);
# if defined(CONTINUATION_STACK_FRAME_SIZE)
  static void __continuation_static_invoke_stub(struct __ContinuationStub *cont_stub);
# endif
  static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub);
  static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub);
#endif

#if !CONTINUATION_USE_LONGJMP
/**
 * @internal
 * @brief Internal help function to evaluate the stack shift when invoking a continuation directly.
 * @param init_null: pointer to a continuation structure when be recursive called internally, or NULL is desired.
 * @return offset of frame address between stack frames.
 */
static int __continuation_invoke_frame_tail_offset(void *init_null)
{
  static int(* volatile continuation_invoke_frame_tail_offset)(void *) = __continuation_invoke_frame_tail_offset;
  static int frame_tail_offset = -1;

  struct __ContinuationStubFrameTail {
    struct __ContinuationStub cont_stub;
    void *frame_tail;
  };
  struct __ContinuationStubFrameTail *arg;
  arg = (struct __ContinuationStubFrameTail *)init_null;
  (void)STATIC_ASSERT_OR_ZERO(offsetof(struct __ContinuationStubFrameTail, cont_stub) == 0
          , self_constraint_of_struct_ContinuationStubFrameTail);

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
      frame_tail_offset = (size_t)arg->frame_tail - (size_t)__continuation_init_frame_tail(NULL, NULL);
      CONTINUATION_DESTRUCT(arg->cont_stub.cont);
      continuation_stub_longjmp(arg->cont_stub.return_buf, 1);
    }
  } else {
    arg->frame_tail = __continuation_init_frame_tail(NULL, NULL);
    if (continuation_stub_setjmp(arg->cont_stub.return_buf) == 0) {
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
  volatile char stack_frame[CONTINUATION_STACK_FRAME_SIZE + 2 * CONTINUATION_STACK_FRAME_PADDING];
/*
 * The new gcc makes the stack frame base pointer less than 128 + stack_frame pointer,
 * so force a dynamic stack allocation with variable length array.
 */
# if defined(__GNUC__) && __GNUC__ >= 7
  volatile char temp[cont_stub->cont->stack_frame_size + CONTINUATION_STACK_FRAME_SIZE  + CONTINUATION_STACK_FRAME_PADDING];
# endif
  CONTINUATION_STUB_INVOKE(cont_stub);
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
# if defined(__GNUC__) && __GNUC__ >= 7
  temp[0] = 0;
# endif
  assert(0 && "error: don't use keyword 'return' inside continuation");
} /* __continuation_static_invoke_stub */
#endif

#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE) \
  || CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
static void __continuation_dynamic_invoke_stub(struct __ContinuationStub *cont_stub)
{
/*
 * GCC 7 set stack frame base pointer to stack frame pointer + 128 mostly,
 * so relocate the stack frame base pointer by increasing the size of stack frame statically.
 */
# if defined(__GNUC__) && __GNUC__ >= 7 || defined(__x86_64__) || defined(_WIN64)
  volatile char temp[256];
# endif
#if CONTINUATION_USE_C99_VLA \
  || CONTINUATION_USE_ALLOCA \
  || defined(CONTINUATION_EXTEND_STACK_FRAME)
# if defined(CONTINUATION_DEBUG)
  char *stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL);
# endif
#endif
#if CONTINUATION_USE_C99_VLA
  volatile char stack_frame[cont_stub->cont->stack_frame_size + CONTINUATION_STACK_FRAME_PADDING];
#elif CONTINUATION_USE_ALLOCA
  volatile char *stack_frame = (char *)alloca(cont_stub->cont->stack_frame_size + CONTINUATION_STACK_FRAME_PADDING);
#elif defined(CONTINUATION_EXTEND_STACK_FRAME)
  volatile char *stack_frame;
  CONTINUATION_EXTEND_STACK_FRAME(stack_frame, cont_stub->cont->stack_frame_size + CONTINUATION_STACK_FRAME_PADDING);
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
  BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (size_t)stack_frame_tail >= (size_t)__continuation_init_frame_tail(NULL, NULL) + cont_stub->cont->stack_frame_size
      && BOOST_PP_IF(CONTINUATION_USE_C99_VLA, "c99 VLA stack frame extend facility compliance test failed"
          , BOOST_PP_IF(CONTINUATION_USE_ALLOCA, "alloca() stack frame extend facility compliance test failed"
            , "compiler specified CONTINUATION_EXTEND_STACK_FRAME facility compliance test failed")) BOOST_PP_RPAREN());
# endif
#endif
  CONTINUATION_STUB_INVOKE(cont_stub);
  FORCE_NO_OMIT_FRAME_POINTER();
  stack_frame[0] = 0;
# if defined(__GNUC__) && __GNUC__ >= 7 || defined(__x86_64__) || defined(_WIN64)
  temp[0] = 0;
# endif
  assert(0 && "error: don't use keyword 'return' inside continuation");
} /* __continuation_dynamic_invoke_stub */
#endif

#if (!defined(CONTINUATION_STACK_FRAME_REVERSE) || CONTINUATION_STACK_FRAME_REVERSE)
static void __continuation_recursive_invoke_stub(struct __ContinuationStub *cont_stub)
{
  static void(* volatile continuation_recursive_invoke)(struct __ContinuationStub *) = &__continuation_recursive_invoke_stub;
  volatile char stack_frame[CONTINUATION_STACK_BLOCK_SIZE];
  if ((size_t)(cont_stub->addr.stack_frame_addr - &stack_frame[0]) < cont_stub->size.stack_frame_size + CONTINUATION_STACK_FRAME_PADDING) {
    if (cont_stub->cont->invoke == __continuation_init_invoke_stub) {
      cont_stub->cont->stack_frame_addr = (char *)&stack_frame[0];
    }
    continuation_recursive_invoke(cont_stub);
  }
  if (cont_stub->cont->invoke == __continuation_init_invoke_stub) {
    cont_stub->addr.stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL);
#if !CONTINUATION_USE_LONGJMP
    {
      static int frame_tail_offset = -1;
      if (frame_tail_offset == -1) {
        static int(* volatile continuation_invoke_frame_tail_offset)(void *) = __continuation_invoke_frame_tail_offset;
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
  volatile size_t dynamic_frame_size; /* use volatile to prevent optimization in the following calculation */
#   if defined(CONTINUATION_DEBUG)
  char *stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL);
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
    BOOST_PP_EXPAND(assert BOOST_PP_LPAREN() (size_t)stack_frame_tail >= (size_t)__continuation_init_frame_tail(NULL, NULL) + dynamic_frame_size
      && BOOST_PP_IF(CONTINUATION_USE_C99_VLA, "c99 VLA stack frame extend facility compliance test failed"
          , BOOST_PP_IF(CONTINUATION_USE_ALLOCA, "alloca() stack frame extend facility compliance test failed"
            , "compiler specified CONTINUATION_EXTEND_STACK_FRAME facility compliance test failed")) BOOST_PP_RPAREN());
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
    cont_stub->addr.stack_frame_tail = (char *)__continuation_init_frame_tail(NULL, NULL);
#if !CONTINUATION_USE_LONGJMP
    {
      static int frame_tail_offset = -1;
      if (frame_tail_offset == -1) {
        static int(* volatile continuation_invoke_frame_tail_offset)(void *) = __continuation_invoke_frame_tail_offset;
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
/** @endcond */
/** @} Miscellaneous */

#endif /* __CONTINUATION_H */
