#include <stdio.h>
#include "Matrix.hpp"
#include "Mesh.hpp"
#include "venus.hpp"

extern "C" void draw_mesh()
{
	Mesh m(
		venus::vertexCount,
		venus::vertices,
		0,
		0,
		venus::triangleCount,
		venus::triangles,
		venus::triangleNormals
	);

	static float u = 0;
	static Matrix perspective = \
		Matrix::translation(400, 400, 0) * \
		Matrix::perspective(90, 1, 10);
	u += 0.02;
	Matrix rotation = Matrix::rotation(-1.7, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
	Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 6) * rotation * Matrix::scaling(7);
	m.transform(m0, rotation);
	m.drawEdges(0xFF);
}
