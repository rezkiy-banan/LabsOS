#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

void mycat(int argc, char *argv[], int showEnds, int numberNonblank, int numberAll) {
    int lineNumber = 1;
    FILE *file;

    for (int i = optind; i < argc; i++) { 
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", argv[0]);
        return 1;
    }

    int showEnds = 0, numberNonblank = 0, numberAll = 0;
    int opt;

    
    while ((opt = getopt(argc, argv, "Ebn")) != -1) {
        switch (opt) {
            case 'E':
                showEnds = 1;
                break;
            case 'b':
                numberNonblank = 1;
                break;
            case 'n':
                numberAll = 1;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-E] [-b] [-n] [FILE]...\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Вызов функции для обработки файлов
    mycat(argc, argv, showEnds, numberNonblank, numberAll);
    
    return 0;
}
