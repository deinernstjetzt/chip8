#include "cpu.h"

#include <stdint.h>

struct _C8Cpu {
    GObject parent;
    C8Display* display;
    C8Keystate* keystate;
    C8CpuStatus status;
    uint8_t mem[4096];
    uint8_t regs[16];
    uint16_t stack[16];
    uint16_t pc;
    uint16_t ir;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
    int wait_reg;
};

G_DEFINE_TYPE(C8Cpu, c8_cpu, G_TYPE_OBJECT)

typedef struct C8CpuInst {
    uint16_t nnn;
    uint8_t nn;
    uint8_t n;
    uint8_t x;
    uint8_t y;
    uint8_t i;
} C8CpuInst;

static void c8_cpu_init(C8Cpu* self) {
    static const uint8_t sprites[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80,
    };

    self->display = NULL;
    self->keystate = NULL;
    self->status = C8_CPU_STATUS_OK;

    memset(self->mem, 0, sizeof(self->mem));
    memset(self->regs, 0, sizeof(self->regs));
    memset(self->stack, 0, sizeof(self->stack));
    memcpy(self->mem, sprites, sizeof(sprites));

    self->pc = 0x200;
    self->ir = 0;
    self->sp = 0;
    self->dt = 0;
    self->st = 0;
}

static void c8_cpu_dispose(GObject* self) {
    C8Cpu* c = C8_CPU(self);

    g_clear_object(&c->display);
    g_clear_object(&c->keystate);

    return G_OBJECT_CLASS(c8_cpu_parent_class)->dispose(self);
}

static void c8_cpu_class_init(C8CpuClass* class) {
    G_OBJECT_CLASS(class)->dispose = c8_cpu_dispose;
}

static bool c8_cpu_ok(C8Cpu* self) {
    return self->status == C8_CPU_STATUS_OK &&
           self->status == C8_CPU_STATUS_OK_BLOCKED;
}

static C8CpuInst c8_cpu_fetch(C8Cpu* self) {
    C8CpuInst res = {0};

    if (self->pc + 2 > 4096) {
        self->status = C8_CPU_STATUS_ERR_MEM_ACCESS;
        return res;
    }

    uint8_t a = self->mem[self->pc++];
    uint8_t b = self->mem[self->pc++];

    uint16_t raw = a << 8 | b;

    res.nnn = raw & 0xFFF;
    res.nn = raw & 0xFF;
    res.n = raw & 0xF;
    res.x = (raw & 0xF00) >> 8;
    res.y = (raw & 0xF0) >> 4;
    res.i = (raw & 0xF000) >> 12;

    return res;
}

static bool c8_cpu_draw_sprite(C8Cpu* self, int x, int y,
                               int n, const uint8_t* sprite) {
    bool pixel_off = false;

    for (int j = 0; j < n; ++j) {
        uint8_t row = sprite[j];

        for (int i = 0; i < 8; ++i) {
            int act_x = (x + i) & 63;
            int act_y = (y + j) & 31;

            bool old = c8_display_get_pixel(self->display, act_x, act_y);
            bool new = old ^ (row >> (7 - i) & 1);

            c8_display_set_pixel(self->display, act_x, act_y, new);
            pixel_off |= old && !new;
        }
    }

    return pixel_off;
}

