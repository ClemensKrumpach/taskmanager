#ifndef PROCESS_H
#define PROCESS_H

typedef struct{
    int pid;
    char name[256];
    long memory_kb;
} Process;

void list_processes(void);

#endif