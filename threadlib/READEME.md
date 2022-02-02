# Threadlib
This small project related to implementing a library for thread management in C code.
All implementation and notes taken from Udemy course [**Part B (ADVANCE) Multithreading Design Patterns ( pthreads )**](https://www.udemy.com/course/multithreading-design-patterns/) 
Purpose of this project to practice and learn about multi-threaded program design and thread managements, so it may have some bugs, dump implementation, and poor design.
Special thanks to the course instructor @sachinites, and all this work reference to him.

This library implement a various of threads managements data structure:
- Thread Pausing
- Thread Pool
- Thread barriers
- Thread Wait Queues

Some of mentioned threads data structure have been used for demo purposes in small programs. I didn't spent much time in these demo programs, so it poorly implementation and lack of cleanups.

This library uses a "Glued Linked List" as main data structure, a type of Linked List data structure.
This data structure is all implemented by the instructor @sachinites.


# Detailed Explanation
I will write a detailed note about the library, it is not important and maybe skipped.

## Thread Management
In order to implement advanced concepts based on multi-threaded, it require the developer to have a full control on execution unit - a thread
We need to keep track of each thread status

- is thread executing or blocked ?
- is thread blocked on CV, then which CV ?
- is thread a reader thread or writer thread ?
- is thread queued up in the thread pool ?
- etc.

To track threads we need to make a data structure that have the thread object and status.

## Thread Pool
Thread pools is a parking state for blocked threads (ready to resume) 

- Thread pool can be modeled as any data structure (list, tree, etc.) which can hold `thread_t` objects
- Thread in a thread pool is read-to-use-state.
- Threads blocked on CV are stored in thread pools
- When we have a work W, it picks up an unused thread from thread pool, assign W to the thread and signal the thread
    - assign work
    - signal thread
    - `pthread_create()` here is expensive.
- A thread is placed back in thread pool after it has completed its work
- in init phase, we create pre-defined number of threads in thread pool
- This pattern called Worker-Crew pattern

## Thread Wait Queues
Wait Queues is a thread synchronization data structure.
It will hold threads and keep them blocked state until some condition met

## Thread Barriers 
Thread barrier is a thread synchronization data structure which blocks all threads at particular line of code until specified number of threads arrive the barrier point.
Thread barriers used when you want to wait for number of tasks to be completed before proceed.
