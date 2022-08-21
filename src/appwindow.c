#include "appwindow.h"
#include "console.h"

struct _C8AppWindow {
    AdwApplicationWindow parent;
    GtkWidget* open_rom;
    GtkWidget* start_pause;
    GtkWidget* stop;
    GtkWidget* content_box;
};

G_DEFINE_TYPE(C8AppWindow, c8_app_window, ADW_TYPE_APPLICATION_WINDOW)

static void c8_app_window_load_rom(C8AppWindow* self, GFile* file);

static void on_rom_select(GtkNativeDialog* native, int response, void* user_data) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(native);
        GFile* file = gtk_file_chooser_get_file(chooser);

        c8_app_window_load_rom(C8_APP_WINDOW(user_data), file);
    }

    g_object_unref(native);
}

static void on_open_rom(GtkButton* button, void* user_data) {
    C8AppWindow* self = C8_APP_WINDOW(user_data);

    GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new(
        "Open ROM",
        GTK_WINDOW(self),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Open",
        "_Cancel"
    );

    g_signal_connect(file_chooser, "response", G_CALLBACK(on_rom_select), self);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}

static void c8_app_window_init(C8AppWindow* self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->open_rom, "clicked", G_CALLBACK(on_open_rom), self);

    C8Display* disp = c8_display_new();

    for (int x = 0; x < 32; ++x) {
        for (int y = 0; y < 32; ++y) {
            c8_display_set_pixel(disp, x, y, true);
        }
    }

    C8Console* con = c8_console_new(disp, NULL);
    gtk_box_append(GTK_BOX(self->content_box), GTK_WIDGET(con));
}

static void c8_app_window_class_init(C8AppWindowClass* class) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class),
        "/com/github/vrfabian/chip8/res/appwindow.xml"
    );

    gtk_widget_class_bind_template_child_full(
        GTK_WIDGET_CLASS(class),
        "open-rom-button",
        true,
        G_STRUCT_OFFSET(C8AppWindow, open_rom)
    );

    gtk_widget_class_bind_template_child_full(
        GTK_WIDGET_CLASS(class),
        "emu-start-pause-button",
        true,
        G_STRUCT_OFFSET(C8AppWindow, start_pause)
    );

    gtk_widget_class_bind_template_child_full(
        GTK_WIDGET_CLASS(class),
        "emu-stop-button",
        true,
        G_STRUCT_OFFSET(C8AppWindow, stop)
    );

    gtk_widget_class_bind_template_child_full(
        GTK_WIDGET_CLASS(class),
        "content-box",
        true,
        G_STRUCT_OFFSET(C8AppWindow, content_box)
    );
}

static void c8_app_window_load_rom(C8AppWindow* self, GFile* file) {
    g_print("Loading rom...\n");
}

C8AppWindow* c8_app_window_new(void) {
    return g_object_new(C8_TYPE_APP_WINDOW, NULL);
}
