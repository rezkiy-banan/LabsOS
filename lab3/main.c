#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

// Функция для обработки завершения программы
void exit_handler() {
    printf("Program exit handler executed.\n");
}

// Обработчик сигнала SIGINT
void sigint_handler(int signo) {
    printf("Received SIGINT (Ctrl+C).\n");
}

// Обработчик сигнала SIGTERM
void sigterm_handler(int signo) {
    printf("Received SIGTERM.\n");
}

int main() {
    // Установка обработчика для завершения программы
    if (atexit(exit_handler) != 0) {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    // Установка обработчика SIGINT с помощью signal()
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    // Установка обработчика SIGTERM с помощью sigaction()
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Вызов fork()
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при вызове fork()
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Child process: PID = %d\n", getpid());
    } else {
        // Родительский процесс
        printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
    }

    // Дайте процессу время для обработки сигналов
    for (int i = 0; i < 10; i++) {
        printf("Working... (PID = %d)\n", getpid());
        sleep(1);
    }

    return 0;
}