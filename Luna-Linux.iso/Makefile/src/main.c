/**
 * Luna Linux Builder - Система сборки дистрибутива на C
 * Основной файл программы
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

// Конфигурация сборки
typedef struct {
    char distro_name[64];
    char distro_short_name[32];
    char version[16];
    char codename[32];
    char ubuntu_version[16];
    char ubuntu_codename[32];
    char arch[16];
    char workdir[256];
    char chroot[256];
    char imagedir[256];
    char isodir[256];
    char output_iso[256];
    int verbose;
    int clean_build;
} BuildConfig;

// Цвета для вывода
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_RESET   "\033[0m"

// Прототипы функций
void print_banner();
void init_config(BuildConfig *config);
int create_directory_structure(BuildConfig *config);
int execute_command(const char *cmd, int show_output);
int build_base_system(BuildConfig *config);
int customize_grub(BuildConfig *config);
int install_kde_plasma(BuildConfig *config);
int install_calamares(BuildConfig *config);
int install_additional_software(BuildConfig *config);
int prepare_iso_files(BuildConfig *config);
int create_boot_structure(BuildConfig *config);
int create_iso_image(BuildConfig *config);
int cleanup_build(BuildConfig *config);
void print_progress(int step, int total, const char *message);
int write_file(const char *filename, const char *content);

// Глобальные переменные
BuildConfig g_config;

int main(int argc, char *argv[]) {
    int option;
    int result = 0;

    // Инициализация конфигурации
    init_config(&g_config);

    // Парсинг аргументов командной строки
    while ((option = getopt(argc, argv, "vch")) != -1) {
        switch (option) {
            case 'v':
                g_config.verbose = 1;
                break;
            case 'c':
                g_config.clean_build = 1;
                break;
            case 'h':
                printf("Использование: %s [опции]\n", argv[0]);
                printf("  -v    Подробный вывод\n");
                printf("  -c    Полная очистка перед сборкой\n");
                printf("  -h    Эта справка\n");
                return 0;
            default:
                fprintf(stderr, "Неизвестная опция: %c\n", option);
                return 1;
        }
    }

    // Вывод баннера
    print_banner();

    // Проверка прав
    if (getuid() != 0) {
        printf(COLOR_RED "Ошибка: программа должна запускаться с правами root\n" COLOR_RESET);
        printf("Используйте: sudo %s\n", argv[0]);
        return 1;
    }

    printf(COLOR_CYAN "Начало сборки Luna Linux\n" COLOR_RESET);
    printf(COLOR_YELLOW "Дата и время: %s" COLOR_RESET, ctime(&(time_t){time(NULL)}));

    // Основные шаги сборки
    const char *steps[] = {
        "Создание структуры каталогов",
        "Построение базовой системы",
        "Настройка GRUB с кастомной темой",
        "Установка KDE Plasma с Wayland",
        "Установка графического установщика Calamares",
        "Установка дополнительного ПО",
        "Подготовка файлов для ISO",
        "Создание загрузочной структуры",
        "Создание ISO образа",
        "Завершение сборки"
    };

    int total_steps = sizeof(steps) / sizeof(steps[0]);

    // Выполнение шагов сборки
    for (int i = 0; i < total_steps; i++) {
        print_progress(i + 1, total_steps, steps[i]);

        int step_result = 0;
        switch (i) {
            case 0:
                step_result = create_directory_structure(&g_config);
                break;
            case 1:
                step_result = build_base_system(&g_config);
                break;
            case 2:
                step_result = customize_grub(&g_config);
                break;
            case 3:
                step_result = install_kde_plasma(&g_config);
                break;
            case 4:
                step_result = install_calamares(&g_config);
                break;
            case 5:
                step_result = install_additional_software(&g_config);
                break;
            case 6:
                step_result = prepare_iso_files(&g_config);
                break;
            case 7:
                step_result = create_boot_structure(&g_config);
                break;
            case 8:
                step_result = create_iso_image(&g_config);
                break;
            case 9:
                step_result = cleanup_build(&g_config);
                break;
        }

        if (step_result != 0) {
            printf(COLOR_RED "\nОшибка на шаге %d: %s\n" COLOR_RESET, i + 1, steps[i]);
            result = 1;
            break;
        }
    }

    if (result == 0) {
        printf(COLOR_GREEN "\n═══════════════════════════════════════════\n");
        printf("Сборка Luna Linux успешно завершена!\n");
        printf("ISO файл: %s\n", g_config.output_iso);

        // Проверка размера файла
        struct stat st;
        if (stat(g_config.output_iso, &st) == 0) {
            double size_mb = st.st_size / (1024.0 * 1024.0);
            printf("Размер: %.2f MB\n", size_mb);
        }

        printf("═══════════════════════════════════════════\n" COLOR_RESET);

        // Инструкция для записи на USB
        printf(COLOR_YELLOW "\nДля записи на USB используйте:\n" COLOR_RESET);
        printf("dd if=\"%s\" of=/dev/sdX bs=4M status=progress && sync\n", g_config.output_iso);
        printf(COLOR_YELLOW "\nИли используйте Etcher/Rufus/Ventoy\n" COLOR_RESET);
    }

    return result;
}

/**
 * Вывод баннера программы
 */
