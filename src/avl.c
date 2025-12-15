#define _DEFAULT_SOURCE // Nécessaire pour utiliser strdup avec certaines versions de compilateur
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// --- FONCTIONS UTILITAIRES ---

/**
 * Retourne le maximum entre deux entiers.
 * Utilisé pour calculer la hauteur d'un nœud en fonction de ses enfants.
 */
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * Récupère la hauteur d'un nœud de manière sécurisée.
 * Si le nœud est NULL (inexistant), sa hauteur est 0.
 * Cela évite les erreurs de segmentation (crash) lors des calculs.
 */
int hauteur(avl *n) {
    if (n == NULL) return 0;
    return n->hauteur;
}

/**
 * Constructeur : Crée un nouveau nœud AVL.
 * @param id : L'identifiant unique (ex: nom de l'usine).
 * @param donnee : Pointeur vers la structure de données associée (volume, capacité...).
 */
avl* creerAVL(char *id, void *donnee) {
    avl *n = malloc(sizeof(avl));
    if (n == NULL) exit(1); // Arrêt d'urgence si plus de mémoire
    
    n->id = strdup(id); // Copie la chaîne ID (indispensable pour éviter les pointeurs morts)
    n->hauteur = 1;     // Un nouveau nœud est toujours une feuille (hauteur 1)
    n->gauche = NULL;
    n->droite = NULL;
    n->donnee = donnee; // Stocke les données spécifiques (Structure Usine par ex)
    return n;
}

// --- FONCTIONS DE ROTATION (Équilibrage) ---

/**
 * Rotation Droite (Right Rotate).
 * Utilisée quand l'arbre penche trop à GAUCHE (Left-Left Case).
 * Fait descendre le nœud 'y' vers la droite et remonte 'x'.
 */
avl* rotationDroite(avl *y) {
    avl *x = y->gauche;
    avl *T2 = x->droite;

    // La rotation
    x->droite = y;
    y->gauche = T2;

    // Mise à jour des hauteurs (y d'abord car il est devenu enfant de x)
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;

    return x; // x devient la nouvelle racine de ce sous-arbre
}

/**
 * Rotation Gauche (Left Rotate).
 * Utilisée quand l'arbre penche trop à DROITE (Right-Right Case).
 * Fait descendre le nœud 'x' vers la gauche et remonte 'y'.
 */
avl* rotationGauche(avl *x) {
    avl *y = x->droite;
    avl *T2 = y->gauche;

    // La rotation
    y->gauche = x;
    x->droite = T2;

    // Mise à jour des hauteurs
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;

    return y; // y devient la nouvelle racine
}

/**
 * Calcule le facteur d'équilibre d'un nœud.
 * Formule : Hauteur Gauche - Hauteur Droite.
 * Si résultat > 1 : Trop lourd à gauche.
 * Si résultat < -1 : Trop lourd à droite.
 */
int obtenirEquilibre(avl *n) {
    if (n == NULL) return 0;
    return hauteur(n->gauche) - hauteur(n->droite);
}

// --- FONCTION D'INSERTION ---

/**
 * Insère un nouvel élément ou retourne le nœud existant.
 * Gère l'équilibrage automatique de l'arbre.
 */
avl* insererAVL(avl *noeud, char *id, void *donnee) {
    // 1. Insertion normale d'Arbre Binaire de Recherche (BST)
    if (noeud == NULL) {
        return creerAVL(id, donnee); // On a trouvé la place vide, on crée le nœud
    }

    int cmp = strcmp(id, noeud->id); // Comparaison alphabétique

    if (cmp < 0)
        noeud->gauche = insererAVL(noeud->gauche, id, donnee); // Plus petit -> Gauche
    else if (cmp > 0)
        noeud->droite = insererAVL(noeud->droite, id, donnee); // Plus grand -> Droite
    else {
        // Égalité : L'ID existe déjà. On ne fait rien et on retourne le nœud trouvé.
        // C'est utile pour pouvoir modifier ses données (donnee) après coup.
        return noeud;
    }

    // 2. Mise à jour de la hauteur du nœud ancêtre actuel
    noeud->hauteur = 1 + max(hauteur(noeud->gauche), hauteur(noeud->droite));

    // 3. Vérification de l'équilibre (Balance Factor)
    int balance = obtenirEquilibre(noeud);

    // 4. Si déséquilibré, on applique les rotations

    // Cas Gauche-Gauche (Trop lourd à gauche simple)
    if (balance > 1 && strcmp(id, noeud->gauche->id) < 0)
        return rotationDroite(noeud);

    // Cas Droite-Droite (Trop lourd à droite simple)
    if (balance < -1 && strcmp(id, noeud->droite->id) > 0)
        return rotationGauche(noeud);

    // Cas Gauche-Droite (Forme de zigzag à gauche)
    if (balance > 1 && strcmp(id, noeud->gauche->id) > 0) {
        noeud->gauche = rotationGauche(noeud->gauche); // On transforme en Gauche-Gauche
        return rotationDroite(noeud);
    }

    // Cas Droite-Gauche (Forme de zigzag à droite)
    if (balance < -1 && strcmp(id, noeud->droite->id) < 0) {
        noeud->droite = rotationDroite(noeud->droite); // On transforme en Droite-Droite
        return rotationGauche(noeud);
    }

    // Retourne le pointeur (inchangé si pas de rotation)
    return noeud;
}

// --- RECHERCHE ET NETTOYAGE ---

/**
 * Recherche un nœud par son ID.
 * Complexité : O(log n) grâce à la structure AVL.
 * Retourne NULL si non trouvé.
 */
avl* rechercherAVL(avl *noeud, char *id) {
    if (noeud == NULL) return NULL; // Pas trouvé
    
    int cmp = strcmp(id, noeud->id);
    
    if (cmp == 0) return noeud; // Trouvé !
    if (cmp < 0) return rechercherAVL(noeud->gauche, id); // Chercher à gauche
    return rechercherAVL(noeud->droite, id); // Chercher à droite
}

/**
 * Libère toute la mémoire de l'arbre (Destructeur).
 * Utilise un parcours post-ordre (Enfants d'abord, Parent ensuite).
 */
void libererAVL(avl *noeud) {
    if (noeud != NULL) {
        libererAVL(noeud->gauche); // Nettoie gauche
        libererAVL(noeud->droite); // Nettoie droite
        
        free(noeud->id);     // Libère la chaîne de caractères (strdup)
        free(noeud->donnee); // Libère la structure de données associée (IMPORTANT)
        free(noeud);         // Libère le nœud lui-même
    }
}
