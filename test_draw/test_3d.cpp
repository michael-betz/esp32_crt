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
	float z = 1 * cos(u / 10);
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
