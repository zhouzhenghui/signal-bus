c-sigslot
=========

构建于源代码层次continuation和无锁并行设施上的通用的C/C++事件驱动，异步/并行基础架构。它底层使用一个无锁优先队列数据结构，从而可以不使用锁来书写并行化程序。 

Event-driven, universal asychronous/parallel infrastructure for C/C++, built on source level continuation and lock-free concurrency mechanism for C language. It use a universal lock-free priority queue as the foundation data-structure, so that one can programming concurrency without any lock. 

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
  
  return 0;
}
```
Outputs display:
Hello World!

**Closure vs. Callback**
The simple example shows that a closure is somthing like a callback, which is often defined as a function pointer in C language.

这个简单的例子展示了一个closure类似于一个回调函数，在c语言中通常定义为一个函数指针。

The difference is that a clousre binds the value of "local variables" besides the arguments passed in when it is invoked.

不同在于closure除了可以访问被调用是传进的实际参数外，还可以访问被绑定的“本地变量”。

Instead of a function pointer, we use a CLOSURE_CONNECT macro to connected the closure to some codes be execute when the closure is invoked, and to write the codes in place.

我们使用CLOSURE_CONNECT来把closure连接到它被调用时执行的代码，同时就地书写这些代码。


function like macro CLOSURE_CONNECT has four parameters:

仿函数宏有四个参数

```
CLOSURE_CONNECT(closure_ptr, initialization, continuation, finaliztion);
```

The first parameter is a pointer of the closure which be connected. The next three parameters are code blocks which will be executed when the closure be connected, invoked and freed respectively. There can be a pair of parentheses around the code block to make it seems like a statement block.

第一个参数是被连接的closure的指针，剩下的三个参数分别是在连接closure时，调用closure时和释放closure时执行的代码。可以使用一对括号把代码块括起来，以使其看起来像语句块。

The local variables which value is missing by the closure can be defined inside the host function where the CLOSURE_CONNECT is called, or inside the "initializtion" block.

执行closure需要访问的那些变量可以是定义在使用CLOSURE_CONNECT做连接动作的宿主函数里，也可以是在“initialization”初始化语句块里。

A futher example:

一个进一步的示例：

```c
#include <stdio.h>
#include <continuation/closure.h>

int main()
{
  int sum = 5;
  CLOSURE(int) closure_sum;

  CLOSURE_INIT(&closure_sum);
  
  CLOSURE_CONNECT(&closure_sum
    , (/* empty initialization */)
    , (
      /* continuation */
      printf("the sum result is: %d\n", sum + CLOSURE_ARG_OF_(&closure_sum)->_1);
    )
    , (/* empty finalization */)
  );
  
  CLOSURE_RUN(&closure_sum, 5);
  
  CLOSURE_FREE(&closure_sum);
  
  return 0;
}
```
Outputs:
the sum result is: 10
