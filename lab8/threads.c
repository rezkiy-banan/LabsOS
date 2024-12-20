#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define BUFFER_SIZE 256

// Общий массив символов
char shared_buffer[BUFFER_SIZE];

// Счетчик записей
int record_number = 0;

// Мьютекс и условная переменная для синхронизации
pthread_mutex_t mutex;
pthread_cond_t cond;

// Флаг для контроля обновления массива
int buffer_updated = 0;

// Функция для пишущего потока
void *writer_thread(void *arg) {
    while (1) {
        // Блокировка мьютекса
        pthread_mutex_lock(&mutex);

        // Запись данных в общий массив
        memset(shared_buffer, 0, BUFFER_SIZE); // Очистка массива
        snprintf(shared_buffer, BUFFER_SIZE, "Record number: %d", ++record_number);

        printf("[Writer] Updated buffer: %s\n", shared_buffer);

        // Установка флага и оповещение читающих потоков
        buffer_updated = 1;
        pthread_cond_broadcast(&cond);

        // Разблокировка мьютекса
        pthread_mutex_unlock(&mutex);

        // Имитация работы
        sleep(1);
    }
    return NULL;
}

// Функция для читающих потоков
void *reader_thread(void *arg) {
    pthread_t tid = pthread_self();

    while (1) {
        // Блокировка мьютекса
        pthread_mutex_lock(&mutex);

        // Ожидание обновления массива
        while (!buffer_updated) {
            pthread_cond_wait(&cond, &mutex);
        }

        // Чтение данных из общего массива
        printf("[Reader %lu] Buffer content: %s\n", tid, shared_buffer);

        // Сброс флага (только для демонстрации, может остаться в 1)
        buffer_updated = 0;

        // Разблокировка мьютекса
        pthread_mutex_unlock(&mutex);

        // Имитация работы
        usleep(500000); // 0.5 секунды
    }
    return NULL;
}

int main() {
    // Инициализация мьютекса и условной переменной
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        return EXIT_FAILURE;
    }
    if (pthread_cond_init(&cond, NULL) != 0) {
        perror("Failed to initialize condition variable");
        return EXIT_FAILURE;
    }

    // Создание потоков
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        perror("Failed to create writer thread");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_READERS; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, NULL) != 0) {
            perror("Failed to create reader thread");
            return EXIT_FAILURE;
        }
    }

    // Ожидание завершения потоков (в данном случае потоки работают бесконечно)
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    // Уничтожение мьютекса и условной переменной
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return EXIT_SUCCESS;
}
