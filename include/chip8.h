//FILE RAPPRESENTA LO STATO INTERNO DELL'EMU
//file header serve per dichiarare funzioni e datatypes senza definirli 

//ifndef serve per eseguire chip8.h SOLO se non è ancora definito
#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE 4096
#define REG_NUM 16 //considero solo [V0, VE] (multipurpose)
#define STACK_SIZE 16

#define KEYPAD_SIZE 16

#define DISPLAY_X 64
#define DISPLAY_Y 32
#define FONTSET_SIZE 80


typedef struct {
    uint8_t ram[RAM_SIZE];
    uint8_t V[REG_NUM];

    uint16_t i; //indice di registro
    uint16_t pc;

    uint16_t stack[STACK_SIZE];
    uint8_t sp;

    bool keypad[KEYPAD_SIZE];

    uint8_t delay_timer;
    uint8_t sound_timer;

    bool display[DISPLAY_X * DISPLAY_Y];
    bool flag;

}Chip8;

void chip8Init(Chip8* chip8);
bool chip8Rom(Chip8* chip8, char* filename);
void chip8Cycle(Chip8* chip8); //definisce il ciclo della CPU

#endif