#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>

#define HEADER_SIZE 256

void print_help() {
    printf("Использование:\n");
    printf("./archiver arch_name -i (--input) file1  # Добавить файл в архив\n");
    printf("./archiver arch_name -e (--extract) file1  # Извлечь файл из архива\n");
    printf("./archiver arch_name -s (--stat)  # Показать содержимое архива\n");
    printf("./archiver -h (--help)  # Показать справку\n");
}

int add_file_to_archive(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return -1;
    }

    struct stat file_stat;
    if (stat(file_name, &file_stat) == -1) {
        perror("Ошибка чтения информации о файле");
        close(archive_fd);
        return -1;
    }

    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("Ошибка открытия файла");
        close(archive_fd);
        return -1;
    }

    char header[HEADER_SIZE];
    snprintf(header, HEADER_SIZE, "%s %ld %o\n", file_name, file_stat.st_size, file_stat.st_mode);
    if (write(archive_fd, header, HEADER_SIZE) == -1) {
        perror("Ошибка записи заголовка в архив");
        close(archive_fd);
        close(file_fd);
        return -1;
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(archive_fd, buffer, bytes_read) == -1) {
            perror("Ошибка записи файла в архив");
            close(archive_fd);
            close(file_fd);
            return -1;
        }
    }

    close(file_fd);
    close(archive_fd);
    return 0;
}

int extract_file_from_archive(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return -1;
    }

    char header[HEADER_SIZE];
    while (read(archive_fd, header, HEADER_SIZE) > 0) {
        char stored_file_name[256];
        long file_size;
        mode_t file_mode;
        int num_items = sscanf(header, "%s %ld %o", stored_file_name, &file_size, &file_mode);
        if (num_items != 3) {
            fprintf(stderr, "Ошибка: некорректный заголовок\n");
            close(archive_fd);
            return -1;
        }

        if (strcmp(stored_file_name, file_name) == 0) {
            int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
            if (file_fd == -1) {
                perror("Ошибка создания файла");
                close(archive_fd);
                return -1;
            }

            char buffer[1024];
            ssize_t bytes_to_read;
            // Приводим file_size к типу size_t
            size_t size_to_read = (file_size > sizeof(buffer)) ? sizeof(buffer) : (size_t)file_size;

            while (file_size > 0 && (bytes_to_read = read(archive_fd, buffer, size_to_read)) > 0) {
                write(file_fd, buffer, bytes_to_read);
                file_size -= bytes_to_read;
                size_to_read = (file_size > sizeof(buffer)) ? sizeof(buffer) : (size_t)file_size; // Обновляем значение
            }

            close(file_fd);
            close(archive_fd);
            return 0;
        } else {
            lseek(archive_fd, file_size, SEEK_CUR);
        }
    }

    printf("Файл %s не найден в архиве\n", file_name);
    close(archive_fd);
    return -1;
}

void show_archive_stat(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return;
    }

    char header[HEADER_SIZE];
    printf("Содержимое архива %s:\n", archive_name);
    while (read(archive_fd, header, HEADER_SIZE) > 0) {
        char file_name[256];
        long file_size;
        mode_t file_mode;
        sscanf(header, "%s %ld %o", file_name, &file_size, &file_mode);
        printf("Файл: %s, Размер: %ld, Права: %o\n", file_name, file_size, file_mode);
        lseek(archive_fd, file_size, SEEK_CUR);
    }

    close(archive_fd);
}

int delete_file_from_archive(const char *archive_name, const char *file_name) {
    // Открываем оригинальный архив для чтения
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return -1;
    }

    // Создаем временный архив для записи
    int temp_fd = open("temp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("Ошибка создания временного архива");
        close(archive_fd);
        return -1;
    }

    char header[HEADER_SIZE];
    int file_found = 0; // Переменная для отслеживания, найден ли файл

    // Читаем заголовки и содержимое файлов
    while (read(archive_fd, header, HEADER_SIZE) > 0) {
        char stored_file_name[256];
        long file_size;
        mode_t file_mode;
        sscanf(header, "%s %ld %o", stored_file_name, &file_size, &file_mode);

        // Если файл не совпадает с удаляемым, записываем его в временный архив
        if (strcmp(stored_file_name, file_name) == 0) {
            file_found = 1; // Отметить, что файл найден
            lseek(archive_fd, file_size, SEEK_CUR); // Пропустить содержимое этого файла
            continue;
        }

        // Записываем заголовок в временный архив
        write(temp_fd, header, HEADER_SIZE);

        // Копируем содержимое файла в временный архив
        char buffer[1024];
        ssize_t bytes_to_read;
        while (file_size > 0 && (bytes_to_read = read(archive_fd, buffer, (file_size > sizeof(buffer)) ? sizeof(buffer) : file_size)) > 0) {
            write(temp_fd, buffer, bytes_to_read);
            file_size -= bytes_to_read;
        }
    }

    if (file_found) {
        printf("Файл %s был удален из архива.\n", file_name);
    } else {
        printf("Файл %s не найден в архиве.\n", file_name);
    }

    close(archive_fd);
    close(temp_fd);

    // Заменяем оригинальный архив временным
    remove(archive_name);
    rename("temp_archive", archive_name);

    return file_found ? 0 : -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (argc < 3) {
        print_help();
        return EXIT_FAILURE;
    }

    const char *archive_name = argv[1];
    const char *option = argv[2];

    if (strcmp(option, "-i") == 0 || strcmp(option, "--input") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Ошибка: укажите файл для добавления\n");
            return EXIT_FAILURE;
        }
        return add_file_to_archive(archive_name, argv[3]);
    } else if (strcmp(option, "-e") == 0 || strcmp(option, "--extract") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Ошибка: укажите файл для извлечения\n");
            return EXIT_FAILURE;
        }
        return extract_file_from_archive(archive_name, argv[3]);
    } else if (strcmp(option, "-s") == 0 || strcmp(option, "--stat") == 0) {
        show_archive_stat(archive_name);
        return EXIT_SUCCESS;
    } else if (strcmp(option, "-d") == 0 || strcmp(option, "--delete") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Ошибка: укажите файл для удаления\n");
            return EXIT_FAILURE;
        }
        return delete_file_from_archive(archive_name, argv[3]);
    } else {
        print_help();
        return EXIT_FAILURE;
    }
}