c-sigslot
=========

构建于源代码层次continuation和无锁并行设施上的通用的C/C++事件驱动，异步/并行基础架构。

Event-driven, universal asychronous/parallel infrastructure for C/C++, built on source level continuation and lock-free concurrency mechanism for C language.

通过提供closure，并行化coroutine和异步的signal-slot机制，极大的帮助了使用命令式语言书写异步和并行代码。

Several language level facilities are afforded, which help a lot in writing asychronous and parallel code in imperative language, including closure, concurrent coroutine and the asychronous signal-slot mechanism. 

Example
-------

A simple example of closure:

closure的简单示例：

```c
#include <stdio.h>
#include <continuation/closure.h>

int main()
{
  CLOSURE(char *) closure;

  CLOSURE_INIT(closure);
  
  CLOSURE_CONNECT(closure
    , (/* empty initialization */)
    , (
      /* continuation */
      printf("%s\n", CLOSURE_ARG_OF_(&closure)->_1);
    )
    , (/* empty finalization */)
  );
  
  CLOSURE_RUN(closure, "Hello World!");
  
  CLOSURE_FREE(closure);
  
  return 0;
}
```


