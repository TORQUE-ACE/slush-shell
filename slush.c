#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <pwd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_ARGS 64
#define LOGFILE ".slush_history.log"

int show_network = 0; // 0 = hidden, 1 = show
int stealth_mode = 1; // 1 = no logging
int rootmask_mode = 0; // 1 = root@host

void log_command(const char *cmd) {
    if (stealth_mode) return;

    const char *home = getenv("HOME");
    if (!home) return;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", home, LOGFILE);

    FILE *fp = fopen(path, "a");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(fp, "[%02d-%02d-%04d %02d:%02d:%02d] %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec, cmd);
    fclose(fp);
}

void get_ip_and_interface(char *ip, size_t ip_len, char *iface, size_t iface_len) {
    struct ifaddrs *ifaddr, *ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        int family = ifa->ifa_addr->sa_family;
        if ((ifa->ifa_flags & IFF_LOOPBACK) != 0) continue;
        if (family == AF_INET) {
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, ip_len, NULL, 0, NI_NUMERICHOST);
            strncpy(iface, ifa->ifa_name, iface_len);
            break;
        }
    }
    freeifaddrs(ifaddr);
}

void create_default_slushrc(const char *path) {
    FILE *file = fopen(path, "w");
    if (!file) return;

    fprintf(file,
        "# .slushrc - Configuration for slush shell\n"
        "# Network info option : show|hide (Default:hide)\n"
        "network=hide\n"
        "# Log commands option : on|off (Default:off)\n"
        "log=off\n"
        "# Rootmask hides the username with root :) option : on|off (Default:off)\n"
        "rootmask=off\n"
    );

    fclose(file);
}

void load_slushrc() {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[512];
    snprintf(path, sizeof(path), "%s/.slushrc", home);

    struct stat st;
    if (stat(path, &st) != 0) {
        create_default_slushrc(path);
    }

    FILE *file = fopen(path, "r");
    if (!file) return;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "network=hide", 12) == 0) {
            show_network = 0;
        } else if (strncmp(line, "network=show", 13) == 0) {
            show_network = 1;
        } else if (strncmp(line, "log=off", 7) == 0) {
            stealth_mode = 1;
        } else if (strncmp(line, "log=on", 6) == 0) {
            stealth_mode = 0;
        } else if (strncmp(line, "rootmask=on", 11) == 0) {
            rootmask_mode = 1;
        } else if (strncmp(line, "rootmask=off", 12) == 0) {
            rootmask_mode = 0;
        }
    }

    fclose(file);
}

void print_prompt(char *prompt, size_t size) {
    char hostname[128], cwd[512], ip[128] = "", iface[64] = "";
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    const char *user = rootmask_mode ? "root" : getenv("USER");

    if (show_network) {
        get_ip_and_interface(ip, sizeof(ip), iface, sizeof(iface));
        snprintf(prompt, size, "< %s@%s | %s ^_^ %s | %s > ", user, hostname, ip, iface, cwd);
    } else {
        snprintf(prompt, size, "< %s@%s | %s > ", user, hostname, cwd);
    }
}

void execute_command(char **tokens) {
    if (tokens[0] == NULL) return;

    if (strcmp(tokens[0], "cd") == 0) {
        const char *target_dir = tokens[1] ? tokens[1] : getenv("HOME");
        if (chdir(target_dir) != 0) {
            perror("slush ");
        }
        return;
    }

    if (strcmp(tokens[0], "network") == 0 && tokens[1]) {
        if (strcmp(tokens[1], "show") == 0) show_network = 1;
        else if (strcmp(tokens[1], "hide") == 0) show_network = 0;
        return;
    }

    if (strcmp(tokens[0], "log") == 0 && tokens[1]) {
        if (strcmp(tokens[1], "on") == 0) stealth_mode = 0;
        else if (strcmp(tokens[1], "off") == 0) stealth_mode = 1;
        return;
    }

    if (strcmp(tokens[0], "rootmask") == 0 && tokens[1]) {
        if (strcmp(tokens[1], "on") == 0) rootmask_mode = 1;
        else if (strcmp(tokens[1], "off") == 0) rootmask_mode = 0;
        return;
    }

    char full_cmd[1024] = "";
    for (int j = 0; tokens[j]; j++) {
        strcat(full_cmd, tokens[j]);
        strcat(full_cmd, " ");
    }
    log_command(full_cmd);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(tokens[0], tokens);
        perror("slush ");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("slush ");
    }
}

int main() {
    load_slushrc();

    char *line;
    char *tokens[MAX_ARGS];
    char prompt[1024];

    while (1) {
        print_prompt(prompt, sizeof(prompt));
        line = readline(prompt);
        if (!line) break;

        if (*line) add_history(line);

        int i = 0;
        char *token = strtok(line, " ");
        while (token && i < MAX_ARGS - 1) {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }
        tokens[i] = NULL;

        execute_command(tokens);
        free(line);
    }

    return 0;
}
