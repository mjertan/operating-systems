#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iomanip>

using namespace std;

int N = 30;

struct Data {
    int *array;
    int id;
    int sum;
    double avg;
};

pthread_mutex_t printMutex;

void* generate(void* arg) {
    Data* data = (Data*)arg;
    unsigned int seed = time(0) + data->id;
    for (int i = 0; i < N; ++i) {
        data->array[i] = rand_r(&seed) % 1000;
    }
    pthread_exit(NULL);
}

void* calculate(void* arg) {
    Data* data = (Data*)arg;
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += data->array[i];
    }

    data->sum = sum;
    data->avg = sum / (double)N;

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <threads_per_type>" << endl;
        return 1;
    }

    int threadCount = atoi(argv[1]);
    if (threadCount <= 0) {
        cout << "Thread count must be a positive integer." << endl;
        return 1;
    }

    // Initialization of threads, data structures, and arrays for storing generated numbers
    pthread_t generateThreads[threadCount], calculateThreads[threadCount];
    Data threadData[threadCount];
    int arrays[threadCount][N];

    pthread_mutex_init(&printMutex, NULL);

    // Creating threads for generating random numbers
    for (int i = 0; i < threadCount; ++i) {
        threadData[i] = {arrays[i], i + 1, 0, 0.0};
        pthread_create(&generateThreads[i], NULL, generate, &threadData[i]);
    }

    // Waiting for all generation threads to finish
    for (int i = 0; i < threadCount; ++i) {
        pthread_join(generateThreads[i], NULL);
    }

    // Creating threads for calculating sum and average
    for (int i = 0; i < threadCount; ++i) {
        pthread_create(&calculateThreads[i], NULL, calculate, &threadData[i]);
    }

    // Waiting for all calculation threads to finish
    for (int i = 0; i < threadCount; ++i) {
        pthread_join(calculateThreads[i], NULL);
    }

    // Printing all sums
    pthread_mutex_lock(&printMutex);
    for (int i = 0; i < threadCount; ++i) {
        cout << "array" << threadData[i].id << " sum = " << threadData[i].sum << endl;
    }
    pthread_mutex_unlock(&printMutex);

    sleep(1);

    // Printing all averages
    pthread_mutex_lock(&printMutex);
    for (int i = 0; i < threadCount; ++i) {
        cout << "array" << threadData[i].id << " average = " 
             << fixed << setprecision(2) << threadData[i].avg << endl;
    }
    pthread_mutex_unlock(&printMutex);

    pthread_mutex_destroy(&printMutex);
    return 0;
}
