#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <QWindow>
#include <iostream>
#include "SDL_SmartPointer.h"

class SDLContainer {
public:
        SDLContainer() = default;
        ~SDLContainer() = default;

        void render(const uint8_t* display);
        void initSDL();
        void createNativeWindow();
        void resize(int width, int height);

        SDL_Renderer *renderer = nullptr;
        SDL_Window *window = nullptr;
        SDL_SmartPointer<SDL_Texture> texture{nullptr};
        SDL_SmartPointer<SDL_Surface> surface{nullptr};
        QWindow* embedded = nullptr;
};