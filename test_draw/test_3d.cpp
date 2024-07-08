#include <stdio.h>
#include "Matrix.hpp"
#include "Mesh.hpp"
// #include "venus.hpp"
#include "numbers.h"


// static Mesh m(
// 	venus::vertexCount,
// 	venus::vertices,
// 	venus::edgeCount,
// 	venus::edges,
// 	venus::triangleCount,
// 	venus::triangles,
// 	venus::triangleNormals
// );

Mesh *g_model = 0;


void get_model(unsigned numeral)
{
	numeral %= sizeof(numbers::lvs) / sizeof(numbers::lvs[0]);
	unsigned nVertices = numbers::lvs[numeral];
	unsigned nEdges = numbers::les[numeral];
	unsigned startVertice = 0;
	unsigned startEdge = 0;
	for (unsigned i=0; i<numeral; i++) {
		startVertice += numbers::lvs[i];
		startEdge += numbers::les[i];
	}
	if (g_model != 0)
		delete g_model;
	g_model = new Mesh(
		nVertices,
		&numbers::vertices[startVertice],
		nEdges,
		&numbers::edges[startEdge]
	);
}

void drawObj()
{
	static Matrix perspective = Matrix::translation(128, 128, 0) * Matrix::scaling(1024) * Matrix::perspective(90, 1, 10);
	static float u=0, v=0;
	u += 0.01;
	v = sin(u);
	Matrix rotation = Matrix::rotation(u, 1, 0, 0) * Matrix::rotation(v, 0, 1, 0);
	Matrix m0 = perspective * Matrix::translation(0, 0, 5) * rotation;
	g_model->transform(m0);
	// g_model.drawTriangles(graphics, 50);
	// graphics.flush();
	g_model->drawEdges(50);
	// g_model.drawVertices(graphics, 50);
}

extern "C" void draw_mesh()
{
	// static Matrix perspective = Matrix::translation(128, 128, 0) * Matrix::scaling(1024, 1024, 1024) * Matrix::perspective(90, 1, 10);
	// static float u = 0;
	// u += 0.02;
	// Matrix rotation = Matrix::rotation(1.7, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
	// Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 6) * rotation * Matrix::scaling(7);
	// m.transform(m0, rotation);
	// m.drawEdges(30);

	static int i = 0;
	if ((i++ % 500) == 0) {
		get_model(i / 500);
	}

	drawObj();
}



// 1. Initialize 3D objects (which edges to show, their positions and orientations)
// 2. Set camera position
// 3. call calc_transform_matrix(entity, camera) for each entity, it calculates the transform matrix
// 4. the draw thread has a list of entities and uses the transform_matrix to get transformed vertices
//    it then uses the indexes from edges to draw lines between some of the transformed vertices


// stores all vertices for all 3D objects. These come in packets of 3 for x, y, z.
static const int all_vertices[] = {100, 200, 300, 110, 120, 130};

// stores the edges for all 3D objects, these are indices into all_vertices.
// These come in packets of 2 for start-point, end-point of the line
// The index is relative to the start_index of the object.
static const uint16_t all_edges[] = {1, 2, 3, 4};

// end_vertice of the object
static const uint16_t end_of_vertices[] = {8, 12, 15};

// end edge of the object
static const uint16_t end_of_edges[] = {3, 6, 10};

int get_ptr_and_len(const uint16_t *end_index_array, unsigned end_index_array_len, unsigned obj_index, unsigned *start_index_out)
{
	// Find the beginning and length
	if (obj_index >= end_index_array_len)
		return -1;

	unsigned end_index = end_index_array[obj_index];
	unsigned start_index = 0;

	if (obj_index > 0)
		start_index = end_index_array[obj_index - 1];

	if (start_index_out != NULL)
		*start_index_out = start_index;

	return end_index - start_index;
}

