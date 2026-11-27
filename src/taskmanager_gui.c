#include <gtk/gtk.h>

#include "process.h"

typedef enum {
    COLUMN_PID,
    COLUMN_NAME,
    COLUMN_MEMORY
} ProcessColumn;

typedef struct {
    GListStore *rows;
    GtkLabel *status;
    guint refresh_timer;
} Dashboard;

static void setup_cell(GtkSignalListItemFactory *factory,
                       GtkListItem *item, gpointer user_data) {
    (void)factory;

    ProcessColumn column = GPOINTER_TO_INT(user_data);
    GtkWidget *label = gtk_label_new(NULL);

    gtk_label_set_xalign(GTK_LABEL(label),
                         column == COLUMN_NAME ? 0.0f : 1.0f);
    gtk_list_item_set_child(item, label);
}

static void bind_cell(GtkSignalListItemFactory *factory,
                      GtkListItem *item, gpointer user_data) {
    (void)factory;

    ProcessColumn column = GPOINTER_TO_INT(user_data);
    GObject *row = gtk_list_item_get_item(item);
    const Process *process = g_object_get_data(row, "process");
    GtkLabel *label = GTK_LABEL(gtk_list_item_get_child(item));
    char number[32];

    switch (column) {
        case COLUMN_PID:
            g_snprintf(number, sizeof(number), "%d", process->pid);
            gtk_label_set_text(label, number);
            break;

        case COLUMN_NAME: {
            char *name = g_utf8_make_valid(process->name, -1);
            gtk_label_set_text(label, name);
            g_free(name);
            break;
        }

        case COLUMN_MEMORY:
            if (process->memory_kb < 0) {
                gtk_label_set_text(label, "—");
            } else {
                g_snprintf(number, sizeof(number), "%ld", process->memory_kb);
                gtk_label_set_text(label, number);
            }
            break;
    }
}

static void add_column(GtkColumnView *table, const char *title,
                       ProcessColumn field) {
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    gpointer field_data = GINT_TO_POINTER(field);

    g_signal_connect(factory, "setup", G_CALLBACK(setup_cell), field_data);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_cell), field_data);

    GtkColumnViewColumn *column = gtk_column_view_column_new(title, factory);
    gtk_column_view_column_set_expand(column, field == COLUMN_NAME);
    gtk_column_view_append_column(table, column);
    g_object_unref(column);
}

static gboolean refresh_processes(gpointer user_data) {
    Dashboard *dashboard = user_data;
    ProcessList processes = {0};

    if (collect_processes(&processes) != 0) {
        gtk_label_set_text(dashboard->status,
                           "Prozessliste konnte nicht aktualisiert werden.");
        return G_SOURCE_CONTINUE;
    }

    GPtrArray *new_rows = g_ptr_array_new_with_free_func(g_object_unref);

    for (size_t i = 0; i < processes.count; ++i) {
        GObject *row = g_object_new(G_TYPE_OBJECT, NULL);
        Process *copy = g_new(Process, 1);

        *copy = processes.items[i];
        g_object_set_data_full(row, "process", copy, g_free);
        g_ptr_array_add(new_rows, row);
    }

    guint old_count = g_list_model_get_n_items(G_LIST_MODEL(dashboard->rows));
    g_list_store_splice(dashboard->rows, 0, old_count,
                       new_rows->pdata, new_rows->len);

    char status[80];
    g_snprintf(status, sizeof(status), "%zu Prozesse", processes.count);
    gtk_label_set_text(dashboard->status, status);

    g_ptr_array_unref(new_rows);
    free_processes(&processes);
    return G_SOURCE_CONTINUE;
}

static void free_dashboard(gpointer user_data) {
    Dashboard *dashboard = user_data;

    if (dashboard->refresh_timer != 0) {
        g_source_remove(dashboard->refresh_timer);
    }

    g_object_unref(dashboard->rows);
    g_free(dashboard);
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWindow *existing = gtk_application_get_active_window(app);
    if (existing != NULL) {
        gtk_window_present(existing);
        return;
    }

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Taskmanager");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    Dashboard *dashboard = g_new0(Dashboard, 1);
    dashboard->rows = g_list_store_new(G_TYPE_OBJECT);
    dashboard->status = GTK_LABEL(gtk_label_new("Prozesse werden geladen …"));
    g_object_set_data_full(G_OBJECT(window), "dashboard",
                          dashboard, free_dashboard);

    GtkNoSelection *model =
        gtk_no_selection_new(G_LIST_MODEL(g_object_ref(dashboard->rows)));
    GtkWidget *table = gtk_column_view_new(GTK_SELECTION_MODEL(model));

    add_column(GTK_COLUMN_VIEW(table), "PID", COLUMN_PID);
    add_column(GTK_COLUMN_VIEW(table), "Name", COLUMN_NAME);
    add_column(GTK_COLUMN_VIEW(table), "RSS (KiB)", COLUMN_MEMORY);
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(table), TRUE);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), table);

    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_append(GTK_BOX(layout), scroll);
    gtk_box_append(GTK_BOX(layout), GTK_WIDGET(dashboard->status));
    gtk_window_set_child(GTK_WINDOW(window), layout);

    refresh_processes(dashboard);
    dashboard->refresh_timer =
        g_timeout_add_seconds(1, refresh_processes, dashboard);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new("org.example.Taskmanager",
                            G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return result;
}