void print_banner() {
    printf(COLOR_BLUE "╔════════════════════════════════════════════════════╗\n");
    printf("║                    " COLOR_CYAN "Luna Linux Builder" COLOR_BLUE "              ║\n");
    printf("║           " COLOR_YELLOW "Сборка дистрибутива на языке C" COLOR_BLUE "         ║\n");
    printf("╚════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

/**
 * Инициализация конфигурации сборки
 */
void init_config(BuildConfig *config) {
    // Настройки Luna Linux
    strcpy(config->distro_name, "Luna Linux");
    strcpy(config->distro_short_name, "luna-linux");
    strcpy(config->version, "1.0");
    strcpy(config->codename, "stellar");
    strcpy(config->ubuntu_version, "22.04");
    strcpy(config->ubuntu_codename, "jammy");
    strcpy(config->arch, "amd64");

    // Пути
    snprintf(config->workdir, sizeof(config->workdir), "%s/luna-linux-build", getenv("HOME"));
    snprintf(config->chroot, sizeof(config->chroot), "%s/chroot", config->workdir);
    snprintf(config->imagedir, sizeof(config->imagedir), "%s/image", config->workdir);
    snprintf(config->isodir, sizeof(config->isodir), "%s/iso", config->workdir);
    snprintf(config->output_iso, sizeof(config->output_iso),
             "%s/Luna-Linux-%s-%s.iso", getenv("HOME"), config->ubuntu_version, config->arch);

    // Флаги
    config->verbose = 0;
    config->clean_build = 0;
}

/**
 * Создание структуры каталогов
 */
int create_directory_structure(BuildConfig *config) {
    printf(COLOR_YELLOW "Создание структуры каталогов...\n" COLOR_RESET);

    // Очистка предыдущей сборки при необходимости
    if (config->clean_build) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", config->workdir);
        execute_command(cmd, 0);
    }

    // Создание основных каталогов
    const char *dirs[] = {
        config->workdir,
        config->chroot,
        config->imagedir,
        config->isodir,
        NULL
    };

    for (int i = 0; dirs[i] != NULL; i++) {
        if (mkdir(dirs[i], 0755) != 0 && errno != EEXIST) {
            perror("Ошибка создания каталога");
            return 1;
        }
    }

    return 0;
}

/**
 * Построение базовой системы Ubuntu
 */
int build_base_system(BuildConfig *config) {
    printf(COLOR_YELLOW "Построение базовой системы...\n" COLOR_RESET);

    // Проверка наличия mmdebstrap
    if (execute_command("which mmdebstrap", 0) != 0) {
        printf(COLOR_RED "Ошибка: mmdebstrap не установлен\n" COLOR_RESET);
        printf("Установите: apt install mmdebstrap\n");
        return 1;
    }

    // Команда для создания базовой системы
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "mmdebstrap --variant=important "
        "--include=systemd,systemd-sysv,dbus,locales,kbd,console-setup,network-manager "
        "%s %s http://archive.ubuntu.com/ubuntu/",
        config->ubuntu_codename, config->chroot);

    return execute_command(cmd, config->verbose);
}

/**
 * Настройка кастомного GRUB с темой Luna Linux
 */
