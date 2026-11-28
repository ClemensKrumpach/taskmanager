#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <gtk/gtk.h>

#include "process.h"

GtkWidget *process_table_new(GListStore *rows);

void process_table_update(GListStore *rows, const ProcessList *processes,
                          unsigned long long total_kib);

#endif