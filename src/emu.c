#include "emu.h"
#include "cpu.h"

#include <stdio.h>

struct _C8Emu {
    GObject parent;
    C8Display* disp;
    C8Keystate* keystate;
    C8Cpu* cpu;
    C8EmuStatus status;
    gint64 last_update;
    unsigned clocks_per_timer;
    size_t rom_size;
    uint8_t rom[4096 - 512];
};

G_DEFINE_TYPE(C8Emu, c8_emu, G_TYPE_OBJECT)

static void c8_emu_init(C8Emu* self) {
    self->disp = NULL;
    self->keystate = NULL;
    self->clocks_per_timer = 10;
    self->status = C8_EMU_STATUS_STOPPED;
}

static void c8_emu_dispose(GObject* self_obj) {
    C8Emu* self = C8_EMU(self_obj);

    g_clear_object(&self->disp);
    g_clear_object(&self->keystate);

    return G_OBJECT_CLASS(c8_emu_parent_class)->dispose(self_obj);
}

static void c8_emu_class_init(C8EmuClass* class) {
    G_OBJECT_CLASS(class)->dispose = c8_emu_dispose;
}

C8Emu* c8_emu_new(C8Display* disp, C8Keystate* keystate) {
    C8Emu* self = g_object_new(C8_TYPE_EMU, NULL);

    self->disp = disp;
    self->keystate = keystate;

    return self;
}

C8EmuStatus c8_emu_status(C8Emu* self) {
    return self->status;
}

void c8_emu_update(C8Emu* self) {
    g_assert(self->status != C8_EMU_STATUS_FAILED);

    gint64 time = g_get_monotonic_time();

    while (self->status == C8_EMU_STATUS_RUNNING &&
           (time - self->last_update) >= 1000000 / 60) {
        for (int i = 0; i < self->clocks_per_timer; ++i) {
            c8_cpu_step(self->cpu);

            if (!c8_cpu_ok(self->cpu)) {
                self->status = C8_EMU_STATUS_FAILED;
                break;
            }
        }

        if (c8_cpu_ok(self->cpu)) {
            c8_cpu_step_timers(self->cpu);
            self->last_update += 1000000 / 60;
        }
    }
}

void c8_emu_start(C8Emu* self) {
    g_assert(self->status != C8_EMU_STATUS_FAILED);

    if (self->status == C8_EMU_STATUS_STOPPED) {
        g_object_ref(self->disp);
        g_object_ref(self->keystate);

        self->cpu = c8_cpu_new(self->disp, self->keystate, self->rom_size, self->rom);
        self->status = C8_EMU_STATUS_RUNNING;
        self->last_update = g_get_monotonic_time();
    } else if (self->status == C8_EMU_STATUS_PAUSED) {
        self->status = C8_EMU_STATUS_RUNNING;
        self->last_update = g_get_monotonic_time();
    }
}

void c8_emu_stop(C8Emu* self) {
    g_clear_object(&self->cpu);
    c8_display_clear(self->disp);
    
    self->status = C8_EMU_STATUS_STOPPED;
}

void c8_emu_pause(C8Emu* self) {
    if (self->status == C8_EMU_STATUS_RUNNING) {
        self->status = C8_EMU_STATUS_PAUSED;
    }
}

void c8_emu_restart(C8Emu* self) {
    c8_emu_stop(self);
    c8_emu_start(self);
}

void c8_emu_load_rom(C8Emu* self, size_t n, const uint8_t* rom) {
    c8_emu_stop(self);

    for (size_t i = 0; i < n && i < 4096 - 512; ++i) {
        self->rom[i] = rom[i];
    }

    self->rom_size = n;
}
