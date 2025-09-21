# Task 4b — Monitors (Advanced Patterns)

Advanced monitor usage with mutex + condition variables.  
Implements a classic concurrency pattern (e.g., **Bounded Buffer**, **Readers–Writers with fairness**, **Barriers**, or **Dining Philosophers** using monitors).

## Build
```bash
g++ -std=c++11 -O2 -pthread main.cpp -o task4b
