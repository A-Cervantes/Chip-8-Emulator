#pragma once
#include <stdint.h>
typedef unsigned _BitInt(12) uint12_t;
#define MEMORY_SIZE (4096)

struct chip8 {
    uint8_t keystore[16];
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[16];
    uint12_t I;
    uint12_t pc;
    uint16_t stacker[16];
    uint8_t sp;
    uint8_t dtime;
    uint8_t stime; // while != 0; beep
    uint8_t displayer[64 * 32];
};

void romopen(const char *romfile, struct chip8 *chip);
void chipstart(struct chip8 *chip);
uint16_t popoff(struct chip8 *chip);
void pushon(uint16_t addon, struct chip8 *chip);
void deopcode(struct chip8 *chip);
