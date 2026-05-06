#include "option.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Global pointer for font access in draw_button
static TTF_Font* global_font = NULL;
static SDL_Renderer* global_renderer = NULL;

void init_button(Button *btn, int x, int y, int w, int h, const char *label) {
    btn->rect = (SDL_Rect){x, y, w, h};
    btn->hovered = 0;
    btn->anim_scale = 0;
    strncpy(btn->label, label, 49);
    btn->label[49] = '\0';
}

void init_options(Options *opt, SDL_Renderer *renderer) {
    printf("Initializing options menu...\n");
    opt->renderer = renderer;
    opt->volume = 50;
    opt->is_fullscreen = 0;
    opt->last_anim_time = 0;
    opt->font = NULL;
    opt->hover_sound = NULL;
    opt->music = NULL;
    
    // Store globally for draw_button
    global_renderer = renderer;
    
    // Initialize buttons - clean black buttons
    init_button(&opt->buttons[0], 540, 180, 200, 50, "VOLUME UP");
    init_button(&opt->buttons[1], 540, 260, 200, 50, "VOLUME DOWN");
    init_button(&opt->buttons[2], 540, 340, 200, 50, "FULLSCREEN");
    init_button(&opt->buttons[3], 540, 420, 200, 50, "WINDOWED");
    init_button(&opt->buttons[4], 540, 500, 200, 50, "BACK");
    
    // Load sound (optional)
    opt->hover_sound = Mix_LoadWAV("hover.wav");
    if (!opt->hover_sound) {
        // Continue without sound
    }
    
    // Load music (optional)
    opt->music = Mix_LoadMUS("music.mp3");
    if (opt->music) {
        Mix_PlayMusic(opt->music, -1);
        Mix_VolumeMusic(opt->volume);
    }
    
    // Load font
    opt->font = TTF_OpenFont("font.ttf", 24);
    global_font = opt->font;
    if (!opt->font) {
        printf("Font not found\n");
    }
}

void update_button_animations(Options *opt) {
    Uint32 current_time = SDL_GetTicks();
    if (current_time - opt->last_anim_time < 16) return;
    opt->last_anim_time = current_time;
    
    for (int i = 0; i < 5; i++) {
        if (opt->buttons[i].hovered) {
            if (opt->buttons[i].anim_scale < 10) {
                opt->buttons[i].anim_scale += 2;
                if (opt->buttons[i].anim_scale > 10) opt->buttons[i].anim_scale = 10;
            }
        } else {
            if (opt->buttons[i].anim_scale > 0) {
                opt->buttons[i].anim_scale -= 2;
                if (opt->buttons[i].anim_scale < 0) opt->buttons[i].anim_scale = 0;
            }
        }
    }
}

