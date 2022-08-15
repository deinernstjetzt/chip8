#include "app.h"
#include "appwindow.h"

struct _C8App {
    AdwApplication parent;
};

G_DEFINE_TYPE(C8App, c8_app, ADW_TYPE_APPLICATION)

static void c8_app_init(C8App* self) {}

static void c8_app_activate(GApplication* self) {
    C8AppWindow* win = c8_app_window_new();
    
    gtk_application_add_window(GTK_APPLICATION(self), GTK_WINDOW(win));
    gtk_window_present(GTK_WINDOW(win));
}

static void c8_app_class_init(C8AppClass* class) {
    G_APPLICATION_CLASS(class)->activate = c8_app_activate;
}

C8App* c8_app_new(void) {
    return g_object_new(C8_TYPE_APP, NULL);
}
