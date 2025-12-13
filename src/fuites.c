#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuites.h"
#include "avl.h"

NoeudGraphe* creerNoeudGraphe(char *id) {
    NoeudGraphe *n = malloc(sizeof(NoeudGraphe));
    n->id = strdup(id);
    n->fuite_pct = 0.0;
    n->premier_fils = NULL;
    n->frere_suivant = NULL;
    return n;
}

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

int traiter_fuites(FILE *fichier, char *id_usine) {
    char ligne[1024];
    avl *index = NULL;

    // 1. PHASE DE CHARGEMENT (C'est ça qui est long, et c'est inévitable)
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

    // 2. VERIFICATION ET ECRITURE DU RESULTAT
    FILE *sortie = fopen("fuites_output.dat", "w");
    if (!sortie) {
        libererAVL(index);
        return 1;
    }

    // On écrit l'en-tête standard
    fprintf(sortie, "Identifiant;Volume perdu (M.m3/an)\n");

    avl *depart = rechercherAVL(index, id_usine);

    if (!depart) {
        // CAS : USINE INTROUVABLE -> ON ECRIT -1 ET ON S'ARRETE LA
        // Pas de calcul, on sort direct.
        fprintf(sortie, "%s;-1\n", id_usine);
        printf("Usine introuvable : %s\n", id_usine);
    } 
    else {
        // CAS : USINE TROUVEE -> ON CALCULE
        // Note: Le sujet ne précise pas le volume d'entrée initial, 
        // on garde 1 million ou la valeur par défaut pour les tests.
        double pertes = calculer_pertes((NoeudGraphe*)depart->donnee, 1000000.0); // Base arbitraire pour tester %
        fprintf(sortie, "%s;%f\n", id_usine, pertes);
    }

    fclose(sortie);
    libererAVL(index);
    return 0;
}