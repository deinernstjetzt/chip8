#include "console.h"

struct _C8Console {
    GtkWidget parent;
    C8Display* disp;
    C8Keystate* keys;
};

G_DEFINE_TYPE(C8Console, c8_console, GTK_TYPE_WIDGET)

static void c8_console_init(C8Console* self) {
    self->disp = NULL;
    self->keys = NULL;
}

static void c8_console_dispose(GObject* self_obj) {
    C8Console* self = C8_CONSOLE(self_obj);

    g_clear_object(&self->disp);
    g_clear_object(&self->keys);

    G_OBJECT_CLASS(c8_console_parent_class)->dispose(self_obj);
}

static void c8_console_measure(GtkWidget* self_wid, GtkOrientation o,
                               int for_size, int* min, int* nat,
                               int* min_base, int* nat_base) {
    if (o == GTK_ORIENTATION_HORIZONTAL) {
        if (nat) {
            *nat = for_size * 2;
        }

        if (min) {
            *min = for_size / 32 * 32 * 2;
        }
    } else {
        if (nat) {
            *nat = for_size / 2;
        }

        if (min) {
            *min = for_size / 64 * 64 / 2;
        }
    }

    if (min_base) {
        *min_base = -1;
    }

    if (nat_base) {
        *nat_base = -1;
    }
}

static GtkSizeRequestMode c8_console_get_request_mode(GtkWidget* self_wid) {
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void c8_console_snapshot(GtkWidget* self_wid, GtkSnapshot* snapshot) {
    C8Console* self = C8_CONSOLE(self_wid);

    int w = gtk_widget_get_width(self_wid);
    int h = gtk_widget_get_height(self_wid);

    int scale = MIN(w / 64, h / 32);
    int start_x = (w - scale * 64) / 2;
    int start_y = (h - scale * 32) / 2;

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            graphene_rect_t rect = GRAPHENE_RECT_INIT(
                start_x + x * scale,
                start_y + y * scale,
                scale,
                scale
            );

            GdkRGBA color;

            if (c8_display_get_pixel(self->disp, x, y)) {
                color.alpha = 1.0f;
                color.red = 1.0f;
                color.green = 1.0f;
                color.blue = 1.0f;
            } else {
                color.alpha = 1.0f;
                color.red = 0.0f;
                color.green = 0.0f;
                color.blue = 0.0f;
            }

            gtk_snapshot_append_color(snapshot, &color, &rect);
        }
    }
}

static void c8_console_class_init(C8ConsoleClass* class) {
    G_OBJECT_CLASS(class)->dispose = c8_console_dispose;

    GTK_WIDGET_CLASS(class)->snapshot = c8_console_snapshot;
    GTK_WIDGET_CLASS(class)->measure = c8_console_measure;
    GTK_WIDGET_CLASS(class)->get_request_mode = c8_console_get_request_mode;
}

C8Console* c8_console_new(C8Display* disp, C8Keystate* keys) {
    C8Console* res = g_object_new(C8_TYPE_CONSOLE, NULL);

    res->disp = disp;
    res->keys = keys;

    return res;
}
