#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../include/chip8.h"
#define FONTSET_SIZE 80

const uint8_t fontset[FONTSET_SIZE] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

void chip8Init(Chip8 *chip8){
    //settando tutto a 0 per evitare garbage
    memset(chip8->ram, 0, sizeof(chip8->ram));
    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->keypad, 0 ,sizeof(chip8->keypad));
    memset(chip8->display, 0, sizeof(chip8->display));

    chip8->i = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->flag = false;

    for(int i = 0; i < FONTSET_SIZE; i++){
        chip8->ram[0x050 + i] = fontset[i];
    }

    chip8->pc = 0x200; //0x200 inizio spazio dedicato a ROM

}

bool chip8Rom(Chip8* chip8, char* filename){
    FILE *romFile = fopen(filename, "rb");
    if(romFile == NULL){
        printf("\nCouldn't open ROM file");
        return false;
    }

    fseek(romFile, 0, SEEK_END);
    long size = ftell(romFile);
    rewind(romFile);

    long max_rom_size = RAM_SIZE - 0x200;
    if(size > max_rom_size){
        printf("\nROM file doesn't fit in memory!");
        fclose(romFile);
        return false;
    }

    //salvo num byte letti cosi' posso sapere se ho letto correttamente tutta la ROM
    //ram[0x200] e' lo starting point per scrittura
    size_t bytes_read = fread(&chip8->ram[0x200], sizeof(uint8_t), size, romFile);
    if(bytes_read != size){
        printf("\nBytes read don't match ROM size!");
        fclose(romFile);
        return false;
    }

    fclose(romFile);
    return true;

}

void chip8Draw(Chip8 *chip8, uint8_t x, uint8_t y, uint8_t rows){
    //start pos dello sprite caricata nei registri Vx, Vy
    uint8_t start_x = chip8->V[x] % DISPLAY_X;
    uint8_t start_y = chip8->V[y] % DISPLAY_Y;
    chip8->V[0xF] = 0;
    int index;

    for(int r=0; r<rows; r++){
        if(start_y + r >= DISPLAY_Y){
            break;
        } 

        uint8_t sprite = chip8->ram[chip8->i + r];

        for(int c = 0; c<8; c++){
            if(start_x + c >= DISPLAY_X){
                break;
            }

            uint8_t sprite_px = (sprite & (0x80 >> c)) ? 1 : 0;
            if (sprite_px!= 0){
                index = (start_x + c) + ((start_y + r) * DISPLAY_X);
                if(chip8->display[index] == 1){
                    chip8->V[0xF] = 1;
                }

                chip8->display[index] ^= 1;
            }
        }
    }

    chip8->flag = true;

}

