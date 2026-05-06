#include "enigme.h"


typedef struct {
    float x, y;
    float vx, vy;
    int vie, actif;
} Particule;

#define MAX_PARTICULES 150

Particule particules[MAX_PARTICULES];

void init_particules() {
    for(int i = 0; i < MAX_PARTICULES; i++)
        particules[i].actif = 0;
}

void spawn_particules(int x, int y, int count) {
    for(int i = 0; i < MAX_PARTICULES && count > 0; i++) {
        if(!particules[i].actif) {
            particules[i].x = x;
            particules[i].y = y;
            particules[i].vx = (rand() % 200 - 100) / 10.0;
            particules[i].vy = (rand() % 200 - 200) / 10.0;
            particules[i].vie = 60;
            particules[i].actif = 1;
            count--;
        }
    }
}

void update_particules() {
    for(int i = 0; i < MAX_PARTICULES; i++) {
        if(particules[i].actif) {
            particules[i].x += particules[i].vx;
            particules[i].y += particules[i].vy;
            particules[i].vy += 0.3;
            particules[i].vie--;
            if(particules[i].vie <= 0 || particules[i].y > SCREEN_H)
                particules[i].actif = 0;
        }
    }
}

void draw_particules(SDL_Renderer* r) {
    for(int i = 0; i < MAX_PARTICULES; i++) {
        if(particules[i].actif) {
            int alpha = particules[i].vie * 4;
            if(alpha > 255) alpha = 255;
            SDL_SetRenderDrawColor(r, 215, 180, 50, alpha);
            SDL_RenderDrawPoint(r, (int)particules[i].x, (int)particules[i].y);
        }
    }
}

void afficher_texte(SDL_Renderer* r, TTF_Font* f, char* txt, int x, int y, SDL_Color c) {
    SDL_Surface* s = TTF_RenderText_Blended(f, txt, c);
    if(s) {
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, NULL, &rect);
        SDL_DestroyTexture(t);
        SDL_FreeSurface(s);
    }
}

