#ifndef HISTO_H
#define HISTO_H
#include <stdio.h>

/* Structure pour stocker les statistiques d'une usine */
typedef struct {
    double capacite;
    double vol_source;
    double vol_traite;
} DonneesHisto;

/* Fonction principale des histogrammes*/
int traiter_histo(FILE *fichier, char *type);

#endif
