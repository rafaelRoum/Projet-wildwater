#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int max(int a, int b) {
    return (a > b) ? a : b;
}

int hauteur(avl *n) {
    if (n == NULL) return 0;
    return n->hauteur;
}

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

avl* rotationDroite(avl *y) {
    avl *x = y->gauche;
    avl *T2 = x->droite;
    x->droite = y;
    y->gauche = T2;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    return x;
}

avl* rotationGauche(avl *x) {
    avl *y = x->droite;
    avl *T2 = y->gauche;
    y->gauche = x;
    x->droite = T2;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    return y;
}

int obtenirEquilibre(avl *n) {
    if (n == NULL) return 0;
    return hauteur(n->gauche) - hauteur(n->droite);
}

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

avl* rechercherAVL(avl *noeud, char *id) {
    if (noeud == NULL) return NULL;
    int cmp = strcmp(id, noeud->id);
    if (cmp == 0) return noeud;
    if (cmp < 0) return rechercherAVL(noeud->gauche, id);
    return rechercherAVL(noeud->droite, id);
}

void libererAVL(avl *noeud) {
    if (noeud != NULL) {
        libererAVL(noeud->gauche);
        libererAVL(noeud->droite);
        free(noeud->id);
        free(noeud->donnee);
        free(noeud);
    }
}