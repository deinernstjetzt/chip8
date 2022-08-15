#ifndef C8_KEYSTATE_H
#define C8_KEYSTATE_H

#include <stdbool.h>
#include <glib-object.h>

#define C8_TYPE_KEYSTATE (c8_keystate_get_type())
G_DECLARE_FINAL_TYPE(C8Keystate, c8_keystate, C8, KEYSTATE, GObject)

C8Keystate* c8_keystate_new(void);

void c8_keystate_press(C8Keystate* self, int key);

void c8_keystate_release(C8Keystate* self, int key);

bool c8_keystate_is_pressed(C8Keystate* self, int key);

void c8_keystate_enable_press_notify(C8Keystate* self);

void c8_keystate_disable_press_notifty(C8Keystate* self);

bool c8_keystate_get_press_notify(C8Keystate* self, int* out_key);

#endif