#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"

static int is_numeric(const char *text) {               //Pointer zeigt auf das erste Zeichen von 'text'
    if (text[0] == '\0') {
        return 0;
    }

    for (size_t i = 0; text[i] != '\0'; ++i) {
        if (!isdigit((unsigned char) text[i])) {        //isdigit --> überprüft ob ein Zeichen zwischen '0' und '9' ist, deshalb das 'unsigned char'
            return 0;
        }
    }

    return 1;
}

static long get_process_memory(int pid) {
    char path[256];
    char line[256];

    snprintf(path, sizeof(path), "/proc/%d/status", pid);    //erstellt unseren String den wir mit 'fopen' lesen können

    FILE *file = fopen(path, "r");                         
    if (file == NULL) {
        return -1;
    }

    long memory_kb;
    while (fgets(line, sizeof(line), file) != NULL) {           //zeilenweise!!!
        if (strncmp(line, "VmRSS:", 6) == 0 &&
            sscanf(line + 6, "%ld", &memory_kb) == 1) {         //'line + 6' - Pointer Arithmetik, überspringt quasi 'VmRSS:'. &memory_kb ist unsere Zieladresse in der Wert gespeichert wird.
            fclose(file);
            return memory_kb;
        }
    }

    fclose(file);
    return -1;
}

int collect_processes(ProcessList *list) {
    if (list == NULL || list->items != NULL) {          //erwarte leere ProcessList
        return -1;
    }

    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir /proc");
        return -1;
    }

    size_t capacity = 10;           //theoretisch maxAnzahl an Prozessen
    size_t count = 0;               //tatsächliche Anzahl an Prozessen
    Process *processes = malloc(capacity * sizeof(*processes));     //Pointer auf reservierten Speicherbereich.
                                                                    //D.h. wir initialisieren hier Speicher für 10 theoretische Process Objekte
    if (processes == NULL) {
        perror("malloc");
        closedir(dir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!is_numeric(entry->d_name)) {
            continue;
        }

        Process process = {0};              //lokale Struktur, alle Attribute auf NULL
        process.pid = atoi(entry->d_name);  

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/comm", process.pid);     //comm --> command name

        FILE *file = fopen(path, "r");
        if (file == NULL) {
            continue;
        }

        if (fgets(process.name, sizeof(process.name), file) == NULL) {
            fclose(file);
            continue;
        }
        fclose(file);

        process.name[strcspn(process.name, "\n")] = '\0';
        process.memory_kb = get_process_memory(process.pid);

        if (count == capacity) {
            size_t new_capacity = capacity * 2;
            Process *larger = realloc(processes,
                                      new_capacity * sizeof(*processes));
            if (larger == NULL) {
                perror("realloc");
                free(processes);
                closedir(dir);
                return -1;
            }

            processes = larger;
            capacity = new_capacity;
        }

        processes[count] = process;
        ++count;
    }

    closedir(dir);

    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j) {
            if (processes[j].memory_kb > processes[i].memory_kb) {
                Process temporary = processes[i];
                processes[i] = processes[j];
                processes[j] = temporary;
            }
        }
    }

    list->items = processes;
    list->count = count;
    return 0;
}

void free_processes(ProcessList *list) {
    if (list == NULL) {
        return;
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
}