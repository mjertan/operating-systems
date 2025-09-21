#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fcntl.h> 

using namespace std;

#define BUFFER_SIZE 5

// Global buffer
int buffer[BUFFER_SIZE];
int inPos = 0;
int outPos = 0;

pthread_mutex_t mutex;

sem_t *WRITE;
sem_t *FULL;
sem_t *EMPTY;

int producerCount;
int numbersPerProducer;

void *producer(void *arg) {
    int id = *(int *)arg; 
    for (int i = 0; i < numbersPerProducer; ++i) {
        sem_wait(FULL);
        
        pthread_mutex_lock(&mutex);
        int randomNum = rand() % 1000;
        buffer[inPos] = randomNum;
        cout << "Producer " << id << " sends \"" << randomNum << "\"" << endl; 
        usleep(50000);
        inPos = (inPos + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex); 
 
        sem_post(EMPTY);
    }
    cout << "Producer " << id << " finished sending" << endl;
    pthread_exit(NULL);
}

void *consumer(void *) {
    int sum = 0;
    for (int i = 0; i < producerCount * numbersPerProducer; ++i) {
        sem_wait(EMPTY);
        
        pthread_mutex_lock(&mutex);
        int receivedNum = buffer[outPos];
        cout << "Consumer receives " << receivedNum << endl;
        sum += receivedNum;
        outPos = (outPos + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex); 
            
        sem_post(FULL);
    }
    cout << "Consumer - sum of received numbers = " << sum << endl;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <producers> <numbers_per_producer>" << endl;
        return 1;
    }
    producerCount = atoi(argv[1]);
    numbersPerProducer = atoi(argv[2]);
    
    if (producerCount <= 0) {
        cout << "Producer count must be positive." << endl;
        return 1;
    }
    if (numbersPerProducer <= 0) {
        cout << "Number of random numbers must be positive." << endl;
        return 1;
    }

    WRITE = sem_open("/sem_write", O_CREAT, 0644, 1);
    FULL = sem_open("/sem_full", O_CREAT, 0644, BUFFER_SIZE);
    EMPTY = sem_open("/sem_empty", O_CREAT, 0644, 0);

    if (WRITE == SEM_FAILED || FULL == SEM_FAILED || EMPTY == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pthread_mutex_init(&mutex, NULL);

    srand(time(NULL));

    pthread_t producers[producerCount];
    int producer_ids[producerCount];
    for (int i = 0; i < producerCount; ++i) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    for (int i = 0; i < producerCount; ++i) {
        pthread_join(producers[i], NULL);
    }

    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex);
    sem_close(WRITE);
    sem_close(FULL);
    sem_close(EMPTY);
    sem_unlink("/sem_write");
    sem_unlink("/sem_full");
    sem_unlink("/sem_empty");
    
    return 0;
}
