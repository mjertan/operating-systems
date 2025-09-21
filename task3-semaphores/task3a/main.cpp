#include <iostream>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/wait.h>

using namespace std;

#define CANNIBALS 0
#define MISSIONARIES 1
#define SHORE_LEFT 0
#define SHORE_RIGHT 1

struct SharedMemory {
    int cannibals_count;
    int missionaries_count;
    int passengers_count;
    int current_shore; 
    sem_t boarding_sem;
    sem_t boat_sem;
    sem_t mutex;
};

SharedMemory *shared_mem;
int mem_id;

void release_resources(int sig) {
    (void)sig; // unused parameter

    // release shared memory
    shmdt(shared_mem);
    shmctl(mem_id, IPC_RMID, 0);
    cout << "\nResources released. Program ending." << endl;
    exit(0);
}

void cannibal(int shore) {
    if (shore != shared_mem->current_shore) exit(0);

    sem_wait(&shared_mem->boarding_sem);
    sem_wait(&shared_mem->mutex);

    if (shared_mem->passengers_count < 7 && 
        (shared_mem->cannibals_count + 1 <= shared_mem->missionaries_count || shared_mem->missionaries_count == 0)) {
        shared_mem->cannibals_count++;
        shared_mem->passengers_count++;
        cout << "Cannibal boarded. Current cannibals: " << shared_mem->cannibals_count << endl;
    }

    if (shared_mem->passengers_count >= 3) {
        sem_post(&shared_mem->boat_sem);
    }

    sem_post(&shared_mem->mutex);
    sem_post(&shared_mem->boarding_sem);
    exit(0);
}

void missionary(int shore) {
    if (shore != shared_mem->current_shore) exit(0); 

    sem_wait(&shared_mem->boarding_sem);
    sem_wait(&shared_mem->mutex);

    if (shared_mem->passengers_count < 7) {
        shared_mem->missionaries_count++;
        shared_mem->passengers_count++;
        cout << "Missionary boarded. Current missionaries: " << shared_mem->missionaries_count << endl;
    }

    if (shared_mem->passengers_count >= 3) {
        sem_post(&shared_mem->boat_sem);
    }

    sem_post(&shared_mem->mutex);
    sem_post(&shared_mem->boarding_sem);
    exit(0);
}

void boat() {
    while (true) {
        sem_wait(&shared_mem->boat_sem); // wait until at least 3 passengers onboard

        sem_wait(&shared_mem->mutex);
        if (shared_mem->passengers_count >= 3 && shared_mem->cannibals_count <= shared_mem->missionaries_count) {
            sleep(1); // wait for possible additional passengers
            cout << "------------------------------------------------" << endl;
            cout << "Passengers onboard: " << shared_mem->missionaries_count << " missionaries, "
                 << shared_mem->cannibals_count << " cannibals." << endl;
            cout << "------------------------------------------------" << endl;
            cout << "Boat ready to depart." << endl;
            shared_mem->current_shore = (shared_mem->current_shore == SHORE_RIGHT) ? SHORE_LEFT : SHORE_RIGHT;
            shared_mem->cannibals_count = 0;
            shared_mem->missionaries_count = 0;
            shared_mem->passengers_count = 0;
        }
        sem_post(&shared_mem->mutex);

        sleep(2); // crossing the river
        cout << "\nPassengers disembarked on " 
             << (shared_mem->current_shore == SHORE_RIGHT ? "right shore." : "left shore.") << endl;
        sem_post(&shared_mem->boarding_sem); // boarding again from new shore
    }
}

int main() {
    signal(SIGINT, release_resources);

    // Init shared memory
    mem_id = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0600 | IPC_CREAT);
    if (mem_id == -1) {
        cout << "No shared memory!" << endl;
        exit(1);
    }

    shared_mem = (SharedMemory *)shmat(mem_id, nullptr, 0);
    if (shared_mem == (void *)-1) {
        cout << "Cannot attach to shared memory!" << endl;
        shmctl(mem_id, IPC_RMID, 0);
        exit(1);
    }

    shared_mem->cannibals_count = 0;
    shared_mem->missionaries_count = 0;
    shared_mem->passengers_count = 0;
    shared_mem->current_shore = SHORE_RIGHT;

    // Init unnamed semaphores in shared memory
    sem_init(&shared_mem->boarding_sem, 1, 1);
    sem_init(&shared_mem->boat_sem, 1, 0);
    sem_init(&shared_mem->mutex, 1, 1);

    // Boat process
    pid_t boat_pid = fork();
    if (boat_pid == 0) {
        boat();
        exit(0);
    }

    // Passenger generator process
    srand(time(NULL));
    int counter = 0;
    while (true) {
        sleep(1); // Every second a new passenger is created

        int shore = shared_mem->current_shore;
        pid_t pass_pid = fork();
        if (pass_pid == 0) {
            cannibal(shore);
        }

        if (counter % 2 == 0) {
            pid_t miss_pid = fork();
            if (miss_pid == 0) {
                missionary(shore);
            }
        }

        counter++;
        waitpid(-1, NULL, 0);
    }

    waitpid(boat_pid, NULL, 0);

    release_resources(0);

    return 0;
}
