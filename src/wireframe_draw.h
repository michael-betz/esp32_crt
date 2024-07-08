#pragma once
#include <stdint.h>

// holds all the parameters needed to draw a 3D object at a specific position
typedef struct {
    int x; int y; int z;    // linear position
    int u; int v; int w;    // angular orientation
    int transform_matrix[4][4]; // (derived from the parameters above)

    const int *vertices;    // 3 rows, n_vertices columns
    uint16_t n_vertices;

    const uint16_t *edges;  // 2 rows, n_edges columns
    uint16_t n_edges;
} obj_3d_t;

// holds the vertices and edge-indices of N wireframe 3D objects
typedef struct {
    // how many different 3D objects do we store here
    const uint16_t n_objects;

    // stores all vertices for all 3D objects. These come in packets of 3 for x, y, z.
    const int *all_vertices;

    // stores the edges for all 3D objects, these are indices into all_vertices.
    // These come in packets of 2 for start-point, end-point of the line
    // The index is relative to the start_index of the object.
    const uint16_t *all_edges;

    // vertices_ends[x - 1] is the first vertice of the object
    // vertices_ends[x] is the first vertice of the next object
    const uint16_t *vertices_ends;

    // end edge of the object
    const uint16_t *edges_ends;
} edges_3d_t;