int customize_grub(BuildConfig *config) {
    printf(COLOR_YELLOW "Настройка кастомного GRUB с логотипом Луны...\n" COLOR_RESET);

    // Создание скрипта настройки GRUB
    const char *grub_setup =
        "#!/bin/bash\n"
        "set -e\n\n"
        "# Установка GRUB\n"
        "apt update\n"
        "apt install -y grub2-common grub-pc grub-efi-amd64 grub-efi-amd64-bin\n\n"
        "# Создание кастомной темы Luna Linux\n"
        "mkdir -p /boot/grub/themes/luna-linux\n\n"
        "# Создание файла темы\n"
        "cat > /boot/grub/themes/luna-linux/theme.txt << 'EOF'\n"
        "# Luna Linux GRUB Theme\n\n"
        "desktop-color: \"#0f0f1a\"\n"
        "desktop-image: \"background.png\"\n\n"
        "+ boot_menu {\n"
        "    left = 30%\n"
        "    top = 30%\n"
        "    width = 40%\n"
        "    height = 40%\n"
        "    item_font = \"Unifont Regular 16\"\n"
        "    item_color = \"#ffffff\"\n"
        "    selected_item_color = \"#ff6600\"\n"
        "    item_height = 40\n"
        "    item_spacing = 10\n"
        "}\n\n"
        "+ label {\n"
        "    text = \"Luna Linux\"\n"
        "    color = \"#ff6600\"\n"
        "    font = \"Unifont Regular 24\"\n"
        "    left = 50%\n"
        "    top = 20%\n"
        "    align = \"center\"\n"
        "}\n\n"
        "+ label {\n"
        "    text = \"Stellar Edition\"\n"
        "    color = \"#aaaaaa\"\n"
        "    font = \"Unifont Regular 16\"\n"
        "    left = 50%\n"
        "    top = 26%\n"
        "    align = \"center\"\n"
        "}\n"
        "EOF\n\n"
        "# Создание фонового изображения (простой градиент)\n"
        "echo 'iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAOxAAADsQBlSsOGwAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAArSURBVHic7cEBDQAAAMKg9U9tCF8gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB8GQNkAAECp1Zh3QAAAABJRU5ErkJggg==' | base64 -d > /boot/grub/themes/luna-linux/background.png\n\n"
        "# Настройка конфигурации GRUB\n"
        "cat > /etc/default/grub << 'EOF'\n"
        "GRUB_DEFAULT=0\n"
        "GRUB_TIMEOUT=10\n"
        "GRUB_TIMEOUT_STYLE=menu\n"
        "GRUB_DISTRIBUTOR=\"Luna Linux\"\n"
        "GRUB_CMDLINE_LINUX_DEFAULT=\"quiet splash\"\n"
        "GRUB_CMDLINE_LINUX=\"\"\n"
        "GRUB_BACKGROUND=\"/boot/grub/themes/luna-linux/background.png\"\n"
        "GRUB_THEME=\"/boot/grub/themes/luna-linux/theme.txt\"\n"
        "GRUB_GFXMODE=auto\n"
        "GRUB_DISABLE_OS_PROBER=false\n"
        "GRUB_DISABLE_RECOVERY=\"true\"\n"
        "EOF\n\n"
        "# Обновление GRUB\n"
        "update-grub\n";

    // Сохранение скрипта во временный файл
    char script_path[] = "/tmp/setup-grub.sh";
    if (write_file(script_path, grub_setup) != 0) {
        return 1;
    }

    // Копирование скрипта в chroot и выполнение
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/tmp/", script_path, config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chmod +x %s/tmp/setup-grub.sh", config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chroot %s /bin/bash /tmp/setup-grub.sh", config->chroot);
    return execute_command(cmd, config->verbose);
}

/**
 * Установка KDE Plasma с поддержкой Wayland
 */
