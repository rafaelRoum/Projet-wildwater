#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histo.h"
#include "avl.h"

// Fonction récursive pour écrire les données triées (Inverse = Z -> A)
void parcoursInverse(avl *noeud, FILE *sortie, char *mode) {
    if (noeud == NULL) return;

    // 1. Sous-arbre Droit (plus grand)
    parcoursInverse(noeud->droite, sortie, mode);

    DonneesHisto *d = (DonneesHisto*)noeud->donnee;
    
    // CORRECTION : Les données d'entrée sont en milliers (k m3)
    // La sortie doit être en Millions (M m3). On divise par 1000.
    double cap_m = d->capacite / 1000.0;
    double src_m = d->vol_source / 1000.0;
    double real_m = d->vol_traite / 1000.0;
    double valeur = 0.0;

    if (strcmp(mode, "max") == 0) valeur = cap_m;
    else if (strcmp(mode, "src") == 0) valeur = src_m;
    else if (strcmp(mode, "real") == 0) valeur = real_m;

    // Gestion du cas "all" (Utilisé pour générer factories.dat pour Leaks)
    if (strcmp(mode, "all") == 0) {
        // On garde la précision max pour ce fichier intermédiaire
        fprintf(sortie, "%s;%f;%f;%f\n", noeud->id, d->capacite, d->vol_source, d->vol_traite);
    }
    // Gestion des graphiques (on ignore les 0)
    else if (valeur > 0.0) {
        fprintf(sortie, "%s;%f\n", noeud->id, valeur);
    }

    // 2. Sous-arbre Gauche (plus petit)
    parcoursInverse(noeud->gauche, sortie, mode);
}

int traiter_histo(FILE *fichier, char *type) {
    char ligne[2048]; // Augmenté par sécurité
    avl *racine = NULL;

    rewind(fichier); // On s'assure d'être au début du fichier

    while (fgets(ligne, sizeof(ligne), fichier)) {
        ligne[strcspn(ligne, "\r\n")] = 0; // Nettoyage fin de ligne

        // Utilisation de strsep (plus sûr que strtok pour les champs vides)
        char *ptr = ligne;
        char *col1 = strsep(&ptr, ";"); // ID Station (ou -)
        char *col2 = strsep(&ptr, ";"); // Amont (Source ou Factory)
        char *col3 = strsep(&ptr, ";"); // Aval (Factory ou -)
        char *col4 = strsep(&ptr, ";"); // Volume / Capacité
        char *col5 = strsep(&ptr, ";"); // Fuite

        if (!col2 || !col3 || !col4) continue;

        // CAS 1 : Définition d'usine ( - ; Factory ; - ; Capacité ; - )
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0) {
            if (strcmp(col4, "-") != 0) {
                // On cherche ou on crée le nœud
                avl *n = rechercherAVL(racine, col2);
                if (!n) {
                    DonneesHisto *data = calloc(1, sizeof(DonneesHisto)); // calloc met tout à 0
                    racine = insererAVL(racine, col2, data);
                    n = rechercherAVL(racine, col2);
                }
                DonneesHisto *d = (DonneesHisto*)n->donnee;
                d->capacite = atof(col4);
            }
        }
        // CAS 2 : Source vers Usine ( - ; Source ; Factory ; Vol ; Leak )
        else if (strcmp(col1, "-") == 0 && strcmp(col3, "-") != 0) {
            // Ici, l'élément important est l'aval (col3 = l'usine)
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
            // Formule : Volume entrant * (1 - %fuite/100)
            d->vol_traite += vol * (1.0 - fuite/100.0);
        }
    }

    // Détermination du nom de fichier
    char nomFichier[64];
    if (strcmp(type, "all") == 0) {
        // CORRECTION IMPORTANTE : Leaks cherche "factories.dat"
        sprintf(nomFichier, "factories.dat"); 
    } else {
        sprintf(nomFichier, "histo_%s.dat", type);
    }

    FILE *sortie = fopen(nomFichier, "w");
    if (sortie) {
        // En-tête (Optionnel mais propre)
        // if (strcmp(type, "all") != 0) fprintf(sortie, "identifier;valeur\n");
        
        parcoursInverse(racine, sortie, type);
        fclose(sortie);
        printf("Fichier généré : %s\n", nomFichier);
    } else {
        perror("Erreur création fichier");
    }

    libererAVL(racine);
    return 0;
}
