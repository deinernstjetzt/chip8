#include "keystate.h"

#include <stdint.h>

struct _C8Keystate {
    GObject parent;
    uint16_t state;
    bool press_notify_enabled;
    bool press_received;
    int press_key;
};

G_DEFINE_TYPE(C8Keystate, c8_keystate, G_TYPE_OBJECT)

static void c8_keystate_init(C8Keystate* self) {
    self->state = 0;
    self->press_notify_enabled = false;
}

static void c8_keystate_class_init(C8KeystateClass* class) {}

C8Keystate* c8_keystate_new(void) {
    return g_object_new(C8_TYPE_KEYSTATE, NULL);
}

void c8_keystate_press(C8Keystate* self, int key) {
    g_assert(0 <= key && key < 16);

    if (self->press_notify_enabled && !self->press_received) {
        self->press_key = key;
    }

    self->state |= 1 << key;
}

void c8_keystate_release(C8Keystate* self, int key) {
    g_assert(0 <= key && key < 16);
    self->state &= ~(1 << key);
}

bool c8_keystate_is_pressed(C8Keystate* self, int key) {
    g_assert(0 <= key && key < 16);
    return self->state >> key & 1;
}

void c8_keystate_enable_press_notify(C8Keystate* self) {
    if (!self->press_notify_enabled) {
        self->press_notify_enabled = true;
        self->press_received = false;
    }
}

void c8_keystate_disable_press_notifty(C8Keystate* self) {
    self->press_notify_enabled = false;
}

bool c8_keystate_get_press_notify(C8Keystate* self, int* out_key) {
    g_assert(self->press_notify_enabled);

    if (out_key) {
        *out_key = self->press_key;
    }

    return self->press_received;
}
