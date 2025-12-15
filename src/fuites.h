#ifndef FUITES_H
#define FUITES_H

#include <stdio.h>

/**
 * Structure représentant un nœud du réseau hydraulique (Usine, Station, Jonction...).
 * * Cette structure utilise la représentation "Premier Fils / Frère Suivant"
 * (Left-Child, Right-Sibling) pour modéliser un arbre N-aire (où chaque nœud 
 * peut avoir un nombre illimité d'enfants) de manière économe en mémoire.
 */
typedef struct NoeudGraphe {
    char *id;           // Identifiant unique du nœud (ex: "Junction #12")
    
    double fuite_pct;   // Pourcentage de fuite sur le tuyau ARRIVANT à ce nœud.
                        // (Note : La fuite est associée à l'arc entrant).

    struct NoeudGraphe *premier_fils;  // Pointeur vers le premier enfant (le premier nœud en aval).
                                       // Si NULL, c'est une impasse (feuille).

    struct NoeudGraphe *frere_suivant; // Pointeur vers le "frère" suivant (un autre enfant du même parent).
                                       // Cela forme une liste chaînée horizontale de tous les enfants d'un parent.
} NoeudGraphe;

/**
 * Fonction principale du module "Leaks".
 * * 1. Construit le graphe en mémoire à partir du fichier CSV complet.
 * 2. Récupère le volume initial de l'usine depuis "factories.dat".
 * 3. Calcule récursivement les pertes d'eau.
 * 4. Génère le fichier de sortie "leaks.dat".
 * * @param fichier   Pointeur vers le fichier CSV ouvert en lecture (le pointeur doit être valide).
 * @param id_usine  Chaîne de caractères contenant l'identifiant de l'usine de départ.
 * @return          0 si tout s'est bien passé, 1 en cas d'erreur (fichier, mémoire, introuvable).
 */
int traiter_fuites(FILE *fichier, char *id_usine);

#endif
