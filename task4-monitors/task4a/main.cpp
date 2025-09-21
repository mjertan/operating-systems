#include <iostream>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

class Monitor {
private:
    pthread_mutex_t mtx;
    pthread_cond_t all_ready;

    int processCount;
    int finishedProcesses;
    int numbers[100];

public:
    Monitor(int n) : processCount(n), finishedProcesses(0) {
        pthread_mutexattr_t mutex_attr;
        pthread_condattr_t cond_attr;

        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mtx, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);

        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&all_ready, &cond_attr);
        pthread_condattr_destroy(&cond_attr);
    }

    ~Monitor() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&all_ready);
    }

    void enterNumber(int id) {
        pthread_mutex_lock(&mtx);

        cout << "\nProcess " << id << ". enter number: ";
        cin >> numbers[id];
        finishedProcesses++;

        if (finishedProcesses == processCount) {
            pthread_cond_broadcast(&all_ready);
        }

        pthread_mutex_unlock(&mtx);
    }

    void waitAll() {
        pthread_mutex_lock(&mtx);
        while (finishedProcesses < processCount) {
            pthread_cond_wait(&all_ready, &mtx);
        }
        pthread_mutex_unlock(&mtx);
    }

    void printNumber(int id) {
        pthread_mutex_lock(&mtx);
        cout << "\nProcess " << id << ". entered number is: " << numbers[id] << "\n";
        pthread_mutex_unlock(&mtx);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " process_count\n";
        return 1;
    }

    int processCount = atoi(argv[1]);
    if (processCount <= 0 || processCount > 100) {
        cout << "Process count must be positive (<= 100).\n";
        return 1;
    }

    Monitor* monitor = static_cast<Monitor*>(mmap(NULL, sizeof(Monitor), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    new (monitor) Monitor(processCount);

    pid_t pid;
    for (int i = 0; i < processCount; ++i) {
        pid = fork();
        if (pid == 0) {
            monitor->enterNumber(i);
            monitor->waitAll();
            monitor->printNumber(i);
            _exit(0);
        }
    }

    while (wait(NULL) > 0);

    monitor->~Monitor();
    munmap(monitor, sizeof(Monitor));

    cout << "\nAll processes finished.\n";
    return 0;
}
