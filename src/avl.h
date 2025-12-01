#include <stdio.h>
#ifndef AVL_H
#define AVL_H

typedef struct FactoryNode {
    char id[128];
    double cap_max;
    double vol_src; //volume capte par les sources
    double vol_real; //volume sortie de l usine
    int height;
    int overload;
    struct FactoryNode *left;
    struct FactoryNode *right;
} FactoryNode;

FactoryNode* insert_factory(FactoryNode *node, const char *id, double cap_max);
FactoryNode* search_factory(FactoryNode *node, const char *id);
void print_reverse_inorder(FactoryNode *root);
void prf(FactoryNode *root, FILE *fp);

#endif
