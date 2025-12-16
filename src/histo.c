#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // Pour isspace
#include <string.h>
#include "histo.h"
#include "avl.h"

// --- FONCTION UTILITAIRE : TRIM ---
// Enlève les espaces au début et à la fin d'une chaîne.
// Indispensable car " Usine A" != "Usine A"
char *trim(char *str) {
    char *end;

    // Trim début
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return str; // Chaîne vide

    // Trim fin
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Terminateur null
    end[1] = '\0';

    return str;
}

// --- PARCOURS (Identique, avec le filtre Factory pour 'all') ---
void parcoursInverse(avl *noeud, FILE *sortie, char *mode) {
    if (noeud == NULL) return;

    parcoursInverse(noeud->droite, sortie, mode);

    DonneesHisto *d = (DonneesHisto*)noeud->donnee;
    
    // k m3 -> M m3
    double cap_m = d->capacite / 1000.0;
    double src_m = d->vol_source / 1000.0;
    double real_m = d->vol_traite / 1000.0;
    
    double valeur = 0.0;
    if (strcmp(mode, "max") == 0) valeur = cap_m;
    else if (strcmp(mode, "src") == 0) valeur = src_m;
    else if (strcmp(mode, "real") == 0) valeur = real_m;

    if (strcmp(mode, "all") == 0) {
        // Filtre un peu plus large pour ne rien rater dans factories.dat
        // Si tu veux TOUT garder (même les jonctions), enlève le if(est_usine).
        int est_usine = 0;
        if (strstr(noeud->id, "Factory") || strstr(noeud->id, "Facility") || 
            strstr(noeud->id, "Plant") || strstr(noeud->id, "Station")) {
            est_usine = 1;
        }
        
        // Pour le debug, si tu penses qu'il en manque, commente le if(est_usine)
        if (est_usine) {
            fprintf(sortie, "%s;%f;%f;%f\n", noeud->id, d->capacite, d->vol_source, d->vol_traite);
        }
    }
    // Pour les graphiques, on accepte tout ce qui a une valeur > 0
    else if (valeur > 0.0) {
        fprintf(sortie, "%s;%f\n", noeud->id, valeur);
    }

    parcoursInverse(noeud->gauche, sortie, mode);
}

// --- TRAITEMENT PRINCIPAL ---
int traiter_histo(FILE *fichier, char *type) {
    char ligne[4096]; // Buffer doublé par sécurité
    avl *racine = NULL;

    rewind(fichier); 

    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Nettoyage fin de ligne
        ligne[strcspn(ligne, "\r\n")] = 0; 
        
        // Copie pour parsing
        char *ptr = ligne;
        
        // Récupération des colonnes avec TRIM immédiat
        char *raw_col1 = strsep(&ptr, ";");
        char *raw_col2 = strsep(&ptr, ";");
        char *raw_col3 = strsep(&ptr, ";");
        char *raw_col4 = strsep(&ptr, ";");
        char *raw_col5 = strsep(&ptr, ";");

        if (!raw_col2 || !raw_col3 || !raw_col4) continue;

        char *col1 = trim(raw_col1);
        char *col2 = trim(raw_col2);
        char *col3 = trim(raw_col3);
        char *col4 = trim(raw_col4);
        // col5 peut être null si fin de ligne
        char *col5 = raw_col5 ? trim(raw_col5) : NULL;

        // ---------------------------------------------------------
        // LOGIQUE ÉLARGIE POUR NE RIEN RATER
        // ---------------------------------------------------------

        // 1. Si la colonne 1 contient un ID (Usine émettrice), on l'enregistre.
        // Même si on n'a pas son volume source ici, elle DOIT exister dans l'arbre.
        if (strcmp(col1, "-") != 0 && strlen(col1) > 0) {
             avl *n = rechercherAVL(racine, col1);
             if (!n) {
                 DonneesHisto *data = calloc(1, sizeof(DonneesHisto));
                 racine = insererAVL(racine, col1, data);
             }
        }

        // 2. Définition Usine (via Col 2)
        // Format : - ; ID_Usine ; - ; Cap ; -
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0 && strcmp(col2, "-") != 0) {
            if (strcmp(col4, "-") != 0) {
                avl *n = rechercherAVL(racine, col2);
                if (!n) {
                    DonneesHisto *data = calloc(1, sizeof(DonneesHisto)); 
                    racine = insererAVL(racine, col2, data);
                    n = rechercherAVL(racine, col2);
                }
                DonneesHisto *d = (DonneesHisto*)n->donnee;
                // Si capacité déjà définie, on ne l'écrase pas par 0 (sécurité)
                double cap = atof(col4);
                if (cap > 0) d->capacite = cap;
            }
        }

        // 3. Source -> Usine (via Col 3)
        // Format : - ; Source ; ID_Usine ; Vol ; Leak
        else if (strcmp(col1, "-") == 0 && strcmp(col3, "-") != 0) {
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

    // --- ÉCRITURE SORTIE ---
    char nomFichier[64];
    if (strcmp(type, "all") == 0) sprintf(nomFichier, "factories.dat"); 
    else sprintf(nomFichier, "histo_%s.dat", type);

    FILE *sortie = fopen(nomFichier, "w");
    if (sortie) {
        parcoursInverse(racine, sortie, type);
        fclose(sortie);
    } else {
        perror("Erreur création fichier");
    }

    libererAVL(racine);
    return 0;
}
