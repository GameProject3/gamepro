#ifndef OPTION_H
#define OPTION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    int hovered;
    int anim_scale;
    char label[50];
} Button;

typedef struct {
    Button buttons[5];
    Mix_Chunk *hover_sound;
    Mix_Music *music;
    TTF_Font *font;
    SDL_Renderer *renderer;
    int volume;
    int is_fullscreen;
    Uint32 last_anim_time;
} Options;

void init_options(Options *opt, SDL_Renderer *renderer);
void handle_events(Options *opt, SDL_Event *event, int *running, SDL_Window *window);
void render_options(Options *opt, SDL_Renderer *renderer, SDL_Texture *background);
void free_options(Options *opt);

#endif
