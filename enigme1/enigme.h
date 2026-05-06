#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_ENIGMES 50
#define SCREEN_W 1024
#define SCREEN_H 768
#define MAX_QUESTIONS_PER_LEVEL 10


typedef struct {
    char question[512];
    char rep1[256];
    char rep2[256];
    char rep3[256];
    int bonne_reponse;    
    int niveau;       
    int deja_vue;     
} EnigmeData;


typedef struct {
    EnigmeData data;
    SDL_Texture* tex_question;
    SDL_Texture* tex_rep1;
    SDL_Texture* tex_rep2;
    SDL_Texture* tex_rep3;
    SDL_Rect rect_q;
    SDL_Rect rect_r1;
    SDL_Rect rect_r2;
    SDL_Rect rect_r3;
    int temps_restant;
    Uint32 temps_debut;
    int temps_max;
    int actif;
    int niveau_actuel;
} Enigme;


int charger_enigmes(const char* fichier, EnigmeData* tab);
int compter_questions_disponibles(EnigmeData* tab, int nb, int niveau);
Enigme generer_question(EnigmeData* tab, int nb, int niveau, SDL_Renderer* renderer, TTF_Font* font);
void afficher_question(SDL_Renderer* renderer, Enigme* e, TTF_Font* font);
int verifier_reponse(Enigme* e, int x, int y, int* score, int* vies, int* niveau, int* questions_restantes);
void mettre_a_jour_temps(Enigme* e);
void liberer_question(Enigme* e);
void reinitialiser_deja_vues(EnigmeData* tab, int nb);

#endif
