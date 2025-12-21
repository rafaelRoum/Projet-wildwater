#ifndef HISTO_H
#define HISTO_H
#include <stdio.h>

/* Structure pour stocker les statistiques d'une usine : sa capacité maximale,
   le volume total reçu des sources et le volume réellement traité (après fuites). */
typedef struct {
    double capacite;
    double vol_source;
    double vol_traite;
} DonneesHisto;

/* Fonction principale qui analyse le fichier d'entrée pour extraire les données 
   des usines et générer le fichier de sortie correspondant au type d'histogramme demandé. */
int traiter_histo(FILE *fichier, char *type);

#endif
