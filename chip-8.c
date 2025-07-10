#include "chip-8.h"
#include <SDL2/SDL.h>
#include <stddef.h> // for NULL
#include <stddef.h> // for size_t
#include <stdint.h> // for uint32_t
#include <stdio.h>  // for printf
#include <stdlib.h> // for EXIT_FAILURE

#define VERTICAL (32)
#define HORIZONTAL (64)
#define SCREEN_SIZE (64)
#define SCALE_FACTOR (14)
typedef unsigned _BitInt(12) uint12_t;
#define MEMORY_SIZE (4096)
#define INSTRUCT_SPEED (678)

// One central chip
struct chip8 chip;
uint8_t toplayornot = 0;


// function to clear the screen before next rom
void clearout(SDL_Renderer *const renderer) {
    SDL_RenderClear(renderer);
}

//decrement dtime and stime; also have sound
void tick() {
    if (chip.dtime > 0) {
        chip.dtime -= 1;
    }

    if (chip.stime > 0) {
    	if(toplayornot == 0){
    		printf("\a"); // Bell 
    		fflush(stdout);
    	}
        chip.stime -= 1;
    }

    toplayornot = chip.stime;
}

int main(int argc, char const *const *const argv) {

    // A rom needs to be passed in
    if (argc < 2) {
        printf("YOU NEED TO PASS A ROM IN!\n");
        exit(10);
    }

	// Intialize everything to start fresh
	chipstart(&chip);
 
    // Open the rom file
    romopen(argv[1], &chip);
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return EXIT_FAILURE;
    }

    // Create Window
    SDL_Window *const window =
        SDL_CreateWindow("Angel Cervantes Chip-8", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, HORIZONTAL * SCALE_FACTOR,
                         VERTICAL * SCALE_FACTOR, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Error for window");
        return EXIT_FAILURE;
    }

    // Create Renderer
    SDL_Renderer *const renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL) {
        printf("Error for renderer");
        return EXIT_FAILURE;
    }

	//clear screen
    clearout(renderer);

	// main loop
    while (1) {
		// handle key logic
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // While there are events to process user quits
            if (event.type == SDL_QUIT) {
                goto done;
            }
            // 1 or true = Down ; 0 or false = Up
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_1:
                    chip.keystore[0x1] = 1;
                    break;
                case SDLK_2:
                    chip.keystore[0x2] = 1;
                    break;
                case SDLK_3:
                    chip.keystore[0x3] = 1;
                    break;
                case SDLK_4:
                    chip.keystore[0xC] = 1;
                    break;
                case SDLK_q:
                    chip.keystore[0x4] = 1;
                    break;
                case SDLK_w:
                    chip.keystore[0x5] = 1;
                    break;
                case SDLK_e:
                    chip.keystore[0x6] = 1;
                    break;
                case SDLK_r:
                    chip.keystore[0xD] = 1;
                    break;
                case SDLK_a:
                    chip.keystore[0x7] = 1;
                    break;
                case SDLK_s:
                    chip.keystore[0x8] = 1;
                    break;
                case SDLK_d:
                    chip.keystore[0x9] = 1;
                    break;
                case SDLK_f:
                    chip.keystore[0xE] = 1;
                    break;
                case SDLK_z:
                    chip.keystore[0xA] = 1;
                    break;
                case SDLK_x:
                    chip.keystore[0x0] = 1;
                    break;
                case SDLK_c:
                    chip.keystore[0xB] = 1;
                    break;
                case SDLK_v:
                    chip.keystore[0xF] = 1;
                    break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_1:
                    chip.keystore[0x1] = 0;
                    break;
                case SDLK_2:
                    chip.keystore[0x2] = 0;
                    break;
                case SDLK_3:
                    chip.keystore[0x3] = 0;
                    break;
                case SDLK_4:
                    chip.keystore[0xC] = 0;
                    break;
                case SDLK_q:
                    chip.keystore[0x4] = 0;
                    break;
                case SDLK_w:
                    chip.keystore[0x5] = 0;
                    break;
                case SDLK_e:
                    chip.keystore[0x6] = 0;
                    break;
                case SDLK_r:
                    chip.keystore[0xD] = 0;
                    break;
                case SDLK_a:
                    chip.keystore[0x7] = 0;
                    break;
                case SDLK_s:
                    chip.keystore[0x8] = 0;
                    break;
                case SDLK_d:
                    chip.keystore[0x9] = 0;
                    break;
                case SDLK_f:
                    chip.keystore[0xE] = 0;
                    break;
                case SDLK_z:
                    chip.keystore[0xA] = 0;
                    break;
                case SDLK_x:
                    chip.keystore[0x0] = 0;
                    break;
                case SDLK_c:
                    chip.keystore[0xB] = 0;
                    break;
                case SDLK_v:
                    chip.keystore[0xF] = 0;
                    break;
                default:
                    break;
                }
            }
        }

        // Execute the chip 8 instructions
        for (uint32_t calcloop = 0; calcloop < INSTRUCT_SPEED / 60; calcloop++) {
            deopcode(&chip);
        }

        // decrease dtime and stime
        tick();

		//Delay of 60 fps
        SDL_Delay(16);

        SDL_Rect rectangle = {.x = 0, .y = 0, .w = SCALE_FACTOR, .h = SCALE_FACTOR};

		//loop through whole chip display, if it's set to 1 draw white rectangle, otherwise draw black rectangle 
        for (uint16_t count = 0; count < HORIZONTAL * VERTICAL; count++) {
            rectangle.x = (count % HORIZONTAL) * SCALE_FACTOR;
            rectangle.y = (count / HORIZONTAL) * SCALE_FACTOR;
			
            if (chip.displayer[count] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &rectangle);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &rectangle);
            }
        }
        SDL_RenderPresent(renderer);
    }
// To leave 
done:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
