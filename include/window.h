#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include "chip8.h"

bool windowInit(int scale);
void rendering(Chip8 *chip8);

#endif
