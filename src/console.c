#include "console.h"

struct _C8Console {
    GtkWidget parent;
    C8Display* disp;
    C8Keystate* keys;
    bool focused;
};

G_DEFINE_TYPE(C8Console, c8_console, GTK_TYPE_WIDGET)

static const int c8_keymap[16][2] = {
        {GDK_KEY_1, 1},
        {GDK_KEY_2, 2},
        {GDK_KEY_3, 3},
        {GDK_KEY_4, 0xC},
        {GDK_KEY_q, 4},
        {GDK_KEY_w, 5},
        {GDK_KEY_e, 6},
        {GDK_KEY_r, 0xD},
        {GDK_KEY_a, 7},
        {GDK_KEY_s, 8},
        {GDK_KEY_d, 9},
        {GDK_KEY_f, 0xE},
        {GDK_KEY_y, 0xA},
        {GDK_KEY_x, 0},
        {GDK_KEY_c, 0xB},
        {GDK_KEY_v, 0xF},
    };

static gboolean c8_console_on_key_pressed(GtkEventControllerKey* ek,
                                          guint keyval,
                                          guint keycode,
                                          GdkModifierType state,
                                          gpointer user_data) {
    C8Console* self = C8_CONSOLE(user_data);
    bool result = false;
    
    for (int i = 0; i < 16 && !result; ++i) {
        if (keyval == c8_keymap[i][0]) {
            c8_keystate_press(self->keys, c8_keymap[i][1]);
            result = true;
        }
    }

    return result;
}

static gboolean c8_console_on_key_released(GtkEventControllerKey* ek,
                                           guint keyval,
                                           guint keycode,
                                           GdkModifierType state,
                                           gpointer user_data) {
    C8Console* self = C8_CONSOLE(user_data);
    bool result = false;
    
    for (int i = 0; i < 16 && !result; ++i) {
        if (keyval == c8_keymap[i][0]) {
            c8_keystate_release(self->keys, c8_keymap[i][1]);
            result = true;
        }
    }

    return result;
}

static gboolean c8_console_on_legacy_event(GtkEventControllerLegacy* el,
                                           GdkEvent* event,
                                           gpointer user_data) {
    gboolean result = false;
    
    if (gdk_event_get_event_type(event) == GDK_BUTTON_PRESS) {
        C8Console* self = C8_CONSOLE(user_data);
        gtk_widget_grab_focus(GTK_WIDGET(self));

        result = true;
    }

    return true;
}

static void c8_console_init(C8Console* self) {
    self->disp = NULL;
    self->keys = NULL;

    gtk_widget_set_can_focus(GTK_WIDGET(self), true);
    gtk_widget_set_focusable(GTK_WIDGET(self), true);
    gtk_widget_set_focus_on_click(GTK_WIDGET(self), true);
    gtk_widget_set_can_target(GTK_WIDGET(self), true);

    GtkEventController* el = gtk_event_controller_legacy_new();

    g_signal_connect(el, "event", G_CALLBACK(c8_console_on_legacy_event), self);
    gtk_widget_add_controller(GTK_WIDGET(self), el);

    GtkEventController* ek = gtk_event_controller_key_new();

    g_signal_connect(ek, "key-pressed", G_CALLBACK(c8_console_on_key_pressed), self);
    g_signal_connect(ek, "key-released", G_CALLBACK(c8_console_on_key_released), self);
    gtk_widget_add_controller(GTK_WIDGET(self), ek);
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
