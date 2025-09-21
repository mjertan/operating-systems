# Task 2b — Processes & Threads (Advanced)

Further exercises with Unix processes and POSIX threads.
Focus: shared state, synchronization with `pthread_mutex_t` and `pthread_cond_t`
(avoids semaphores; those are in Task 3).

## Build
```bash
g++ -std=c++11 -O2 -pthread main.cpp -o task2b
