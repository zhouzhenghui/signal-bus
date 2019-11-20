#include <stdio.h>

/*
  #define SLOT_STACK_FRAME_SIZE 32768
  #define SLOT_USE_LONGJMP 1
  #define SLOT_USE_C99_VLA 1
  #define SLOT_USE_ALLOCA 1
  #define SLOT_NO_FRAME_POINTER
*/

#define BOOST_PP_VARIADICS 1
#define CLOSURE_DEBUG

#include <continuation/closure.h>

int main()
{
  CLOSURE1(const char *) closure;
  CLOSURE1(int) closure_sum;

  setbuf(stdout,NULL);
  printf("A simple closure.\n");

  CLOSURE_INIT(&closure);

  CLOSURE_CONNECT(&closure
    , (/* empty initialization */)
    , (
      /* continuation */
      printf("Hello, %s!\n", CLOSURE_ARG_OF_(&closure)->_1);
    )
    , (
      /* finalization */
      printf("Goodbye, %s!\n",  CLOSURE_ARG_OF_(&closure)->_1);
    )
  );
  CLOSURE1_RUN(&closure, "Closure");
  CLOSURE_FREE(&closure);

  printf("A simple closure using the keyword-likely interface.\n");
  closure_init(&closure);
  closure_if (&closure) {
    printf("Hello, %s!\n", CLOSURE_ARG_OF_(&closure)->_1);
  } else {
    printf("Goodbye, %s!\n",  CLOSURE_ARG_OF_(&closure)->_1);
  }
  closure_run(&closure, "Closure");
  closure_free(&closure);

  printf("\nA improved closure with retained variables.\n");

  CLOSURE_INIT(&closure_sum);

  CLOSURE_CONNECT(&closure_sum
    , (
      /* initialization */
      int sum = 0;
      CLOSURE_RETAIN_VAR(sum);
    )
    , (
      /* continuation */
      sum += CLOSURE_ARG_OF_(&closure_sum)->_1;
    )
    , (
      /* finalization */
      printf("the sum result is: %d\n", sum);
    )
  );

  {
    int i;
    for (i = 1; i <= 10; ++i) {
      CLOSURE1_RUN(&closure_sum, i);
    }
  }

  CLOSURE_FREE(&closure_sum);

  int sum = 0;
  closure_if (&closure_sum) {
    sum += CLOSURE_ARG_OF_(&closure_sum)->_1;
    assert(CLOSURE_VAR(sum) != sum);
    CLOSURE_COMMIT_VAR(sum);
  } else {
    assert(CLOSURE_VAR(sum) == sum);
    printf("the sum result is: %d\n", sum);
  }

  {
    int i;
    for (i = 1; i <= 10; ++i) {
      CLOSURE1_RUN(&closure_sum, i);
    }
  }

  CLOSURE_FREE(&closure_sum);

  return 0;
}
