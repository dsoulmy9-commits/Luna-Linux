/**
 * config.c - Реализация модуля конфигурации
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

// Инициализация конфигурации по умолчанию
void config_init(BuildConfig *config) {
    // Основные настройки
    strcpy(config->distro_name, "Luna Linux");
    strcpy(config->distro_short_name, "luna");
    strcpy(config->version, "1.0");
    strcpy(config->codename, "stellar");
    strcpy(config->base_distro, "ubuntu");
    strcpy(config->base_version, "22.04");
    strcpy(config->architecture, "amd64");

    // Пути
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }

    snprintf(config->work_dir, sizeof(config->work_dir), "%s/luna-build", home);
    snprintf(config->chroot_dir, sizeof(config->chroot_dir), "%s/chroot", config->work_dir);
    snprintf(config->iso_dir, sizeof(config->iso_dir), "%s/iso", config->work_dir);
    snprintf(config->output_iso, sizeof(config->output_iso),
             "%s/Luna-Linux-1.0-amd64.iso", home);

    // Настройки
    config->verbose = false;
    config->clean_build = true;
    config->keep_chroot = false;

    // Базовые пакеты
    config->base_packages_count = 8;
    config->base_packages = malloc(config->base_packages_count * sizeof(char*));
    config->base_packages[0] = strdup("systemd");
    config->base_packages[1] = strdup("dbus");
    config->base_packages[2] = strdup("locales");
    config->base_packages[3] = strdup("sudo");
    config->base_packages[4] = strdup("network-manager");
    config->base_packages[5] = strdup("grub2");
    config->base_packages[6] = strdup("linux-image-generic");
    config->base_packages[7] = strdup("initramfs-tools");

    // Пакеты рабочего стола
    config->desktop_packages_count = 10;
    config->desktop_packages = malloc(config->desktop_packages_count * sizeof(char*));
    config->desktop_packages[0] = strdup("kde-plasma-desktop");
    config->desktop_packages[1] = strdup("plasma-workspace-wayland");
    config->desktop_packages[2] = strdup("kwin-wayland");
    config->desktop_packages[3] = strdup("sddm");
    config->desktop_packages[4] = strdup("dolphin");
    config->desktop_packages[5] = strdup("konsole");
    config->desktop_packages[6] = strdup("firefox");
    config->desktop_packages[7] = strdup("calamares");
    config->desktop_packages[8] = strdup("plymouth-themes");
    config->desktop_packages[9] = strdup("grub2-themes");

    // Дополнительные пакеты
    config->additional_packages_count = 5;
    config->additional_packages = malloc(config->additional_packages_count * sizeof(char*));
    config->additional_packages[0] = strdup("libreoffice");
    config->additional_packages[1] = strdup("vlc");
    config->additional_packages[2] = strdup("gimp");
    config->additional_packages[3] = strdup("neofetch");
    config->additional_packages[4] = strdup("git");
}

// Загрузка конфигурации из файла
void config_load_from_file(BuildConfig *config, const char *filename) {
    // Реализация загрузки из JSON/INI файла
    printf("Загрузка конфигурации из %s\n", filename);
    // TODO: Реализовать парсинг файла конфигурации
}

// Сохранение конфигурации в файл
void config_save_to_file(BuildConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Ошибка сохранения конфигурации");
        return;
    }

    fprintf(fp, "[Luna Linux Build Configuration]\n");
    fprintf(fp, "distro_name = %s\n", config->distro_name);
    fprintf(fp, "version = %s\n", config->version);
    fprintf(fp, "codename = %s\n", config->codename);
    fprintf(fp, "base_distro = %s\n", config->base_distro);
    fprintf(fp, "base_version = %s\n", config->base_version);
    fprintf(fp, "architecture = %s\n", config->architecture);
    fprintf(fp, "work_dir = %s\n", config->work_dir);
    fprintf(fp, "output_iso = %s\n", config->output_iso);

    fclose(fp);
}

// Вывод конфигурации
void config_print(BuildConfig *config) {
    printf("=== Конфигурация Luna Linux Builder ===\n");
    printf("Дистрибутив: %s %s (%s)\n",
           config->distro_name, config->version, config->codename);
    printf("База: %s %s %s\n",
           config->base_distro, config->base_version, config->architecture);
    printf("Рабочий каталог: %s\n", config->work_dir);
    printf("Выходной ISO: %s\n", config->output_iso);
    printf("Режим: %s\n", config->verbose ? "подробный" : "обычный");
    printf("=======================================\n");
}

// Освобождение памяти
void config_free(BuildConfig *config) {
    for (int i = 0; i < config->base_packages_count; i++) {
        free(config->base_packages[i]);
    }
    free(config->base_packages);

    for (int i = 0; i < config->desktop_packages_count; i++) {
        free(config->desktop_packages[i]);
    }
    free(config->desktop_packages);

    for (int i = 0; i < config->additional_packages_count; i++) {
        free(config->additional_packages[i]);
    }
    free(config->additional_packages);
}

// Расширение путей с переменными
char* config_expand_path(const char *path) {
    if (strstr(path, "~") == path) {
        const char *home = getenv("HOME");
        if (!home) {
            struct passwd *pw = getpwuid(getuid());
            home = pw->pw_dir;
        }

        char *expanded = malloc(strlen(home) + strlen(path));
        sprintf(expanded, "%s%s", home, path + 1);
        return expanded;
    }

    return strdup(path);
}
