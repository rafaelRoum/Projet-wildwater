#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node structure used for AVL balancing (stores Factory/Station data)
typedef struct FactoryNode {
    char id[128];
    
    // Histo Data
    double cap_max;
    double vol_src;  
    double vol_real; 
    
    // AVL Data
    int height;
    struct FactoryNode *left;
    struct FactoryNode *right;
} FactoryNode;

// Prototypes for AVL functions

FactoryNode* insert_node(FactoryNode *node, const char *id, FactoryNode **inserted_node);
FactoryNode* search_node(FactoryNode *node, const char *id);
void print_reverse_inorder(FactoryNode *root, FILE *fp);
void free_avl_tree(FactoryNode *root);

#endif