
# Reduce Map Algorthim
this app will solve a simple problem by implementing reduce map algorthim using threads in C language
The purpse of this application is to practice and demonstrate the implementaion of posix threads.
Please note this application works in UNIX based systems `pthread`
This application implemented by beginner c programer.
# Problem
We have a big text file and we want to count number of words in the whole file using Reduce Map algorthim.

# Design

- We will have a Moderator thread that will splits a big work into smaller chunks and create the worker threads.
- each worker thread will count number of words in fixed range of lines, let say we have 1200 lines in file, each thread will read 400 lines.
- Worker threads in theory they called `mappers` defined in this app as struct `worker_thread`.
- `worker_thread` work on non-shared data independently.
- `moderator thread` have to wait for the worker threads to join.
- The thread who wait for all workers to finish is called `reducer thread`, which in this case it is the `moderator thread`.

## Moderator Thread Workflow
we want `moderator thread` to prepare the work by count number of lines and divide it equally to the worker theads.
- Validate and open input file
- Calculate the file size.
- Split the file size to `worker_thread`'s, and create the `worker_thread` instances. In this implementation we have maximum of 3 `worker_thread`'s defined as `NUM_OF_THREADS`
- `worker_thread` instances created will be stored in local array.
- Let `worker_thread` instances to start the work and wait them to finish.
- Read the return value of each thread which is the number of words read by the threads and sum them up.
- Destroy the `worker_thread` instances and clean up

## Worker Threads Workflow
A struct made for `worker_thread`'s, that have the attribute needed for thread callback function and the thread pointer.
each `worker_thread` will go to the same work flow.
- locate and point to the `start_byte` of the meant to be read, which it is the start read location.
- Compute the `read_chunk` size
- read the file character by character.
- count number of word by maintaining a `word_started` flag.
- return the number of word have been read.

# Conclusion

This is done by a beginner c programmer, so please consider it may have some issues.

The purpose  of this implementation is to practice the concept of multi-threading programming in c.

Special thanks to @sachinites, for Udemy [**Multithreading & Thread Synchronization - C/C++**](https://www.udemy.com/course/multithreading_parta/#instructor-1) course, the motivation and the idea of this practice comes from him.