#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 4096

typedef struct {
    char filename[256];
    off_t file_size;
    mode_t file_mode;
    uid_t file_uid;
    gid_t file_gid;
    time_t file_mtime;
} FileHeader;

// Вывод справки
void print_help() {
    printf("Использование:\n");
    printf("  ./archiver archive_name -i (--input) file1        Добавить файл в архив\n");
    printf("  ./archiver archive_name -e (--extract) file1      Извлечь файл из архива\n");
    printf("  ./archiver archive_name -d (--delete) file1       Удалить файл из архива\n");
    printf("  ./archiver archive_name -s (--stat)               Показать содержимое архива\n");
    printf("  ./archiver -h (--help)                            Показать справку\n");
}

// Проверка ошибок для операций с файлами
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Добавление файла в архив
int add_file(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (archive_fd < 0) handle_error("Ошибка открытия архива");

    int file_fd = open(file_name, O_RDONLY);
    if (file_fd < 0) {
        close(archive_fd);
        handle_error("Ошибка открытия файла");
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        close(file_fd);
        close(archive_fd);
        handle_error("Ошибка получения информации о файле");
    }

    FileHeader header = {
        .file_size = file_stat.st_size,
        .file_mode = file_stat.st_mode,
        .file_uid = file_stat.st_uid,
        .file_gid = file_stat.st_gid,
        .file_mtime = file_stat.st_mtime
    };
    strncpy(header.filename, file_name, sizeof(header.filename) - 1);

    // Запись заголовка файла
    if (write(archive_fd, &header, sizeof(header)) != sizeof(header)) {
        close(file_fd);
        close(archive_fd);
        handle_error("Ошибка записи заголовка файла");
    }

    // Копирование содержимого файла в архив
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(archive_fd, buffer, bytes_read) != bytes_read) {
            close(file_fd);
            close(archive_fd);
            handle_error("Ошибка записи данных файла в архив");
        }
    }

    close(file_fd);
    close(archive_fd);
    printf("Файл '%s' добавлен в архив '%s'\n", file_name, archive_name);
    return 0;
}

// Извлечение файла из архива
int extract_file(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) handle_error("Ошибка открытия архива");

    FileHeader header;
    int found = 0;

    while (read(archive_fd, &header, sizeof(header)) == sizeof(header)) {
        if (strcmp(header.filename, file_name) == 0) {
            int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, header.file_mode);
            if (file_fd < 0) {
                close(archive_fd);
                handle_error("Ошибка создания файла для извлечения");
            }

            char buffer[BUFFER_SIZE];
            ssize_t bytes_to_read = header.file_size;

            while (bytes_to_read > 0) {
                ssize_t bytes_read = read(archive_fd, buffer, bytes_to_read > BUFFER_SIZE ? BUFFER_SIZE : bytes_to_read);
                if (bytes_read <= 0) handle_error("Ошибка чтения данных файла");

                if (write(file_fd, buffer, bytes_read) != bytes_read) {
                    close(file_fd);
                    close(archive_fd);
                    handle_error("Ошибка записи данных в файл");
                }
                bytes_to_read -= bytes_read;
            }

            close(file_fd);
            printf("Файл '%s' извлечён из архива '%s'\n", file_name, archive_name);
            found = 1;
            break;
        } else {
            lseek(archive_fd, header.file_size, SEEK_CUR);
        }
    }

    if (!found) {
        fprintf(stderr, "Файл '%s' не найден в архиве '%s'\n", file_name, archive_name);
    }

    close(archive_fd);
    return found ? 0 : -1;
}

// Удаление файла из архива
int delete_file(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) handle_error("Ошибка открытия архива для чтения");

    char temp_name[] = "temp_archive_XXXXXX";
    int temp_fd = mkstemp(temp_name);
    if (temp_fd < 0) handle_error("Ошибка создания временного архива");

    FileHeader header;
    int file_found = 0;

    while (read(archive_fd, &header, sizeof(header)) == sizeof(header)) {
        if (strcmp(header.filename, file_name) == 0) {
            file_found = 1;
            lseek(archive_fd, header.file_size, SEEK_CUR);
        } else {
            if (write(temp_fd, &header, sizeof(header)) != sizeof(header)) {
                close(temp_fd);
                close(archive_fd);
                unlink(temp_name);
                handle_error("Ошибка записи данных в новый архив");
            }

            char buffer[BUFFER_SIZE];
            ssize_t bytes_left = header.file_size;
            while (bytes_left > 0) {
                ssize_t bytes_read = read(archive_fd, buffer, bytes_left > BUFFER_SIZE ? BUFFER_SIZE : bytes_left);
                if (bytes_read <= 0) handle_error("Ошибка чтения данных файла");

                if (write(temp_fd, buffer, bytes_read) != bytes_read) {
                    close(temp_fd);
                    close(archive_fd);
                    unlink(temp_name);
                    handle_error("Ошибка записи данных в новый архив");
                }
                bytes_left -= bytes_read;
            }
        }
    }

    close(archive_fd);
    close(temp_fd);

    if (file_found) {
        if (rename(temp_name, archive_name) < 0) {
            unlink(temp_name);
            handle_error("Ошибка замены старого архива");
        }
        printf("Файл '%s' удалён из архива '%s'\n", file_name, archive_name);
    } else {
        fprintf(stderr, "Файл '%s' не найден в архиве '%s'\n", file_name, archive_name);
        unlink(temp_name);
    }

    return file_found ? 0 : -1;
}

// Вывод содержимого архива
void print_archive_contents(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) handle_error("Ошибка открытия архива");

    FileHeader header;
    printf("Содержимое архива '%s':\n", archive_name);

    while (read(archive_fd, &header, sizeof(header)) == sizeof(header)) {
        printf("Файл: %s, Размер: %ld байт, Права: %o, UID: %d, GID: %d, Последнее изменение: %s",
               header.filename, header.file_size, header.file_mode, header.file_uid, header.file_gid, ctime(&header.file_mtime));
        lseek(archive_fd, header.file_size, SEEK_CUR);
    }

    close(archive_fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_help();
        return EXIT_FAILURE;
    }

    const char *archive_name = argv[1];
    const char *option = argv[2];

    if (strcmp(option, "-i") == 0 || strcmp(option, "--input") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Укажите файл для добавления в архив\n");
            return EXIT_FAILURE;
        }
        return add_file(archive_name, argv[3]);
    } else if (strcmp(option, "-e") == 0 || strcmp(option, "--extract") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Укажите файл для извлечения из архива\n");
            return EXIT_FAILURE;
        }
        return extract_file(archive_name, argv[3]);
    } else if (strcmp(option, "-d") == 0 || strcmp(option, "--delete") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Укажите файл для удаления из архива\n");
            return EXIT_FAILURE;
        }
        return delete_file(archive_name, argv[3]);
    } else if (strcmp(option, "-s") == 0 || strcmp(option, "--stat") == 0) {
        print_archive_contents(archive_name);
    } else if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0) {
        print_help();
    } else {
        fprintf(stderr, "Неизвестная опция: %s\n", option);
        print_help();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
