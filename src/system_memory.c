#include "system_memory.h"

#include <stdio.h>

bool read_system_memory(SystemMemory *memory) {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        return false;
    }

    SystemMemory sample = {0};
    bool found_total = false;
    bool found_available = false;
    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "MemTotal: %llu", &sample.total_kib) == 1) {
            found_total = true;
        } else if (sscanf(line, "MemAvailable: %llu",
                          &sample.available_kib) == 1) {
            found_available = true;
        }

        if (found_total && found_available) {
            break;
        }
    }

    fclose(file);

    if (!found_total || !found_available ||
        sample.total_kib == 0 || sample.available_kib > sample.total_kib) {
        return false;
    }

    *memory = sample;
    return true;
}