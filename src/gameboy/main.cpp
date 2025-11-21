#include <SDL3/SDL.h>
#include <iostream>
#include "components/emu.h"

int main(int argc, char* argv[])
{
    Emu emu; // Create emulator instance
    
    emu.get_rom().cart_load("/home/user/Documents/projects/sdl-gameboy/roms/dmg-acid2.gb");

    emu.ctx.running = true;
    emu.ctx.paused = false;
    emu.ctx.ticks = 0;
    emu.get_cpu().cpu_init();
    
    while(emu.ctx.running) {
        if (emu.ctx.paused) {
            SDL_Delay(10);
            continue;
        }

        if (!emu.get_cpu().cpu_step()) {
            printf("CPU Stopped\n");
            return -3;
        }

        emu.ctx.ticks++;
    }
}