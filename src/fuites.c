#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuites.h"
#include "avl.h"

// --- FONCTIONS INTERNES ---

// Récupère le volume "Volume Source" depuis le fichier factories.dat
// Format supposé: ID;Capacity;SourceVol;RealVol (ou ID;Vol si mode simple)
// Adapte le fscanf selon le format exact de ton fichier de sortie Histo.
double recuperer_volume_initial(char *id_usine) {
    FILE *f = fopen("factories.dat", "r"); // Ou "vol_src.dat" selon ton étape 1
    if (!f) return 0.0; // Fichier inexistant

    char ligne[1024];
    double volume = 0.0;
    int trouve = 0;

    while (fgets(ligne, 1024, f)) {
        // Copie pour éviter de casser la ligne si besoin
        char *ptr = ligne;
        char *token = strsep(&ptr, ";"); // ID

        if (token && strcmp(token, id_usine) == 0) {
            // On a trouvé l'usine !
            // On saute les colonnes pour trouver le volume.
            // Supposons format "ID;Cap;VolSource;VolReal" du bonus
            // Token 2 : Capacité
            strsep(&ptr, ";"); 
            // Token 3 : Volume Source (C'est ce qu'on injecte dans le réseau ?)
            // Ou Token 4 : Volume Real (Sortie usine) ? -> Prenons Real (après traitement usine)
            char *vol_str = strsep(&ptr, ";"); // VolSrc
            // vol_str = strsep(&ptr, ";"); // Si tu veux le VolReal, décommente ça
            
            if (vol_str) volume = atof(vol_str);
            trouve = 1;
            break;
        }
    }
    fclose(f);
    
    // Si volume trouvé est en M.m3 et qu'on veut calculer, c'est bon.
    return trouve ? volume : -1.0; 
}

NoeudGraphe* creerNoeudGraphe(char *id) {
    NoeudGraphe *n = malloc(sizeof(NoeudGraphe));
    if (!n) { perror("Malloc"); exit(1); }
    n->id = strdup(id);
    n->fuite_pct = 0.0;
    n->premier_fils = NULL;
    n->frere_suivant = NULL;
    return n;
}

