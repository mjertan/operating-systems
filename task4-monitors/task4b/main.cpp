#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

using namespace std;

class BarberShopMonitor {
private:
    int waitingCount;
    int maxSeats;
    bool open;
    bool barberSleeping;
    int clientQueue[100];
    int front, rear;

    pthread_mutex_t mtx;
    pthread_cond_t client_ready;
    pthread_cond_t barber_ready;

public:
    BarberShopMonitor(int maxSeats) 
        : waitingCount(0), maxSeats(maxSeats), open(true), barberSleeping(false), front(0), rear(0) {
        pthread_mutexattr_t mutex_attr;
        pthread_condattr_t cond_attr;

        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mtx, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);

        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&client_ready, &cond_attr);
        pthread_cond_init(&barber_ready, &cond_attr);
        pthread_condattr_destroy(&cond_attr);
    }

    ~BarberShopMonitor() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&client_ready);
        pthread_cond_destroy(&barber_ready);
    }

    void addClient(int id) {
        pthread_mutex_lock(&mtx);
        if (!open) {
            cout << "Client(" << id << "): Barber shop is closed." << endl;
            pthread_mutex_unlock(&mtx);
            _exit(0);
        }

        if (waitingCount < maxSeats) {
            clientQueue[rear] = id;
            rear = (rear + 1) % 100;
            waitingCount++;
            cout << "Client(" << id << "): Entering waiting room." << endl;
            pthread_cond_signal(&client_ready);
        } else {
            cout << "Client(" << id << "): No seats available." << endl;
        }
        pthread_mutex_unlock(&mtx);
    }

    void shaveClient() {
        pthread_mutex_lock(&mtx);
        while (waitingCount == 0 && open) {
            barberSleeping = true;
            cout << "Barber: Sleeping until clients arrive." << endl;
            pthread_cond_wait(&client_ready, &mtx);
        }

        if (!open && waitingCount == 0) {
            pthread_mutex_unlock(&mtx);
            return;
        }

        barberSleeping = false;
        int client_id = clientQueue[front];
        front = (front + 1) % 100;
        waitingCount--;
        cout << "Barber: Shaving client(" << client_id << ")." << endl;
        pthread_cond_signal(&barber_ready);
        pthread_mutex_unlock(&mtx);

        sleep(1);
        cout << "Barber: Client(" << client_id << ") finished." << endl;
    }

    void closeShop() {
        pthread_mutex_lock(&mtx);
        open = false;
        pthread_cond_broadcast(&client_ready);
        pthread_mutex_unlock(&mtx);
    }

    bool isOpen() {
        pthread_mutex_lock(&mtx);
        bool status = open;
        pthread_mutex_unlock(&mtx);
        return status;
    }

    int waitingClients() {
        pthread_mutex_lock(&mtx);
        int count = waitingCount;
        pthread_mutex_unlock(&mtx);
        return count;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <seats> <clients>" << endl;
        return 1;
    }

    int maxSeats = atoi(argv[1]);
    int clientCount = atoi(argv[2]);

    BarberShopMonitor* shop = static_cast<BarberShopMonitor*>(mmap(NULL, sizeof(BarberShopMonitor), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    new (shop) BarberShopMonitor(maxSeats);

    pid_t barber_pid = fork();
    if (barber_pid == 0) {
        while (shop->isOpen() || shop->waitingClients() > 0) {
            shop->shaveClient();
        }
        cout << "Barber: Closing shop." << endl;
        _exit(0);
    }

    for (int i = 0; i < clientCount; ++i) {
        usleep(500000);
        pid_t client_pid = fork();
        if (client_pid == 0) {
            shop->addClient(i);
            _exit(0);
        }
    }

    sleep(10); // Shop open for 10 seconds
    shop->closeShop();

    while (wait(NULL) > 0);

    shop->~BarberShopMonitor();
    munmap(shop, sizeof(BarberShopMonitor));

    cout << "Barber shop closed." << endl;
    return 0;
}