int install_kde_plasma(BuildConfig *config) {
    printf(COLOR_YELLOW "Установка KDE Plasma с Wayland...\n" COLOR_RESET);

    const char *kde_setup =
        "#!/bin/bash\n"
        "set -e\n\n"
        "apt update\n"
        "apt install -y \\\n"
        "    kde-plasma-desktop \\\n"
        "    plasma-workspace-wayland \\\n"
        "    kwin-wayland \\\n"
        "    sddm \\\n"
        "    sddm-theme-breeze \\\n"
        "    plasma-nm \\\n"
        "    plasma-pa \\\n"
        "    dolphin \\\n"
        "    konsole \\\n"
        "    kate \\\n"
        "    ark\n\n"
        "# Настройка SDDM\n"
        "cat > /etc/sddm.conf << 'EOF'\n"
        "[Autologin]\n"
        "User=luna\n"
        "Session=plasmawayland\n\n"
        "[Theme]\n"
        "Current=breeze\n\n"
        "[Wayland]\n"
        "CompositorCommand=kwin_wayland --no-lockscreen\n"
        "EOF\n\n"
        "# Создание пользователя luna\n"
        "useradd -m -s /bin/bash luna || true\n"
        "echo \"luna:luna\" | chpasswd\n"
        "usermod -aG sudo luna\n"
        "echo \"luna ALL=(ALL) NOPASSWD:ALL\" > /etc/sudoers.d/luna\n"
        "chmod 440 /etc/sudoers.d/luna\n";

    char script_path[] = "/tmp/setup-kde.sh";
    if (write_file(script_path, kde_setup) != 0) {
        return 1;
    }

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/tmp/", script_path, config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chmod +x %s/tmp/setup-kde.sh", config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chroot %s /bin/bash /tmp/setup-kde.sh", config->chroot);
    return execute_command(cmd, config->verbose);
}

/**
 * Установка графического установщика Calamares
 */
int install_calamares(BuildConfig *config) {
    printf(COLOR_YELLOW "Установка Calamares...\n" COLOR_RESET);

    const char *calamares_setup =
        "#!/bin/bash\n"
        "set -e\n\n"
        "apt install -y calamares calamares-settings-ubuntu\n\n"
        "# Создание конфигурации для Luna Linux\n"
        "mkdir -p /etc/calamares\n"
        "cp -r /usr/share/calamares/* /etc/calamares/\n\n"
        "# Брендинг Luna Linux\n"
        "mkdir -p /usr/share/calamares/branding/luna-linux\n"
        "cat > /usr/share/calamares/branding/luna-linux/branding.desc << 'EOF'\n"
        "---\n"
        "componentName:  Luna Linux\n"
        "shortName:      Luna\n"
        "version:        1.0\n"
        "bootloaderEntryName: \"Luna Linux\"\n"
        "welcomeStyleCalamares: true\n"
        "---\n"
        "EOF\n";

    char script_path[] = "/tmp/setup-calamares.sh";
    if (write_file(script_path, calamares_setup) != 0) {
        return 1;
    }

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/tmp/", script_path, config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chmod +x %s/tmp/setup-calamares.sh", config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chroot %s /bin/bash /tmp/setup-calamares.sh", config->chroot);
    return execute_command(cmd, config->verbose);
}

/**
 * Установка дополнительного программного обеспечения
 */
int install_additional_software(BuildConfig *config) {
    printf(COLOR_YELLOW "Установка дополнительного ПО...\n" COLOR_RESET);

    const char *software_setup =
        "#!/bin/bash\n"
        "set -e\n\n"
        "apt update\n"
        "apt install -y \\\n"
        "    firefox \\\n"
        "    libreoffice \\\n"
        "    vlc \\\n"
        "    gimp \\\n"
        "    neofetch \\\n"
        "    curl \\\n"
        "    wget \\\n"
        "    git \\\n"
        "    nano\n\n"
        "# Создание системных идентификаторов Luna Linux\n"
        "echo \"Luna Linux Stellar 1.0\" > /etc/luna-linux-release\n"
        "cat > /etc/os-release << 'EOF'\n"
        "NAME=\"Luna Linux\"\n"
        "VERSION=\"1.0 (Stellar)\"\n"
        "ID=luna\n"
        "ID_LIKE=ubuntu debian\n"
        "PRETTY_NAME=\"Luna Linux Stellar\"\n"
        "VERSION_ID=\"1.0\"\n"
        "HOME_URL=\"https://luna-linux.org\"\n"
        "SUPPORT_URL=\"https://forum.luna-linux.org\"\n"
        "BUG_REPORT_URL=\"https://bugs.luna-linux.org\"\n"
        "PRIVACY_POLICY_URL=\"https://luna-linux.org/privacy\"\n"
        "VERSION_CODENAME=stellar\n"
        "UBUNTU_CODENAME=jammy\n"
        "EOF\n\n"
        "cat > /etc/lsb-release << 'EOF'\n"
        "DISTRIB_ID=LunaLinux\n"
        "DISTRIB_RELEASE=1.0\n"
        "DISTRIB_CODENAME=stellar\n"
        "DISTRIB_DESCRIPTION=\"Luna Linux Stellar\"\n"
        "EOF\n\n"
        "# Чистка системы\n"
        "apt autoremove -y\n"
        "apt clean\n";

    char script_path[] = "/tmp/setup-software.sh";
    if (write_file(script_path, software_setup) != 0) {
        return 1;
    }

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/tmp/", script_path, config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chmod +x %s/tmp/setup-software.sh", config->chroot);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "chroot %s /bin/bash /tmp/setup-software.sh", config->chroot);
    return execute_command(cmd, config->verbose);
}

