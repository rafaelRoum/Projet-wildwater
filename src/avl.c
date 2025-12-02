#include "avl.h"

int height(FactoryNode *n) { return n ? n->height : 0; }
int maxi(int a, int b) { return (a > b) ? a : b; }

FactoryNode* new_factory_node(const char *id) {
    FactoryNode *node = malloc(sizeof(FactoryNode));
    if (!node) exit(1);
    
    strcpy(node->id, id);
    node->cap_max = 0;
    node->vol_src = 0;
    node->vol_real = 0;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

// AVL Rotations (kept from base code)
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

int get_balance(FactoryNode *n) { return n ? height(n->left) - height(n->right) : 0; }

// Insert node into AVL
FactoryNode* insert_node(FactoryNode *node, const char *id, FactoryNode **inserted_node) {
    if (!node) {
        FactoryNode *new = new_factory_node(id);
        if (inserted_node) *inserted_node = new;
        return new;
    }

    int cmp = strcmp(id, node->id);
    if (cmp < 0)
        node->left = insert_node(node->left, id, inserted_node);
    else if (cmp > 0)
        node->right = insert_node(node->right, id, inserted_node);
    else {
        if (inserted_node) *inserted_node = node;
        return node;
    }

    node->height = 1 + maxi(height(node->left), height(node->right));
    int balance = get_balance(node);

    // 4 case rebalancing logic
    if (balance > 1 && strcmp(id, node->left->id) < 0) return right_rotate(node);
    if (balance < -1 && strcmp(id, node->right->id) > 0) return left_rotate(node);
    if (balance > 1 && strcmp(id, node->left->id) > 0) { node->left = left_rotate(node->left); return right_rotate(node); }
    if (balance < -1 && strcmp(id, node->right->id) < 0) { node->right = right_rotate(node->right); return left_rotate(node); }
    
    return node;
}

FactoryNode* search_node(FactoryNode *node, const char *id) {
    if (!node) return NULL;
    int cmp = strcmp(id, node->id);
    if (cmp == 0) return node;
    if (cmp < 0) return search_node(node->left, id);
    return search_node(node->right, id);
}

// In-order traversal (Reverse alphabetical for output CSV)
void print_reverse_inorder(FactoryNode *root, FILE *fp) {
    if (!root) return;
    print_reverse_inorder(root->right, fp);
    
    // Only print nodes that represent an active station/factory
    if (root->cap_max > 0 || root->vol_src > 0) {
        fprintf(fp, "%s;%.2f;%.2f;%.2f\n", 
                root->id, root->cap_max, root->vol_src, root->vol_real);
    }
    
    print_reverse_inorder(root->left, fp);
}

void free_avl_tree(FactoryNode *root) {
    if (!root) return;
    free_avl_tree(root->left);
    free_avl_tree(root->right);
    free(root);
}