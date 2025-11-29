#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int is_factory(char *col1, char *aval, char *fuite) {
    return strcmp(col1, "-") == 0 &&
           strcmp(aval, "-") == 0 &&
           strcmp(fuite, "-") == 0;
}

int is_source(char *col1, char *aval) {
    return strcmp(col1, "-") == 0 && strcmp(aval, "-") != 0;
}

int main(void) {

    FILE *f = fopen("../data/test.dat", "r");
    if (!f) { perror("fopen"); return 1; }

    FactoryNode *root = NULL;
    char line[256];

    //------------------------------------------------------
    // PASSAGE 1 : CHARGER LES USINES
    //------------------------------------------------------
    rewind(f);
    while (fgets(line, sizeof(line), f)) {

        char tmp[2048];
        strcpy(tmp, line);

        char *col1  = strtok(tmp, ";\n");
        char *amont = strtok(NULL, ";\n");
        char *aval  = strtok(NULL, ";\n");
        char *vol   = strtok(NULL, ";\n");
        char *fuite = strtok(NULL, ";\n");

        if (!col1 || !amont || !aval || !vol || !fuite)
            continue;

        if (is_factory(col1, aval, fuite)) {
            double cap = atof(vol);
            root = insert_factory(root, amont, cap);
        }
    }

    //------------------------------------------------------
    // PASSAGE 2 : CHARGER LES SOURCES
    //------------------------------------------------------
    rewind(f);
    while (fgets(line, sizeof(line), f)) {

        char tmp[2048];
        strcpy(tmp, line);

        char *col1  = strtok(tmp, ";\n");
        char *src   = strtok(NULL, ";\n");
        char *aval  = strtok(NULL, ";\n");   // usine
        char *vol   = strtok(NULL, ";\n");
        char *fuite = strtok(NULL, ";\n");

        if (!col1 || !src || !aval || !vol || !fuite)
            continue;

        if (is_source(col1, aval)) {

            FactoryNode *u = search_factory(root, aval);
            if (u) {
                double volume = atof(vol);
                double fui = atof(fuite);
                u->vol_src += volume;
                u->vol_real+= volume * (100-fui)/100;
            }
        }
    }

    fclose(f);

    printf("================================ USINES ================================\n");
    print_reverse_inorder(root);
    return 0;
}
