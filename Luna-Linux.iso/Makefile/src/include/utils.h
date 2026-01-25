/**
 * utils.h - Утилиты для сборки
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

// Выполнение команды с выводом
int execute_cmd(const char *cmd, bool verbose);
int execute_cmd_chroot(const char *chroot, const char *cmd, bool verbose);

// Работа с файлами
bool file_exists(const char *path);
bool dir_exists(const char *path);
int copy_file(const char *src, const char *dst);
int write_to_file(const char *path, const char *content);
char* read_file(const char *path);

// Работа с процессами
pid_t spawn_process(const char *cmd, int *stdin_fd, int *stdout_fd, int *stderr_fd);
int wait_process(pid_t pid);

// Логирование
void log_info(const char *format, ...);
void log_warning(const char *format, ...);
void log_error(const char *format, ...);
void log_debug(const char *format, ...);

// Проверка зависимостей
bool check_dependency(const char *cmd);
bool check_all_dependencies();

#endif // UTILS_H
