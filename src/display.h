#ifndef C8_DISPLAY_H
#define C8_DISPLAY_H

#include <stdbool.h>
#include <glib-object.h>

#define C8_TYPE_DISPLAY (c8_display_get_type())
G_DECLARE_FINAL_TYPE(C8Display, c8_display, C8, DISPLAY, GObject)

C8Display* c8_display_new(void);

bool c8_display_get_pixel(C8Display* self, int x, int y);

void c8_display_set_pixel(C8Display* self, int x, int y, bool value);

#endif
