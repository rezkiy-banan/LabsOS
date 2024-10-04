#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mygrep(const char *pattern, FILE *file) {
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, pattern)) {
            printf("%s", line);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s pattern [file...]\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    if (argc == 2) {
        // Read from stdin
        mygrep(pattern, stdin);
    } else {
        // Read from files
        for (int i = 2; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                perror("mygrep");
                return 1;
            }
            mygrep(pattern, file);
            fclose(file);
        }
    }
    return 0;
}