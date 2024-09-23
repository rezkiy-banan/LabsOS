#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define RESET_COLOR "\033[0m"
#define BLUE_COLOR "\033[34m"
#define GREEN_COLOR "\033[32m"
#define CYAN_COLOR "\033[36m"

// Функция для обработки и вывода списка файлов
void list_directory(const char *dir_name, int show_hidden, int long_format) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char filepath[1024];
    char *file_type_color;

    if ((dir = opendir(dir_name)) == NULL) {
        perror("opendir");
        return;
    }

    // Массив для хранения имён файлов
    char *file_names[1024];
    int count = 0;
    long total_blocks = 0;

    // Чтение содержимого директории
    while ((entry = readdir(dir)) != NULL) {
        // Если не нужно показывать скрытые файлы и файл скрыт, пропускаем его
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_name, entry->d_name);
        
        if (lstat(filepath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }

        // Подсчёт общего количества блоков
        total_blocks += file_stat.st_blocks;

        file_names[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    // Сортировка списка файлов
    qsort(file_names, count, sizeof(char *), (int (*)(const void *, const void *)) strcmp);

    // Вывод строки "total"
    if (long_format) {
        printf("total %ld\n", total_blocks / 2);  // В некоторых системах используется деление на 2
    }

    // Вывод информации о каждом файле
    for (int i = 0; i < count; i++) {
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_name, file_names[i]);
        
        if (lstat(filepath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }

        // Определение типа файла и выбор цвета
        if (S_ISDIR(file_stat.st_mode)) {
            file_type_color = BLUE_COLOR;
        } else if (S_ISLNK(file_stat.st_mode)) {
            file_type_color = CYAN_COLOR;
        } else if (file_stat.st_mode & S_IXUSR) {
            file_type_color = GREEN_COLOR;
        } else {
            file_type_color = RESET_COLOR;
        }

        // Длинный формат вывода
        if (long_format) {
            // Права доступа
            printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
            printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

            // Количество ссылок, владелец, группа, размер
            printf(" %ld", (long) file_stat.st_nlink);
            printf(" %s", getpwuid(file_stat.st_uid)->pw_name);
            printf(" %s", getgrgid(file_stat.st_gid)->gr_name);
            printf(" %5ld", (long) file_stat.st_size);

            // Время изменения
            char timebuf[64];
            struct tm *tm_info = localtime(&file_stat.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
            printf(" %s ", timebuf);
        }

        // Имя файла с цветом
        printf("%s%s%s\n", file_type_color, file_names[i], RESET_COLOR);
        free(file_names[i]);
    }
}

// Основная функция программы
int main(int argc, char *argv[]) {
    int opt;
    int show_hidden = 0;
    int long_format = 0;
    char *dir_name = ".";  // по умолчанию текущая директория

    // Обработка опций командной строки
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            case 'a':
                show_hidden = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-a] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Если указан путь к директории, используем его
    if (optind < argc) {
        dir_name = argv[optind];
    }

    // Вызываем функцию для отображения содержимого директории
    list_directory(dir_name, show_hidden, long_format);
    
    return 0;
}
