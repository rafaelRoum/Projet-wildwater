#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int height(FactoryNode *n) {
    return n ? n->height : 0;
}
int maxi(int a, int b) {
    return (a > b) ? a : b;
}



FactoryNode* new_node(const char *id, double cap_max) {
    FactoryNode *node = malloc(sizeof(FactoryNode));
    strcpy(node->id, id);
    node->cap_max = cap_max;
    node->vol_src = 0;
    node->vol_real = 0;
    node->overload = 0;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

FactoryNode* right_rotate(FactoryNode *y) {
    FactoryNode *x = y->left;
    FactoryNode *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = maxi(height(y->left), height(y->right)) + 1;
    x->height = maxi(height(x->left), height(x->right)) + 1;

    return x;
}

FactoryNode* left_rotate(FactoryNode *x) {
    FactoryNode *y = x->right;
    FactoryNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = maxi(height(x->left), height(x->right)) + 1;
    y->height = maxi(height(y->left), height(y->right)) + 1;

    return y;
}

int get_balance(FactoryNode *n) {
    return n ? height(n->left) - height(n->right) : 0;
}

FactoryNode* insert_factory(FactoryNode *node, const char *id, double cap_max) {
    if (!node) return new_node(id, cap_max);

    int cmp = strcmp(id, node->id);

    if (cmp < 0)
        node->left = insert_factory(node->left, id, cap_max);
    else if (cmp > 0)
        node->right = insert_factory(node->right, id, cap_max);
    else
        return node; // déjà existant

    node->height = 1 + maxi(height(node->left), height(node->right));

    int balance = get_balance(node);

    // 4 cas
    if (balance > 1 && strcmp(id, node->left->id) < 0)
        return right_rotate(node);

    if (balance < -1 && strcmp(id, node->right->id) > 0)
        return left_rotate(node);

    if (balance > 1 && strcmp(id, node->left->id) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && strcmp(id, node->right->id) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

FactoryNode* search_factory(FactoryNode *node, const char *id) {
    if (!node){
        //printf("NULL\n"); // ICI §§§
        return NULL;
    }
    int cmp = strcmp(id, node->id);
    if (cmp == 0) return node;
    if (cmp < 0) return search_factory(node->left, id);
    return search_factory(node->right, id);
}

void print_reverse_inorder(FactoryNode *root) {
    if (!root) return;
    print_reverse_inorder(root->right);
    printf("%s | max=%.2f | src=%.2f | real=%.2f | overload : %d\n",
           root->id, root->cap_max, root->vol_src, root->vol_real,root->overload);

    print_reverse_inorder(root->left);
}
void prf(FactoryNode *root, FILE *fp) { // je suis laaaa
    if (!root) return;

    prf(root->right, fp);

    fprintf(fp, "%s;%.2f;%.2f;%.2f;%d\n",
            root->id, root->cap_max, root->vol_src, root->vol_real, root->overload);

    prf(root->left, fp);
}

// -------- PASS 4 : Déterminer PASS / FAIL pour chaque usine --------
void check_factories(FactoryNode *root) {
    if (!root) return;
    check_factories(root->left);

    if (root->vol_real <= root->cap_max) {
        root->overload = 0;
    } else {
        root->overload =1;
    }

    check_factories(root->right);
}
