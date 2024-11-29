#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/shared_semaphore"
#define BUFFER_SIZE 256

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

int main() {
    int shm_fd;
    char *shared_memory;
    sem_t *sem;

    // Открываем существующий семафор
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("Ошибка открытия семафора");
        exit(EXIT_FAILURE);
    }

    // Открываем существующую разделяемую память
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка открытия разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    // Отображаем разделяемую память в адресное пространство
    shared_memory = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Ошибка отображения разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    printf("Принимающий процесс запущен. PID: %d\n", getpid());

    while (1) {
        char time_buffer[BUFFER_SIZE];
        get_current_time(time_buffer, sizeof(time_buffer));

        sem_wait(sem);
        printf("Принимающий процесс:\n");
        printf("Текущее время: %s\n", time_buffer);
        printf("Получено сообщение: %s\n\n", shared_memory);
        sem_post(sem);

        sleep(1);
    }

    // Очистка ресурсов
    munmap(shared_memory, BUFFER_SIZE);

    return 0;
}

