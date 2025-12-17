#include "SDLContainer.h"
#include <iostream>
#include <QWindow>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "SDL_SmartPointer.h"

namespace {
    constexpr int DEFAULT_WINDOW_WIDTH = 800;
    constexpr int DEFAULT_WINDOW_HEIGHT = 600;
    constexpr int DEFAULT_SURFACE_WIDTH = 640;
    constexpr int DEFAULT_SURFACE_HEIGHT = 480;
    constexpr int COLOR_MAX_VALUE = 256;
}

// Instance members are defined in the class now

void SDLContainer::initSDL()
{
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }
   
    window = SDL_CreateWindow("SDL Window",
                              DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        printf("Unable to create SDL window: %s\n", SDL_GetError());
        return;
    }
    
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("Unable to create SDL renderer: %s\n", SDL_GetError());
        return;
    }
    surface.reset(SDL_CreateSurface(DEFAULT_SURFACE_WIDTH, DEFAULT_SURFACE_HEIGHT, SDL_PIXELFORMAT_RGBA32));
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
    Uint32 color = SDL_MapRGB(fmt, nullptr, rand() % COLOR_MAX_VALUE, rand() % COLOR_MAX_VALUE, rand() % COLOR_MAX_VALUE);
    SDL_FillSurfaceRect(surface.get(), NULL, color); // Random color background
    texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    render();
}

void SDLContainer::createNativeWindow()
{
    if (!window) {
        printf("Error: SDL window is null\n");
        return;
    }

    const char* driver = SDL_GetCurrentVideoDriver();
    if (!driver) {
        printf("Error: Unable to determine SDL video driver: %s\n", SDL_GetError());
        return;
    }

    if (SDL_strcmp(driver, "x11") != 0) {
        printf("Error: Not running on X11 (current driver: %s)\n", driver);
        return;
    }

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    Sint64 xwindow = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    if (xwindow == 0) {
        printf("Error: Failed to get X11 window handle\n");
        return;
    }

    embedded = QWindow::fromWinId(static_cast<WId>(xwindow));
    if (!embedded) {
        printf("Error: Failed to create QWindow from native handle\n");
        return;
    }
    
    // Don't hide the window - the QWindow needs the SDL window to remain visible
    printf("Successfully created embedded QWindow\n");
}


void SDLContainer::render()
{
    if (!renderer) {
        return;
    }
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture.get(), NULL, NULL);
    SDL_RenderPresent(renderer);
    std::cout << "Rendered SDL content" << std::endl;
}

void SDLContainer::resize(int width, int height)
{
    if (!window) {
        printf("Error: SDL window is null\n");
        return;
    }
    std::cout << "Resizing SDL window to " << width << "x" << height << std::endl;
    
    SDL_SetWindowSize(window, width, height);

}


