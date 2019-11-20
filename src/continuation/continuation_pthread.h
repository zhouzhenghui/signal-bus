/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_PTHREAD_H
#define __CONTINUATION_PTHREAD_H

/**
 * @defgroup continuation_pthread continuation of pthread
 * @ingroup continuation
 * @brief Continuation implementation compatible with pthread as well as an asynchronous continuation implementation based on pthread.
 * @see continuation
 * 
 * @{
 */

/**
 * @file
 * @brief The head file for continuation implementation compatible to pthread.
 * @note Include only or firstly for any continuation based implementations if pthread is involved.
 */

#if !defined(HAVE_PTHREAD)
# if defined(__CONTINUATION_H)
#  error "should just include the \"continuation_pthread.h\" for asynchronous facility implementation under pthread specification"
# endif
# define HAVE_PTHREAD 1
#endif

/**
 * @name Continuation variable names
 * These names will intrude into the user's namespace.
 * @{
 */
/**
 * @brief Name of a asynchronous task/continuation used by the library.
 */
#define __ASYNC_TASK __async_task
/** @} */

#include <pthread.h>
#include "continuation.h"

/**
 * @name External variables
 * @{
 */
/**
 * @internal
 * @brief Index of TLS storage that holds the asynchronous continuation internally.
 * @see struct __AsyncTask
 */
extern pthread_key_t __async_pthread_key;
/** @} */

/**
 * @internal
 * @brief Structure type represents the continuation that runs asynchronous.
 */
struct __AsyncTask {
  struct __ContinuationStub cont_stub; /**< the continuation stub. */
  struct __Continuation cont; /**< the continuation. */
  int quitable; /**< indicated the thread can quit or not. */
  pthread_mutex_t mutex; /**< pthread mutex for synchronization between host function and continuation. */
  pthread_cond_t running; /**< pthread condition variable. */
};

/** @cond */
STATIC_ASSERT(offsetof(struct __AsyncTask, cont_stub) == 0, self_contraint_of_inheritance_hierarchy_of_struct_AsyncTask_failed);
/** @endcond */

/**
 * @name Function pointers
 * Pointers to the functions that should not be inlined.
 * @{
 */
/**
 * @internal
 * @brief Internal help function to copy the stack frame from the parent thread.
 * @details It is a internal help function for ASYNC_RUN(). 
 * @param async_task: pointer to the asynchronous task/continuation.
 * @return \p async_task: the missing \p async_task the value of callee may be overwritten by the function itself.
 * @see ASYNC_RUN()
 */
extern struct __AsyncTask *(*__async_copy_stack_frame)(struct __AsyncTask *);
/**@}*/

#ifdef __cplusplus
extern "C" {
#endif
  /**
   * @internal
   * @brief Internal help function to create a pthread routine.
   * @details The pthread routine is used for running the task/continuation asynchronously.
   * @return the pthread_t type id of the thread.
   */
  extern pthread_t __async_pthread_create();
#ifdef __cplusplus
} /* extern "C" */
#endif

/** @cond */
#define __ASYNC_RUN(continuation) \
    __async_pthread_create(); \
    { \
      struct __AsyncTask *__ASYNC_TASK = (struct __AsyncTask *)pthread_getspecific(__async_pthread_key); \
      assert(__ASYNC_TASK != NULL); \
      CONTINUATION_CONNECT(&__ASYNC_TASK->cont, __ASYNC_TASK \
        , () \
        , ( \
            __ASYNC_TASK = __async_copy_stack_frame(__ASYNC_TASK); \
            pthread_cond_signal(&__ASYNC_TASK->running); \
            { \
              __PP_REMOVE_PARENS(continuation); \
            } \
            CONTINUATION_DESTRUCT(&__ASYNC_TASK->cont); \
        ) \
      ); \
      pthread_cond_wait(&__ASYNC_TASK->running, &__ASYNC_TASK->mutex); \
      __ASYNC_TASK->quitable = 1; \
      pthread_cond_signal(&__ASYNC_TASK->running); \
      pthread_mutex_unlock(&__ASYNC_TASK->mutex); \
    }
/** @endcond */

/**
 * @brief Run a statements block asynchronous through pthread.
 * 
 * @details It evaluates as a expression of pthread_t type that is the
 * id of the continuation thread.
 * 
 * @param ...: the statements to run asynchronously.
 * 
 * @warning If variadic macro isn't supported, the statements block
 * should not contains any "," operators outside of any semantic parentheses.
 * 
 * @see __ASYNC_RUN()
 * 
 * @par Example:
 * @code
 *  pthread_t pthread_id = ASYNC_RUN(...);
 *  ...
 *  pthread_join(pthread_id);
 * @endcode
 */
#define ASYNC_RUN() /* Empty defintion for Doxygen */
#undef ASYNC_RUN

/** @cond */
#if BOOST_PP_VARIADICS
# define ASYNC_RUN(...) __ASYNC_RUN((__VA_ARGS__))
#else
# define ASYNC_RUN __ASYNC_RUN
#endif
/** @endcond */

/**
 * @brief Get pointer to a local variable in the parent thread.
 * @param v: name of the variable.
 */
#define ASYNC_HOST_VAR_ADDR(v) \
  CONTINUATION_HOST_VAR_ADDR(&__ASYNC_TASK->cont_stub, v)

/**
 * @brief Get the reference to a local variable in the parent thread.
 * @param v: name of the variable.
 */
#define ASYNC_HOST_VAR(v) \
  CONTINUATION_HOST_VAR(&__ASYNC_TASK->cont_stub, v)

#endif /* __CONINUATION_PTHREAD_H */