static void c8_cpu_exec(C8Cpu* self, C8CpuInst inst) {
    switch (inst.i) {
    case 0: {
        if (inst.nn == 0xE0) {
            c8_display_clear(self->display);
        } else if (inst.nn == 0xEE) {
            if (self->sp == 0) {
                self->status = C8_CPU_STATUS_ERR_STACK;
            } else {
                self->pc = self->stack[--self->sp];
            }
        } else {
            self->status = C8_CPU_STATUS_ERR_ILL_INST;
        }

        break;
    }

    case 1: {
        self->pc = inst.nnn;
        break;
    }

    case 2: {
        if (self->sp == 16) {
            self->status = C8_CPU_STATUS_ERR_STACK;
        } else {
            self->stack[self->sp++] = self->pc;
            self->pc = inst.nnn;
        }

        break;
    }

    case 3: {
        if (self->regs[inst.x] == inst.nn) {
            self->pc += 2;
        }

        break;
    }

    case 4: {
        if (self->regs[inst.x] != inst.nn) {
            self->pc += 2;
        }

        break;
    }

    case 5: {
        if (self->regs[inst.x] == self->regs[inst.y]) {
            self->pc += 2;
        }

        break;
    }

    case 6: {
        self->regs[inst.x] = inst.nn;
        break;
    }

    case 7: {
        self->regs[inst.x] += inst.nn;
    }

    case 8: {
        uint8_t* x = &self->regs[inst.x];
        uint8_t* y = &self->regs[inst.y];
        uint8_t* f = &self->regs[15];

        switch (inst.n) {
        case 0: {
            *x = *y;
            break;
        }

        case 1: {
            *x |= *y;
            break;
        }

        case 2: {
            *x &= *y;
            break;
        }

        case 3: {
            *x ^= *y;
            break;
        }

        case 4: {
            *x += *y;
            *f = *x < *y;

            break;
        }

        case 5: {
            bool nborrow = *x >= *y;
            *x -= *y;
            *f = nborrow;

            break;
        }

        case 6: {
            bool t = *x & 1;
            *x >>= 1;
            *f = t;

            break;
        }

        case 7: {
            bool nborrow = *y >= *x;
            *x = *y - *x;
            *f = nborrow;

            break;
        }

        case 0xE: {
            bool t = *x >> 7 & 1;
            *x <<= 1;
            *f = t;

            break;
        }

        default: {
            self->status = C8_CPU_STATUS_ERR_ILL_INST;
            break;
        }
        }

        break;
    }

    case 9: {
        if (self->regs[inst.x] != self->regs[inst.y]) {
            self->pc += 2;
        }

        break;
    }

    case 0xA: {
        self->ir = inst.nnn;
        break;
    }

    case 0xB: {
        self->pc = inst.nnn + self->regs[0];
        break;
    }

    case 0xC: {
        self->regs[inst.x] = rand() & inst.nn;
        break;
    }

    case 0xD: {
        if (self->ir + inst.n > 4096) {
            self->status = C8_CPU_STATUS_ERR_MEM_ACCESS;
        } else {
            bool res = c8_cpu_draw_sprite(self, self->regs[inst.x],
                                          self->regs[inst.y], inst.n,
                                          &self->mem[self->ir]);
            
            self->regs[15] = res;
        }

        break;
    }

    case 0xE: {
        int k = self->regs[inst.x];

        if (k >= 16) {
            break;
        }

        if (inst.nn == 0x9E) {
            if (c8_keystate_is_pressed(self->keystate, k)) {
                self->pc += 2;
            }
        } else if (inst.nn == 0xA1) {
            if (!c8_keystate_is_pressed(self->keystate, k)) {
                self->pc += 2;
            }
        } else {
            self->status = C8_CPU_STATUS_ERR_ILL_INST;
        }

        break;
    }

    case 0xF: {
        switch (inst.nn) {
        case 0x07: {
            self->regs[inst.x] = self->dt;
            break;
        }

        case 0x0A: {
            self->wait_reg = inst.x;
            self->status = C8_CPU_STATUS_OK_BLOCKED;

            c8_keystate_enable_press_notify(self->keystate);
            break;
        }

        case 0x15: {
            self->dt = self->regs[inst.x];
            break;
        }

        case 0x18: {
            self->st = self->regs[inst.x];
            break;
        }

        case 0x1E: {
            self->ir += self->regs[inst.x];
            break;
        }

        case 0x29: {
            self->ir = (self->regs[inst.x] & 0xF) * 5;
            break;
        }

        case 0x33: {
            if (self->ir + 3 > 4096) {
                self->status = C8_CPU_STATUS_ERR_MEM_ACCESS;
                break;
            }

            uint8_t n = self->regs[inst.x];
            uint8_t h = n / 100;
            n = n % 100;
            uint8_t t = n / 10;
            uint8_t o = n % 10;

            self->mem[self->ir + 0] = h;
            self->mem[self->ir + 1] = t;
            self->mem[self->ir + 2] = o;

            break;
        }

        case 0x55: {
            if (self->ir + inst.x >= 4096) {
                self->status = C8_CPU_STATUS_ERR_MEM_ACCESS;
                break;
            }

            for (int i = 0; i <= inst.x; ++i) {
                self->mem[self->ir + i] = self->regs[i];
            }

            break;
        }

        case 0x65: {
            if (self->ir + inst.x >= 4096) {
                self->status = C8_CPU_STATUS_ERR_MEM_ACCESS;
                break;
            }

            for (int i = 0; i <= inst.x; ++i) {
                self->regs[i] = self->mem[self->ir + i];
            }

            break;
        }

        default: {
            self->status = C8_CPU_STATUS_ERR_ILL_INST;
            break;
        }
        }

        break;
    }
    }
}

C8Cpu* c8_cpu_new(C8Display* disp, C8Keystate* keystate,
                  size_t n, const uint8_t* rom) {
    g_assert(n <= 4096 - 512);

    C8Cpu* self = g_object_new(C8_TYPE_CPU, NULL);

    self->display = disp;
    self->keystate = keystate;

    for (size_t i = 0; i < n; ++i) {
        self->mem[i + 512] = rom[i];
    }

    return self;
}

void c8_cpu_step(C8Cpu* self) {
    g_assert(c8_cpu_ok(self));

    if (self->status == C8_CPU_STATUS_OK) {
        C8CpuInst inst = c8_cpu_fetch(self);

        if (!c8_cpu_ok(self)) {
            return;
        }

        c8_cpu_exec(self, inst);
    } else {
        int key;

        if (c8_keystate_get_press_notify(self->keystate, &key)) {
            self->regs[self->wait_reg] = key;
            self->status = C8_CPU_STATUS_OK;

            c8_keystate_disable_press_notifty(self->keystate);
        }
    }
}

void c8_cpu_step_timers(C8Cpu* self) {
    g_assert(c8_cpu_ok(self));

    if (self->dt) {
        self->dt--;
    }

    if (self->st) {
        self->st--;
    }
}

C8CpuStatus c8_cpu_status(C8Cpu* self) {
    return self->status;
}
