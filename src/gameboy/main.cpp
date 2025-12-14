#include <SDL3/SDL.h>
#include <iostream>
#include "components/emu.h"

int main(int argc, char* argv[])
{
    Emu emu; // Create emulator instance
    emu.set_component_pointers(); // Set up component pointers
    emu.get_rom().cart_load("/Users/user/Documents/sdl-gameboy/roms/cpu_instrs.gb");

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
        }

        emu.ctx.ticks++;
    }
}