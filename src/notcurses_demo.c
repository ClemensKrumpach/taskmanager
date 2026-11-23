#include <stdint.h>
#include <stdlib.h>

#include "process.h"

#include <notcurses/notcurses.h>

int main(void) {
    struct notcurses_options options = {.flags = NCOPTION_SUPPRESS_BANNERS,};  //unterdrückt Notcurses’ Startinformationen und die Leistungsstatistik beim Beenden

    struct notcurses *nc = notcurses_core_init(&options, NULL);  //startet notcurses im Core-Modus. &options übergibt Einstellungen
    if (nc == NULL) {
        return EXIT_FAILURE;
    }

    struct ncplane *plane = notcurses_stdplane(nc);                             //holt die Standard-Zeichenfläche des Terminals. plane ist eine Art Zeichenpuffer in den wir schreiben, notcurses zeigt ihn anschließend im terminal
    
    ProcessList processes = {0};

    if (collect_processes(&processes) != 0) {
        notcurses_stop(nc);
        return EXIT_FAILURE;
    }

    unsigned rows = ncplane_dim_y(plane);

    ncplane_putstr_yx(plane, 0, 0, "Taskmanager | q: Beenden");
    ncplane_printf_yx(plane, 2, 0, "%7s  %10s  %s",
                     "PID", "RSS (KiB)", "NAME");

    for (size_t i = 0; i < processes.count && i + 3 < rows; ++i) {
        const Process *process = &processes.items[i];
        int row = (int)(i + 3);

        if (process->memory_kb < 0) {
            ncplane_printf_yx(plane, row, 0, "%7d  %10s  %s",
                             process->pid, "-", process->name);
        } else {
            ncplane_printf_yx(plane, row, 0, "%7d  %10ld  %s",
                             process->pid, process->memory_kb, process->name);
        }
    }

    free_processes(&processes);
   
    if (notcurses_render(nc) != 0) {
        notcurses_stop(nc);
        return EXIT_FAILURE;
    }

    uint32_t key;
    do {
        ncinput input = {0};
        key = notcurses_get_blocking(nc, &input);
        if (key == (uint32_t) -1) {
            notcurses_stop(nc);
            return EXIT_FAILURE;
        }
    } while (key != 'q' && key != 'Q' && key != NCKEY_EOF);

    return notcurses_stop(nc) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}