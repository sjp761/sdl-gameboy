#include <SDL3/SDL.h>
#include <iostream>
#include "components/emu.h"
#include "../sdl/SDL_MainComponents.h"

int main(int argc, char* argv[])
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    
    // Initialize main components and debug windows
    SDL_MainComponents::init();
    SDL_MainComponents::initDebugWindows();
    
    Emu emu; // Create emulator instance
    emu.set_component_pointers(); // Set up component pointers
    emu.get_rom().cart_load("/home/user/Documents/projects/sdl-gameboy/roms/rimm.gb");

    emu.ctx.running = true;
    emu.ctx.paused = false;
    emu.ctx.ticks = 0;
    emu.get_cpu().cpu_init();
    
    SDL_Event event;
    uint64_t last_update = SDL_GetTicks();
    
    while(emu.ctx.running) {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                emu.ctx.running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    emu.ctx.running = false;
                }
            }
        }
        
        if (emu.ctx.paused) {
            SDL_Delay(10);
            continue;
        }

        if (!emu.get_cpu().cpu_step()) {
            printf("CPU Stopped\n");
        }

        emu.ctx.ticks++;
        
        // Update debug windows periodically (every ~60ms)
        uint64_t current_time = SDL_GetTicks();
        if (current_time - last_update > 60) {
            if (SDL_MainComponents::tile_viewer.is_open()) {
                SDL_MainComponents::tile_viewer.update(emu.get_bus().vram);
                SDL_MainComponents::tile_viewer.render();
            }
            
            last_update = current_time;
        }
    }
    
    // Cleanup
    SDL_Quit();
    return 0;
}