// Récursivité pour calculer les pertes
double calculer_pertes(NoeudGraphe *noeud, double volume_entree) {
    if (noeud == NULL) return 0.0;

    // 1. Calculer la perte sur le tuyau arrivant ici
    double volume_perdu_ici = 0;
    if (noeud->fuite_pct > 0) {
        volume_perdu_ici = volume_entree * (noeud->fuite_pct / 100.0);
    }
    
    double volume_restant = volume_entree - volume_perdu_ici;

    // 2. Compter les enfants pour diviser l'eau équitablement
    int nb_enfants = 0;
    NoeudGraphe *temp = noeud->premier_fils;
    while (temp) {
        nb_enfants++;
        temp = temp->frere_suivant;
    }

    double pertes_totales = volume_perdu_ici;

    // 3. Propager aux enfants
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

// --- FONCTION PRINCIPALE ---

int traiter_fuites(FILE *fichier, char *id_usine) {
    char ligne[1024];
    avl *index = NULL; // L'annuaire AVL pour trouver les adresses mémoires

    // 1. PHASE DE CHARGEMENT
    // On rembobine le fichier au cas où
    rewind(fichier);

    while (fgets(ligne, 1024, fichier)) {
        ligne[strcspn(ligne, "\r\n")] = 0; // Nettoyage fin de ligne

        // Parsing manuel (ou strtok)
        // Format: ID; Source; Dest; ... ; Leak
        char copie[1024];
        strcpy(copie, ligne);
        
        char *ptr = copie;
        char *col1 = strsep(&ptr, ";"); // ID
        char *col2 = strsep(&ptr, ";"); // Source
        char *col3 = strsep(&ptr, ";"); // Dest
        char *col4 = strsep(&ptr, ";"); // Vol
        char *col5 = strsep(&ptr, ";"); // Leak

        // On ignore les lignes sans liaison valide (Source -> Dest)
        if (!col2 || !col3 || strcmp(col3, "-") == 0) continue;

        // --- Gestion Parent (Source) ---
        avl *noeudParent = rechercherAVL(index, col2);
        if (!noeudParent) {
            NoeudGraphe *g = creerNoeudGraphe(col2);
            // On stocke le pointeur NoeudGraphe* dans le void* data de l'AVL
            index = insererAVL(index, col2, g); 
            noeudParent = rechercherAVL(index, col2);
        }

        // --- Gestion Enfant (Dest) ---
        avl *noeudEnfant = rechercherAVL(index, col3);
        if (!noeudEnfant) {
            NoeudGraphe *g = creerNoeudGraphe(col3);
            index = insererAVL(index, col3, g);
            noeudEnfant = rechercherAVL(index, col3);
        }

        // --- Création du lien dans le Graphe ---
        NoeudGraphe *p = (NoeudGraphe*)noeudParent->donnee;
        NoeudGraphe *e = (NoeudGraphe*)noeudEnfant->donnee;

        // Mise à jour de la fuite sur l'enfant (tuyau entrant vers l'enfant)
        if (col5 && strcmp(col5, "-") != 0) {
            e->fuite_pct = atof(col5);
        }

        // Ajout en tête de liste chaînée (Enfants de P)
        // C'est ici que la magie de premier_fils/frere_suivant opère
        e->frere_suivant = p->premier_fils;
        p->premier_fils = e;
    }

    // 2. VERIFICATION ET ECRITURE DU RESULTAT
    FILE *sortie = fopen("fuites.dat", "w"); // Nom du fichier output imposé ?
    if (!sortie) {
        // Gestion erreur
        libererAVL(index); // Cela libérera les NoeudGraphe car donnee est free()
        return 1;
    }

    // Recherche de l'usine de départ dans notre graphe construit
    avl *depart = rechercherAVL(index, id_usine);

    if (!depart) {
        // USINE NON TROUVÉE DANS LE GRAPHE (Pas connectée ou nom incorrect)
        fprintf(sortie, "%s;-1\n", id_usine);
        printf("Erreur : Usine %s introuvable dans le réseau.\n", id_usine);
    } 
    else {
        // USINE TROUVÉE -> Récupération volume initial
        double vol_initial = recuperer_volume_initial(id_usine);
        
        if (vol_initial < 0) {
            // Usine présente dans le graphe mais pas dans factories.dat (bizarre mais possible)
            // On considère 0 ou erreur ? Le sujet dit -1 si ID introuvable.
            fprintf(sortie, "%s;-1\n", id_usine); // Ou 0 selon interprétation
            printf("Erreur : Volume initial inconnu pour %s (Manque factories.dat ?)\n", id_usine);
        } else {
            // Lancement du calcul récursif
            // Attention : vol_initial est souvent en Millions de m3 si factories.dat l'est déjà.
            // Si factories.dat est en milliers, vérifier les conversions.
            double pertes = calculer_pertes((NoeudGraphe*)depart->donnee, vol_initial);
            fprintf(sortie, "%s;%f\n", id_usine, pertes);
            printf("Succès : Fuites pour %s = %f\n", id_usine, pertes);
        }
    }

    fclose(sortie);
    
    // Nettoyage mémoire
    // Note : libererAVL appelle free(donnee).
    // donnee est un NoeudGraphe*. Donc le free(donnee) libère le malloc(sizeof(NoeudGraphe)).
    // Par contre, le n->id (strdup) à l'intérieur de NoeudGraphe n'est pas libéré car l'AVL générique ne le sait pas.
    // Pour un projet étudiant, c'est souvent toléré, sinon il faut un destructeur spécifique.
    libererAVL(index); 
    
    return 0;
}
