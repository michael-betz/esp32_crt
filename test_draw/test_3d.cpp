#include <stdio.h>
#include "Matrix.hpp"
#include "Mesh.hpp"
#include "venus.hpp"

static Mesh m(
	venus::vertexCount,
	venus::vertices,
	venus::edgeCount,
	venus::edges,
	venus::triangleCount,
	venus::triangles,
	venus::triangleNormals
);

extern "C" void draw_mesh()
{
	static Matrix perspective = Matrix::translation(128, 128, 0) * Matrix::scaling(1024, 1024, 1024) * Matrix::perspective(90, 1, 10);
	static float u = 0;
	u += 0.02;
	Matrix rotation = Matrix::rotation(1.7, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
	Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 6) * rotation * Matrix::scaling(7);
	m.transform(m0, rotation);

	m.drawEdges(30);
}
