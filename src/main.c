#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h" // AVL structures for Histo
#include "leaks.h" // Graph structures for Leaks

// Global graph helper functions using AVL nodes (defined here to respect src/ structure)

// Structure used for Leaks mode (defined in avl.h but implemented here)
typedef struct FactoryNode GraphNode; 

// Function to add a connection (Edge) between two nodes
void add_edge_to_node(FactoryNode *parent, FactoryNode *child, double leak_percent) {
    // We repurpose the AVL node (FactoryNode) to also act as the Graph node
    Edge *edge = malloc(sizeof(Edge));
    if (!edge) exit(1);
    
    edge->child = child;
    edge->leak_percent = leak_percent;
    edge->next = parent->edges; 
    parent->edges = edge;
}

// Recursive Leaks calculation (DFS)
double calculate_total_leaks(FactoryNode *node, double current_flow) {
    if (!node || node->visited) return 0;
    node->visited = 1; 

    double total_loss = 0;
    Edge *curr = node->edges;

    while (curr) {
        double lost_here = 0;
        double next_flow = current_flow;

        if (curr->leak_percent > 0) {
            lost_here = current_flow * (curr->leak_percent / 100.0);
            next_flow -= lost_here;
        }
        
        total_loss += lost_here;
        
        // Recursively calculate downstream leaks
        if (curr->child) {
            total_loss += calculate_total_leaks(curr->child, next_flow);
        }
        curr = curr->next;
    }
    
    node->visited = 0; 
    return total_loss;
}


// Helper to check if a token is the placeholder "-"
int is_dash(char *s) {
    return (s && strcmp(s, "-") == 0);
}

// Helper to convert string to double, handling "-"
double parse_double(char *s) {
    if (is_dash(s)) return 0.0;
    return atof(s);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <csv_file> <mode> [arg]\n", argv[0]);
        return 1;
    }

    char *csv_path = argv[1];
    char *mode = argv[2];

    FILE *f = fopen(csv_path, "r");
    if (!f) { perror("File error"); return 1; }

    FactoryNode *root = NULL;
    char line[1024];

    // -------------------------------------------------------------
    // PASS 1: LOAD ALL DATA (NODES, Histo DATA, and EDGES)
    // -------------------------------------------------------------
    while (fgets(line, sizeof(line), f)) {
        char tmp[1024];
        strcpy(tmp, line);

        // Uses strtok as required (relying on "-" placeholder)
        char *col1  = strtok(tmp, ";\n"); // Factory ID (optional)
        char *col2  = strtok(NULL, ";\n"); // Upstream ID
        char *col3  = strtok(NULL, ";\n"); // Downstream ID
        char *col4  = strtok(NULL, ";\n"); // Volume/Capacity
        char *col5  = strtok(NULL, ";\n"); // Leak %

        if (!col1 || !col2 || !col3 || !col4) continue;

        // 1. Identify Nodes (Upstream and Downstream must be in the AVL)
        FactoryNode *u = NULL;
        if (!is_dash(col2)) root = insert_node(root, col2, &u);
        
        FactoryNode *v = NULL;
        if (!is_dash(col3)) root = insert_node(root, col3, &v);
        
        // 2. Load Factory Capacity (Factory;-;-;Capacity;-)
        if (!is_dash(col1) && is_dash(col2) && is_dash(col3)) {
            FactoryNode *fac = NULL;
            root = insert_node(root, col1, &fac);
            fac->cap_max = parse_double(col4);
        }

        // 3. Load Connections (Edges) and Source Volumes
        if (u && v) {
            double leak = parse_double(col5);
            add_edge_to_node(u, v, leak);

            // If Source -> Factory Line (Histo Data)
            if (is_dash(col1) && !is_dash(col2) && !is_dash(col3)) {
                double vol = parse_double(col4);
                v->vol_src += vol;
                v->vol_real += vol * (1.0 - (leak / 100.0));
            }
        }
    }
    fclose(f);

    // -------------------------------------------------------------
    // EXECUTE MODE
    // -------------------------------------------------------------
    
    if (strcmp(mode, "histo") == 0) {
        FILE *g = fopen("data/output_histo.dat", "w");
        if (!g) return 1;
        fprintf(g, "Station;Capacity;Source;Real\n");
        print_reverse_inorder(root, g);
        fclose(g);
    } 
    else if (strcmp(mode, "leaks") == 0) {
        if (argc < 4) return 1;
        char *target_id = argv[3];
        
        FactoryNode *target = search_node(root, target_id);
        if (target) {
            // We use the calculated real output volume as the flow input
            double initial_flow = target->vol_real; 
            
            // Fallback if real volume is zero (e.g., if data file is incomplete)
            if (initial_flow == 0 && target->cap_max > 0) initial_flow = target->cap_max;

            double total_loss = calculate_total_leaks(target, initial_flow);
            printf("%f\n", total_loss); // Output to stdout
        } else {
            printf("-1\n"); // Not found
        }
    }

    free_tree(root);
    return 0;
}