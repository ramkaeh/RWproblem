#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Semafory i zmienne globalne
sem_t mutex;
sem_t write_lock;
int read_count = 0;

void *reader(void *id) {
    int reader_id = *((int *)id);
    while (1) {
        usleep(rand() % 1000000);  // Symulacja czasu między próbami wejścia do czytelni
        printf("Reader %d wants to enter.\n", reader_id);

        sem_wait(&mutex);
        read_count++;
        if (read_count == 1) {
            sem_wait(&write_lock);
        }
        sem_post(&mutex);

        printf("Reader %d is in the reading room.\n", reader_id);
        usleep(rand() % 1000000);  // Symulacja czytania
        printf("Reader %d is leaving.\n", reader_id);

        sem_wait(&mutex);
        read_count--;
        if (read_count == 0) {
            sem_post(&write_lock);
        }
        sem_post(&mutex);
    }
    return NULL;
}

void *writer(void *id) {
    int writer_id = *((int *)id);
    while (1) {
        usleep(rand() % 1000000);  // Symulacja czasu między próbami wejścia do czytelni
        printf("Writer %d wants to enter.\n", writer_id);

        sem_wait(&write_lock);
        printf("Writer %d is in the reading room.\n", writer_id);
        usleep(rand() % 1000000);  // Symulacja pisania
        printf("Writer %d is leaving.\n", writer_id);
        sem_post(&write_lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_readers> <number_of_writers>\n", argv[0]);
        return 1;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);

    pthread_t readers[num_readers];
    pthread_t writers[num_writers];
    int reader_ids[num_readers];
    int writer_ids[num_writers];

    sem_init(&mutex, 0, 1);
    sem_init(&write_lock, 0, 1);

    for (int i = 0; i < num_readers; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    for (int i = 0; i < num_writers; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    for (int i = 0; i < num_readers; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < num_writers; i++) {
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&write_lock);

    return 0;
}