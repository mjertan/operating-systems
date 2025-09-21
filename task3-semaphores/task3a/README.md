# Task 3a — Semaphores (Basics)

Exercises with semaphores for process and thread synchronization.  
Implements simple coordination between concurrent tasks using **System V** (`semget`, `semop`, `semctl`) or **POSIX semaphores** (`sem_init`, `sem_wait`, `sem_post`).

## Build
```bash
g++ -std=c++11 -O2 -pthread main.cpp -o task3a
