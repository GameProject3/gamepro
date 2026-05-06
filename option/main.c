#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "option.h"
#include <stdio.h>

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        printf("SDL_ttf initialization failed: %s\n", TTF_GetError());
    }
    
    // Create window
    SDL_Window *window = SDL_CreateWindow("Game Options",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load background if exists
    SDL_Texture *background = IMG_LoadTexture(renderer, "background.png");
    
    // Initialize options
    Options opt;
    init_options(&opt, renderer);
    
    // Main loop
    int running = 1;
    SDL_Event event;
    Uint32 frame_time;
    
    while (running) {
        frame_time = SDL_GetTicks();
        
        while (SDL_PollEvent(&event)) {
            handle_events(&opt, &event, &running, window);
        }
        
        SDL_RenderClear(renderer);
        render_options(&opt, renderer, background);
        SDL_RenderPresent(renderer);
        
        // Cap frame rate to 60 FPS
        if (SDL_GetTicks() - frame_time < 16) {
            SDL_Delay(16 - (SDL_GetTicks() - frame_time));
        }
    }
    
    // Cleanup
    free_options(&opt);
    
    if (background) SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
