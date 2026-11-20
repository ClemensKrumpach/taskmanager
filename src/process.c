#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>  // atoi()
#include <string.h>  // strcspn()

#include "process.h"

static int is_numeric(const char *str){
    if(str[0] == '\0'){
        return 0;
    }

    for(int i = 0; str[i] != '\0'; i++){
        if(!isdigit((unsigned char)str[i])){
            return 0;
        }
    }
    return 1;
}

static long get_process_memory(int pid){
    char path[256];
    char line[256];
    long memory_kb = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *fp = fopen(path, "r");

    if (fp == NULL){
        perror("fopen");
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0 &&
            sscanf(line + 6, "%ld", &memory_kb) == 1) {
            fclose(fp);
            return memory_kb;
        }
    }
    fclose(fp);
    return -1;
}

void list_processes(){
    DIR *dir = opendir("/proc");

    if (dir == NULL){
        perror("opendir");
        return;
    }

    struct dirent *entry;

    int capacity = 10;
    int count = 0;
    Process *processes = malloc(capacity * sizeof(Process));

    if (processes == NULL){
        perror("malloc");
        closedir(dir);
        return;
    }

    while((entry = readdir(dir)) != NULL){
        char path[512];

        if(is_numeric(entry->d_name)){
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

            Process process;

            process.pid = atoi(entry->d_name);

            FILE *file = fopen(path, "r");

            if(file == NULL){
                continue;
            }

            if(fgets(process.name, sizeof(process.name), file) != NULL){
                process.name[strcspn(process.name, "\n")] = '\0';
                process.memory_kb = get_process_memory(process.pid);

                if (count == capacity) {
                    int new_capacity = 2 * capacity;
                    Process *temp = realloc(processes, new_capacity * sizeof(Process));
                    if (temp == NULL) {
                        perror("realloc");
                        free(processes);
                        closedir(dir);
                        fclose(file);
                        return;
                    }
                    processes = temp;
                    capacity = new_capacity;
                }
                processes[count] = process;
                count++;
            }


            fclose(file);
        }
    }

    closedir(dir);

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (processes[j].memory_kb > processes[i].memory_kb) {
                Process temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    for (int i = 0; i < count; i++) {
        printf("%d: %s - %ld kB\n", processes[i].pid, processes[i].name, processes[i].memory_kb);
    }

    free(processes);

    printf("\nGefundene Prozesse: %d\n", count);
}