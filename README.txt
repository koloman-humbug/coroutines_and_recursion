The goal here is to investigate if the stack size as a limiting factor when
calling functions recursively can be removed without modifying the algorithmic
structure of those functions by using coroutines.

I have tried this on Ubuntu 24.04 and g++ 13.3.0.
All files are compiled with g++ -O2.
This is done for 2 reasons:
1. Coroutine's symmetric transfer works only with the tail call optimization
   which g++ only supports if -O2 has been passed to g++ (with -O0 and -O1
   I got crashes).
2. In order to have the same conditions for all three setups.

There are 3 programs:
Each calls a function recursively where each call has about 1 KiB of local variables.

recurse.cpp:
This uses plain recursion.

corecurse.cpp:
This turns the recursive function into a coroutine which simply co_returns
the result of the recursive function.

corecurse_symtrans.cpp:
This employs C++ coroutines' symmetric transfer. That means
1. each coroutine's initial_suspend() method returns std::suspend_always, i.e. each coroutine suspends immediately.
2. Each coroutine co_awaits its child and co_returns the child's result.
3. The last child that has reached the desired level of recursion co_returns its result.
4. Each coroutine has a special final awaiter that resumes the caller. That
awaiter is returned on the coroutine's final_suspend() call (symmetric transfer).
What that means is that since each coroutine resumes the next one with a tail
call, the stack does not grow with the recursion depth. The local variables
of all levels live in their coroutine frames on the heap, so the depth is
limited by heap memory instead of stack size.

Running these programs in an environment with
ulimit -s 1024 (stack size 1M)

and with
/usr/bin/time -f "maxrss=%MKB t=%es" <executable>
should yield results similar to this:

1. plain recursion:

stack limit 1MiB:
Command terminated by signal 11
maxrss=4068KB t=0.00s
=> O(1000) recursion levels

stack limit unlimited (ulimit -s unlimited):
1000000 recursive calls
maxrss=1011352KB t=1.35s
=> 1000000 recursion levels

2. simple coroutine recursion via co_return:

stack limit 1MiB:
Command terminated by signal 11
maxrss=21860KB t=0.02s
=> O(16000) recursion levels

3. recursion using symmetric transfer:

stack limit 1MiB:
1000000 recursive calls (hardcoded max upper limit)
maxrss=1112420KB t=2.80s
=> 1000000 recursion levels
