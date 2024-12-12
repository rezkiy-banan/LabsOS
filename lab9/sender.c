// sender.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/stat.h> // Для chmod
#include <fcntl.h> // Для open
#include <sys/file.h> // Для flock
#include <errno.h>

#define SHM_SIZE 256
#define LOCKFILE "sender.lock"
#define SHMFILE "shmfile"
#define SEMFILE "semfile"

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

void handle_existing_instance() {
    printf("Передающий процесс уже запущен. Завершение.\n");
    exit(EXIT_FAILURE);
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
    // Создаем или открываем lock-файл для предотвращения повторного запуска
    int lock_fd = open(LOCKFILE, O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) {
        perror("Ошибка открытия lock-файла");
        exit(EXIT_FAILURE);
    }

    // Пытаемся захватить эксклюзивную блокировку
    if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            handle_existing_instance();
        } else {
            perror("Ошибка установки блокировки");
            close(lock_fd);
            exit(EXIT_FAILURE);
        }
    }

    // Убедимся, что shmfile существует
    FILE *file = fopen(SHMFILE, "a");
    if (!file) {
        perror("Ошибка создания shmfile");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Установим права доступа, если файл был только что создан
    chmod(SHMFILE, 0666);

    // Генерация ключа для разделяемой памяти
    key_t shm_key = ftok(SHMFILE, 65);
    if (shm_key == -1) {
        perror("Ошибка ftok");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    // Создание сегмента разделяемой памяти
    int shmid = shmget(shm_key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Ошибка shmget");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    // Присоединение к разделяемой памяти
    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)(-1)) {
        perror("Ошибка shmat");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    // Создание или получение семафора
    key_t sem_key = ftok(SEMFILE, 66);
    if (sem_key == -1) {
        perror("Ошибка ftok для семафора");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    int semid = semget(sem_key, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("Ошибка создания семафора");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    // Инициализация семафора
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("Ошибка инициализации семафора");
        close(lock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Передающий процесс запущен. PID: %d\n", getpid());

    while (1) {
        char time_buffer[64];
        get_current_time(time_buffer, sizeof(time_buffer));

        sem_operation(semid, -1); // Вход в критическую секцию
        snprintf(shared_memory, SHM_SIZE, "PID: %d, Время: %s", getpid(), time_buffer);
        sem_operation(semid, 1); // Выход из критической секции

        sleep(2); // Имитация работы
    }

    // Отключение от разделяемой памяти
    shmdt(shared_memory);

    // Удаляем блокировку при завершении
    close(lock_fd);
    unlink(LOCKFILE);

    return 0;
}
