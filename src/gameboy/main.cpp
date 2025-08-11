#include <SDL3/SDL.h>
#include <iostream>
#include "components/emu.h"

int main(int argc, char* argv[])
{
    Emu::cmp.rom.cart_load("/home/user/Documents/projects/sdl-gameboy/roms/dmg-acid2.gb");

    Emu::ctx.running = true;
    Emu::ctx.paused = false;
    Emu::ctx.ticks = 0;
    Emu::cmp.cpu.cpu_init();
    while(Emu::ctx.running) {
        if (Emu::ctx.paused) {
            SDL_Delay(10);
            continue;
        }

        if (!Emu::cmp.cpu.cpu_step()) {
            printf("CPU Stopped\n");
            return -3;
        }

        Emu::ctx.ticks++;
    }
}