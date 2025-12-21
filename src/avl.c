#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

/* Retourne le plus grand des deux entiers fournis. */
int max(int a, int b) {
    return (a > b) ? a : b;
}

/* Renvoie la hauteur du nœud. Si le nœud est NULL (arbre vide), retourne 0. */
int hauteur(avl *n) {
    if (n == NULL) return 0;
    return n->hauteur;
}

/* Crée un nouveau nœud AVL, alloue la mémoire, copie l'identifiant et initialise les pointeurs. */
avl* creerAVL(char *id, void *donnee) {
    avl *n = malloc(sizeof(avl));
    if (n == NULL) exit(1);
    n->id = strdup(id);
    n->hauteur = 1;
    n->gauche = NULL;
    n->droite = NULL;
    n->donnee = donnee;
    return n;
}

/* Effectue une rotation simple vers la droite pour rééquilibrer l'arbre. Met à jour les hauteurs des nœuds concernés. */
avl* rotationDroite(avl *y) {
    avl *x = y->gauche;
    avl *T2 = x->droite;
    x->droite = y;
    y->gauche = T2;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    return x;
}

/* Effectue une rotation simple vers la gauche pour rééquilibrer l'arbre. Met à jour les hauteurs des nœuds concernés. */
avl* rotationGauche(avl *x) {
    avl *y = x->droite;
    avl *T2 = y->gauche;
    y->gauche = x;
    x->droite = T2;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    return y;
}

/* Calcule le facteur d'équilibre d'un nœud (différence entre la hauteur gauche et la hauteur droite). */
int obtenirEquilibre(avl *n) {
    if (n == NULL) return 0;
    return hauteur(n->gauche) - hauteur(n->droite);
}

/* Insère un élément dans l'arbre AVL, met à jour la hauteur et effectue les rotations nécessaires pour garder l'arbre équilibré. */
avl* insererAVL(avl *noeud, char *id, void *donnee) {
    if (noeud == NULL) {
        return creerAVL(id, donnee);
    }

    int cmp = strcmp(id, noeud->id);
    if (cmp < 0)
        noeud->gauche = insererAVL(noeud->gauche, id, donnee);
    else if (cmp > 0)
        noeud->droite = insererAVL(noeud->droite, id, donnee);
    else {
        return noeud;
    }

    noeud->hauteur = 1 + max(hauteur(noeud->gauche), hauteur(noeud->droite));
    int balance = obtenirEquilibre(noeud);

    if (balance > 1 && strcmp(id, noeud->gauche->id) < 0)
        return rotationDroite(noeud);

    if (balance < -1 && strcmp(id, noeud->droite->id) > 0)
        return rotationGauche(noeud);

    if (balance > 1 && strcmp(id, noeud->gauche->id) > 0) {
        noeud->gauche = rotationGauche(noeud->gauche);
        return rotationDroite(noeud);
    }

    if (balance < -1 && strcmp(id, noeud->droite->id) < 0) {
        noeud->droite = rotationDroite(noeud->droite);
        return rotationGauche(noeud);
    }

    return noeud;
}

/* Recherche récursivement un nœud dans l'arbre par son identifiant (id). Retourne NULL si non trouvé. */
avl* rechercherAVL(avl *noeud, char *id) {
    if (noeud == NULL) return NULL;
    int cmp = strcmp(id, noeud->id);
    if (cmp == 0) return noeud;
    if (cmp < 0) return rechercherAVL(noeud->gauche, id);
    return rechercherAVL(noeud->droite, id);
}

/* Libère récursivement la mémoire de tout l'arbre (nœuds, identifiants et données). */
void libererAVL(avl *noeud) {
    if (noeud != NULL) {
        libererAVL(noeud->gauche);
        libererAVL(noeud->droite);
        free(noeud->id);
        free(noeud->donnee);
        free(noeud);
    }
}
