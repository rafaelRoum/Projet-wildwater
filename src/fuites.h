#ifndef FUITES_H
#define FUITES_H
#include <stdio.h>

/* Structure représentant un nœud du graphe (réseau de distribution).
   Contient l'identifiant, le pourcentage de fuite, ainsi que les pointeurs
   vers le premier enfant et le frère suivant (représentation "fils aîné / frère suivant"). */
typedef struct NoeudGraphe {
    char *id;
    double fuite_pct;
    struct NoeudGraphe *premier_fils;
    struct NoeudGraphe *frere_suivant;
} NoeudGraphe;

/* Fonction principale qui lit le fichier CSV, construit le graphe,
   calcule le volume perdu pour l'usine spécifiée et génère le fichier de sortie. */
int traiter_fuites(FILE *fichier, char *id_usine);

#endif
