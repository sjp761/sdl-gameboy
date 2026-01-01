#pragma once
#include <memory>
#include <SDL3/SDL.h>
#include <iostream>

template <typename T>
struct SDL_Deleter {
    void operator()(T* ptr) const {
        static_assert(sizeof(T) == 0, "SDL_Deleter not specialized for this type");
    }
};

template <>
struct SDL_Deleter<SDL_Texture> {
    void operator()(SDL_Texture* ptr) const {
        if (ptr) {
            SDL_DestroyTexture(ptr);
        }
    }
};

template <>
struct SDL_Deleter<SDL_Surface> {
    void operator()(SDL_Surface* ptr) const {
        if (ptr) {
            SDL_DestroySurface(ptr);
        }
    }
};

template <typename T>
class SDL_SmartPointer {
public:
    // Constructor
    explicit SDL_SmartPointer(T* rawPtr = nullptr)
        : ptr_(rawPtr) {}

    SDL_SmartPointer(const SDL_SmartPointer&) = delete;
    SDL_SmartPointer& operator=(const SDL_SmartPointer&) = delete;
    SDL_SmartPointer(SDL_SmartPointer&&) noexcept = default;
    SDL_SmartPointer& operator=(SDL_SmartPointer&&) noexcept = default;

    T* get() const { return ptr_.get(); }
    T* operator->() const { return ptr_.get(); }
    T& operator*() const { return *ptr_; }
    explicit operator bool() const { return static_cast<bool>(ptr_); } //Allows checking if the internal pointer is valid
    void reset(T* rawPtr = nullptr) { ptr_.reset(rawPtr); }

    std::unique_ptr<T, SDL_Deleter<T>> ptr_;
};

using SDL_SmartTexture = SDL_SmartPointer<SDL_Texture>;