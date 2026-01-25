/**
 * config.h - Заголовочный файл конфигурации
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Структура конфигурации сборки
typedef struct {
    char distro_name[64];
    char distro_short_name[32];
    char version[16];
    char codename[32];
    char base_distro[32];
    char base_version[16];
    char architecture[16];

    // Пути
    char work_dir[256];
    char chroot_dir[256];
    char iso_dir[256];
    char output_iso[256];

    // Настройки
    bool verbose;
    bool clean_build;
    bool keep_chroot;

    // Пакеты для установки
    char **base_packages;
    int base_packages_count;
    char **desktop_packages;
    int desktop_packages_count;
    char **additional_packages;
    int additional_packages_count;
} BuildConfig;

// Функции работы с конфигурацией
void config_init(BuildConfig *config);
void config_load_from_file(BuildConfig *config, const char *filename);
void config_save_to_file(BuildConfig *config, const char *filename);
void config_print(BuildConfig *config);
void config_free(BuildConfig *config);

// Утилиты
char* config_expand_path(const char *path);

#endif // CONFIG_H