int main() {

    if(SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) return 1;
    
    SDL_Window* win = SDL_CreateWindow("SE7EN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    TTF_Font* font_big = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 56);
    if(!font) return 1;
    
    srand(time(NULL));
    init_particules();
    

    EnigmeData tab[MAX_ENIGMES];
    int nb = charger_enigmes("enigme.txt", tab);
    if(nb == 0) return 1;
    

    int score = 0, vies = 3, niveau = 1, running = 1;
    int enigma_on = 0, show_msg = 0;
    int q_restantes = compter_questions_disponibles(tab, nb, 1);
    int q_niveau2 = compter_questions_disponibles(tab, nb, 2);
    char msg[200] = "";
    Uint32 msg_time = 0;
    Enigme e;
    memset(&e, 0, sizeof(Enigme));
    

    int game_started = 0;
    while(running && !game_started) {
        SDL_Event ev;
        while(SDL_PollEvent(&ev)) {
            if(ev.type == SDL_QUIT) running = 0;
            if(ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_SPACE) game_started = 1;
        }
        
        SDL_SetRenderDrawColor(ren, 20, 20, 40, 255);
        SDL_RenderClear(ren);
        
        afficher_texte(ren, font_big, "SE7EN", SCREEN_W/2 - 80, 150, (SDL_Color){215,180,50,255});
        afficher_texte(ren, font, "APPUYEZ SUR ESPACE", SCREEN_W/2 - 100, SCREEN_H/2, (SDL_Color){255,255,255,255});
        
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    
    if(!running) return 0;
    

    while(running) {
        SDL_Event ev;
        while(SDL_PollEvent(&ev)) {
            if(ev.type == SDL_QUIT) running = 0;
            
            if(ev.type == SDL_MOUSEBUTTONDOWN && enigma_on && e.actif && vies > 0) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                int res = verifier_reponse(&e, x, y, &score, &vies, &niveau, &q_restantes);
                
                if(res == 1) {
                    sprintf(msg, "+100 points | Reste: %d", q_restantes);
                    spawn_particules(x, y, 30);
                    enigma_on = 0;
                    show_msg = 1;
                    msg_time = SDL_GetTicks();
                    liberer_question(&e);
                } else if(res == 2) {
                    sprintf(msg, "NIVEAU 2 !");
                    q_restantes = q_niveau2;
                    enigma_on = 0;
                    show_msg = 1;
                    msg_time = SDL_GetTicks();
                    liberer_question(&e);
                } else if(res == 3) {
                    running = 0; // Victoire
                } else if(res == -1) {
                    sprintf(msg, "-1 vie | Reste: %d", vies);
                    spawn_particules(x, y, 20);
                    enigma_on = 0;
                    show_msg = 1;
                    msg_time = SDL_GetTicks();
                    liberer_question(&e);
                }
            }
        }
        
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_SPACE] && !enigma_on && !show_msg && vies > 0 && q_restantes > 0) {
            e = generer_question(tab, nb, niveau, ren, font);
            if(e.actif) enigma_on = 1;
        }
        
        if(enigma_on && e.actif) {
            mettre_a_jour_temps(&e);
            if(e.temps_restant <= 0) {
                vies--;
                enigma_on = 0;
                sprintf(msg, "Temps ecoule! Vies: %d", vies);
                show_msg = 1;
                msg_time = SDL_GetTicks();
                liberer_question(&e);
            }
        }
        
        if(show_msg && SDL_GetTicks() - msg_time > 2000) show_msg = 0;
        
        update_particules();
        
 
        for(int i = 0; i < SCREEN_H; i++) {
            int c = 40 + sin(SDL_GetTicks() * 0.002 + i * 0.02) * 20;
            SDL_SetRenderDrawColor(ren, c/2, c/3, c, 255);
            SDL_RenderDrawLine(ren, 0, i, SCREEN_W, i);
        }
        
        draw_particules(ren);
        

        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 200);
        SDL_Rect panel = {20, 20, 250, 120};
        SDL_RenderFillRect(ren, &panel);
        
        char txt[50];
        sprintf(txt, "Score: %d", score);
        afficher_texte(ren, font, txt, 35, 35, (SDL_Color){215,180,50,255});
        sprintf(txt, "Vies: %d", vies);
        afficher_texte(ren, font, txt, 35, 70, (SDL_Color){255,255,255,255});
        sprintf(txt, "Niveau: %d", niveau);
        afficher_texte(ren, font, txt, 35, 105, (SDL_Color){215,180,50,255});
        
        sprintf(txt, "Restant: %d", q_restantes);
        afficher_texte(ren, font, txt, SCREEN_W - 150, 35, (SDL_Color){215,180,50,255});
        
        if(!enigma_on && !show_msg && vies > 0 && q_restantes > 0) {
            afficher_texte(ren, font_big, "ESPACE", SCREEN_W/2 - 80, SCREEN_H - 100, (SDL_Color){215,180,50,255});
        }
        
        if(show_msg) {
            int w, h;
            TTF_SizeText(font, msg, &w, &h);
            SDL_Rect bg = {(SCREEN_W - w)/2 - 20, SCREEN_H/2 - 30, w + 40, h + 30};
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 220);
            SDL_RenderFillRect(ren, &bg);
            afficher_texte(ren, font, msg, (SCREEN_W - w)/2, SCREEN_H/2 - 15, (SDL_Color){215,180,50,255});
        }
        
        if(enigma_on && e.actif) {
            afficher_question(ren, &e, font);
        }
        
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    
    if(vies > 0) {
        afficher_texte(ren, font_big, "VICTOIRE !", SCREEN_W/2 - 100, SCREEN_H/2 - 100, (SDL_Color){215,180,50,255});
    } else {
        afficher_texte(ren, font_big, "GAME OVER", SCREEN_W/2 - 100, SCREEN_H/2 - 100, (SDL_Color){200,50,50,255});
    }
    
    char score_txt[50];
    sprintf(score_txt, "Score final: %d", score);
    afficher_texte(ren, font, score_txt, SCREEN_W/2 - 80, SCREEN_H/2, (SDL_Color){255,255,255,255});
    SDL_RenderPresent(ren);
    SDL_Delay(4000);
    

    liberer_question(&e);
    TTF_CloseFont(font);
    TTF_CloseFont(font_big);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
