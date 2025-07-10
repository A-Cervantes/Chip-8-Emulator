#include "chip-8.h"
#include <SDL2/SDL.h>
#include <stddef.h> // for NULL
#include <stddef.h> // for size_t
#include <stdint.h> // for uint32_t
#include <stdio.h>  // for printf
#include <stdlib.h> // for EXIT_FAILURE, for rand
#include <string.h> // memset
#define SCREEN_SIZE (100)
#define SCALE_FACTOR (4)
#define MEMORY_SIZE (4096)
#define START_POS (0x200)
#define SPRITE_WIDTH (8)
typedef unsigned _BitInt(12) uint12_t;

// clear everything for new game
void chipstart(struct chip8 *chip) {

    // Clear everything out
    for (int i = 0; i < MEMORY_SIZE; i++) {
        chip->memory[i] = 0;
    }

    for (int j = 0; j < 16; j++) {
        chip->V[j] = 0;
        chip->stacker[j] = 0;
    }

    chip->I = 0;
    chip->pc = START_POS;
    chip->sp = 0;
    chip->dtime = 0;
    chip->stime = 0;

    // Font taken from article about chip-8
    const uint8_t font[] = {
        0xF0, 0x90, 0x90,
        0x90, 0xF0, // 0
        0x20, 0x60, 0x20,
        0x20, 0x70, // 1
        0xF0, 0x10, 0xF0,
        0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0,
        0x10, 0xF0, // 3
        0x90, 0x90, 0xF0,
        0x10, 0x10, // 4
        0xF0, 0x80, 0xF0,
        0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0,
        0x90, 0xF0, //         0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0,
        0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0,
        0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0,
        0x90, 0x90, // A
        0xE0, 0x90, 0xE0,
        0x90, 0xE0, // B
        0xF0, 0x80, 0x80,
        0x80, 0xF0, // C
        0xE0, 0x90, 0x90,
        0x90, 0xE0, // D
        0xF0, 0x80, 0xF0,
        0x80, 0xF0, // E
        0xF0, 0x80, 0xF0,
        0x80, 0x80 // F
    };

    // load in the font
    for (uint8_t fontload = 0; fontload < sizeof(font); fontload++) {
        chip->memory[0x50 + fontload] = font[fontload];
    }
}

// Function that reads the romfile and stores into the buffer
void romopen(const char *romfile, struct chip8 *chip) {

    FILE *rom = fopen(romfile, "rb");

    if (rom == NULL) {
        printf("I could not open the file!!! file\n");
        return;
    }

    fseek(rom, 0, SEEK_END);        // Look at the end of file
    const long rompos = ftell(rom); // Value of the end of file; num of bytes
    rewind(rom);                    // Puts the file pointer to the start

    if (rompos > (MEMORY_SIZE - 0x200)) {
        printf("THIS IS TOO BIG!");
        fclose(rom);
        exit(0);
    }
    // Storing bytes into memory
    size_t readfile =
        fread(&chip->memory[START_POS], 1, MEMORY_SIZE - START_POS, rom);

    if (readfile == 0) {
        printf("byte has not been read");
        return;
    }

    fclose(rom);
    return;
}

// function to pop off the stack
uint16_t popoff(struct chip8 *chip) {

    // Check if we haven't allocated anything
    if (chip->sp == 0) {
        printf("I shall not pop off the map");
        return 0;
    }
    // decrease and then pop off the stack
    chip->sp--;
    uint16_t toreturn = chip->stacker[chip->sp];
    return toreturn;
}

// function to push on to the stack
void pushon(uint16_t addon, struct chip8 *chip) {

    // check if we can't push anymore on the stack
    if (chip->sp == 16) {
        printf("I shall not push off the map");
        return;
    }
    // Push on to stack; increment the sp
    chip->stacker[chip->sp] = addon;
    chip->sp++;
}

