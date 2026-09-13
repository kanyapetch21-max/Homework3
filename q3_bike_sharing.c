#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct Edge {
    int to;
    int capacity;
    int flow;
    int original;
    struct Edge *rev;
    struct Edge *next;
} Edge;

typedef struct {
    int from;
    Edge *edge;
} OriginalEdge;

static Edge *new_edge(int to, int capacity, int original) {
    Edge *e = (Edge *)malloc(sizeof(Edge));
    if (e == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    e->to = to;
    e->capacity = capacity;
    e->flow = 0;
    e->original = original;
    e->rev = NULL;
    e->next = NULL;
    return e;
}

static void push_front(Edge **graph, int u, Edge *e) {
    e->next = graph[u];
    graph[u] = e;
}

static void add_edge(Edge **graph, OriginalEdge **originals, int *edge_count,
                     int *edge_cap, int u, int v, int capacity) {
    Edge *forward = new_edge(v, capacity, 1);
    Edge *reverse = new_edge(u, 0, 0);

    if (*edge_count >= *edge_cap) {
        *edge_cap *= 2;
        *originals = (OriginalEdge *)realloc(*originals, sizeof(OriginalEdge) * (*edge_cap));
        if (*originals == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }

    forward->rev = reverse;
    reverse->rev = forward;

    push_front(graph, u, forward);
    push_front(graph, v, reverse);

    (*originals)[*edge_count].from = u;
    (*originals)[*edge_count].edge = forward;
    (*edge_count)++;
}

static int bfs(Edge **graph, int n, int source, int sink, int *parent_node, Edge **parent_edge) {
    int *queue = (int *)malloc(sizeof(int) * n);
    int front = 0;
    int back = 0;
    int i;

    if (queue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (i = 0; i < n; i++) {
        parent_node[i] = -1;
        parent_edge[i] = NULL;
    }

    parent_node[source] = source;
    queue[back++] = source;

    while (front < back) {
        int u = queue[front++];
        Edge *e;

        for (e = graph[u]; e != NULL; e = e->next) {
            int residual = e->capacity - e->flow;
            if (parent_node[e->to] == -1 && residual > 0) {
                parent_node[e->to] = u;
                parent_edge[e->to] = e;
                if (e->to == sink) {
                    free(queue);
                    return 1;
                }
                queue[back++] = e->to;
            }
        }
    }

    free(queue);
    return 0;
}

static int edmonds_karp(Edge **graph, int n, int source, int sink) {
    int *parent_node = (int *)malloc(sizeof(int) * n);
    Edge **parent_edge = (Edge **)malloc(sizeof(Edge *) * n);
    int max_flow = 0;

    if (parent_node == NULL || parent_edge == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    while (bfs(graph, n, source, sink, parent_node, parent_edge)) {
        int bottle = INT_MAX;
        int v;

        for (v = sink; v != source; v = parent_node[v]) {
            Edge *e = parent_edge[v];
            int residual = e->capacity - e->flow;
            if (residual < bottle) {
                bottle = residual;
            }
        }

        for (v = sink; v != source; v = parent_node[v]) {
            Edge *e = parent_edge[v];
            e->flow += bottle;
            e->rev->flow -= bottle;
        }

        max_flow += bottle;
    }

    free(parent_node);
    free(parent_edge);
    return max_flow;
}

static void free_graph(Edge **graph, int n) {
    int i;
    for (i = 0; i < n; i++) {
        Edge *e = graph[i];
        while (e != NULL) {
            Edge *next = e->next;
            free(e);
            e = next;
        }
    }
    free(graph);
}

int main(int argc, char **argv) {
    FILE *fp;
    Edge **graph;
    OriginalEdge *originals;
    int *demand;
    int edge_cap = 16;
    int edge_count = 0;
    int n, demand_count;
    int u, v, capacity;
    int i;
    int total_demand = 0;
    int max_flow;
    int all_satisfied = 1;
    int *received;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s bike_input.txt\n", argv[0]);
        fprintf(stderr, "Input format:\n");
        fprintf(stderr, "  N\n");
        fprintf(stderr, "  E\n");
        fprintf(stderr, "  u v capacity   repeated E times\n");
        fprintf(stderr, "  D\n");
        fprintf(stderr, "  node demand    repeated D times\n");
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Cannot open input file");
        return 1;
    }

    if (fscanf(fp, "%d", &n) != 1 || n <= 1) {
        fprintf(stderr, "Invalid number of nodes\n");
        fclose(fp);
        return 1;
    }

    graph = (Edge **)calloc((size_t)n, sizeof(Edge *));
    originals = (OriginalEdge *)malloc(sizeof(OriginalEdge) * edge_cap);
    demand = (int *)calloc((size_t)n, sizeof(int));
    if (graph == NULL || originals == NULL || demand == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return 1;
    }

    if (fscanf(fp, "%d", &edge_count) != 1 || edge_count < 0) {
        fprintf(stderr, "Invalid edge count\n");
        fclose(fp);
        free(graph);
        free(originals);
        free(demand);
        return 1;
    }

    {
        int declared_edges = edge_count;
        edge_count = 0;
        while (declared_edges-- > 0) {
            if (fscanf(fp, "%d %d %d", &u, &v, &capacity) != 3 ||
                u < 0 || u >= n || v < 0 || v >= n || capacity < 0) {
                fprintf(stderr, "Invalid edge data\n");
                fclose(fp);
                free_graph(graph, n);
                free(originals);
                free(demand);
                return 1;
            }
            add_edge(graph, &originals, &edge_count, &edge_cap, u, v, capacity);
        }
    }

    if (fscanf(fp, "%d", &demand_count) != 1 || demand_count < 0) {
        fprintf(stderr, "Invalid demand count\n");
        fclose(fp);
        free_graph(graph, n);
        free(originals);
        free(demand);
        return 1;
    }

    while (demand_count-- > 0) {
        int node, need;
        if (fscanf(fp, "%d %d", &node, &need) != 2 || node < 0 || node >= n || need < 0) {
            fprintf(stderr, "Invalid demand data\n");
            fclose(fp);
            free_graph(graph, n);
            free(originals);
            free(demand);
            return 1;
        }
        demand[node] += need;
        total_demand += need;
    }

    fclose(fp);

    max_flow = edmonds_karp(graph, n, 0, n - 1);

    received = (int *)calloc((size_t)n, sizeof(int));
    if (received == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free_graph(graph, n);
        free(originals);
        free(demand);
        return 1;
    }

    for (i = 0; i < edge_count; i++) {
        Edge *e = originals[i].edge;
        received[e->to] += e->flow;
    }

    for (i = 0; i < n; i++) {
        if (demand[i] > 0 && received[i] < demand[i]) {
            all_satisfied = 0;
        }
    }

    printf("Bike sharing max flow from %d to %d = %d\n", 0, n - 1, max_flow);
    printf("Total customer reservations = %d\n", total_demand);
    printf("All reservations satisfied at every location? %s\n\n", all_satisfied ? "YES" : "NO");

    printf("Flow assigned to each transportation edge:\n");
    for (i = 0; i < edge_count; i++) {
        Edge *e = originals[i].edge;
        printf("  %d -> %d : flow %d / capacity %d\n",
               originals[i].from, e->to, e->flow, e->capacity);
    }

    printf("\nReservation check by location:\n");
    for (i = 0; i < n; i++) {
        if (demand[i] > 0) {
            printf("  location %d: received %d, demand %d, %s\n",
                   i, received[i], demand[i],
                   received[i] >= demand[i] ? "satisfied" : "not satisfied");
        }
    }

    free_graph(graph, n);
    free(originals);
    free(demand);
    free(received);

    return 0;
}
