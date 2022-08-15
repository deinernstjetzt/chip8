#include "appwindow.h"

struct _C8AppWindow {
    AdwApplicationWindow parent;
};

G_DEFINE_TYPE(C8AppWindow, c8_app_window, ADW_TYPE_APPLICATION_WINDOW)

static void c8_app_window_init(C8AppWindow* self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void c8_app_window_class_init(C8AppWindowClass* class) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class),
        "/com/github/vrfabian/chip8/res/appwindow.xml"
    );
}

C8AppWindow* c8_app_window_new(void) {
    return g_object_new(C8_TYPE_APP_WINDOW, NULL);
}
