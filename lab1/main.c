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


int file_name_cmp(const void *a, const void *b) {
    const char *name_a = *(const char **)a;
    const char *name_b = *(const char **)b;

    
    if (strcmp(name_a, ".") == 0) return -1;
    if (strcmp(name_b, ".") == 0) return 1;
    if (strcmp(name_a, "..") == 0) return -1;
    if (strcmp(name_b, "..") == 0) return 1;

    return strcmp(name_a, name_b);  
}

void print_symlink_target(const char *filepath) {
    char link_target[1024];
    ssize_t len = readlink(filepath, link_target, sizeof(link_target) - 1);
    if (len != -1) {
        link_target[len] = '\0';
        printf(" -> %s", link_target);
    }
}

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

    char *file_names[1024];
    int count = 0;
    long total_blocks = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_name, entry->d_name);
        
        if (lstat(filepath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }

        total_blocks += file_stat.st_blocks;
        file_names[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    
    qsort(file_names, count, sizeof(char *), file_name_cmp);

    if (long_format) {
        printf("total %ld\n", total_blocks / 2);  
    }

    for (int i = 0; i < count; i++) {
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_name, file_names[i]);

        if (lstat(filepath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }

        
        if (S_ISDIR(file_stat.st_mode)) {
            file_type_color = BLUE_COLOR;
        } else if (S_ISLNK(file_stat.st_mode)) {
            file_type_color = CYAN_COLOR;
        } else if (file_stat.st_mode & S_IXUSR) {
            file_type_color = GREEN_COLOR;
        } else {
            file_type_color = RESET_COLOR;
        }

        
        if (long_format) {
    
            printf((S_ISDIR(file_stat.st_mode)) ? "d" :
                   (S_ISLNK(file_stat.st_mode)) ? "l" : "-");
            printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %2ld", (long) file_stat.st_nlink);
            printf(" %-8s", getpwuid(file_stat.st_uid)->pw_name);
            printf(" %-8s", getgrgid(file_stat.st_gid)->gr_name);
            printf(" %8ld", (long) file_stat.st_size);

            
            char timebuf[64];
            struct tm *tm_info = localtime(&file_stat.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
            printf(" %s ", timebuf);
        }

        
        printf("%s%s%s", file_type_color, file_names[i], RESET_COLOR);

    
        if (S_ISLNK(file_stat.st_mode)) {
            print_symlink_target(filepath);
        }

        printf("\n");
        free(file_names[i]);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int show_hidden = 0;
    int long_format = 0;
    char *dir_name = ".";  

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

    
    if (optind < argc) {
        dir_name = argv[optind];
    }


    list_directory(dir_name, show_hidden, long_format);
    
    return 0;
}
