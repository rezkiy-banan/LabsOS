#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <errno.h>

#define SHM_SIZE 256
#define SHMFILE "shmfile"
#define SEMFILE "semfile"

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

// Функция для управления семафором
void sem_operation(int semid, int op) {
    struct sembuf sb = {0, op, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("Ошибка операции с семафором");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Генерация ключа для разделяемой памяти
    key_t shm_key = ftok(SHMFILE, 65); // Используем один и тот же файл для shm и семафора
    if (shm_key == -1) {
        perror("Ошибка ftok для разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    // Подключение к существующему сегменту разделяемой памяти
    int shmid = shmget(shm_key, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("Ошибка shmget");
        exit(EXIT_FAILURE);
    }

    // Присоединение к разделяемой памяти
    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)(-1)) {
        perror("Ошибка shmat");
        exit(EXIT_FAILURE);
    }

    // Генерация ключа для семафора
    key_t sem_key = ftok(SEMFILE, 66); // Используем тот же файл для генерации семафора
    if (sem_key == -1) {
        perror("Ошибка ftok для семафора");
        exit(EXIT_FAILURE);
    }

    // Подключение к существующему семафору
    int semid = semget(sem_key, 1, 0666);
    if (semid == -1) {
        perror("Ошибка подключения к семафору");
        exit(EXIT_FAILURE);
    }

    printf("Принимающий процесс запущен. PID: %d\n", getpid());

    while (1) {
        char time_buffer[64];
        get_current_time(time_buffer, sizeof(time_buffer));

        sem_operation(semid, -1); // Вход в критическую секцию
        printf("Принимающий процесс:\n");
        printf("Текущее время: %s\n", time_buffer);
        printf("Получено сообщение: %s\n\n", shared_memory);
        sem_operation(semid, 1); // Выход из критической секции

        sleep(1); // Имитация работы
    }

    // Отключение от разделяемой памяти
    shmdt(shared_memory);

    return 0;
}
