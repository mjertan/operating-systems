# Task 2a — Processes & Threads (Basics)

Exercises with Unix processes (`fork/exec`) and POSIX threads (`pthread_create/join`).
Includes simple examples of shared state and basic synchronization.

## Build
```bash
g++ -std=c++11 -O2 -pthread main.cpp -o task2a
