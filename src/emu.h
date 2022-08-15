#ifndef C8_EMU_H
#define C8_EMU_H

#include "display.h"
#include "keystate.h"

#include <stdint.h>

typedef enum C8EmuStatus {
    C8_EMU_STATUS_STOPPED,
    C8_EMU_STATUS_RUNNING,
    C8_EMU_STATUS_PAUSED,
    C8_EMU_STATUS_FAILED,
} C8EmuStatus;

#define C8_TYPE_EMU (c8_emu_get_type())
G_DECLARE_FINAL_TYPE(C8Emu, c8_emu, C8, EMU, GObject)

C8Emu* c8_emu_new(C8Display* disp, C8Keystate* keystate);

C8EmuStatus c8_emu_status(C8Emu* self);

void c8_emu_update(C8Emu* self);

void c8_emu_start(C8Emu* self);

void c8_emu_stop(C8Emu* self);

void c8_emu_pause(C8Emu* self);

void c8_emu_restart(C8Emu* self);

void c8_emu_load_rom(C8Emu* self, size_t n, const uint8_t* rom);

#endif
