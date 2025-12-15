#ifndef HISTO_H
#define HISTO_H

#include <stdio.h>

/**
 * Structure contenant les données statistiques d'une usine.
 * Cette structure sera stockée dans le champ "void* donnee" de l'AVL générique.
 */
typedef struct {
    double capacite;    // Capacité maximale de l'usine (Lue depuis la ligne de définition).
    double vol_source;  // Volume brut reçu des sources (Somme des entrées).
    double vol_traite;  // Volume réellement traité (Volume reçu - Fuites en amont).
} DonneesHisto;

/**
 * Fonction principale du module Histo.
 * 1. Lit le fichier CSV.
 * 2. Remplit un AVL avec les données des usines.
 * 3. Génère un fichier de sortie (.dat) trié par ordre alphabétique inverse.
 * * @param fichier Pointeur vers le fichier CSV ouvert.
 * @param type    Le mode de fonctionnement : "max", "src", "real" ou "all".
 * @return        0 si succès, un code erreur sinon.
 */
int traiter_histo(FILE *fichier, char *type);

#endif
