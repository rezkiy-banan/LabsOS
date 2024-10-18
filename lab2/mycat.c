#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mycat(int argc, char *argv[], int showEnds, int numberNonblank, int numberAll) {
    int lineNumber = 1;
    FILE *file;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') { // это файл, а не флаг
            file = fopen(argv[i], "r");
            if (file == NULL) {
                perror("mycat");
                exit(1);
            }

            char line[1024];
            while (fgets(line, sizeof(line), file)) {
                if (numberAll || (numberNonblank && strlen(line) > 1)) {
                    printf("%6d\t", lineNumber++);
                }
                printf("%s", line);
                if (showEnds) printf("$");
            }

            fclose(file);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", argv[0]);
        return 1;
    }

    int showEnds = 0, numberNonblank = 0, numberAll = 0;

    // Сначала обрабатываем все флаги
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char *option = argv[i] + 1;
            if (strchr(option, 'E')) showEnds = 1;
            if (strchr(option, 'b')) numberNonblank = 1;
            if (strchr(option, 'n')) numberAll = 1;
        }
    }

    // Потом обрабатываем файлы
    mycat(argc, argv, showEnds, numberNonblank, numberAll);
    
    return 0;
}
