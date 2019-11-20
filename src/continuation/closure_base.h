/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CLOSURE_BASE_H
#define __CLOSURE_BASE_H

/**
 * @file
 * @ingroup closure
 * @brief The basic declarations for closure.
 */

#include "continuation_base.h"
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <preprocessor/variadic_size_or_zero.h>

/**
 * @internal
 * @brief Represent a variable captured by a closure.
 */
struct __ClosureVar {
  void *addr; /**< address of the variable. */
  size_t size; /**< size of the variable. */
  void *value; /**< pointer to the retained value of the variable. */
};

/**
 * @internal
 * @brief Debug version of struct __ClosureVar.
 * @see struct __ClosureVar
 */
struct __ClosureVarDebug {
  const char *name; /**< pointer to name string of the variable. */
  void *addr; /**< address of the variable. */
  size_t size; /**< size of the variable. */
  void *value; /**< pointer to the storage of the variable in backup stack frame. */
};

/**
 * @internal
 * @brief Type of array of captured variables.
 */
typedef VECTOR(struct __ClosureVar) __ClosureVarVector;
/**
 * @internal
 * @brief Type of array of captured variables in debug mode.
 */
typedef VECTOR(struct __ClosureVarDebug) __ClosureVarDebugVector;

/**
 * @internal
 * @brief The closure structure.
 * @details It is the underlying structure of CLOSURE().
 * @see CLOSURE()
 */
struct __Closure {
  struct __Continuation cont; /**< the continuation structure of closure. */
  int connected; /**< indicates the closure is connected or not. */
#ifdef CLOSURE_DEBUG
  __ClosureVarDebugVector argv;
#else
  __ClosureVarVector argv;
#endif /**< array of captured variables. */
  char *frame; /**< storage for backup stack frame of continuation. */
};

/**
 * @internal
 * @brief The stub structure for invoking a closure.
 */
struct __ClosureStub {
  struct __ContinuationStub cont_stub; /**< the continuation stub. */
  struct __Closure *closure; /**< pointer to the closure. */
};

/** @cond */
STATIC_ASSERT(offsetof(struct __ClosureStub, cont_stub) == 0, internal_constraint_of_struct_ClosureStub_failed);
/** @endcond */

/**
 * @internal
 * @brief Initialize a internal closure structure.
 * @details The closure are marked as unconnected.
 * @param closure: pointer to the closure.
 * @see CLOSURE_INIT()
 */
inline static void __closure_init(struct __Closure *closure)
{
  closure->connected = 0;
  /* VECTOR_INIT(&closure->argv); */
  /* closure->frame = NULL; */
}

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @internal
 * @brief Internal help function to invoke a closure.
 */
  extern void __closure_invoke(struct __Closure *closure);
  /**
   * @internal
   * @brief Internal help function to CLOSURE_CONNECT().
   */
  extern void __closure_init_vars(struct __Closure *closure, __ClosureVarVector *argv);
  /**
   * @internal
   * @brief Internal help function to CLOSURE_CONNECT().
   */
  extern void __closure_init_vars_debug(struct __Closure *closure, __ClosureVarDebugVector *argv, const char *file, unsigned int line);
  /**
   * @internal
   * @brief Internal help function to CLOSURE_CONNECT().
   */
  extern void __closure_commit_vars(__ClosureVarVector *argv, size_t stack_frame_offset);
  /**
   * @internal
   * @brief Internal help function to CLOSURE_CONNECT().
   */
  extern void __closure_commit_vars_debug(__ClosureVarDebugVector *argv, size_t stack_frame_offset, const char *file, unsigned int line);
#ifdef __cplusplus
}
#endif

/**
 * @internal
 * @brief Call a closure.
 * @details It is the underlying function of CLOSURE_RUN().
 * @param closure: pointer to the closure.
 * @see CLOSURE_RUN()
 */
inline static void __closure_run(struct __Closure *closure)
{
  if (closure->connected) {
    __closure_invoke(closure);
  }
}

/**
 * @internal
 * @brief Free a closure.
 * @details It is the underlying function of CLOSURE_FREE().
 * @param closure: pointer to the closure.
 * @warning It is defined in header for including the platform dependent implementation of CONTINUATION_DESTRUCT().
 * @see CLOSURE_FREE()
 */
inline static void __closure_free(struct __Closure *closure)
{
  if (closure->connected) {
    closure->connected = 0;
    __closure_invoke(closure);
    CONTINUATION_DESTRUCT(&closure->cont);
    VECTOR_FREE(&closure->argv);
    free(closure->frame);
  }
}

#endif /* __CLOSURE_BASE_H */
