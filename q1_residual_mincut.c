#include <stdio.h>
#include <string.h>

#define N 8
#define MAX_EDGES 32

typedef struct {
    int from;
    int to;
    int flow;
    int cap;
} Edge;

static const char *name[N] = {"s", "1", "2", "3", "4", "5", "6", "t"};

static int load_edges_from_file(const char *filename, Edge edges[MAX_EDGES]) {
    FILE *fp = fopen(filename, "r");
    int n;
    int edge_count;
    int i;

    if (fp == NULL) {
        perror("Cannot open input file");
        return -1;
    }

    if (fscanf(fp, "%d", &n) != 1 || n != N) {
        fprintf(stderr, "Invalid number of nodes. This question uses N = %d.\n", N);
        fclose(fp);
        return -1;
    }

    if (fscanf(fp, "%d", &edge_count) != 1 || edge_count < 0 || edge_count > MAX_EDGES) {
        fprintf(stderr, "Invalid number of edges\n");
        fclose(fp);
        return -1;
    }

    for (i = 0; i < edge_count; i++) {
        if (fscanf(fp, "%d %d %d %d",
                   &edges[i].from, &edges[i].to, &edges[i].flow, &edges[i].cap) != 4) {
            fprintf(stderr, "Invalid edge data at line %d\n", i + 3);
            fclose(fp);
            return -1;
        }

        if (edges[i].from < 0 || edges[i].from >= N ||
            edges[i].to < 0 || edges[i].to >= N ||
            edges[i].flow < 0 || edges[i].cap < 0 ||
            edges[i].flow > edges[i].cap) {
            fprintf(stderr, "Invalid edge: %d %d %d %d\n",
                    edges[i].from, edges[i].to, edges[i].flow, edges[i].cap);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return edge_count;
}

static void dfs(int u, int residual[N][N], int visited[N]) {
    int v;

    visited[u] = 1;
    for (v = 0; v < N; v++) {
        if (!visited[v] && residual[u][v] > 0) {
            dfs(v, residual, visited);
        }
    }
}

int main(int argc, char **argv) {
    Edge edges[MAX_EDGES] = {
        {0, 1, 10, 10},  /* s -> 1 */
        {0, 3, 5, 5},    /* s -> 3 */
        {0, 5, 13, 15},  /* s -> 5 */
        {1, 2, 8, 9},
        {1, 3, 0, 4},
        {1, 4, 2, 15},
        {3, 4, 8, 8},
        {3, 5, 0, 4},
        {6, 3, 3, 6},
        {5, 6, 13, 16},
        {2, 4, 0, 15},
        {2, 7, 8, 10},
        {4, 6, 0, 15},
        {4, 7, 10, 10},
        {6, 7, 10, 10}
    };
    int edge_count = 15;
    int residual[N][N];
    int visited[N];
    int i, u, v;
    int cut_capacity = 0;
    int max_flow = 0;

    if (argc == 2) {
        edge_count = load_edges_from_file(argv[1], edges);
        if (edge_count < 0) {
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [input.txt]\n", argv[0]);
        return 1;
    }

    memset(residual, 0, sizeof(residual));
    memset(visited, 0, sizeof(visited));

    for (i = 0; i < edge_count; i++) {
        u = edges[i].from;
        v = edges[i].to;
        residual[u][v] += edges[i].cap - edges[i].flow;
        residual[v][u] += edges[i].flow;
    }

    for (i = 0; i < edge_count; i++) {
        if (edges[i].from == 0) {
            max_flow += edges[i].flow;
        }
    }

    dfs(0, residual, visited);

if (visited[N-1]) {
    fprintf(stderr,
        "Warning: sink t is still reachable in the residual graph.\n"
        "The given flow is NOT maximal (an augmenting path exists),\n"
        "so the computed min-cut below is INVALID.\n");
}

for (u = 1; u < N - 1; u++) {
    int in = 0, out = 0;
    for (i = 0; i < edge_count; i++) {
        if (edges[i].to   == u) in  += edges[i].flow;
        if (edges[i].from == u) out += edges[i].flow;
    }
    if (in != out)
        fprintf(stderr, "Warning: flow conservation violated at node %s (in=%d, out=%d)\n",
                name[u], in, out);
}

    printf("Question 1: residual graph and minimum cut\n");
    printf("Given flow value = %d\n\n", max_flow);

    printf("Residual edges with positive residual capacity:\n");
    for (u = 0; u < N; u++) {
        for (v = 0; v < N; v++) {
            if (residual[u][v] > 0) {
                printf("  %s -> %s : %d\n", name[u], name[v], residual[u][v]);
            }
        }
    }

    printf("\nReachable set S in the residual graph:\n  S = { ");
    for (i = 0; i < N; i++) {
        if (visited[i]) {
            printf("%s ", name[i]);
        }
    }
    printf("}\n");

    printf("  T = { ");
    for (i = 0; i < N; i++) {
        if (!visited[i]) {
            printf("%s ", name[i]);
        }
    }
    printf("}\n");

    printf("\nCut edges from S to T in the original graph:\n");
    for (i = 0; i < edge_count; i++) {
        u = edges[i].from;
        v = edges[i].to;
        if (visited[u] && !visited[v]) {
            printf("  %s -> %s : capacity %d\n", name[u], name[v], edges[i].cap);
            cut_capacity += edges[i].cap;
        }
    }

    printf("\nMinimum cut capacity = %d\n", cut_capacity);

    return 0;
}
