#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <errno.h> // Подключаем для использования errno и EEXIST

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/shared_semaphore"
#define BUFFER_SIZE 256

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

void handle_existing_instance() {
    printf("Программа уже запущена. Завершение.\n");
    exit(EXIT_FAILURE);
}

int main() {
    int shm_fd;
    char *shared_memory;
    sem_t *sem;

    // Проверяем, есть ли уже запущенная программа
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) { // Обрабатываем ошибку существующего семафора
            handle_existing_instance();
        } else {
            perror("Ошибка создания семафора");
            exit(EXIT_FAILURE);
        }
    }

    // Создаём разделяемую память
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка создания разделяемой памяти");
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    // Задаём размер разделяемой памяти
    if (ftruncate(shm_fd, BUFFER_SIZE) == -1) {
        perror("Ошибка установки размера разделяемой памяти");
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    // Отображаем разделяемую память в адресное пространство
    shared_memory = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Ошибка отображения разделяемой памяти");
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Передающий процесс запущен. PID: %d\n", getpid());

    while (1) {
        char time_buffer[BUFFER_SIZE / 2]; // Для безопасности используем половину размера
        get_current_time(time_buffer, sizeof(time_buffer));

        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "PID: %d, Время: %s", getpid(), time_buffer);

        sem_wait(sem);
        strncpy(shared_memory, message, BUFFER_SIZE - 1); // Гарантируем, что строка будет null-terminated
        shared_memory[BUFFER_SIZE - 1] = '\0'; // Явно добавляем завершающий нулевой символ
        sem_post(sem);

        sleep(1);
    }

    // Очистка ресурсов
    munmap(shared_memory, BUFFER_SIZE);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    return 0;
}
