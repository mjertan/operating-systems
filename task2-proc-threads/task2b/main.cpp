#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>

using namespace std;

int iterations = 5; // number of iterations

struct Data {
    int turn;
    int flag[2];
};

void enterCritical(Data* data, int i, int j) {
    data->flag[i] = 1;
    while (data->flag[j] != 0) {
        if (data->turn == j) {
            data->flag[i] = 0;
            while (data->turn == j) {
                // Waiting
            }
            data->flag[i] = 1;
        }
    }
}

void exitCritical(Data* data, int i, int j) {
    data->turn = j;
    data->flag[i] = 0;
}

void process(int i, Data* data) {
    for (int k = 1; k <= iterations; ++k) {
        enterCritical(data, i, 1 - i); // Entering critical section
        for (int m = 1; m <= 5; ++m) {
            cout << "Process: " << i + 1 << ", C.S. no: " << k << " (" << m << "/5)" << endl;
            sleep(1); // Simulating work in the critical section
        }
        exitCritical(data, i, 1 - i); // Exiting critical section
    }
}

int main() {
    // Creating shared memory for inter-process communication
    int shm_id = shmget(IPC_PRIVATE, sizeof(Data), IPC_CREAT | 0666); 
    // 0666 means read and write permissions for all users
    if (shm_id < 0) {
        cout << "Error creating shared memory." << endl;
        return 1;
    }

    // Attaching shared memory to process address space
    Data* data = (Data*)shmat(shm_id, NULL, 0);
    data->turn = 0;
    data->flag[0] = 0;
    data->flag[1] = 0;

    // Creating child process
    pid_t pid = fork();
    if (pid < 0) {
        cout << "Error creating process." << endl;
        return 1;
    } else if (pid == 0) {
        // Child process simulating process 1
        process(1, data);
        shmdt(data); // Detaching shared memory
    } else {
        // Parent process simulating process 0
        process(0, data);
        wait(NULL); // Waiting for child process to finish
        shmdt(data); // Detaching shared memory
        shmctl(shm_id, IPC_RMID, NULL); // Removing shared memory segment
    }

    return 0;
}
