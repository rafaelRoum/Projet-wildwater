#include <stdio.h>
#include <string.h>
#include "histo.h"  // Ton module pour les stats (AVL + Tri)
#include "fuites.h" // Ton module pour les fuites (Graphe + Récursivité)

int main(int argc, char *argv[]) {
    // 1. VÉRIFICATION DES ARGUMENTS
    // On attend : ./prog fichier.csv commande option
    // argc = 4 car argv[0] est le nom du programme
    if (argc < 4) {
        // Il est important d'écrire sur stderr (2) pour ne pas polluer la sortie standard
        fprintf(stderr, "Erreur : Pas assez d'arguments.\n");
        fprintf(stderr, "Usage : %s <csv> <histo|leaks> <mode|id>\n", argv[0]);
        return 1;
    }

    char *fichier_csv = argv[1];
    char *commande = argv[2];
    char *option = argv[3]; // Peut être "src", "max" ou l'ID "Factory #1"

    // 2. OUVERTURE DU FICHIER UNIQUE
    // On ouvre le fichier ici une seule fois pour éviter de le faire dans chaque module.
    FILE *f = fopen(fichier_csv, "r");
    if (f == NULL) {
        // perror affiche l'erreur système exacte (ex: "No such file or directory")
        perror("Erreur critique d'ouverture du fichier");
        return 2;
    }

    int res = 0;

    // 3. AIGUILLAGE (DISPATCHER)
    if (strcmp(commande, "histo") == 0) {
        // Appel du module Histo (AVL classique)
        res = traiter_histo(f, option);
    }
    // ATTENTION : Le sujet demande souvent la commande "leaks". 
    // J'ajoute "leaks" OU "fuites" pour que ton code marche dans tous les cas.
    else if (strcmp(commande, "fuites") == 0 || strcmp(commande, "leaks") == 0) {
        // Appel du module Fuites (Graphe N-aire)
        res = traiter_fuites(f, option);
    }
    else {
        fprintf(stderr, "Commande inconnue : '%s'\n", commande);
        res = 3;
    }

    // 4. NETTOYAGE
    fclose(f); // Très important de fermer le fichier avant de quitter
    return res;
}
