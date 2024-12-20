#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_SIZE 256
#define NUM_READERS 10

char shared_array[ARRAY_SIZE];
int record_counter = 0;

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void* writer_thread(void* arg) {
    while (1) {
        pthread_rwlock_wrlock(&rwlock);

        // Запись в общий массив
        snprintf(shared_array, ARRAY_SIZE, "Record %d", record_counter++);
        printf("Writer: updated shared_array to '%s'\n", shared_array);

        pthread_rwlock_unlock(&rwlock);
        sleep(1); // Пауза между записями
    }
    return NULL;
}

void* reader_thread(void* arg) {
    long tid = (long)arg;

    while (1) {
        pthread_rwlock_rdlock(&rwlock);

        // Чтение общего массива
        printf("Reader %ld: %s\n", tid, shared_array);

        pthread_rwlock_unlock(&rwlock);
        usleep(1000000); // Задержка для предотвращения "забивания" вывода
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];
    pthread_t writer;

    // Создание пишущего потока
    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        perror("Failed to create writer thread");
        return EXIT_FAILURE;
    }

    // Создание читающих потоков
    for (long i = 0; i < NUM_READERS; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, (void*)i) != 0) {
            perror("Failed to create reader thread");
            return EXIT_FAILURE;
        }
    }

    // Ожидание завершения потоков (в данном случае они работают бесконечно)
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    

    return EXIT_SUCCESS;
}
