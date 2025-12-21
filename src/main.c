#include <stdio.h>
#include <string.h>
#include "histo.h"
#include "fuites.h"

/* Point d'entrée du programme. 
   Vérifie que les arguments sont corrects (fichier CSV, type de commande, option).
   Ouvre le fichier de données et délègue le traitement au module correspondant ("histo" ou "fuites").
   Retourne 0 si tout s'est bien passé, sinon un code d'erreur non nul. */
int main(int argc, char *argv[]) {
    if (argc < 4) return 1;

    char *fichier_csv = argv[1];
    char *commande = argv[2];
    char *option = argv[3];

    FILE *f = fopen(fichier_csv, "r");
    if (f == NULL) return 2;

    int res = 0;
    if (strcmp(commande, "histo") == 0) {
        res = traiter_histo(f, option);
    }
    else if (strcmp(commande, "fuites") == 0) {
        res = traiter_fuites(f, option);
    }
    else {
        res = 3;
    }

    fclose(f);
    return res;
}