/**
 * Подготовка файлов для создания ISO образа
 */
int prepare_iso_files(BuildConfig *config) {
    printf(COLOR_YELLOW "Подготовка файлов для ISO...\n" COLOR_RESET);

    // Поиск и копирование ядра
    char find_cmd[512];
    char vmlinuz_path[512];
    FILE *fp;

    snprintf(find_cmd, sizeof(find_cmd), "find %s/boot -name 'vmlinuz-*' -type f | head -1", config->chroot);
    fp = popen(find_cmd, "r");
    if (fp == NULL) return 1;

    if (fgets(vmlinuz_path, sizeof(vmlinuz_path), fp) == NULL) {
        pclose(fp);
        return 1;
    }
    pclose(fp);

    // Удаление символа новой строки
    vmlinuz_path[strcspn(vmlinuz_path, "\n")] = 0;

    // Копирование vmlinuz
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/vmlinuz", vmlinuz_path, config->imagedir);
    if (execute_command(cmd, config->verbose) != 0) return 1;

    // Поиск и копирование initrd
    snprintf(find_cmd, sizeof(find_cmd), "find %s/boot -name 'initrd.img-*' -type f | head -1", config->chroot);
    fp = popen(find_cmd, "r");
    if (fp == NULL) return 1;

    char initrd_path[512];
    if (fgets(initrd_path, sizeof(initrd_path), fp) == NULL) {
        pclose(fp);
        return 1;
    }
    pclose(fp);

    initrd_path[strcspn(initrd_path, "\n")] = 0;
    snprintf(cmd, sizeof(cmd), "cp %s %s/initrd", initrd_path, config->imagedir);
    if (execute_command(cmd, config->verbose) != 0) return 1;

    // Создание squashfs образа
    printf(COLOR_YELLOW "Создание squashfs образа...\n" COLOR_RESET);
    snprintf(cmd, sizeof(cmd),
        "mksquashfs %s %s/filesystem.squashfs -comp xz -b 1M -noappend",
        config->chroot, config->imagedir);

    return execute_command(cmd, config->verbose);
}

/**
 * Создание загрузочной структуры для LiveCD
 */
