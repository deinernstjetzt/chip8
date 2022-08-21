#include "appwindow.h"
#include "console.h"
#include "emu.h"

struct _C8AppWindow {
    AdwApplicationWindow parent;
    GtkWidget* open_rom;
    GtkWidget* start_pause;
    GtkWidget* stop;
    GtkWidget* content_box;
    C8Emu* emu;
    C8Console* con;
    C8EmuStatus old_emu_status;
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

static void c8_app_window_update_media_buttons(C8AppWindow* self) {
    C8EmuStatus status = c8_emu_status(self->emu);

    if (status == C8_EMU_STATUS_RUNNING) {
        gtk_button_set_icon_name(GTK_BUTTON(self->start_pause), "media-playback-pause");
        gtk_widget_set_sensitive(self->start_pause, true);
        gtk_widget_set_sensitive(self->stop, true);
    } else if (status == C8_EMU_STATUS_PAUSED) {
        gtk_button_set_icon_name(GTK_BUTTON(self->start_pause), "media-playback-start");
        gtk_widget_set_sensitive(self->start_pause, true);
        gtk_widget_set_sensitive(self->stop, true);
    } else if (status == C8_EMU_STATUS_STOPPED) {
        gtk_button_set_icon_name(GTK_BUTTON(self->start_pause), "media-playback-start");
        gtk_widget_set_sensitive(self->start_pause, true);
        gtk_widget_set_sensitive(self->stop, false);
    } else {
        gtk_button_set_icon_name(GTK_BUTTON(self->start_pause), "media-playback-start");
        gtk_widget_set_sensitive(self->start_pause, false);
        gtk_widget_set_sensitive(self->stop, false);
    }
}

static gboolean on_redraw(void* user_data) {
    GWeakRef* weak = user_data;
    C8AppWindow* self = g_weak_ref_get(weak);
    gboolean res;

    if (self) {
        C8EmuStatus status = c8_emu_status(self->emu);

        if (status != self->old_emu_status) {
            self->old_emu_status = status;
            c8_app_window_update_media_buttons(self);
        }

        if (status == C8_EMU_STATUS_RUNNING) {
            c8_emu_update(self->emu);
            gtk_widget_queue_draw(GTK_WIDGET(self->con));
        }
        
        res = G_SOURCE_CONTINUE;
    } else {
        g_free(weak);

        res = G_SOURCE_REMOVE;
    }

    return res;
}

static void on_play_pressed(GtkWidget* button, gpointer user_data) {
    C8AppWindow* self = C8_APP_WINDOW(user_data);
    C8EmuStatus status = c8_emu_status(self->emu);

    if (status == C8_EMU_STATUS_PAUSED || status == C8_EMU_STATUS_STOPPED) {
        c8_emu_start(self->emu);
        gtk_widget_grab_focus(GTK_WIDGET(self->con));
    } else if (status == C8_EMU_STATUS_RUNNING) {
        c8_emu_pause(self->emu);
    }
}

static void on_stop_pressed(GtkWidget* button, gpointer user_data) {
    C8AppWindow* self = C8_APP_WINDOW(user_data);
    C8EmuStatus status = c8_emu_status(self->emu);

    if (status == C8_EMU_STATUS_RUNNING || status == C8_EMU_STATUS_PAUSED) {
        c8_emu_stop(self->emu);
    }
}

static void c8_app_window_init(C8AppWindow* self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->open_rom, "clicked", G_CALLBACK(on_open_rom), self);

    C8Keystate* keys = c8_keystate_new();
    C8Display* disp = c8_display_new();

    self->con = c8_console_new(g_object_ref(disp), g_object_ref(keys));
    gtk_box_append(GTK_BOX(self->content_box), GTK_WIDGET(g_object_ref(self->con)));

    self->emu = c8_emu_new(disp, keys);

    GWeakRef* weak = g_malloc(sizeof(*weak));
    g_weak_ref_init(weak, self);
    g_timeout_add(1000 / 60, on_redraw, weak);

    self->old_emu_status = C8_EMU_STATUS_STOPPED;

    g_signal_connect(self->start_pause, "clicked", G_CALLBACK(on_play_pressed), self);
    g_signal_connect(self->stop, "clicked", G_CALLBACK(on_stop_pressed), self);
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
    GError* error = NULL;
    GBytes* bytes = g_file_load_bytes(file, NULL, NULL, &error);

    if (error) {
        printf("Error %d: %s\n", error->code, error->message);
        g_error_free(error);

        return;
    }

    size_t size;
    guint8* rom;

    rom = g_bytes_unref_to_data(bytes, &size);
    c8_emu_load_rom(self->emu, size, rom);
    g_free(rom);
}

C8AppWindow* c8_app_window_new(void) {
    return g_object_new(C8_TYPE_APP_WINDOW, NULL);
}
