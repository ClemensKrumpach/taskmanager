#include <stdint.h>
#include <stdlib.h>

#include <notcurses/notcurses.h>

int main(void) {
    struct notcurses_options options = {.flags = NCOPTION_SUPPRESS_BANNERS,};

    struct notcurses *nc = notcurses_core_init(&options, NULL);
    if (nc == NULL) {
        return EXIT_FAILURE;
    }

    struct ncplane *plane = notcurses_stdplane(nc);
    ncplane_putstr_yx(plane, 1, 2, "Notcurses funktioniert. q zum beenden");
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