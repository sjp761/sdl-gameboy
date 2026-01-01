#include "SDLContainer.h"
#include <iostream>
#include <QWindow>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "SDL_SmartPointer.h"

#ifdef __APPLE__
#include <objc/runtime.h>
#include <objc/message.h>
#endif

namespace {
    constexpr int DEFAULT_WINDOW_WIDTH = 800;
    constexpr int DEFAULT_WINDOW_HEIGHT = 600;
    constexpr int DEFAULT_SURFACE_WIDTH = 160;
    constexpr int DEFAULT_SURFACE_HEIGHT = 144;
    constexpr int COLOR_MAX_VALUE = 256;
}

// Instance members are defined in the class now

void SDLContainer::initSDL()
{   
    srand(static_cast<unsigned int>(time(nullptr)));
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }
   
    window = SDL_CreateWindow("SDL Window",
                              DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!window) {
        printf("Unable to create SDL window: %s\n", SDL_GetError());
        return;
    }
    
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("Unable to create SDL renderer: %s\n", SDL_GetError());
        return;
    }
    // Create streaming texture for direct pixel updates
    texture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 
                                    DEFAULT_SURFACE_WIDTH, DEFAULT_SURFACE_HEIGHT));
    
    // Fill with random color initially
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
    Uint32 color = SDL_MapRGB(fmt, nullptr, rand() % COLOR_MAX_VALUE, rand() % COLOR_MAX_VALUE, rand() % COLOR_MAX_VALUE);
    void* pixels;
    int pitch;
    if (SDL_LockTexture(texture.get(), nullptr, &pixels, &pitch)) {
        SDL_memset(pixels, color & 0xFF, pitch * DEFAULT_SURFACE_HEIGHT);
        SDL_UnlockTexture(texture.get());
    }
    std::cout << std::hex << color << std::dec << std::endl;
    render(nullptr); // Initial render to clear screen
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


    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    if (SDL_strcmp(driver, "x11") == 0)
    {
        Sint64 xwindow = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xwindow == 0) 
        {
            printf("Error: Failed to get X11 window handle\n");
            return;
        }
        embedded = QWindow::fromWinId(static_cast<WId>(xwindow));
        if (!embedded) 
        {
            printf("Error: Failed to create QWindow from native handle\n");
            return;
        }
    }
    else if (SDL_strcmp(driver,"cocoa") == 0)
    {
        void* cocoaWindow = SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
        if (!cocoaWindow) 
        {
            printf("Error: Failed to get Cocoa window pointer\n");
            return;
        }
        
        // Doing Objective-C message send to get the contentView, probably hacky but works
        typedef void* (*contentViewFunc)(void*, SEL); // Creates a function pointer with generic and selector, returns void*
        contentViewFunc contentView = (contentViewFunc)objc_msgSend; // Casts objc_msgSend to our function pointer type
        void* nsView = contentView(cocoaWindow, sel_registerName("contentView")); // Calls [cocoaWindow contentView]
        
        embedded = QWindow::fromWinId(reinterpret_cast<WId>(nsView));
    }

    
    printf("Successfully created embedded QWindow\n");
}


void SDLContainer::render(const uint8_t* display)
{
    if (!renderer || !texture.get()) {
        return;
    }
    
    if (display) {
        // Lock texture and copy pixel data
        void* pixels;
        int pitch;
        if (SDL_LockTexture(texture.get(), nullptr, &pixels, &pitch))
        {
            uint32_t* pixel_data = static_cast<uint32_t*>(pixels);
            for (int y = 0; y < DEFAULT_SURFACE_HEIGHT; y++) 
            {
                for (int x = 0; x < DEFAULT_SURFACE_WIDTH; x++)
                {
                    uint8_t palette_index = display[y * DEFAULT_SURFACE_WIDTH + x];
                    // Map palette index (0-3) to color
                    if (palette_index < 4) {
                        pixel_data[y * (pitch / 4) + x] = gameboy_palette[palette_index];
                    }
                }
            }
            SDL_UnlockTexture(texture.get());
        } 
        else 
        {
            printf("SDL_LockTexture failed: %s\n", SDL_GetError());
        }
    }
    
    SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_NEAREST);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer);
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