void chip8Cycle(Chip8 *chip8){
    //fetch
    //ricostruisco opcode shiftando il primo pezzo di 8 bit, poi logical OR con il seguente
    uint16_t opcode = (chip8->ram[chip8->pc] << 8) | chip8->ram[chip8->pc + 1];
    chip8->pc += 2;


    //decode
    uint8_t u = (opcode & 0xF000) >> 12; //identificatore dell'operazione
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t N = (opcode & 0x000F); //identificatore sub-istruzione
    uint8_t KK = (opcode & 0x00FF); //byte
    uint16_t NNN = (opcode & 0x0FFF); //addr

    uint8_t underflow;
    uint8_t shifted;


    //exec
    switch (u)
    {
    case 0x0:
        //CLS
        if(KK == 0xE0){
            memset(chip8->display, 0 ,sizeof(chip8->display));
            chip8->flag = true;
        }
        //RET
        else if(KK == 0xEE){
            chip8->sp--;
            chip8->pc = chip8->stack[chip8->sp];
        }
        break;

    case 0x1:
        //JMP NNN
        chip8->pc = NNN;
        break;
    
    case 0x2:
        //CALL NNN
        chip8->stack[chip8->sp] = chip8->pc;
        chip8->sp++;
        chip8->pc = NNN;
        break;
    
    //0X3 - 0X5 skip oper if
    case 0x3:
        if(chip8->V[x] == KK){
            chip8->pc += 2;
        }
        break;
    
    case 0x4:
        if(chip8->V[x] != KK){
            chip8->pc += 2;
        }
        break;

    case 0x5:
        if(chip8->V[x] == chip8->V[y]){
            chip8->pc += 2;
        }
        break;
    
    case 0x6:
        //LDI VX, KK
        chip8->V[x] = KK;
        break;

    case 0x7:
        //ADD VX, KK
        chip8->V[x] = chip8->V[x] + KK;
        break;

    case 0x8:
        switch (N)
        {
        case 0x0:
            //LD VX, VY
            chip8->V[x] = chip8->V[y];
            break;
        
        case 0x1:
            //OR VX, VY
            chip8->V[x] |= chip8->V[y];
            break;
        
        case 0x2:
            //AND VX, VY
            chip8->V[x] &= chip8->V[y];
            break;

        case 0x3:
            //XOR VX, VY
            chip8->V[x] ^= chip8->V[y];
            break;

        case 0x4:
            //ADD VX, VY
            chip8->V[x] += chip8->V[y];
            break;
        
        case 0x5:
            //SUB VX, VY
            chip8->V[x] -= chip8->V[y];
            break;

        case 0x6:
            //SHR VX, VY
            //!CHECK
            chip8->V[x] = chip8->V[y];
            shifted = chip8->V[x] & 0x1;
            chip8->V[x] = chip8->V[x] >> 1;
            chip8->V[0xF] = shifted;

            break;
        
        case 0x7:
            //SUBN VX, VY
            underflow = (chip8->V[y] < chip8->V[x]) ? 1 : 0;
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            chip8->V[0xF] = underflow;
            break;
        
        case 0xE:
            //SHL VX, VY
            //!CHECK
            chip8->V[x] = chip8->V[y];
            shifted = (chip8->V[x] >> 7) & 0x1;
            chip8->V[x] = chip8->V[x] << 1;
            chip8->V[0xF] = shifted;
            break;
            
        default:
            printf("\nOpcode not found!");
            break;
        }
        break;

    case 0x9:
        //SNE VX, VY
        if(chip8->V[x] != chip8->V[y]){
            chip8->pc += 2;
        }
        break;

    case 0xA:
        //LD I, NNN
        chip8->i = NNN;
        break;
    
    case 0xB:
        //JMP NNN, V0
        chip8->pc = NNN + chip8->V[0];
        break;
    
    case 0xC:
        //RND VX, KK
        //chip8->V[x] = (rand() % 256) & KK;
        break;

    case 0xD:
        //DRW VX, VY, N
        chip8Draw(chip8, x, y, N);
        break;

    case 0xE:
        switch (KK & 0x00FF) {
        case 0x9E:
            // SKP VX
            if (chip8->keypad[chip8->V[x]]) {
                chip8->pc += 2;
            }
            break;
        case 0xA1:
            // SKNP VX
            if (!chip8->keypad[chip8->V[x]]) {
                chip8->pc += 2;
            }
            break;

        default:
            break;
        }
        break;

    case 0xF:
        switch (KK)
        {
        case 0x07:
            //LD VX, delaytimer
            chip8->V[x] = chip8->delay_timer;
            break;

        case 0x0A:
            //TODO
            break;

        case 0x15:
            //LD delaytimer, VX
            chip8->delay_timer = chip8->V[x];
            break;
        
        case 0x18:
            //LD soundtimer, VX
            chip8->sound_timer = chip8->V[x];
            break;

        case 0x1E:
            //ADD I, VX
            chip8->i += chip8->V[x];
            break;
        
        case 0x29:
            //LD F, VX
            chip8->i = 0x050 + (chip8->V[x] * 5);
            break;

        case 0x33:
            //LD B, VX
            chip8->ram[chip8->i] = chip8->V[x] / 100;
            chip8->ram[chip8->i + 1] = (chip8->V[x] / 10) % 10;
            chip8->ram[chip8->i + 2] = chip8->V[x] % 10;
            break;

        case 0x55:
            //LD [I], VX
            for (int idx = 0; idx <= x; idx++) {
                chip8->ram[chip8->i + idx] = chip8->V[idx];
            }
            break;

        case 0x65:
            //LD VX, [I]
            for (int idx = 0; idx <= x; idx++) {
                chip8->V[idx] = chip8->ram[chip8->i + idx];
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

}

/*
int main(){
    Chip8 chip8;
    char *filename;
    bool loaded;

    chip8Init(&chip8);

    printf("\nInizializzato con successo");
    printf("\npc: %d", chip8.pc);

    printf("\nInserisci la ROM\n");
    scanf("%s", filename);
    loaded = chip8Rom(&chip8, filename);
    if(loaded) printf("\nRom caricata");

    if (chip8Rom(&chip8, filename)) {
        printf("ROM loaded successfully!\n");
        printf("First 16 bytes in RAM at 0x200:\n");
        
        for (int i = 0; i < 16; i++) {
            printf("%02X ", chip8.ram[0x200 + i]);
        }
        printf("\n");
    }


    return 0;
}

*/