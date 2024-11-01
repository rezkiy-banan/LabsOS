#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int change_mode(const char *mode_str, const char *file) {
    struct stat st;
    if (stat(file, &st) == -1) {
        perror("Ошибка получения информации о файле");
        return -1;
    }

    mode_t new_mode = st.st_mode;

    if (isdigit(mode_str[0])) {
        // Если введено числовое представление прав доступа
        new_mode = strtol(mode_str, NULL, 8);
        if (chmod(file, new_mode) == -1) {
            perror("Ошибка изменения прав доступа");
            return -1;
        }
    } else {
        // Символьное представление прав доступа
        int add = (mode_str[1] == '+');
        char who = mode_str[0];
        char perm_type = mode_str[2];

        mode_t mask = 0;

        // Определяем нужную маску прав в зависимости от символов
        if (perm_type == 'r') mask = S_IRUSR | S_IRGRP | S_IROTH;
        else if (perm_type == 'w') mask = S_IWUSR | S_IWGRP | S_IWOTH;
        else if (perm_type == 'x') mask = S_IXUSR | S_IXGRP | S_IXOTH;

        // Применяем маску для конкретного пользователя
        if (who == 'u') mask &= S_IRWXU;
        else if (who == 'g') mask &= S_IRWXG;
        else if (who == 'o') mask &= S_IRWXO;

        if (add) {
            new_mode |= mask;
        } else {
            new_mode &= ~mask;
        }

        // Применяем новые права
        if (chmod(file, new_mode) == -1) {
            perror("Ошибка изменения прав доступа");
            return -1;
        }
    }

    return 0;
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <режим> <файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (change_mode(argv[1], argv[2]) == -1) {
        return EXIT_FAILURE;
    }

    printf("Права доступа успешно изменены.\n");
    return EXIT_SUCCESS;
}
