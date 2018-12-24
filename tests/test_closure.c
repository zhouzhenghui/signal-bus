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
  CLOSURE(const char *) closure;
  CLOSURE(int) closure_sum;

  setbuf(stdout,NULL);
  printf("A simple closure\n");

  CLOSURE_INIT(&closure);

  CLOSURE_CONNECT(&closure
    , (/* empty initialization */)
    , (
      /* continuation */
      printf("%s\n", CLOSURE_ARG_OF_(&closure)->_1);
    )
    , (/* empty finalization */)
  );

  CLOSURE_RUN(&closure, "Hello World!");

  CLOSURE_FREE(&closure);

  printf("\nA improved closure with retained variables\n");

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
      CLOSURE_RUN(&closure_sum, i);
    }
  }

  CLOSURE_FREE(&closure_sum);

  return 0;
}
