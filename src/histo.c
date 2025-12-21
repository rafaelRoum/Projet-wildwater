#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histo.h"
#include "avl.h"

/* Parcourt l'arbre AVL de manière récursive inverse (Droite -> Nœud -> Gauche) pour obtenir un ordre décroissant. 
   Sélectionne la donnée appropriée selon le mode ("max", "src", "real" ou "all") et l'écrit dans le fichier de sortie. */
void parcoursInverse(avl *noeud, FILE *sortie, char *mode) {
    if (noeud == NULL) return;

    parcoursInverse(noeud->droite, sortie, mode);

    DonneesHisto *d = (DonneesHisto*)noeud->donnee;
    

    double valeur = 0.0; 

    if (strcmp(mode, "max") == 0) valeur = d->capacite;
    else if (strcmp(mode, "src") == 0) valeur = d->vol_source;
    else if (strcmp(mode, "real") == 0) valeur = d->vol_traite;

    // Gestion du cas "all" pour le bonus
    if (strcmp(mode, "all") == 0) {
        fprintf(sortie, "%s;%f;%f;%f\n", noeud->id, d->capacite, d->vol_source, d->vol_traite);
    }
    // Gestion des cas classiques avec filtre > 0
    else if (valeur > 0.0) {
        fprintf(sortie, "%s;%f\n", noeud->id, valeur);
    }

    parcoursInverse(noeud->gauche, sortie, mode);
}

/* Fonction principale du module : lit le fichier CSV ligne par ligne pour remplir l'AVL avec les données des usines.
   Gère deux cas : la définition de la capacité de l'usine et l'accumulation des volumes (source -> usine) en appliquant les fuites.
   Génère ensuite le fichier de sortie spécifique demandé. */
int traiter_histo(FILE *fichier, char *type) {
    char ligne[1024];
    avl *racine = NULL;

    while (fgets(ligne, 1024, fichier)) {
        ligne[strcspn(ligne, "\r\n")] = 0;

        char *col1 = strtok(ligne, ";");
        char *col2 = strtok(NULL, ";");
        char *col3 = strtok(NULL, ";");
        char *col4 = strtok(NULL, ";");
        char *col5 = strtok(NULL, ";");

        if (!col2 || !col3 || !col4) continue;

        // CAS 1 : Définition de l'usine (Ligne type : -;Usine;-;Capacité;-)
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0) {
            if (strcmp(col4, "-") != 0) {
                avl *n = rechercherAVL(racine, col2);
                if (!n) {
                    DonneesHisto *data = calloc(1, sizeof(DonneesHisto));
                    racine = insererAVL(racine, col2, data);
                    n = rechercherAVL(racine, col2);
                }
                DonneesHisto *d = (DonneesHisto*)n->donnee;
                d->capacite = atof(col4);
            }
        }
        // CAS 2 : Lien Source -> Usine (Ligne type : -;Source;Usine;Volume;Fuite)
        // CORRECTION ICI : On ajoute la vérification que col4 n'est PAS un tiret
        // Cela permet d'exclure les lignes Usine -> Stockage (qui ont col4 == "-")
        else if (strcmp(col1, "-") == 0 && strcmp(col3, "-") != 0 && strcmp(col4, "-") != 0) {
            avl *n = rechercherAVL(racine, col3);
            if (!n) {
                DonneesHisto *data = calloc(1, sizeof(DonneesHisto));
                racine = insererAVL(racine, col3, data);
                n = rechercherAVL(racine, col3);
            }
            DonneesHisto *d = (DonneesHisto*)n->donnee;
            double vol = atof(col4);
            double fuite = (col5 && strcmp(col5, "-") != 0) ? atof(col5) : 0.0;
            
            d->vol_source += vol;
            d->vol_traite += vol * (1.0 - fuite/100.0);
        }
    }

    char nomFichier[64];
    if (strcmp(type, "all") == 0) sprintf(nomFichier, "histo_all.dat");
    else sprintf(nomFichier, "histo_%s.dat", type);

    FILE *sortie = fopen(nomFichier, "w");
    
    if (sortie) {
        // En-têtes adaptés
        if (strcmp(type, "all") == 0) fprintf(sortie, "identifier;max volume;source volume;real volume\n");
        else fprintf(sortie, "identifier;valeur (M.m3)\n");

        parcoursInverse(racine, sortie, type);
        fclose(sortie);
    }

    libererAVL(racine);
    return 0;
}
