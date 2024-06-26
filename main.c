//zagodzenie pisarzy
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

// Semafory i zmienne globalne
sem_t mutex;
sem_t write_lock;
int read_count = 0;

int write_count = 0;

int sleepTime = 100000;
int num_readers = 10;
int num_writers = 4;
    
void *reader(void *id) {
    int reader_id = *((int *)id);
    while (1) {
        usleep(sleepTime);  // Symulacja czasu między próbami wejścia do czytelni
        
        
        
        sem_wait(&mutex); //Uzyskanie dostepu do sekcji krytycznej chronicej licznik czytelnikow
        
        read_count++;
        
        if (read_count == 1 ) {
            sem_wait(&write_lock);// Pierwszy czytelnik blokuje dostep pisarzom
        }
        sem_post(&mutex); //Zwolnienie semafora
        makeMessage();
        
        usleep(sleepTime);  // Symulacja czytania
        
        

        sem_wait(&mutex);
        read_count--;
        if (read_count == 0) {
            sem_post(&write_lock); //Ostatni czytelnik odblokowuje dostep pisarzom
        }
        sem_post(&mutex);
        
    }
    return NULL;
}

void *writer(void *id) {
    int writer_id = *((int *)id);
    while (1) {
        usleep(2 * sleepTime);  // Symulacja czasu między próbami wejścia do czytelni
        
        
        

        sem_wait(&write_lock); //Pisarz blokuje dostep do czytelni dla innych pisarzy i czytelnikow
    
        
        write_count++;
        makeMessage();
        
        usleep(sleepTime);  // Symulacja pisania
        write_count--;
        
        
        sem_post(&write_lock); //Pisarz odblokowuje dostep do czytelni
        
    }
    return NULL;
}
//Tworzenie wiadomosci
void makeMessage(){
    printf("ReaderQ: %i WriterQ: %i [in: R: %i W: %i]\n",num_readers-read_count ,num_writers-write_count, read_count, write_count);

}

int parseParameters(int argc, char **argv) // Funkcja wczytująca opcje programu: liczbę czytelników i pisarzy.
{
    int option;
    while ((option = getopt(argc, argv, ":R:W:")) != -1)
    {
        switch (option)
        {
        case 'R':
            if (sscanf(optarg, "%u", &num_readers) < 1) 
                return -1; 
            break;
        case 'W':
            if (sscanf(optarg, "%u", &num_writers) < 1) 
                return -2; 
            break;
        case ':':
            printf("opcja wymaga podania wartosci\n"); 
            return -3; 
            break;
        case '?':
            printf("nieznana opcja: %c\n", optopt); 
            return -4; 
            break;
        default:
            printf("blad"); 
            return -5; 
            break;
        }
    }
    int remainingArguments = argc - optind; 
    if (remainingArguments != 0) 
        return -6; 
    return 0; 
}


int main(int argc, char *argv[]) {
    if (parseParameters < 0 ){ //Wczytywanie opcji
        
        return -1;
    }

    

    pthread_t readers[num_readers];
    pthread_t writers[num_writers];
    int reader_ids[num_readers];
    int writer_ids[num_writers];

    sem_init(&mutex, 0, 1);
    sem_init(&write_lock, 0, 1);

    for (int i = 0; i < num_readers; i++) { //Rozpoczynamy watki czytelnikow
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    sleep(2); //Czekamy 2 sekundy przed rozpoczeciem watkow pisarzy, aby opznic moment ich wejscia do czytelni

    for (int i = 0; i < num_writers; i++) { //Rozpoczynamy watki pisarzy
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    for (int i = 0; i < num_readers; i++) { //Czekamy glownym watkiem na zakonczenie watkow czytelnikow
        pthread_join(readers[i], NULL); 
    }

    for (int i = 0; i < num_writers; i++) { //Czekamy glownym watkiem na zakonczenie watkow pisarzy
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&write_lock);

    return 0;
}