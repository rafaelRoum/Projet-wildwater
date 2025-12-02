#ifndef LEAKS_H
#define LEAKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h" // Includes the main node structure

// Structure for Adjacency List (Graph edges)
typedef struct Edge {
    FactoryNode *child;
    double leak_percent;
    struct Edge *next;
} Edge;

// Extended Node structure to hold graph data (used locally in leaks.c)
typedef struct GraphNode {
    FactoryNode *avl_node; // Pointer to the main AVL node
    int visited;
    Edge *edges;
} GraphNode;

// Prototypes for graph/leak functions
GraphNode* create_graph_node(FactoryNode *avl_node);
void add_edge_to_graph(GraphNode *parent, FactoryNode *child, double leak_percent);
double calculate_total_leaks(GraphNode *node, double current_flow);
void free_graph(GraphNode *node);

#endif