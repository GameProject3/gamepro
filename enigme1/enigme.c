#include "enigme.h"


int charger_enigmes(const char* fichier, EnigmeData* tab) {
    FILE* f = fopen(fichier, "r");
    if (!f) {
        printf("Erreur: Impossible d'ouvrir %s\n", fichier);
        return 0;
    }
    
    int count = 0;
    char ligne[1024];
    
    while (fgets(ligne, sizeof(ligne), f) && count < MAX_ENIGMES) {

        ligne[strcspn(ligne, "\r\n")] = 0;
        if (strlen(ligne) < 3) continue;
        

        char niveau_char = ligne[0];
        if (niveau_char == '1') tab[count].niveau = 1;
        else if (niveau_char == '2') tab[count].niveau = 2;
        else continue;
        
        char* token = strtok(ligne + 2, "?");
        if (token) strcpy(tab[count].question, token);
        else continue;
        
        token = strtok(NULL, "!");
        if (token) strcpy(tab[count].rep1, token);
        
        token = strtok(NULL, "!");
        if (token) strcpy(tab[count].rep2, token);
        
        token = strtok(NULL, "!");
        if (token) strcpy(tab[count].rep3, token);
        
        token = strtok(NULL, "!");
        if (token) tab[count].bonne_reponse = atoi(token);
        
        tab[count].deja_vue = 0;
        count++;
    }
    
    fclose(f);
    printf("%d énigmes chargées sur le thème SE7EN\n", count);
    return count;
}


int compter_questions_disponibles(EnigmeData* tab, int nb, int niveau) {
    int count = 0;
    for (int i = 0; i < nb; i++) {
        if (tab[i].niveau == niveau && tab[i].deja_vue == 0) {
            count++;
        }
    }
    return count;
}


void reinitialiser_deja_vues(EnigmeData* tab, int nb) {
    for (int i = 0; i < nb; i++) {
        tab[i].deja_vue = 0;
    }
}