void draw_button(SDL_Renderer *renderer, Button *btn, TTF_Font *font) {
    // Calculate scale
    float scale = 1.0f + (btn->anim_scale / 10.0f) * 0.05f;
    
    int new_w = (int)(btn->rect.w * scale);
    int new_h = (int)(btn->rect.h * scale);
    int new_x = btn->rect.x + (btn->rect.w - new_w) / 2;
    int new_y = btn->rect.y + (btn->rect.h - new_h) / 2;
    
    SDL_Rect scaled_rect = {new_x, new_y, new_w, new_h};
    
    // Draw button background (black)
    if (btn->hovered) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);  // Dark gray when hovered
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black when normal
    }
    SDL_RenderFillRect(renderer, &scaled_rect);
    
    // Draw border (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &scaled_rect);
    
    // Draw text
    if (font) {
        SDL_Color text_color = {255, 255, 255, 255};
        SDL_Surface *surf = TTF_RenderText_Blended(font, btn->label, text_color);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                int text_x = scaled_rect.x + (scaled_rect.w - surf->w) / 2;
                int text_y = scaled_rect.y + (scaled_rect.h - surf->h) / 2;
                SDL_Rect text_rect = {text_x, text_y, surf->w, surf->h};
                SDL_RenderCopy(renderer, tex, NULL, &text_rect);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y) {
    if (!font) return;
    
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;
    
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect rect = {x, y, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &rect);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void check_hover(Button *btn, int mx, int my, Mix_Chunk *sound, int *played) {
    int was_hovered = btn->hovered;
    btn->hovered = (mx > btn->rect.x && mx < btn->rect.x + btn->rect.w &&
                    my > btn->rect.y && my < btn->rect.y + btn->rect.h);
    
    if (btn->hovered && !was_hovered && sound && !*played) {
        Mix_PlayChannel(-1, sound, 0);
        *played = 1;
    }
}

void handle_events(Options *opt, SDL_Event *event, int *running, SDL_Window *window) {
    static int hover_played = 0;
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    
    // Check hover for all buttons
    for (int i = 0; i < 5; i++) {
        check_hover(&opt->buttons[i], mx, my, opt->hover_sound, &hover_played);
    }
    
    // Reset hover played flag
    int any_hover = 0;
    for (int i = 0; i < 5; i++) {
        if (opt->buttons[i].hovered) any_hover = 1;
    }
    if (!any_hover) hover_played = 0;
    
    if (event->type == SDL_QUIT) {
        *running = 0;
    }
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (opt->buttons[0].hovered) {
            opt->volume += 10;
            if (opt->volume > 100) opt->volume = 100;
            printf("Volume: %d%%\n", opt->volume);
            if (opt->music) {
                Mix_VolumeMusic((opt->volume * MIX_MAX_VOLUME) / 100);
            }
        }
        
        if (opt->buttons[1].hovered) {
            opt->volume -= 10;
            if (opt->volume < 0) opt->volume = 0;
            printf("Volume: %d%%\n", opt->volume);
            if (opt->music) {
                Mix_VolumeMusic((opt->volume * MIX_MAX_VOLUME) / 100);
            }
        }
        
        if (opt->buttons[2].hovered) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            opt->is_fullscreen = 1;
            printf("Fullscreen mode enabled\n");
        }
        
        if (opt->buttons[3].hovered) {
            SDL_SetWindowFullscreen(window, 0);
            opt->is_fullscreen = 0;
            printf("Windowed mode enabled\n");
        }
        
        if (opt->buttons[4].hovered) {
            *running = 0;
            printf("Exiting options menu\n");
        }
    }
}

void render_options(Options *opt, SDL_Renderer *renderer, SDL_Texture *background) {
    // Update animations
    update_button_animations(opt);
    
    // Draw background
    if (background) {
        SDL_RenderCopy(renderer, background, NULL, NULL);
    } else {
        // Simple dark background
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);
    }
    
    // Draw title
    render_text(renderer, opt->font, "GAME OPTIONS", 560, 50);
    
    // Draw separator line
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, 540, 90, 740, 90);
    
    // Draw volume section
    render_text(renderer, opt->font, "VOLUME CONTROL", 565, 115);
    
    char volume_text[50];
    sprintf(volume_text, "VOLUME %d%%", opt->volume);
    render_text(renderer, opt->font, volume_text, 580, 145);
    
    // Draw volume bar
    SDL_Rect volume_bar_bg = {540, 170, 200, 8};
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &volume_bar_bg);
    
    SDL_Rect volume_bar_fill = {540, 170, (opt->volume * 200) / 100, 8};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &volume_bar_fill);
    
    // Draw buttons
    for (int i = 0; i < 5; i++) {
        draw_button(renderer, &opt->buttons[i], opt->font);
    }
    
    // Draw footer hint (only when no buttons hovered)
    int any_hover = 0;
    for (int i = 0; i < 5; i++) {
        if (opt->buttons[i].hovered) any_hover = 1;
    }
    
    if (!any_hover) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
        SDL_RenderDrawLine(renderer, 540, 580, 740, 580);
        
        render_text(renderer, opt->font, "HOVER OVER BUTTONS", 555, 595);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void free_options(Options *opt) {
    if (opt->hover_sound) Mix_FreeChunk(opt->hover_sound);
    if (opt->music) {
        Mix_HaltMusic();
        Mix_FreeMusic(opt->music);
    }
    if (opt->font) TTF_CloseFont(opt->font);
}
