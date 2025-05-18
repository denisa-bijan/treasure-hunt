#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include<stdarg.h>
typedef struct {
    int id;
    char name[40];
    float lant, lont;
    char clue[512];
    int value;
} COMOARA;

typedef struct {
    char name[40];
    int total;
} SCOR;

SCOR scoruri[100];
int n = 0;

void adauga(char *name, int value) {
    for (int i = 0; i < n; i++) {
        if (strcmp(scoruri[i].name, name) == 0) {
            scoruri[i].total += value;
            return;
        }
    }
    strcpy(scoruri[n].name, name);
    scoruri[n].total = value;
    n++;
}

void afiseaza() {
    char linie[256];
    for (int i = 0; i < n; i++) {
        int len = snprintf(linie, sizeof(linie), "%s %d\n", scoruri[i].name, scoruri[i].total);
        write(1, linie, len);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(1, "Usage: ./calculate_score <hunt_id>\n", 36);
        _exit(1);
    }

    char path[128];
    snprintf(path, sizeof(path), "treasure_hunt/%s", argv[1]);

    DIR *dir = opendir(path);
    if (!dir) {
        write(1, "Eroare la deschiderea hunt-ului.\n", 34);
        _exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, "comoara")) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);

            int fd = open(file_path, O_RDONLY);
            if (fd < 0) continue;

            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA)) {
                adauga(c.name, c.value);
            }
            close(fd);
        }
    }

    closedir(dir);
    afiseaza();
    return 0;
}
