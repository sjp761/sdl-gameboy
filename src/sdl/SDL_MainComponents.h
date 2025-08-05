#pragma once
#include "SDL_SmartPointer.h"
#include <SDL3/SDL.h>
#include <tuple>

class SDL_MainComponents
{
    public:
        static SDL_Window* window;
        static SDL_Renderer* renderer;
        static SDL_SmartTexture display;
        static void renderUpdate();
        static void init();

};