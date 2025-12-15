#define _DEFAULT_SOURCE // Nécessaire pour certaines fonctions comme strdup ou strsep sous Linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histo.h"
#include "avl.h"

// --- FONCTION DE PARCOURS ET D'ÉCRITURE ---

/**
 * Parcourt l'AVL et écrit les résultats dans le fichier de sortie.
 * * IMPORTANT : On utilise un parcours "Infixe Inversé" (Droite -> Racine -> Gauche).
 * - L'AVL standard trie de A à Z (Gauche < Racine < Droite).
 * - En parcourant d'abord la Droite, on obtient un tri de Z à A (Décroissant),
 * ce qui est demandé par le sujet pour les graphiques.
 */
void parcoursInverse(avl *noeud, FILE *sortie, char *mode) {
    if (noeud == NULL) return;

    // 1. D'abord le sous-arbre Droit (les plus grands IDs alphabétiquement)
    parcoursInverse(noeud->droite, sortie, mode);

    // 2. Traitement du nœud courant
    DonneesHisto *d = (DonneesHisto*)noeud->donnee;
    
    // CONVERSION D'UNITÉS :
    // - Entrée (CSV) : milliers de m3 (k m3)
    // - Sortie (Graphiques) : Millions de m3 (M m3)
    // -> On divise par 1000.0
    double cap_m = d->capacite / 1000.0;
    double src_m = d->vol_source / 1000.0;
    double real_m = d->vol_traite / 1000.0;
    
    double valeur = 0.0;

    // Sélection de la valeur à afficher selon le mode choisi
    if (strcmp(mode, "max") == 0) valeur = cap_m;
    else if (strcmp(mode, "src") == 0) valeur = src_m;
    else if (strcmp(mode, "real") == 0) valeur = real_m;

    // Gestion du mode "all" (Bonus / Intermédiaire)
    // Ce mode génère "factories.dat" qui sera lu par le module Leaks.
    // On garde toutes les infos pour pouvoir calculer les fuites plus tard.
    if (strcmp(mode, "all") == 0) {
        // Format : ID ; Capacité ; VolSource ; VolReel
        fprintf(sortie, "%s;%f;%f;%f\n", noeud->id, d->capacite, d->vol_source, d->vol_traite);
    }
    // Gestion des modes graphiques standards
    // On n'écrit pas les lignes avec une valeur de 0 pour ne pas surcharger le graphe
    else if (valeur > 0.0) {
        fprintf(sortie, "%s;%f\n", noeud->id, valeur);
    }

    // 3. Enfin le sous-arbre Gauche (les plus petits IDs)
    parcoursInverse(noeud->gauche, sortie, mode);
}

// --- FONCTION PRINCIPALE DU MODULE ---

/**
 * Lit le fichier CSV, remplit l'AVL et génère le fichier de stats.
 * Gère le parsing complexe du CSV avec des champs vides.
 */
int traiter_histo(FILE *fichier, char *type) {
    char ligne[2048]; // Buffer large pour éviter les coupures de lignes longues
    avl *racine = NULL;

    // Sécurité : On s'assure de lire le fichier depuis le début
    rewind(fichier); 

    while (fgets(ligne, sizeof(ligne), fichier)) {
        ligne[strcspn(ligne, "\r\n")] = 0; // Suppression du saut de ligne final

        // PARSING ROBUSTE :
        // On utilise `strsep` au lieu de `strtok`.
        // Pourquoi ? Car le CSV contient des champs vides (ex: ";;").
        // `strtok` les ignorerait et décalerait les colonnes. `strsep` gère ça correctement.
        char *ptr = ligne;
        char *col1 = strsep(&ptr, ";"); // Identifiant Station (ou -)
        char *col2 = strsep(&ptr, ";"); // Amont (Source ou Factory ou -)
        char *col3 = strsep(&ptr, ";"); // Aval (Factory ou -)
        char *col4 = strsep(&ptr, ";"); // Volume / Capacité
        char *col5 = strsep(&ptr, ";"); // Fuite éventuelle

        // Si la ligne est mal formée (manque de colonnes), on la saute
        if (!col2 || !col3 || !col4) continue;

        // --- CAS 1 : Définition d'une usine ---
        // Format : - ; ID_Usine ; - ; Capacité ; -
        // On repère ce cas car col1 et col3 sont vides ("-").
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0) {
            if (strcmp(col4, "-") != 0) {
                // Recherche ou création du nœud dans l'AVL
                avl *n = rechercherAVL(racine, col2);
                if (!n) {
                    // calloc initialise la mémoire à 0 (donc volumes = 0 par défaut)
                    DonneesHisto *data = calloc(1, sizeof(DonneesHisto)); 
                    racine = insererAVL(racine, col2, data);
                    n = rechercherAVL(racine, col2);
                }
                // Mise à jour de la capacité
                DonneesHisto *d = (DonneesHisto*)n->donnee;
                d->capacite = atof(col4);
            }
        }
        // --- CAS 2 : Connexion Source -> Usine ---
        // Format : - ; Source ; ID_Usine ; Volume ; Fuite
        // On repère ce cas car col3 (Aval) n'est PAS vide. C'est l'usine qui reçoit l'eau.
        else if (strcmp(col1, "-") == 0 && strcmp(col3, "-") != 0) {
            
            // On s'intéresse à l'usine réceptrice (col3)
            avl *n = rechercherAVL(racine, col3);
            if (!n) {
                DonneesHisto *data = calloc(1, sizeof(DonneesHisto));
                racine = insererAVL(racine, col3, data);
                n = rechercherAVL(racine, col3);
            }
            DonneesHisto *d = (DonneesHisto*)n->donnee;
            
            double vol = atof(col4);
            // Si col5 (fuite) existe et n'est pas "-", on la convertit, sinon 0
            double fuite = (col5 && strcmp(col5, "-") != 0) ? atof(col5) : 0.0;
            
            // Mise à jour des compteurs
            d->vol_source += vol; // Volume brut
            // Volume réel = Volume brut * (1 - pourcentage_perte)
            d->vol_traite += vol * (1.0 - fuite/100.0);
        }
    }

    // --- GÉNÉRATION DU FICHIER DE SORTIE ---
    
    char nomFichier[64];
    
    // Si mode "all", on crée "factories.dat".
    // C'est CRUCIAL car le module 'Leaks' cherchera ce fichier précis pour connaître les volumes initiaux.
    if (strcmp(type, "all") == 0) {
        sprintf(nomFichier, "factories.dat"); 
    } else {
        // Sinon, on crée un fichier temporaire pour gnuplot (ex: histo_src.dat)
        sprintf(nomFichier, "histo_%s.dat", type);
    }

    FILE *sortie = fopen(nomFichier, "w");
    if (sortie) {
        // On lance l'écriture récursive (triée Z->A)
        parcoursInverse(racine, sortie, type);
        fclose(sortie);
        printf("Succès : Fichier '%s' généré.\n", nomFichier);
    } else {
        perror("Erreur critique : Impossible de créer le fichier de sortie");
    }

    // Nettoyage de la mémoire avant de quitter
    libererAVL(racine);
    return 0;
}
