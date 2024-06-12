#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

// Lock i zmienne globalne
pthread_mutex_t lock;
pthread_cond_t okToRead; //Zmienna warunkowa do synchronizacji czytelnikow
pthread_cond_t okToWrite; //Zmienna warunkowa do synchronizacji pisarzy
int read_que=0;
int read_count = 0;
int write_count = 0;
int num_readers = 10;
int num_writers = 4;
int sleepTime = 100000;

void *reader(void *id) {
    int reader_id = *((int *)id);
    while (1) {
        usleep(sleepTime);  // Symulacja czasu między próbami wejścia do czytelni
        
        read_que++;
        pthread_mutex_lock(&lock); //Zablokowanie mutexu
        while (write_count > 0) {
            pthread_cond_wait(&okToRead, &lock); //Czytelnik czeka na sygna wejscia
        }
        read_que--;
        read_count++;
        pthread_mutex_unlock(&lock); //Odblokowanie mutexu

        makeMessage();
        usleep(sleepTime);  // Symulacja czytania
        

        pthread_mutex_lock(&lock);
        read_count--;
        if (read_count == 0) {
            pthread_cond_signal(&okToWrite); // Sygnal, ze pisarz moze wejsc
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *writer(void *id) {
    int writer_id = *((int *)id);
    while (1) {
        usleep(sleepTime);  // Symulacja czasu między próbami wejścia do czytelni
        
        
        pthread_mutex_lock(&lock);
        
        
        
        while (read_count > 0 || write_count > 0) {
        
            pthread_cond_wait(&okToWrite, &lock); //Pisarz czeka na sygna wejscia
        }
        pthread_mutex_unlock(&lock);
        write_count++;
        makeMessage();
        
        usleep(sleepTime);  // Symulacja pisania
        

        pthread_mutex_lock(&lock);
        write_count--;
        if (write_count == 0) {
            pthread_cond_broadcast(&okToRead); //Sygnal dla wszystkich oczekujacych czytelnikow, ze mozna wejsc
        } else {
            pthread_cond_signal(&okToWrite); //Sygnal dla innego pisarza
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
//Tworzenie wiadomosci
void makeMessage(){
    printf("ReaderQ: %i WriterQ: %i [in: R: %i W: %i]\n",read_que, num_writers-write_count, read_count, write_count);

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
    if (parseParameters(argc,argv)<0) { //Wczytywanie opcji
        return -1;
    }
 
    

    pthread_t readers[num_readers];
    pthread_t writers[num_writers];
    int reader_ids[num_readers];
    int writer_ids[num_writers];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&okToRead, NULL);
    pthread_cond_init(&okToWrite, NULL);

    for (int i = 0; i < num_readers; i++) { //Rozpoczynamy watki czytelnikow
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    for (int i = 0; i < num_writers; i++) {//Rozpoczynamy watki pisarzy
        writer_ids[i] = i + 1; 
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    for (int i = 0; i < num_readers; i++) { //Czekamy glownym watkiem na zakonczenie watkow czytelnikow
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < num_writers; i++) { //Czekamy glownym watkiem na zakonczenie watkow pisarzy
        pthread_join(writers[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&okToRead);
    pthread_cond_destroy(&okToWrite);

    return 0;
}