int create_boot_structure(BuildConfig *config) {
    printf(COLOR_YELLOW "Создание загрузочной структуры LiveCD...\n" COLOR_RESET);

    // Создание каталогов
    const char *dirs[] = {
        "/boot/grub",
        "/casper",
        "/.disk",
        NULL
    };

    for (int i = 0; dirs[i] != NULL; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", config->isodir, dirs[i]);
        mkdir(path, 0755);
    }

    // Копирование файлов
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s/vmlinuz %s/casper/", config->imagedir, config->isodir);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "cp %s/initrd %s/casper/", config->imagedir, config->isodir);
    execute_command(cmd, 0);

    snprintf(cmd, sizeof(cmd), "cp %s/filesystem.squashfs %s/casper/", config->imagedir, config->isodir);
    execute_command(cmd, 0);

    // Создание конфигурации GRUB для LiveCD
    const char *grub_cfg =
        "set timeout=30\n"
        "set default=0\n\n"
        "menuentry \"Start Luna Linux Live (Wayland)\" {\n"
        "    linux /casper/vmlinuz boot=casper noprompt quiet splash ---\n"
        "    initrd /casper/initrd\n"
        "}\n\n"
        "menuentry \"Start Luna Linux Live (Safe Graphics)\" {\n"
        "    linux /casper/vmlinuz boot=casper nomodeset quiet splash ---\n"
        "    initrd /casper/initrd\n"
        "}\n\n"
        "menuentry \"Install Luna Linux\" {\n"
        "    linux /casper/vmlinuz boot=casper noprompt only-ubiquity quiet splash ---\n"
        "    initrd /casper/initrd\n"
        "}\n\n"
        "menuentry \"Boot from first hard disk\" {\n"
        "    set root=(hd0)\n"
        "    chainloader +1\n"
        "}\n";

    char grub_cfg_path[512];
    snprintf(grub_cfg_path, sizeof(grub_cfg_path), "%s/boot/grub/grub.cfg", config->isodir);
    if (write_file(grub_cfg_path, grub_cfg) != 0) {
        return 1;
    }

    // Создание файла информации о диске
    const char *disk_info =
        "Luna Linux Stellar 1.0 amd64\n"
        "Based on Ubuntu 22.04 LTS\n";

    char disk_info_path[512];
    snprintf(disk_info_path, sizeof(disk_info_path), "%s/.disk/info", config->isodir);
    if (write_file(disk_info_path, disk_info) != 0) {
        return 1;
    }

    return 0;
}

/**
 * Создание ISO образа
 */
int create_iso_image(BuildConfig *config) {
    printf(COLOR_YELLOW "Создание ISO образа...\n" COLOR_RESET);

    // Проверка наличия xorriso
    if (execute_command("which xorriso", 0) != 0) {
        printf(COLOR_RED "Ошибка: xorriso не установлен\n" COLOR_RESET);
        printf("Установите: apt install xorriso\n");
        return 1;
    }

    // Команда создания ISO
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "xorriso -as mkisofs \\\n"
        "    -volid \"Luna Linux\" \\\n"
        "    -full-iso9660-filenames \\\n"
        "    -joliet \\\n"
        "    -rational-rock \\\n"
        "    -iso-level 3 \\\n"
        "    -eltorito-boot boot/grub/bios.img \\\n"
        "    -no-emul-boot \\\n"
        "    -boot-load-size 4 \\\n"
        "    -boot-info-table \\\n"
        "    --efi-boot boot/grub/efi.img \\\n"
        "    -efi-boot-part --efi-boot-image \\\n"
        "    --protective-msdos-label \\\n"
        "    -isohybrid-gpt-basdat \\\n"
        "    -o \"%s\" \\\n"
        "    \"%s\"",
        config->output_iso, config->isodir);

    return execute_command(cmd, config->verbose);
}

/**
 * Очистка временных файлов
 */
int cleanup_build(BuildConfig *config) {
    printf(COLOR_YELLOW "Очистка временных файлов...\n" COLOR_RESET);

    // Удаление временных скриптов
    const char *scripts[] = {
        "/tmp/setup-grub.sh",
        "/tmp/setup-kde.sh",
        "/tmp/setup-calamares.sh",
        "/tmp/setup-software.sh",
        NULL
    };

    for (int i = 0; scripts[i] != NULL; i++) {
        unlink(scripts[i]);
    }

    return 0;
}

/**
 * Выполнение системной команды
 */
int execute_command(const char *cmd, int show_output) {
    if (show_output) {
        printf(COLOR_CYAN "Выполнение: %s\n" COLOR_RESET, cmd);
    }

    int status = system(cmd);
    if (status != 0) {
        if (!show_output) {
            printf(COLOR_RED "Ошибка выполнения команды: %s\n" COLOR_RESET, cmd);
        }
        return 1;
    }

    return 0;
}

/**
 * Вывод прогресса выполнения
 */
void print_progress(int step, int total, const char *message) {
    float percentage = (float)step / total * 100;
    printf(COLOR_BLUE "[%d/%d] ", step, total);
    printf(COLOR_CYAN "%.0f%% " COLOR_RESET, percentage);
    printf("%s\n", message);
}

/**
 * Запись содержимого в файл
 */
int write_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Ошибка открытия файла");
        return 1;
    }

    if (fputs(content, fp) == EOF) {
        perror("Ошибка записи в файл");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    chmod(filename, 0755); // Делаем скрипты исполняемыми
    return 0;
}
