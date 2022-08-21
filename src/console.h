#ifndef C8_CONSOLE_H
#define C8_CONSOLE_H

#include "display.h"
#include "keystate.h"

#include <gtk/gtk.h>

#define C8_TYPE_CONSOLE (c8_console_get_type())
G_DECLARE_FINAL_TYPE(C8Console, c8_console, C8, CONSOLE, GtkWidget)

C8Console* c8_console_new(C8Display* disp, C8Keystate* keys);

#endif
