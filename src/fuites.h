#ifndef FUITES_H
#define FUITES_H
#include <stdio.h>

/* Structure représentant un nœud du graphe (réseau de distribution). */
typedef struct NoeudGraphe {
    char *id;
    double fuite_pct;
    struct NoeudGraphe *premier_fils;
    struct NoeudGraphe *frere_suivant;
} NoeudGraphe;

/* Fonction principale  */
int traiter_fuites(FILE *fichier, char *id_usine);

#endif
