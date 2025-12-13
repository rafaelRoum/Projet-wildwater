#ifndef HISTO_H
#define HISTO_H
#include <stdio.h>

typedef struct {
    double capacite;
    double vol_source;
    double vol_traite;
} DonneesHisto;

int traiter_histo(FILE *fichier, char *type);

#endif