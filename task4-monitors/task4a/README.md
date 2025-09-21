# Task 4a — Monitors (Mutex + Condition Variables)

Monitor-based synchronization pattern using `std::mutex` / `pthread_mutex_t` and condition variables (`std::condition_variable` / `pthread_cond_t`).  
Encapsulates shared state and provides safe entry/exit to critical sections.

## Build
```bash
g++ -std=c++11 -O2 -pthread main.cpp -o task4a
