#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/chip8.h"
#include "../include/window.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
uint32_t buffer[DISPLAY_X * DISPLAY_Y]; 

bool windowInit(int scale){
    int result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if(result < 0){
        SDL_Log("errore init: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Chip 8 Emu", DISPLAY_X * scale, DISPLAY_Y * scale, 0);
    if(window == NULL){
        SDL_Log("errore creazione window: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL){
        SDL_Log("errore renderer: %s", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, DISPLAY_X, DISPLAY_Y);
    if(texture == NULL){
        SDL_Log("errore texture: %s", SDL_GetError());
        return false;
    }

    return true;
}

void rendering(Chip8 *chip8){
    for(int i=0; i<DISPLAY_X * DISPLAY_Y; i++){
        if(chip8->display[i] == 1){
            buffer[i] = 0xFFFFFFFF;
        }
        else{
            buffer[i] = 0x000000FF;
        }
    }

    SDL_UpdateTexture(texture, NULL, buffer, DISPLAY_X * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}