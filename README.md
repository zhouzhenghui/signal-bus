Signal Bus
==========

Universal event-driven, asynchronous/parallel infrastructure for C/C++, built on source level continuation and lock-free concurrency mechanism for C language. It use universal lock-free data structures as the foundation algorithms, so that high efficiency and robust concurrency system, including operation system kernels, can be built without any lock.

构建于源代码层次continuation和无锁并行设施上的通用的C/C++事件驱动，异步/并行基础架构。它底层使用无锁数据结构算法实现，从而可以用于在不借助于锁互斥机制的情况下创建高效健壮的并发系统，包括操作系统内核。

Several language level facilities are afforded, which help a lot in writing asynchronous and parallel code in imperative language, including closure, concurrent coroutine and the asynchronous signal-slot mechanism. 

通过提供语言级别的closure，并行化coroutine和异步的signal-slot机制，极大的帮助了使用命令式语言书写异步和并行代码。

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
Outputs:

Hello World!

**Closure vs. Callback**

The simple example shows that a closure is something like a callback, the later is often defined as a function pointer in C language.

这个简单的例子展示了一个closure类似于一个回调函数，后者在c语言中通常定义为一个函数指针。

The difference is that a closure binds the value of "local variables" besides the arguments passed in when it is invoked.

不同在于closure除了可以访问被调用是传进的实际参数外，还可以访问被绑定的“本地变量”。

Instead of a function pointer, we use a CLOSURE_CONNECT macro to connected the closure to some codes be executed when the closure is invoked, and to write the codes in place.

我们使用CLOSURE_CONNECT来把closure连接到它被调用时执行的代码，同时就地书写这些代码。

function like macro CLOSURE_CONNECT has four parameters:

仿函数宏CLOSURE_CONNECT有四个参数：

```
CLOSURE_CONNECT(closure_ptr, initialization, continuation, finalization);
```

The first parameter is a pointer of the closure which be connected. The next three parameters are code blocks which will be executed when the closure be connected, invoked and freed respectively. There can be a pair of parentheses around the code block to make it seems like a statement block.

第一个参数是被连接的closure的指针，剩下的三个参数分别是在连接closure时，调用closure时和释放closure时执行的代码。可以使用一对括号把代码块括起来，以使其看起来像语句块。

The local variables which value is missing by the closure can be defined inside the host function where the CLOSURE_CONNECT reside in, or inside the "initialization" block.

closure被调用时需要访问之前的值的那些变量可以是定义在使用CLOSURE_CONNECT的宿主函数里，也可以是在“initialization”初始化语句块里。

A further example:

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

It is of course that we can also access any local variables in code/statement block.

当然我们也可以访问在语句块的局部变量。

Although not a must, the variables visible to the closure could keep their values between invocations. It is offten implied in those languages that have closure or continuation facility natively.

尽管不是必须的，closure可见的变量的值应该可以在调用过程中保持。对于自带closure或者continuation机制的语言来说这通常是隐含的。

we use macro CLOSURE_RETAIN_VAR and it's derive forms to specify explicitly.

我们使用CLOSURE_RETAIN_VAR宏及其派生形式来明确指定。

A improved example, sum from 1 to 10:

一个改进的示例，从1加到10：

```c
#include <stdio.h>
#include <continuation/closure.h>

int main()
{
  CLOSURE(int) closure_sum;

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
```
Outputs:

the sum result is: 55

Notice that we print the result in finalization block. The finalization is only executed once when macro CLOSURE_FREE is invoked. A closure will be unconnected after then, the status is same as when it had not yet been connected with macro CLOSURE_CONNECT. Nothing to do when invoke an unconnected closure.

注意到我们是在finalization代码块中打印结果。finalization代码块只会在CLOSURE_FREE被调用时执行一次。之后closure将变成未连接的，这个状态和还没有使用CLOSURE_CONNECT进行连接时一样。调用一个未连接的closure将什么也不会发生。

**Keyword likely interface**

The weakness of function likely macro interface is that it is hard to found the defect of the enclosed codes. Thankfully, if the loop initial declaration of C99 or C++ is available, there is a keyword likely interface can be used, and the users can put their codes of the closure in
statement clauses.

仿函数接口的缺点是很难查找被宏包裹的代码里的错误。幸运的是，如果C99或C++标准里循环初始化申明被支持的化，可以使用类关键字的接口，这样用户可以把他们的代码放在外面的子句里。

A same example of above will be:

上面的例子可以这么写：

```c
#include <stdio.h>
#include <continuation/closure.h>

int main()
{
  CLOSURE(int) closure_sum;

  closure_init(&closure_sum);

  int sum = 0;
  closure_if (&closure_sum) {
    /* continuation */
    sum += CLOSURE_ARG_OF_(&closure_sum)->_1;
    CLOSURE_COMMIT_VAR(sum);
  } else {
    /* finalization */
    printf("the sum result is: %d\n", sum);
  }

  {
    int i;
    for (i = 1; i <= 10; ++i) {
      CLOSURE_RUN(&closure_sum, i);
    }
  }

  closure_free(&closure_sum);

  return 0;
}
```
Now by omitting parenthesis, the code can become a little clearer. The clauses to be executed on invocation are arranged as the "if" statement block, where the continuation block will be executed when the condition is true, that is the closure is invoked, and then the finalization block when closure is freed, that is the else clause. Some initialization can be put in front of the closure.

通过省略花括号代码变得更加清晰了。调用发生时要执行子句被安排成像if语句块那样，当条件为真，即闭包调用时，其延续块将被执行，而当闭包被释放时，else子句中的终结块将被执行。如果有初始化的工作可以放在闭包之前。

That's almost all of closure, refer to documents for more details.

这几乎就是closure的全部，更多细节请参考文档。

**Parallelism in Closure/Continuation**

Closure is thread-safe, it is said that, closure can be invoked simultaneously. But one should deal with race conditions himself when accessing shared data in the closure, though some data may be only accessed by the same closure.

Closure是线程安全的，这是说，它可以被同时调用。但是在closure内部访问共享数据时仍然必须处理竞争问题，尽管closure内部数据只能在该closure中被访问。

In the later part, high level facility will be afforded, to provide large-range thread safety by linearizing the execution in place and in a lock-free manner.

在后面部分里，提供了高层次的机制，通过就地，并且是无锁的，线性化执行过程，来提供大范围线程安全性。
