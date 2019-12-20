/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#include "continuation/continuation_base.h"

void(*__continuation_enforce_var)(char * volatile) = 0;

/** @internal */
/**
 * @brief internal function to __continuation_invoke_helper.
 * @details the function simulates a invocation to the continuation
 * and will never return.
 * @param cont_stub: pointer to a continuation stub of the continuation.
 * @note the function is assumed to be executed in a separate stack frame.
 * @see __continuation_invoke_helper
 */
static void continuation_invoke_helper(struct __ContinuationStub *cont_stub);
/**
 * @brief internal helper to continuation_invoke_helper().
 * @param cont_stub: pointer to a continuation stub of the continuation.
 * @param parameters_size: size of parameters space.
 * @note the function is assumed to be executed in a separate stack frame.
 * @see continuation_invoke_helper()
 */
static void __continuation_invoke_recursive(struct __ContinuationStub *cont_stub, size_t parameters_size);
/**
 * @brief internal function to __continuation_init_frame_tail.
 * @param a_null_ptr: a pointer equals NULL when calls.
 * @param a_dummy_ptr: a dummy pointer to make the prototype is compatible to continuation_restore_stack_frame().
 * @return frame pointer/tail of the stack frame.
 * @note the function is assumed to be executed in a separate stack frame.
 * @see __continuation_init_frame_tail
 */
static void*continuation_init_frame_tail(void *a_null_ptr, void *a_dummy_ptr);
/**
 * @brief internal function to continuation_restore_stack_frame.
 * @param cont_stub: pointer to the continuation stub.
 * @param stack_frame: pointer to the storage of the backup stack frame.
 * @return the missing \p cont_stub since the value in callee may be overwritten by the function itself.
 * @note the function is assumed to be executed in a separate stack frame.
 * @see continuation_restore_stack_frame
 */
static struct __ContinuationStub *__continuation_restore_stack_frame(const struct __ContinuationStub *cont_stub, void *stack_frame);
/** @endinternal */

/* these function pointers prevent link-time optimization */
void (*__continuation_invoke_helper)(struct __ContinuationStub *) = continuation_invoke_helper;
void*(*__continuation_init_frame_tail)(void *, void *) = continuation_init_frame_tail;
struct __ContinuationStub *(*continuation_restore_stack_frame)(const struct __ContinuationStub *cont_stub, void *stack_frame) = __continuation_restore_stack_frame;

void __continuation_patch_jmpbuf(int *continuation_jmpcode, jmp_buf *dst, jmp_buf *src)
{
  int i;
  for (i = 0; continuation_jmpcode[i] >= 0; i++) {
    ((void **)dst)[continuation_jmpcode[i]] = ((void **)src)[continuation_jmpcode[i]];
  }
}

static void __continuation_invoke_recursive(struct __ContinuationStub *cont_stub, size_t parameters_size)
{
  volatile char anti_optimize[1024];
  if (parameters_size > 1024) {
    __continuation_invoke_recursive(cont_stub, parameters_size - 1024);
  }
  cont_stub->cont->invoke(cont_stub);
  anti_optimize[0] = 0; /* prevent tail-call optimization */
}

/* helper to pre-extend the stack frame and bypass security cookie */
static void continuation_invoke_helper(struct __ContinuationStub *cont_stub)
{
  static void(* volatile continuation_invoke_recursive)(struct __ContinuationStub *, size_t) = &__continuation_invoke_recursive;
  volatile char anti_optimize[1024];
  cont_stub->addr.stack_frame_addr = (char *)&anti_optimize[0];
  if (cont_stub->cont->stack_parameters_size > 1024) {
#if __STDC_VERSION__ >= 199901L || CONTINUATION_USE_C99_VLA
    volatile char *parameters[cont_stub->cont->stack_parameters_size - 1024];
#elif defined(HAVE_ALLOCA) || CONTINUATION_USE_ALLOCA
    volatile char *parameters = (char *)alloca(cont_stub->cont->stack_parameters_size - 1024);
#elif defined(CONTINUATION_EXTEND_STACK_FRAME)
    volatile char *parameters;
    CONTINUATION_EXTEND_STACK_FRAME(parameters, cont_stub->cont->stack_parameters_size - 1024);
#else
    continuation_invoke_recursive(cont_stub, cont_stub->cont->stack_parameters_size - 1024);
#endif
  }
  cont_stub->cont->invoke(cont_stub);
  anti_optimize[0] = 0; /* prevent tail-call optimization */
}

