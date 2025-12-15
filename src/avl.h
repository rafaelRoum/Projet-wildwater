#ifndef AVL_H
#define AVL_H

typedef struct avl {
    char* id;
    int hauteur;
    struct avl* gauche;
    struct avl* droite;
    void* donnee; // Pointeur vers ta structure InfoUsine
} avl;

// Fonctions de l'arbre (Génériques)
avl* creerAVL(char *id, void *donnee);
avl* insererAVL(avl *noeud, char *id, void *donnee);
avl* rechercherAVL(avl *noeud, char *id);
void libererAVL(avl *noeud);
int hauteur(avl *n);
int max(int a, int b);

// --- AJOUT OBLIGATOIRE POUR LE PROJET ---
// Cette fonction est celle appelée par le main.c pour lancer le travail.
// Elle doit être implémentée dans un fichier .c (ex: processing.c ou à la fin de avl.c)
int process_histogram(const char *filename, const char *mode);

#endif
