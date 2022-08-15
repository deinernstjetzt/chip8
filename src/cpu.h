#ifndef C8_CPU_H
#define C8_CPU_H

#include "display.h"
#include "keystate.h"

#include <stdint.h>

#define C8_TYPE_CPU (c8_cpu_get_type())
G_DECLARE_FINAL_TYPE(C8Cpu, c8_cpu, C8, CPU, GObject)

typedef enum C8CpuStatus {
    C8_CPU_STATUS_OK,
    C8_CPU_STATUS_OK_BLOCKED,
    C8_CPU_STATUS_ERR_STACK,
    C8_CPU_STATUS_ERR_MEM_ACCESS,
    C8_CPU_STATUS_ERR_ILL_INST,
} C8CpuStatus;

C8Cpu* c8_cpu_new(C8Display* disp, C8Keystate* keystate,
                  size_t n, const uint8_t* rom);

void c8_cpu_step(C8Cpu* self);

void c8_cpu_step_timers(C8Cpu* self);

C8CpuStatus c8_cpu_status(C8Cpu* self);

bool c8_cpu_ok(C8Cpu* self);

#endif