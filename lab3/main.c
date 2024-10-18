#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


void exit_handler() {
    printf("Программа завершена.\n");
}


void sigint_handler(int signum) {
    printf("Получен сигнал SIGINT (номер: %d)\n", signum);
    exit(0); 
}


void sigterm_handler(int signum, siginfo_t *info, void *context) {
    printf("Получен сигнал SIGTERM (номер: %d)\n", signum);
    exit(0);
}

int main() {
    
    if (atexit(exit_handler) != 0) {
        perror("Ошибка при регистрации обработчика выхода");
        exit(EXIT_FAILURE);
    }

    
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Не удалось установить обработчик сигнала SIGINT");
        exit(EXIT_FAILURE);
    }

    
    struct sigaction sa;
    sa.sa_sigaction = sigterm_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Не удалось установить обработчик сигнала SIGTERM");
        exit(EXIT_FAILURE);
    }

    
    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка при вызове fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        
        printf("Дочерний процесс (PID: %d)\n", getpid());
        sleep(10);  
        exit(0);    
    } else {
        
        int status;
        printf("Родительский процесс (PID: %d), дочерний процесс PID: %d\n", getpid(), pid);
        
        
        if (wait(&status) == -1) {
            perror("Ошибка при ожидании дочернего процесса");
            exit(EXIT_FAILURE);
        }

        
        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился нормально с кодом: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс был завершён сигналом: %d\n", WTERMSIG(status));
        } else {
            printf("Дочерний процесс завершился с непредвиденным статусом.\n");
        }
    }

    return 0;
}