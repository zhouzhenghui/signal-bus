/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#include "continuation/continuation_base.h"

int __continuation_jmpbuf_initialized = 0;
int __continuation_jmpcode[sizeof(jmp_buf) / sizeof(void *)] = { 0 };
void(*__continuation_enforce_var)(char * volatile) = 0;

/* this function should not be compiled with any optimization */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
  static void continuation_diff_jmpbuf_help(jmp_buf *) __attribute__((optimize("no-omit-frame-pointer")));
  static void continuation_diff_jmpbuf(void) __attribute__((optimize("no-omit-frame-pointer")));
#else
  static void continuation_diff_jmpbuf_help(jmp_buf *);
  static void continuation_diff_jmpbuf(void);
#endif

static void continuation_invoke_helper(struct __ContinuationStub *cont_stub);
static void __continuation_invoke_recursive(struct __ContinuationStub *cont_stub, size_t parameters);
static void continuation_init_invoke_return(struct __ContinuationStub *cont_stub, const void *stack_frame_spot);
static void*continuation_init_frame_tail(void *, void *);
static struct __ContinuationStub *__continuation_restore_stack_frame(const struct __ContinuationStub *cont_stub, void *stack_frame);

/* these function pointers prevent link-time optimization */
void (*__continuation_diff_jmpbuf)(void) = continuation_diff_jmpbuf;
void (*__continuation_invoke_helper)(struct __ContinuationStub *) = continuation_invoke_helper;
void*(*__continuation_init_frame_tail)(void *, void *) = continuation_init_frame_tail;
// void*(*__continuation_init_frame_tail)(void *, void *) = (void *) __continuation_restore_stack_frame;
void (*__continuation_init_invoke_return)(struct __ContinuationStub *, const void *) = continuation_init_invoke_return;
struct __ContinuationStub *(*continuation_restore_stack_frame)(const struct __ContinuationStub *cont_stub, void *stack_frame) = __continuation_restore_stack_frame;

static void continuation_diff_jmpbuf_help(jmp_buf *environment)
{
  setjmp(*environment);
  FORCE_NO_OMIT_FRAME_POINTER();
}

static void continuation_diff_jmpbuf(void)
{
  static void(* volatile diff_jmpbuf_help)(jmp_buf *) = &continuation_diff_jmpbuf_help;
  volatile int jmpcode[sizeof(jmp_buf) / sizeof(void *)];
  jmp_buf jmpbuf1, jmpbuf2;
  if (setjmp(jmpbuf1) == 0) {
    memcpy((void *)&jmpbuf2, (void *)&jmpbuf1, sizeof(jmp_buf));
    if (setjmp(jmpbuf1) == 0) {
      int i, j = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          jmpcode[j++] = i;
        }
      }
      jmpcode[j] = -1;
      memcpy((void *)&jmpbuf1, (void *)&jmpbuf2, sizeof(jmp_buf));
    }
    longjmp(jmpbuf1, 1);
  } else {
    diff_jmpbuf_help(&jmpbuf1);
    {
      volatile int i, j = 0, k = 0;
      for (i = 0; i < sizeof(jmp_buf) / sizeof(void *); i++) {
        if (((void **)&jmpbuf1)[i] != ((void **)&jmpbuf2)[i]) {
          while(jmpcode[j] >= 0 && jmpcode[j] < i) j++;
          if (jmpcode[j] != i) {
            __continuation_jmpcode[k++] = i;
          }
        }
      }
      __continuation_jmpcode[k] = -1;
    }
  }
  __continuation_jmpbuf_initialized = 1;
  FORCE_NO_OMIT_FRAME_POINTER();
}

void __continuation_patch_jmpbuf(jmp_buf *dst, jmp_buf *src)
{
  int i;
  for (i = 0; __continuation_jmpcode[i] >= 0; i++) {
    ((void **)dst)[__continuation_jmpcode[i]] = ((void **)src)[__continuation_jmpcode[i]];
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

/* a proxy for stack frame pre-expand and security cookie bypass */
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

/* to ensure the longjmp can be finished when stack frame size is unknown */
static void continuation_init_invoke_return(struct __ContinuationStub *cont_stub, const void *stack_frame_spot)
{
  assert(cont_stub->cont->stack_frame_tail > (char *)stack_frame_spot + sizeof(void *) && "TODO: incompatible compilation/optimization");
  cont_stub->size.stack_frame_offset = (size_t)stack_frame_spot - (size_t)cont_stub->cont->stack_frame_spot;
  cont_stub->cont->stack_frame_addr -= cont_stub->size.stack_frame_offset;
  longjmp(cont_stub->return_buf, 1);
}

/* I'd like to make the function prototype match with continuation_restore_stack_frame
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
   * Code to make the frame tail within the same function.
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
