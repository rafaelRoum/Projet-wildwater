#ifndef AVL_H
#define AVL_H

typedef struct avl {
    char* id;
    int hauteur;
    struct avl* gauche;
    struct avl* droite;
    void* donnee;
} avl;

avl* creerAVL(char *id, void *donnee);
avl* insererAVL(avl *noeud, char *id, void *donnee);
avl* rechercherAVL(avl *noeud, char *id);
void libererAVL(avl *noeud);
int hauteur(avl *n);
int max(int a, int b);

#endif