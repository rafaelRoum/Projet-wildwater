#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuites.h"
#include "avl.h"

/* Alloue la mémoire pour un nouveau nœud du graphe, copie l'identifiant et initialise les pointeurs à NULL et le taux de fuite à 0. */
NoeudGraphe* creerNoeudGraphe(char *id) {
    NoeudGraphe *n = malloc(sizeof(NoeudGraphe));
    n->id = strdup(id);
    n->fuite_pct = 0.0;
    n->premier_fils = NULL;
    n->frere_suivant = NULL;
    return n;
}

/* Calcule récursivement le volume total d'eau perdu à partir d'un nœud donné. Calcule la perte locale, divise le volume restant entre les enfants et additionne toutes les pertes. */
double calculer_pertes(NoeudGraphe *noeud, double volume_entree) {
    if (noeud == NULL) return 0.0;

    double volume_perdu_ici = volume_entree * (noeud->fuite_pct / 100.0);
    double volume_restant = volume_entree - volume_perdu_ici;

    int nb_enfants = 0;
    NoeudGraphe *temp = noeud->premier_fils;
    while (temp) {
        nb_enfants++;
        temp = temp->frere_suivant;
    }

    double pertes_totales = volume_perdu_ici;
    if (nb_enfants > 0) {
        double vol_par_enfant = volume_restant / nb_enfants;
        temp = noeud->premier_fils;
        while (temp) {
            pertes_totales += calculer_pertes(temp, vol_par_enfant);
            temp = temp->frere_suivant;
        }
    }
    return pertes_totales;
}

/* Fonction principale qui lit le fichier de données, construit le graphe du réseau via un arbre AVL pour l'indexation, puis calcule et écrit les pertes pour l'usine spécifiée dans le fichier de sortie. */
int traiter_fuites(FILE *fichier, char *id_usine) {
    char ligne[1024];
    avl *index = NULL;


    while (fgets(ligne, 1024, fichier)) {
        ligne[strcspn(ligne, "\r\n")] = 0;

        char *col1 = strtok(ligne, ";");
        char *col2 = strtok(NULL, ";");
        char *col3 = strtok(NULL, ";");
        char *col4 = strtok(NULL, ";");
        char *col5 = strtok(NULL, ";");

        if (!col2 || !col3) continue;
        if (strcmp(col3, "-") == 0) continue;

        avl *noeudParent = rechercherAVL(index, col2);
        if (!noeudParent) {
            NoeudGraphe *g = creerNoeudGraphe(col2);
            index = insererAVL(index, col2, g);
            noeudParent = rechercherAVL(index, col2);
        }

        avl *noeudEnfant = rechercherAVL(index, col3);
        if (!noeudEnfant) {
            NoeudGraphe *g = creerNoeudGraphe(col3);
            index = insererAVL(index, col3, g);
            noeudEnfant = rechercherAVL(index, col3);
        }

        NoeudGraphe *p = (NoeudGraphe*)noeudParent->donnee;
        NoeudGraphe *e = (NoeudGraphe*)noeudEnfant->donnee;

        if (col5 && strcmp(col5, "-") != 0) {
            e->fuite_pct = atof(col5);
        }

        e->frere_suivant = p->premier_fils;
        p->premier_fils = e;
    }


    FILE *sortie = fopen("fuites_output.dat", "w");
    if (!sortie) {
        libererAVL(index);
        return 1;
    }

    fprintf(sortie, "Identifiant;Volume perdu (M.m3/an)\n");

    avl *depart = rechercherAVL(index, id_usine);

    if (!depart) {
        // CAS : USINE INTROUVABLE 
        fprintf(sortie, "%s;-1\n", id_usine);
        printf("Usine introuvable : %s\n", id_usine);
    } 
    else {
        // CAS : USINE TROUVEE 
        double pertes = calculer_pertes((NoeudGraphe*)depart->donnee, 1000000.0); 
        fprintf(sortie, "%s;%f\n", id_usine, pertes);
    }

    fclose(sortie);
    libererAVL(index);
    return 0;
}