/* I'd like to make the function prototype match with continuation_restore_stack_frame()
 * to ensure the calling of later will reserve the stack space of parameters
 * and return address, especially required for reversed stack frame.
 */
static void *continuation_init_frame_tail(void *null_ptr, void *frame_tail)
{
  static size_t continuation_frame_tail_forward = 0;
  void * volatile anti_optimization;
/*
  if (&frame_tail > &anti_optimization) {
    assert(&frame_tail > &null_ptr && "Wrong parameter order on stack, not RTL");
    anti_optimization = (void *)((size_t)&frame_tail + sizeof(frame_tail));
    return anti_optimization;
  }
*/
  if (continuation_frame_tail_forward == 0) {
    if (null_ptr) {
      continuation_frame_tail_forward = (size_t)null_ptr - (size_t)&anti_optimization;
      assert(continuation_frame_tail_forward > 0 && "TODO: hasn't yet support a stack which grows upward or strange");
      return NULL;
    } else {
      __continuation_init_frame_tail((void *)&anti_optimization, &frame_tail);
    }
  }
  frame_tail = (void *)&anti_optimization;
  if (frame_tail > (void *)&frame_tail) {
    frame_tail = (void *)&frame_tail;
  }
  if (frame_tail > (void *)&null_ptr) {
    frame_tail = (void *)&null_ptr;
  }
  frame_tail = (char *)frame_tail + continuation_frame_tail_forward;
#if defined(__x86_64__) || defined(_WIN64)
  /* make the stack frame 16 bytes aligned */
  frame_tail = (void *)((((size_t)frame_tail >> 4) - (!!((size_t)frame_tail % 16))) << 4);
#endif
  return frame_tail;
}

static struct __ContinuationStub *__continuation_restore_stack_frame(const struct __ContinuationStub *cont_stub, void *stack_frame)
{
  void * volatile stack_tail;
#if 0
  /*
   * Code to take the frame tail within the same function.
   * See continuation_init_frame_tail().
   */
  if (stack_frame == NULL) {
    static size_t continuation_frame_tail_forward = 0;
    if (continuation_frame_tail_forward == 0) {
      if (cont_stub) {
        continuation_frame_tail_forward = (size_t)cont_stub - (size_t)&stack_tail;
        assert(continuation_frame_tail_forward > 0 && "TODO: hasn't yet support a stack which grows upward or strange");
        return NULL;
      } else {
        __continuation_restore_stack_frame((void *)&stack_tail, NULL);
      }
    }
    stack_frame = (void *)&stack_tail;
    if (stack_frame > (void *)&stack_frame) {
      stack_frame = (void *)&stack_frame;
    }
    if (stack_frame > (void *)&cont_stub) {
      stack_frame = (void *)&cont_stub;
    }
    stack_frame = (char *)stack_frame + continuation_frame_tail_forward;
#if defined(__x86_64__) || defined(_WIN64)
    /* make the stack frame 16 bytes aligned */
    stack_frame = (void *)((((size_t)stack_frame >> 4) - (!!((size_t)stack_frame % 16))) << 4);
#endif
    return stack_frame;
  }
#endif /* mark unused code */
  assert((size_t)cont_stub->addr.stack_frame_tail > (size_t)&stack_tail && "Wrong frame tail in stack frame");
  memcpy(cont_stub->addr.stack_frame_tail, stack_frame, cont_stub->cont->stack_frame_size);
  return (struct __ContinuationStub *)cont_stub;
}

void continuation_stub_invoke(struct __ContinuationStub *cont_stub)
{
  assert(cont_stub->cont != NULL);
  if (continuation_stub_setjmp(cont_stub->return_buf) == 0) {
    __continuation_invoke_helper(cont_stub);
  }
}

void continuation_stub_return(struct __ContinuationStub *cont_stub)
{
  continuation_stub_longjmp(cont_stub->return_buf, 1);
}
