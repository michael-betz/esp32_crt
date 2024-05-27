#pragma once
#include "Matrix.hpp"

extern "C" {
  #include "draw.h"
}

class Mesh
{
  public:
  int vertexCount;
  int triangleCount;
  int edgeCount;
  const float (*vertices)[3];
  const float (*triangleNormals)[3];
  short (*tvertices)[3];
  signed char (*tTriNormals)[3];
  const unsigned short (*triangles)[3];
  const unsigned short (*edges)[2];

  Mesh(int vertCount, const float verts[][3], int edgeCount_ = 0, const unsigned short edges_[][2] = 0, int triCount = 0, const unsigned short tris[][3] = 0, const float triNorms[][3] = 0)
    :vertexCount(vertCount),
    vertices(verts),
    edgeCount(edgeCount_),
    edges(edges_),
    triangleCount(triCount),
    triangles(tris),
    triangleNormals(triNorms)
  {
    tvertices = (short(*)[3]) malloc(sizeof(short) * 3 * vertexCount);
    if(triangleNormals)
      tTriNormals = (signed char(*)[3]) malloc(sizeof(signed char) * 3 * triangleCount);
  }

  ~Mesh()
  {
    delete(tvertices);
  }

  void drawEdges(char color)
  {
    printf("drawEdges(%d)\n", edgeCount);
    for(int i = 0; i < edgeCount; i++) {
      // g.line(tvertices[edges[i][0]][0], tvertices[edges[i][0]][1], tvertices[edges[i][1]][0], tvertices[edges[i][1]][1], color);
      // printf(
      //   "%d %d %d %d %d\n",
      //   tvertices[edges[i][0]][0],
      //   tvertices[edges[i][0]][1],
      //   tvertices[edges[i][1]][0],
      //   tvertices[edges[i][1]][1],
      //   color
      // );
      push_goto(tvertices[edges[i][0]][0], tvertices[edges[i][0]][1]);
      push_line(tvertices[edges[i][1]][0], tvertices[edges[i][1]][1], color);
    }
  }

  void transform(Matrix m, Matrix normTrans = Matrix())
  {
    for(int i = 0; i < vertexCount; i++)
    {
      Vector v = m * Vector(vertices[i][0], vertices[i][1], vertices[i][2]);
      tvertices[i][0] = v[0] / v[3];
      tvertices[i][1] = v[1] / v[3];
      tvertices[i][2] = v[2];
    }
    if(triangleNormals)
      for(int i = 0; i < triangleCount; i++)
      {
        Vector v = normTrans * Vector(triangleNormals[i][0], triangleNormals[i][1], triangleNormals[i][2]);
        tTriNormals[i][0] = v[0] * 127;
        tTriNormals[i][1] = v[1] * 127;
        tTriNormals[i][2] = v[2] * 127;
      }
  }
};

