#include <SDL3/SDL.h>
#include <iostream>
#include "components/emu.h"

int main(int argc, char* argv[])
{
    Emu::cmp.rom.cart_load("/home/user/Documents/projects/sdl-gameboy/roms/dmg-acid2.gb");
}