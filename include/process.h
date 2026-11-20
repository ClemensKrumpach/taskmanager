#ifndef PROCESS_H
#define PROCESS_H

typedef struct{
    int pid;
    char name[256];
    long memory_kb;
} Process;

typedef struct {
    Process *items;
    size_t count;
} ProcessList;

int collect_processes(ProcessList *list);
void free_processes(ProcessList *list);
void list_processes(void);

void list_processes(void);

#endif