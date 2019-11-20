/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_BASE_H
#define __CONTINUATION_BASE_H

/**
 * @file
 * @ingroup continuation
 * @brief The basic declarations for continuation.
 */

#include "continuation_config.h"
#include "misc/no_omit_frame_pointer.h"
#include "misc/continuation_inline.h"
#include "misc/vector.h"

struct __ContinuationStub;

/**
 * @internal
 * @brief Structure type represents a captured continuation.
 * 
 * @see struct __ContinuationStub()
 * @see CONTINUATION_CONNECT()
 */
struct __Continuation {
  int initialized; /**< is the continuation connected or not. */
  char *stack_frame_addr; /**< minimal address of stack frame. */
  const char *stack_frame_spot; /**< an anchor address in the stack frame. */
  char *stack_frame_tail; /**< tail/maximum address of stack frame. */
  size_t stack_frame_size; /**< size of the stack frame. */
  size_t stack_parameters_size; /**< parameters size of the host function. */
  size_t offset_to_frame_tail; /**< maximum offset of any addresses in the stack frame to the \p stack_frame_tail. */
  void(*invoke)(struct __ContinuationStub *); /**< pointer to the invocation stub of the continuation. */
  void *func_addr; /**< entry address of the continuation. */
  jmp_buf invoke_buf; /**< jmp_buf for longjmp() of the continuation. */
};

/**
 * @internal
 * @brief Structure type represents a continuation when it is invoked.
 * 
 * @see struct __Continuation()
 * @see CONTINUATION_CONNECT()
 */
struct __ContinuationStub {
  struct __Continuation *cont; /**< pointer to the continuation. */
  union {
    char *stack_frame_addr;
    char *stack_frame_tail;
  } addr; /**< some pointers of stack frame. */
  union {
    size_t stack_frame_size;
    ptrdiff_t stack_frame_offset;
  } size; /**< some sizes of stack frame. */
  jmp_buf return_buf; /**< jmp_buf for longjmp() to return to the caller. */
};

/**
 * @brief Initialize a continuation.
 * 
 * @param cont: pointer to the continuation structure.
 * @param stack_frame_spot: an anchor address in the stack frame of the continuation.
 * 
 * @see CONTINUATION_CONNECT()
 */
inline static void continuation_init(struct __Continuation *cont, const void *stack_frame_spot)
{
  cont->initialized = 0;
  cont->stack_frame_addr = NULL;
  cont->stack_frame_tail = NULL;
  cont->stack_frame_size = 0;
  cont->stack_parameters_size = CONTINUATION_STACK_PARAMETERS_SIZE;
  cont->stack_frame_spot = (const char *)stack_frame_spot;
  cont->invoke = NULL;
}

/**
 * @name Function pointers
 * Pointers to the functions that should not be inlined.
 * @{
 */
/**
 * @internal
 * @brief Internal helper for CONTINUATION_ENFORCE_VAR().
 * @details It is a pointer to a dummy function to enforce that a variable is stored in the stack.
 * @param addr: address of the variable.
 * @see CONTINUATION_ENFORCE_VAR()
 */
extern void (*__continuation_enforce_var)(char * volatile);

/**
 * @internal
 * @brief Internal helper for continuation invoking.
 * @details It is a pointer to a wrapper of \p invoke of struct __Continuation,
 * that simulates a safe function call to the continuation by pre-extending
 * the stack frame and bypassing security guard of some compilations.
 *
 * The function pointer is used to prevent inlining calls.
 *
 * @param cont_stub: pointer to the continuation stub.
 * @see continuation_invoke()
 * @see continuation_stub_invoke()
 */
extern void (*__continuation_invoke_helper)(struct __ContinuationStub *);

/**
 * @internal
 * @brief Internel helper for retruning from CONTINUATION_INIT_INVOKE().
 * @details It is a pointer to a wrapper of longjmp() to ensure it is safety to
 * return from continuation when the size of stack frame is unknown.
 *
 * The function pointer is used to prevent inlining calls.
 *
 * @param cont_stub: pointer to the continuation stub.
 * @param stack_frame_spot: an anchor address in the stack frame of continuation.
 * @return \p cont_stub itself.
 * @see CONTINUATION_INIT_INVOKE()
 */
extern void (*__continuation_init_invoke_return)(struct __ContinuationStub *, const void *);

/**
 * @internal
 * @brief Internal helper for initializing the stack frame pointer.
 * @details It is a pointer to a function to get the frame pointer/tail of stack frame.
 *
 * The function pointer is used to prevent inlining calls.
 *
 * @param a_null_ptr: a pointer equals NULL when calls.
 * @param a_dummy_ptr: a dummy pointer to make the prototype is compatible to continuation_restore_stack_frame().
 * @return frame pointer/tail of the stack frame.
 * @see CONTINUATION_CONNECT()
 * @see CONTINUATION_INIT_INVOKE()
 */
