#ifndef SYSTEM_MEMORY_H
#define SYSTEM_MEMORY_H

#include <stdbool.h>

typedef struct {
    unsigned long long total_kib;
    unsigned long long available_kib;
} SystemMemory;

bool read_system_memory(SystemMemory *memory);

#endif