#ifndef AVL_H
#define AVL_H

/* Structure représentant un nœud de l'arbre AVL.
Le pointeur générique donnee contiendra les données pour les fuites ou pour les histogrammes*/
typedef struct avl {
    char* id;
    int hauteur;
    struct avl* gauche;
    struct avl* droite;
    void* donnee;
} avl;

/* Alloue et initialise un nouveau nœud AVL avec un identifiant et une donnée. */
avl* creerAVL(char *id, void *donnee);

/* Insère un nouvel élément dans l'arbre AVL en maintenant l'équilibre (rotations si nécessaire). */
avl* insererAVL(avl *noeud, char *id, void *donnee);

/* Recherche un nœud spécifique dans l'arbre grâce à son identifiant. */
avl* rechercherAVL(avl *noeud, char *id);

/* Libère la mémoire allouée pour l'arbre complet (nœuds, clés et données). */
void libererAVL(avl *noeud);

/* Retourne la hauteur d'un nœud donné (0 si le nœud est NULL). */
int hauteur(avl *n);

/* Fonction utilitaire retournant le maximum entre deux entiers. */
int max(int a, int b);

#endif