extern void *(*__continuation_init_frame_tail)(void *, void *);

/**
 * @brief Help function to CONTINUATION_RESTORE_STACK_FRAME().
 * @details It is a pointer to function that restore the stack frame of a continuation.
 * from the backup storage.
 *
 * The function pointer is used to prevent inlining calls.
 *
 * @param cont_stub: pointer to the continuation stub.
 * @param stack_frame: pointer to the storage of backup stack frame.
 * @return the missing \p cont_stub since the value of callee may be overwritten.
 * @see CONTINUATION_RESTORE_STACK_FRAME()
 * @see CONTINUATION_BACKUP_STACK_FRAME()
 */
extern struct __ContinuationStub *(*continuation_restore_stack_frame)(const struct __ContinuationStub *cont_stub, void *stack_frame);
/** @} */

/**
 * @brief Help function to CONTINUATION_BACKUP_STACK_FRAME().
 * @details Copy the stack frame of a continuation to the backup storage. 
 * 
 * @param cont: pointer to the continuation.
 * @param stack_frame: pointer to the backup storage.
 *
 * @note The stack frame can be only backup at the moment
 * a continuation is connected in the host function.
 * 
 * @see CONTINUATION_BACKUP_STACK_FRAME()
 * @see CONTINUATION_RESTORE_STACK_FRAME()
 */
inline static void continuation_backup_stack_frame(const struct __Continuation *cont, void *stack_frame)
{
  memcpy(stack_frame, cont->stack_frame_tail, cont->stack_frame_size);
}

/**
 * @brief Initialize a continuation stub.
 * 
 * @param cont_stub: pointer to the continuation stub.
 * @param cont: pointer to the continuation.
 */
inline static void continuation_stub_init(struct __ContinuationStub *cont_stub, struct __Continuation *cont)
{
  cont_stub->cont = cont;
}

/**
 * @brief Invoke a continuation through continuatin stub.
 * 
 * @param cont_stub: pointer to the continuation stub.
 * 
 * @see continuation_invoke()
 * @see __continuation_invoke_helper()
 */
inline static void continuation_stub_invoke(struct __ContinuationStub *cont_stub)
{
  if (continuation_setjmp(cont_stub->return_buf) == 0) {
    __continuation_invoke_helper(cont_stub);
  }
}

/**
 * @brief Invoke a continuation.
 * 
 * @param cont: pointer to the continuation.
 * 
 * @see continuation_stub_init()
 * @see continuation_stub_invoke()
 * @see __continuation_invoke_helper()
 */
inline static void continuation_invoke(struct __Continuation *cont)
{
  struct __ContinuationStub cont_stub;
  continuation_stub_init(&cont_stub, cont);
  continuation_stub_invoke(&cont_stub);
}

/**
 * @internal
 * @brief Internal function for CONTINUATION_RESERVE_FRAME_ADDR().
 *
 * @see CONTINUATION_RESERVE_FRAME_ADDR()
 * @see CONTINUATION_RESERVE_VAR()
 */
inline static void __continuation_reserve_frame_addr(struct __ContinuationStub *cont_stub, void *addr, size_t size)
{
  char * volatile anti_optimize = (char *)addr;
  if (cont_stub->addr.stack_frame_addr < anti_optimize) {
    cont_stub->addr.stack_frame_addr = anti_optimize + size;
  } else {
    if (cont_stub->cont->stack_frame_tail > anti_optimize) {
      cont_stub->cont->stack_frame_tail = anti_optimize;
    }
  }
}

/**
 * @internal
 * @brief Determine whether a local variable resides in the stack frame of a continuation.
 * 
 * @details It is a help function for CONTINUATION_ASSERT_VAR(), CONTINUATION_ASSERT_VARS(), etc.
 * 
 * @param cont_stub: pointer to the local continuation stub variable.
 * @param addr: pointer to the local variable.
 * @param size: size of the local variable.
 * @return boolean value to indicate is or not.
 *
 * @see CONTINUATION_ASSERT_VAR()
 * @see CONTINUATION_ASSERT_VARS()
 * @see CONTINUATION_ASSERT_VARS_N()
 * @see CONTINUATION_ASSERT_VARSn()
 */
inline static int __continuation_variable_in_stack_frame(const struct __ContinuationStub *cont_stub, const volatile void * volatile addr, size_t size)
{
  return ((size_t)addr + size <= (size_t)cont_stub->addr.stack_frame_tail + cont_stub->cont->stack_frame_size
           && (size_t)addr >= (size_t)cont_stub->addr.stack_frame_tail);
}

/**
 * @internal
 * @brief Destruct the continuation according to platform implementation.
 * @param closure_stub: pointer to the continuation.
 * @see CONTINUATION_DESTRUCT()
 */
inline static void continuation_destruct(struct __Continuation *cont) {
  CONTINUATION_DESTRUCT(cont);
}

#endif /* __CONTINUATION_BASE_H */
