/**
 * utils.c - Реализация утилит
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>

// Цвета для логов
#define LOG_COLOR_INFO    "\033[0;32m"
#define LOG_COLOR_WARNING "\033[1;33m"
#define LOG_COLOR_ERROR   "\033[0;31m"
#define LOG_COLOR_DEBUG   "\033[0;36m"
#define LOG_COLOR_RESET   "\033[0m"

// Выполнение команды
int execute_cmd(const char *cmd, bool verbose) {
    if (verbose) {
        printf(LOG_COLOR_DEBUG "[CMD] %s\n" LOG_COLOR_RESET, cmd);
    }

    int status = system(cmd);
    if (status == -1) {
        log_error("Ошибка выполнения команды: %s", cmd);
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

// Выполнение команды в chroot
int execute_cmd_chroot(const char *chroot, const char *cmd, bool verbose) {
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "chroot %s /bin/bash -c \"%s\"", chroot, cmd);
    return execute_cmd(full_cmd, verbose);
}

// Проверка существования файла
bool file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

// Проверка существования директории
bool dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

// Копирование файла
int copy_file(const char *src, const char *dst) {
    FILE *src_fp = fopen(src, "rb");
    if (!src_fp) {
        log_error("Не удалось открыть файл для чтения: %s", src);
        return -1;
    }

    FILE *dst_fp = fopen(dst, "wb");
    if (!dst_fp) {
        fclose(src_fp);
        log_error("Не удалось открыть файл для записи: %s", dst);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, bytes, dst_fp) != bytes) {
            fclose(src_fp);
            fclose(dst_fp);
            log_error("Ошибка записи в файл: %s", dst);
            return -1;
        }
    }

    fclose(src_fp);
    fclose(dst_fp);
    return 0;
}

// Запись в файл
int write_to_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        log_error("Не удалось открыть файл для записи: %s", path);
        return -1;
    }

    if (fputs(content, fp) == EOF) {
        fclose(fp);
        log_error("Ошибка записи в файл: %s", path);
        return -1;
    }

    fclose(fp);
    return 0;
}

// Чтение файла
char* read_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(size + 1);
    if (!content) {
        fclose(fp);
        return NULL;
    }

    fread(content, 1, size, fp);
    content[size] = '\0';

    fclose(fp);
    return content;
}

// Создание процесса
pid_t spawn_process(const char *cmd, int *stdin_fd, int *stdout_fd, int *stderr_fd) {
    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) { // Дочерний процесс
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(127);
    }

    // Родительский процесс
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    if (stdin_fd) *stdin_fd = stdin_pipe[1];
    if (stdout_fd) *stdout_fd = stdout_pipe[0];
    if (stderr_fd) *stderr_fd = stderr_pipe[0];

    return pid;
}

// Ожидание завершения процесса
int wait_process(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

// Логирование
void log_info(const char *format, ...) {
    va_list args;
    printf(LOG_COLOR_INFO "[INFO] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(LOG_COLOR_RESET "\n");
}

void log_warning(const char *format, ...) {
    va_list args;
    printf(LOG_COLOR_WARNING "[WARNING] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(LOG_COLOR_RESET "\n");
}

void log_error(const char *format, ...) {
    va_list args;
    printf(LOG_COLOR_ERROR "[ERROR] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(LOG_COLOR_RESET "\n");
}

void log_debug(const char *format, ...) {
    va_list args;
    printf(LOG_COLOR_DEBUG "[DEBUG] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(LOG_COLOR_RESET "\n");
}

// Проверка зависимостей
bool check_dependency(const char *cmd) {
    char check_cmd[256];
    snprintf(check_cmd, sizeof(check_cmd), "which %s > /dev/null 2>&1", cmd);
    return system(check_cmd) == 0;
}

bool check_all_dependencies() {
    const char *deps[] = {
        "mmdebstrap",
        "mksquashfs",
        "xorriso",
        "grub-mkrescue",
        "chroot",
        NULL
    };

    bool all_ok = true;
    for (int i = 0; deps[i] != NULL; i++) {
        if (!check_dependency(deps[i])) {
            log_error("Зависимость не найдена: %s", deps[i]);
            all_ok = false;
        }
    }

    return all_ok;
}
