#ifndef C8_APP_H
#define C8_APP_H

#include <adwaita.h>

#define C8_TYPE_APP (c8_app_get_type())
G_DECLARE_FINAL_TYPE(C8App, c8_app, C8, APP, AdwApplication)

C8App* c8_app_new(void);

#endif
