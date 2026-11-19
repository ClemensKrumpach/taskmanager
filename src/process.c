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

    snprintf(path, sizeof(path), "/proc/%s/status", line);

    FILE *fp = fopen(line, "r");

    if (fp == NULL){
        perror("fopen");
        return 1;
    }
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
                
                if (count == capacity) {
                    int new_capacity = 2 * capacity;
                    Process *temp = realloc(processes, new_capacity * sizeof(Process));
                    if (temp == NULL) {
                        perror("realloc");
                        free(processes);
                        closedir(dir);
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

    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", processes[i].pid, processes[i].name);
    }

    printf("\nGefundene Prozesse: %d\n", count);
}