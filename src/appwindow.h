#ifndef C8_APPWINDOW_H
#define C8_APPWINDOW_H

#include <adwaita.h>

#define C8_TYPE_APP_WINDOW (c8_app_window_get_type())
G_DECLARE_FINAL_TYPE(C8AppWindow, c8_app_window, C8, APP_WINDOW, AdwApplicationWindow)

C8AppWindow* c8_app_window_new(void);

#endif