void deopcode(struct chip8 *chip) {
    // Grab two bytes and fused them together
    uint16_t thecode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];

    // increase pc
    chip->pc += 2;

    // break down components of the opcode to use later on
    uint12_t nnnComp = thecode & 0x0FFF;
    uint8_t nnComp = thecode & 0x00FF;
    uint8_t nComp = thecode & 0x000F;
    uint8_t xComp = (thecode & 0x0F00) >> 8;
    uint8_t yComp = (thecode & 0x00F0) >> 4;

    // Look at the instruction nibble
    switch ((thecode >> 12) & 0x0F) {

        case 0x0:
            if (thecode == 0x00E0) {
                // clear the screen
                memset(chip->displayer, 0, sizeof(chip->displayer));

            } else if (thecode == 0x00EE) {
                // return from subroutine
                uint16_t popvalue = popoff(chip);
                chip->pc = popvalue;
            }
            break;

        case 0x1:
            // Jump to NNN
            chip->pc = nnnComp;
            break;

        case 0x2:
            // Push on to the stack; jump to adress of NNN
            pushon(chip->pc, chip);
            chip->pc += 2;
            chip->pc = nnnComp;
            break;

        case 0x3:
            //	if (Vx == NN)
            if (chip->V[xComp] == nnComp) {
                chip->pc += 2; // Skip the next instruction
            }
            break;

        case 0x4:
            // if (Vx != NN)
            if (chip->V[xComp] != nnComp) {
                chip->pc += 2;
            }
            break;

        case 0x5:
            // if (Vx == Vy)
            if (chip->V[xComp] == chip->V[yComp]) {
                chip->pc += 2;
            }
            break;

        case 0x6:
            // Vx = NN
            chip->V[xComp] = nnComp;
            break;

        case 0x7:
            // Vx += NN
            chip->V[xComp] += nnComp;
            break;

        case 0x8:
            if (nComp == 0) {
                // Vx = NN
                chip->V[xComp] = chip->V[yComp];
                break;

            } else if (nComp == 1) {
                // Vx |= Vy
                chip->V[xComp] |= chip->V[yComp];
                break;

            } else if (nComp == 2) {
                // Vx &= Vy
                chip->V[xComp] &= chip->V[yComp];
                break;

            } else if (nComp == 3) {
                // Vx ^= Vy
                chip->V[xComp] ^= chip->V[yComp];
                break;

            } else if (nComp == 4) {
                // Vx += Vy ; check for overflow
                uint16_t sumup = chip->V[xComp] + chip->V[yComp];

                // check for overflow
                if (sumup > 255) {
                    chip->V[15] = 1;
                } else {
                    chip->V[15] = 0;
                }
                // clear out top 8 bits,
                chip->V[xComp] = sumup & 0xFF;
                break;

            } else if (nComp == 5) {
                // Vx -= Vy; changes the VF flag
                if (chip->V[xComp] > chip->V[yComp]) {
                    chip->V[15] = 1;
                } else {
                    chip->V[15] = 0;
                }

                chip->V[xComp] -= chip->V[yComp];
                break;

            } else if (nComp == 6) {
                // Vx >>= 1; stores the least signifact bit of VX prior to shift

                chip->V[15] = chip->V[xComp] & 0x1;

                chip->V[xComp] >>= 1;

                break;

            } else if (nComp == 7) {
                // Vx = Vy - Vx; also changes the V[F] flag
                if (chip->V[yComp] >= chip->V[xComp]) {
                    chip->V[15] = 1;
                } else {
                    chip->V[15] = 0;
                }

                chip->V[xComp] = chip->V[yComp] - chip->V[xComp];
                break;

            } else if (nComp == 14) {
                // Vx <<= 1
                chip->V[15] = (chip->V[xComp] & 0x80) >> 7;
                chip->V[xComp] <<= 1;
                break;
            }
            break;
        case 0x9:
            // if (Vx != Vy)
            if (chip->V[xComp] != chip->V[yComp]) {
                // Skipping instruction
                chip->pc += 2;
            }
            break;

        case 0xA:
            // I = NNN
            chip->I = nnnComp;
            break;

        case 0xB:
            // PC =  V0 + NNN
            chip->pc = chip->V[0] + nnnComp;
            break;

        case 0xC:
            chip->V[xComp] = (rand() % 256) & nnComp;
            break;

        case 0xD: {
            // Get the x & y position
            uint8_t Xloc = chip->V[xComp] % 64;
            uint8_t Yloc = chip->V[yComp] % 32;
            chip->V[15] = 0; // The carryflag

            for (uint8_t shadowrow = 0; shadowrow < nComp; shadowrow++) {
                if (Yloc + shadowrow >= 32) {
                    break;
                }
                uint8_t spritefind = chip->memory[chip->I + shadowrow];
                for (uint8_t shadowcol = 0; shadowcol < 8; shadowcol++) {
                    if (Xloc + shadowcol >= 64) {
                        break;
                    }
                    // Check if set
                    if ((spritefind & (0x80 >> shadowcol)) != 0) {

                        // find correct index for it's location in the array
                        // since it's being
                        // represented as 1d array, but it's a 2d array
                        uint8_t findx = Xloc + shadowcol;
                        uint8_t findy = Yloc + shadowrow;
                        uint16_t pinpoint = findy * 64 + findx;

                        if (chip->displayer[pinpoint] == 1) {
                            chip->V[15] = 1;
                        }
                        chip->displayer[pinpoint] ^= 1;
                    }
                }
            }
            break;
        }
        case 0xE:
            if ((thecode & 0x00FF) == 0x9E) {
                if (chip->keystore[chip->V[xComp]] == 1) {
                    chip->pc += 2;
                }
                break;

            } else if ((thecode & 0x00FF) == 0xA1) {
                if (chip->keystore[chip->V[xComp]] != 1) {
                    chip->pc += 2;
                }
            }
            break;

        case 0xF:
            if (nnComp == 0x07) {
                // Vx = dtime
                chip->V[xComp] = chip->dtime;
                break;

            } else if (nnComp == 0x0A) {
                // Vx =  get key
                uint8_t check = 0;
                for (uint8_t move = 0; move < sizeof(chip->keystore); move++) {
                    if (chip->keystore[move] == 1) {
                        check = 54;
                        chip->V[xComp] = move;
                        break;
                    }
                }

                // Keep waiting until a key
                if (check == 0) {
                    chip->pc -= 2;
                }
                break;

            } else if (nnComp == 0x15) {
                // dtime = Vx
                chip->dtime = chip->V[xComp];
                break;
            } else if (nnComp == 0x18) {
                // stime = Vx
                chip->stime = chip->V[xComp];
                break;

            } else if (nnComp == 0x1E) {
                // I += Vx
                chip->I += chip->V[xComp];
                break;

            } else if (nnComp == 0x29) {
                // start of font addr + Vx * 5
                chip->I = 0x50 + (chip->V[xComp] * 5);
                break;

            } else if (nnComp == 0x33) {
                // BCD calcs
                uint8_t bcdfind = chip->V[xComp];
                chip->memory[chip->I + 2] = bcdfind % 10;
                bcdfind /= 10;
                chip->memory[chip->I + 1] = bcdfind % 10;
                bcdfind /= 10;
                chip->memory[chip->I] = bcdfind;

                break;

            } else if (nnComp == 0x55) {
                // regdump
                for (uint8_t i = 0; i <= xComp; i++) {
                    chip->memory[chip->I + i] = chip->V[i];
                }
                break;

            } else if (nnComp == 0x65) {
                // regload
                for (uint8_t i = 0; i <= xComp; i++) {
                    chip->V[i] = chip->memory[chip->I + i];
                }
                break;
            }

        default:
            // The void of nothing : ( where invalid opcodes go
            break;
    }
}
