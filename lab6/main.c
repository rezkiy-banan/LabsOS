#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define FIFO_NAME "my_fifo"

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

void pipe_example() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Ошибка создания pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка создания дочернего процесса");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        close(pipe_fd[1]);  
        char buffer[BUFFER_SIZE / 2]; // Изменено

        if (read(pipe_fd[0], buffer, BUFFER_SIZE / 2) > 0) { // Изменено
            char child_time[BUFFER_SIZE];
            get_current_time(child_time, sizeof(child_time));
            printf("Дочерний процесс:\n");
            printf("Текущее время: %s\n", child_time);
            printf("Получено из pipe: %s\n", buffer);
        }
        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    } else { 
        close(pipe_fd[0]); 

        sleep(5);

        char parent_time[BUFFER_SIZE];
        get_current_time(parent_time, sizeof(parent_time));

        char message[BUFFER_SIZE / 2]; // Изменено
        snprintf(message, sizeof(message), "PID: %d, Время: %s", getpid(), parent_time);

        write(pipe_fd[1], message, strlen(message) + 1);
        close(pipe_fd[1]);

        wait(NULL); 
    }
}

void fifo_example() {
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        perror("Ошибка создания FIFO");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка создания дочернего процесса");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        int fifo_fd = open(FIFO_NAME, O_RDONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия FIFO для чтения");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE / 2]; // Изменено
        if (read(fifo_fd, buffer, BUFFER_SIZE / 2) > 0) { // Изменено
            char child_time[BUFFER_SIZE];
            get_current_time(child_time, sizeof(child_time));
            printf("Дочерний процесс:\n");
            printf("Текущее время: %s\n", child_time);
            printf("Получено из FIFO: %s\n", buffer);
        }
        close(fifo_fd);
        exit(EXIT_SUCCESS);
    } else { 
        sleep(5);

        int fifo_fd = open(FIFO_NAME, O_WRONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия FIFO для записи");
            exit(EXIT_FAILURE);
        }

        char parent_time[BUFFER_SIZE];
        get_current_time(parent_time, sizeof(parent_time));

        char message[BUFFER_SIZE / 2]; // Изменено
        snprintf(message, sizeof(message), "PID: %d, Время: %s", getpid(), parent_time);

        write(fifo_fd, message, strlen(message) + 1);
        close(fifo_fd);

        wait(NULL);
        unlink(FIFO_NAME); 
    }
}

int main() {
    printf("Пример с pipe:\n");
    pipe_example();
    printf("\nПример с FIFO:\n");
    fifo_example();
    return 0;
}
