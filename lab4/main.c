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

    mode_t current_mode = st.st_mode; 
    mode_t new_mode = current_mode;

    if (isdigit(mode_str[0])) {
        
        if (strlen(mode_str) > 4 || strspn(mode_str, "01234567") != strlen(mode_str)) {
            fprintf(stderr, "Некорректное числовое представление прав доступа: %s\n", mode_str);
            return -1;
        }
        new_mode = strtol(mode_str, NULL, 8);
    } else {
        
        size_t len = strlen(mode_str);
        char who = 'a'; 
        int i = 0;

        
        if (len > 1 && (mode_str[0] == 'u' || mode_str[0] == 'g' || mode_str[0] == 'o' || mode_str[0] == 'a')) {
            who = mode_str[0];
            i++;
        }

        for (; i < len; i++) {
            char operation = mode_str[i];

            
            if (operation != '+' && operation != '-') {
                fprintf(stderr, "Некорректный формат режима: %s\n", mode_str);
                return -1;
            }

            i++;
            while (i < len && strchr("rwx", mode_str[i])) {
                char perm_type = mode_str[i];
                mode_t mask = 0;

                
                switch (perm_type) {
                    case 'r': mask = S_IRUSR | S_IRGRP | S_IROTH; break;
                    case 'w': mask = S_IWUSR | S_IWGRP | S_IWOTH; break;
                    case 'x': mask = S_IXUSR | S_IXGRP | S_IXOTH; break;
                    default:
                        fprintf(stderr, "Некорректный тип прав: %c\n", perm_type);
                        return -1;
                }

                
                switch (who) {
                    case 'u': mask &= S_IRWXU; break;
                    case 'g': mask &= S_IRWXG; break;
                    case 'o': mask &= S_IRWXO; break;
                    case 'a':  break;
                    default:
                        fprintf(stderr, "Некорректный указатель пользователя: %c\n", who);
                        return -1;
                }

                
                if (operation == '+') {
                    new_mode |= mask;
                } else if (operation == '-') {
                    new_mode &= ~mask;
                }

                i++;
            }

            
            i--;
        }
    }

    
    if (current_mode == new_mode) {
        printf("Права доступа не изменены. Текущие права уже соответствуют заданным.\n");
        return 0;
    }

    
    if (chmod(file, new_mode) == -1) {
        perror("Ошибка изменения прав доступа");
        return -1;
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