Enigme generer_question(EnigmeData* tab, int nb, int niveau, SDL_Renderer* renderer, TTF_Font* font) {
    Enigme e;
    memset(&e, 0, sizeof(Enigme));
    e.niveau_actuel = niveau;
    e.temps_max = (niveau == 1) ? 30 : 20;
    e.temps_restant = e.temps_max;
    

    EnigmeData disponibles[MAX_ENIGMES];
    int nb_dispo = 0;
    
    for (int i = 0; i < nb; i++) {
        if (tab[i].niveau == niveau && tab[i].deja_vue == 0) {
            disponibles[nb_dispo++] = tab[i];
        }
    }
    
    if (nb_dispo == 0) {
        e.actif = 0;
        return e;
    }
    

    int idx = rand() % nb_dispo;
    e.data = disponibles[idx];
    

    for (int i = 0; i < nb; i++) {
        if (strcmp(tab[i].question, disponibles[idx].question) == 0) {
            tab[i].deja_vue = 1;
            break;
        }
    }
    
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color or = {215, 180, 50, 255};
    SDL_Surface* surface;
    

    surface = TTF_RenderText_Blended_Wrapped(font, e.data.question, or, SCREEN_W - 200);
    if (surface) {
        e.tex_question = SDL_CreateTextureFromSurface(renderer, surface);
        e.rect_q = (SDL_Rect){(SCREEN_W - surface->w) / 2, 120, surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
    

    surface = TTF_RenderText_Blended_Wrapped(font, e.data.rep1, blanc, SCREEN_W - 300);
    if (surface) {
        e.tex_rep1 = SDL_CreateTextureFromSurface(renderer, surface);
        e.rect_r1 = (SDL_Rect){(SCREEN_W - surface->w) / 2, 320, surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
    
    surface = TTF_RenderText_Blended_Wrapped(font, e.data.rep2, blanc, SCREEN_W - 300);
    if (surface) {
        e.tex_rep2 = SDL_CreateTextureFromSurface(renderer, surface);
        e.rect_r2 = (SDL_Rect){(SCREEN_W - surface->w) / 2, 440, surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
    
    surface = TTF_RenderText_Blended_Wrapped(font, e.data.rep3, blanc, SCREEN_W - 300);
    if (surface) {
        e.tex_rep3 = SDL_CreateTextureFromSurface(renderer, surface);
        e.rect_r3 = (SDL_Rect){(SCREEN_W - surface->w) / 2, 560, surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
    
    e.actif = 1;
    e.temps_debut = SDL_GetTicks();
    
    return e;
}


void afficher_question(SDL_Renderer* renderer, Enigme* e, TTF_Font* font) {
    if (!e->actif) return;
    

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect zone_jeu = {50, 50, SCREEN_W - 100, SCREEN_H - 100};
    SDL_RenderFillRect(renderer, &zone_jeu);
    

    SDL_SetRenderDrawColor(renderer, 215, 180, 50, 255);
    SDL_RenderDrawRect(renderer, &zone_jeu);
    

    if (font) {
        char titre[50];
        sprintf(titre, "PECHE CAPITAL - NIVEAU %d", e->niveau_actuel);
        SDL_Color or = {215, 180, 50, 255};
        SDL_Surface* s = TTF_RenderText_Blended(font, titre, or);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect rect = {(SCREEN_W - s->w) / 2, 60, s->w, s->h};
            SDL_RenderCopy(renderer, t, NULL, &rect);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }
    

    if (e->tex_question) {
        SDL_RenderCopy(renderer, e->tex_question, NULL, &e->rect_q);
    }
    

    if (e->tex_rep1) {
        SDL_RenderCopy(renderer, e->tex_rep1, NULL, &e->rect_r1);
    }
    if (e->tex_rep2) {
        SDL_RenderCopy(renderer, e->tex_rep2, NULL, &e->rect_r2);
    }
    if (e->tex_rep3) {
        SDL_RenderCopy(renderer, e->tex_rep3, NULL, &e->rect_r3);
    }
    

    float progression = (float)e->temps_restant / e->temps_max;
    SDL_Rect barre_fond = {150, SCREEN_H - 80, SCREEN_W - 300, 25};
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
    SDL_RenderFillRect(renderer, &barre_fond);
    

    if (progression > 0.66) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 100, 255);
    } else if (progression > 0.33) {
        int pulse = abs(sin(SDL_GetTicks() * 0.008)) * 80;
        SDL_SetRenderDrawColor(renderer, 255, 150 + pulse, 0, 255);
    } else {
        int pulse = abs(sin(SDL_GetTicks() * 0.015)) * 100;
        SDL_SetRenderDrawColor(renderer, 255, 50, 50 + pulse/2, 255);
    }
    
    SDL_Rect barre_temps = {150, SCREEN_H - 80, (int)((SCREEN_W - 300) * progression), 25};
    SDL_RenderFillRect(renderer, &barre_temps);
    

    SDL_SetRenderDrawColor(renderer, 215, 180, 50, 255);
    SDL_RenderDrawRect(renderer, &barre_fond);
}


int verifier_reponse(Enigme* e, int x, int y, int* score, int* vies, int* niveau, int* questions_restantes) {
    if (!e->actif || e->temps_restant <= 0) return 0;
    
    int choix = 0;
    

    if (x >= e->rect_r1.x && x <= e->rect_r1.x + e->rect_r1.w &&
        y >= e->rect_r1.y && y <= e->rect_r1.y + e->rect_r1.h) {
        choix = 1;
    } else if (x >= e->rect_r2.x && x <= e->rect_r2.x + e->rect_r2.w &&
               y >= e->rect_r2.y && y <= e->rect_r2.y + e->rect_r2.h) {
        choix = 2;
    } else if (x >= e->rect_r3.x && x <= e->rect_r3.x + e->rect_r3.w &&
               y >= e->rect_r3.y && y <= e->rect_r3.y + e->rect_r3.h) {
        choix = 3;
    }
    
    if (choix == 0) return 0;
    
    if (choix == e->data.bonne_reponse) {
        *score += 100;
        (*questions_restantes)--;
        

        if (*score >= 600 && *niveau == 1) {
            *niveau = 2;
            return 2;  
        }
        
        if (*questions_restantes == 0) {
            return 3;  
        }
        return 1;  
    } else {
        (*vies)--;
        return -1;  
    }
}


void mettre_a_jour_temps(Enigme* e) {
    if (!e->actif) return;
    Uint32 ecoule = (SDL_GetTicks() - e->temps_debut) / 1000;
    e->temps_restant = e->temps_max - ecoule;
    if (e->temps_restant < 0) e->temps_restant = 0;
}


void liberer_question(Enigme* e) {
    if (e->tex_question) SDL_DestroyTexture(e->tex_question);
    if (e->tex_rep1) SDL_DestroyTexture(e->tex_rep1);
    if (e->tex_rep2) SDL_DestroyTexture(e->tex_rep2);
    if (e->tex_rep3) SDL_DestroyTexture(e->tex_rep3);
    e->tex_question = NULL;
    e->tex_rep1 = NULL;
    e->tex_rep2 = NULL;
    e->tex_rep3 = NULL;
    e->actif = 0;
}
